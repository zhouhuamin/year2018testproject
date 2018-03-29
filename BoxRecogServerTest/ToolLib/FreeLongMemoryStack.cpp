// FreeLongMemoryStack.cpp: implementation of the CFreeLongMemoryStack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongMemoryStack.h"
#include "SafePrint.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFreeLongMemoryStack::CFreeLongMemoryStack(CCFreeLong_LowDebug* pDebug)
{
	m_CloseFlag.Set(false);
	m_pDebug=pDebug;
	m_pMaxPoint.Set(0);
	m_nAllBlockCount.Set(0);
	m_nMemoryUse.Set(0);

	//����С�ڴ��С������֦��һ���ڵ㣬�����������ڼ䣬��ֻ֦�����������٣�������Ϊ�������������ڴ���Ƭ
	m_pHead=new CFreeLongMemoryStackToken(FREELONG_MEMORY_STACK_BLOCK_MIN,m_pDebug);
}

CFreeLongMemoryStack::~CFreeLongMemoryStack()
{
	FREELONG_DEBUG("Memory Stack: Max Point=0X%p \n",m_pMaxPoint.Get());

	//�����֦,�ݹ�ɾ��
	if(m_pHead)
	{
		delete m_pHead;
		m_pHead=NULL;
	}
}

void CFreeLongMemoryStack::SetCloseFlag(bool bFlag)
{
	m_CloseFlag.Set(bFlag);
}

void* CFreeLongMemoryStack::Malloc(int nSize)
{
	void* pRet=NULL;
	if(nSize<=0)
	{
		FREELONG_DEBUG("CFreeLongMemoryStack::Malloc(): ERROR!nSize=%d\n",nSize);
		return pRet;
	}

	if(m_pHead)
	{
		pRet=m_pHead->Malloc(nSize,m_nAllBlockCount,m_nMemoryUse);
		
		//ͳ�ƹ��ܣ�ͳ������ָ��
		if(m_pMaxPoint.Get()<(int)pRet)
		{
			m_pMaxPoint.Set((int)pRet);
		}
	}
	return pRet;
}

bool  CFreeLongMemoryStack::Free(void* pPoint)
{
	bool bRet=false;
	if(m_pHead)
	{
		bRet=m_pHead->Free(pPoint,m_CloseFlag.Get());
	}

	return bRet;
}

void* CFreeLongMemoryStack::ReMalloc(void* pPoint,                                   //�ɵ�ָ��
										int nNewSize,                                //�µĳߴ�Ҫ��
										bool bCopyOldDataFlag)                       //�Ƿ񱸷ݾ�����
{
	void* pRet=NULL;
	SFreeLongMemoryBlockHead* pOldToken=NULL;
	int nOldLen=0;

	if(nNewSize<=0)
	{
		FREELONG_DEBUG("CFreeLongMemoryStack::ReMalloc(): ERROR!nSize=%d\n",nNewSize);
		goto CFreeLongMemoryStack_Remalloc_Free_Old;
	}


	pOldToken=FREELONG_MEM_BLOCK_HEAD(pPoint);
	nOldLen=pOldToken->m_ulBlockSize;

	if(FREELONG_MEM_BLOCK_SIZE(nNewSize) <= (ULONG)nOldLen)
	{
		pRet=pPoint;
		goto CFreeLongMemoryStack_Remalloc_End_Process;
	}

	//��ȷ���µĳߴ�Ƚϴ�ԭ�е��ڴ��޷����ܣ����������ڴ����Ԫ����������һ��
	
	pRet=m_pHead->Malloc(nNewSize,m_nAllBlockCount,m_nMemoryUse);

	if(pRet && pPoint)
	{
		if(bCopyOldDataFlag)
		{
			memcpy(pRet,pPoint,nOldLen);
		}
	}
		
CFreeLongMemoryStack_Remalloc_Free_Old:
	m_pHead->Free(pPoint,m_CloseFlag.Get());
CFreeLongMemoryStack_Remalloc_End_Process:
	return pRet;
}


void CFreeLongMemoryStack::PrintInfo()
{
	CON_PRINTF("block=%d,use=%d kbytes,biggest=%p \n",
		m_nAllBlockCount.Get(),
		m_nMemoryUse.Get()/1024,
		m_pMaxPoint.Get());

}

void CFreeLongMemoryStack::PrintStack()
{
	if(m_pHead)
	{
		m_pHead->PrintStack();
	}
}