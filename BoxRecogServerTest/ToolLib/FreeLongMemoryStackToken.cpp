// FreeLongMemoryStackToken.cpp: implementation of the CFreeLongMemoryStackToken class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongMemoryStackToken.h"
#include "SafePrint.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CFreeLongMemoryStackToken::CFreeLongMemoryStackToken(int nBlockSize,CCFreeLong_LowDebug* pDebug)
{
	m_pDebug=pDebug;
	m_ulBlockSize=(ULONG)nBlockSize;
	m_nBlockOutSide.Set(0);
	m_nBlockInSide.Set(0);

	m_Lock.EnableWrite();
	{
		m_pFirstSon=NULL;
		m_pNext=NULL;
	}
	m_Lock.DisableWrite();

}

CFreeLongMemoryStackToken::~CFreeLongMemoryStackToken()
{
	if(m_nBlockOutSide.Get())
	{
		FREELONG_DEBUG("FreeLong Memory Stack: lost %d* %d\n",m_ulBlockSize,m_nBlockOutSide.Get());
	}

	m_Lock.EnableWrite();
	{
		if(m_pFirstSon)
		{
			DestroySon(m_pFirstSon);
			m_pFirstSon=NULL;
		}

		if(m_pNext)
		{
			delete m_pNext;
			m_pNext=NULL;
		}
	}
	m_Lock.DisableWrite();

}

void* CFreeLongMemoryStackToken::Malloc(int nSize,                                      //申请的内存大小
										CMRSWInt& nAllBlockCount,                      //统计变量，返回管理过的内存块总数
										CMRSWInt& nMemoryUse)                         //统计变量，返回使用的内存块数量
{
	void* pRet=NULL;
	SFreeLongMemoryBlockHead* pNew=NULL;

	//将要申请的大小和本节点的大小比较
	if((FREELONG_MEM_BLOCK_SIZE(nSize))<m_ulBlockSize)
	{
		//本对象的大小可以满足要求，申请将从本节点右枝完成
		m_Lock.EnableWrite();
		{
			//判断是否有空闲的内存块备用
			if(!m_pFirstSon)
			{
				//如果没有，需要向系统申请一个内存块，申请的大小就是m_ulBlockSize
				//本对象申请的大小全是一样的
				pNew=(SFreeLongMemoryBlockHead*)malloc(m_ulBlockSize);
				if(pNew)
				{
					//统计函数加1，因为这个内存块马上会分配出去,修改内存块+1
					m_nBlockOutSide.Add();
					//这里是帮助上层统计内存总字节数
					nMemoryUse.Add(m_ulBlockSize);
					pNew->m_ulBlockSize=m_ulBlockSize;
					pNew->m_pNext=NULL;

					pRet=FREELONG_MEM_BLOCK_DATA(pNew);
					//统计变量，内存块总数+1
					nAllBlockCount.Add();
				}

				//如果申请失败，则直接返回NULL给应用程序
			}
			else
			{
				//这里有空闲块的情况，直接提取链表第一块，也就是栈上最新加入的内存块
				pNew=m_pFirstSon;
				//这里指针修订，注意，本类中m_pFirstSon已经指向了原来第二块内存
				m_pFirstSon=pNew->m_pNext;

				//同上，分配出去的内存，m_pNext无效，清空，变量如不使用，立即赋初始值
				pNew->m_pNext=NULL;

				//求出p1=p0+8,返回给应用程序
				pRet=FREELONG_MEM_BLOCK_DATA(pNew);

				//注意此处，这是把内部的内存块再分配重用，因此，outside +1,inside -1
				m_nBlockOutSide.Add();
				m_nBlockInSide.Dec();
			}
		}

		m_Lock.DisableWrite();                              //解锁
	}
	else      //下面是本类尺寸不符合分配要求，改由兄弟节点处理，在处理之前，先检查并生成兄弟节点，读写锁的作用域是分别规划的
	{
		m_Lock.EnableWrite();
		{
			//检测兄弟节点是否已经创建，如果没有，则创建
			//注意，这里和m_pNext有写关系，因此，加写锁
			if(!m_pNext)
			{
				m_pNext=new CFreeLongMemoryStackToken(m_ulBlockSize*2,m_pDebug);

				FREELONG_DEBUG("CFreeLongMemoryStackToken::Malloc(): m_ulBlockSize=%d\n",m_ulBlockSize);
			}
		}
		m_Lock.DisableWrite();

		//下面读写模式转换非常重要
		m_Lock.AddRead();
		{
			//将请求传递给兄弟节点的Malloc函数进行处理
			if(m_pNext)
			{
				pRet=m_pNext->Malloc(nSize,nAllBlockCount,nMemoryUse);
			}
		}

		m_Lock.DecRead();
	}

	return pRet;
}



bool CFreeLongMemoryStackToken::Free(void* pPoint,bool bCloseFlag)
{
	bool bRet=false;

	//注意这里，这个pOLd，已经是计算过的p0=p1-8
	SFreeLongMemoryBlockHead* pOld=FREELONG_MEM_BLOCK_HEAD(pPoint);

	//首先检测指定内存块大小是否由本对象管理
	if(m_ulBlockSize == pOld->m_ulBlockSize)
	{
		//判断内存块对象是否超限，如果是，直接释放，不做处理，因此，对于超限的内存块，节点总是存在的，但对象的右枝是空
		if(FREELONG_MEMORY_STACK_MAX_SAVE_BLOCK_SIZE<=m_ulBlockSize)
		{
			free(pOld);
			m_nBlockOutSide.Dec();   //注意，这个修订Outside统计变量
		}
		else if(bCloseFlag)
		{
			free(pOld);
			m_nBlockOutSide.Dec();
		}
		else
		{
			m_Lock.EnableWrite();
			{
				//典型的压栈操作，新加入的内存块，放在第一个
				//原来的被联接到新内存的下一个，先进先出
				pOld->m_pNext=m_pFirstSon;
				m_pFirstSon=pOld;
			}
			m_Lock.DisableWrite();
			m_nBlockOutSide.Dec();
			m_nBlockInSide.Add();
		}

		bRet=true;
	}
	else
	{
		//如果释放的内存是由内存池所创建，则必然存在节点
		m_Lock.AddRead();
		{
			if(m_pNext)
			{
				bRet=m_pNext->Free(pPoint,bCloseFlag);
			}
		}

		m_Lock.DecRead();
	}
	
	return bRet;
}


void CFreeLongMemoryStackToken::DestroySon(SFreeLongMemoryBlockHead* pSon)
{
	SFreeLongMemoryBlockHead* pObjNow=pSon;
	SFreeLongMemoryBlockHead* pObjNext=NULL;
	while(1)
	{
		if(!pObjNow)
		{
			break;
		}
		pObjNext=pObjNow->m_pNext;
		free(pObjNow);

		m_nBlockInSide.Dec();
		pObjNow=pObjNext;
	}
}

void CFreeLongMemoryStackToken::PrintStack()
{
	if(m_nBlockInSide.Get()+m_nBlockOutSide.Get())
	{
		CON_PRINTF(" [%ld] stack: all=%d,out=%d,in=%d\n",
			m_ulBlockSize,                                   //这里是提示内存块的大小
			m_nBlockInSide.Get() + m_nBlockOutSide.Get(),    //这是总的块数
			m_nBlockOutSide.Get(),                           //分配出去的内存块
			m_nBlockInSide.Get());                           //保留备用的内存块
	}

	m_Lock.AddRead();
	if(m_pNext)
	{
		m_pNext->PrintStack();
	}
	m_Lock.DecRead();
}