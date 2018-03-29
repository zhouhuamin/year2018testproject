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

void* CFreeLongMemoryStackToken::Malloc(int nSize,                                      //������ڴ��С
										CMRSWInt& nAllBlockCount,                      //ͳ�Ʊ��������ع�������ڴ������
										CMRSWInt& nMemoryUse)                         //ͳ�Ʊ���������ʹ�õ��ڴ������
{
	void* pRet=NULL;
	SFreeLongMemoryBlockHead* pNew=NULL;

	//��Ҫ����Ĵ�С�ͱ��ڵ�Ĵ�С�Ƚ�
	if((FREELONG_MEM_BLOCK_SIZE(nSize))<m_ulBlockSize)
	{
		//������Ĵ�С��������Ҫ�����뽫�ӱ��ڵ���֦���
		m_Lock.EnableWrite();
		{
			//�ж��Ƿ��п��е��ڴ�鱸��
			if(!m_pFirstSon)
			{
				//���û�У���Ҫ��ϵͳ����һ���ڴ�飬����Ĵ�С����m_ulBlockSize
				//����������Ĵ�Сȫ��һ����
				pNew=(SFreeLongMemoryBlockHead*)malloc(m_ulBlockSize);
				if(pNew)
				{
					//ͳ�ƺ�����1����Ϊ����ڴ�����ϻ�����ȥ,�޸��ڴ��+1
					m_nBlockOutSide.Add();
					//�����ǰ����ϲ�ͳ���ڴ����ֽ���
					nMemoryUse.Add(m_ulBlockSize);
					pNew->m_ulBlockSize=m_ulBlockSize;
					pNew->m_pNext=NULL;

					pRet=FREELONG_MEM_BLOCK_DATA(pNew);
					//ͳ�Ʊ������ڴ������+1
					nAllBlockCount.Add();
				}

				//�������ʧ�ܣ���ֱ�ӷ���NULL��Ӧ�ó���
			}
			else
			{
				//�����п��п�������ֱ����ȡ�����һ�飬Ҳ����ջ�����¼�����ڴ��
				pNew=m_pFirstSon;
				//����ָ���޶���ע�⣬������m_pFirstSon�Ѿ�ָ����ԭ���ڶ����ڴ�
				m_pFirstSon=pNew->m_pNext;

				//ͬ�ϣ������ȥ���ڴ棬m_pNext��Ч����գ������粻ʹ�ã���������ʼֵ
				pNew->m_pNext=NULL;

				//���p1=p0+8,���ظ�Ӧ�ó���
				pRet=FREELONG_MEM_BLOCK_DATA(pNew);

				//ע��˴������ǰ��ڲ����ڴ���ٷ������ã���ˣ�outside +1,inside -1
				m_nBlockOutSide.Add();
				m_nBlockInSide.Dec();
			}
		}

		m_Lock.DisableWrite();                              //����
	}
	else      //�����Ǳ���ߴ粻���Ϸ���Ҫ�󣬸����ֵܽڵ㴦���ڴ���֮ǰ���ȼ�鲢�����ֵܽڵ㣬��д�����������Ƿֱ�滮��
	{
		m_Lock.EnableWrite();
		{
			//����ֵܽڵ��Ƿ��Ѿ����������û�У��򴴽�
			//ע�⣬�����m_pNext��д��ϵ����ˣ���д��
			if(!m_pNext)
			{
				m_pNext=new CFreeLongMemoryStackToken(m_ulBlockSize*2,m_pDebug);

				FREELONG_DEBUG("CFreeLongMemoryStackToken::Malloc(): m_ulBlockSize=%d\n",m_ulBlockSize);
			}
		}
		m_Lock.DisableWrite();

		//�����дģʽת���ǳ���Ҫ
		m_Lock.AddRead();
		{
			//�����󴫵ݸ��ֵܽڵ��Malloc�������д���
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

	//ע��������pOLd���Ѿ��Ǽ������p0=p1-8
	SFreeLongMemoryBlockHead* pOld=FREELONG_MEM_BLOCK_HEAD(pPoint);

	//���ȼ��ָ���ڴ���С�Ƿ��ɱ��������
	if(m_ulBlockSize == pOld->m_ulBlockSize)
	{
		//�ж��ڴ������Ƿ��ޣ�����ǣ�ֱ���ͷţ�����������ˣ����ڳ��޵��ڴ�飬�ڵ����Ǵ��ڵģ����������֦�ǿ�
		if(FREELONG_MEMORY_STACK_MAX_SAVE_BLOCK_SIZE<=m_ulBlockSize)
		{
			free(pOld);
			m_nBlockOutSide.Dec();   //ע�⣬����޶�Outsideͳ�Ʊ���
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
				//���͵�ѹջ�������¼�����ڴ�飬���ڵ�һ��
				//ԭ���ı����ӵ����ڴ����һ�����Ƚ��ȳ�
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
		//����ͷŵ��ڴ������ڴ�������������Ȼ���ڽڵ�
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
			m_ulBlockSize,                                   //��������ʾ�ڴ��Ĵ�С
			m_nBlockInSide.Get() + m_nBlockOutSide.Get(),    //�����ܵĿ���
			m_nBlockOutSide.Get(),                           //�����ȥ���ڴ��
			m_nBlockInSide.Get());                           //�������õ��ڴ��
	}

	m_Lock.AddRead();
	if(m_pNext)
	{
		m_pNext->PrintStack();
	}
	m_Lock.DecRead();
}