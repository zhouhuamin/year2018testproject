// MemoryRegister.cpp: implementation of the CMemoryRegister class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemoryRegister.h"
#include "FreeLongMemoryStackToken.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



#define CONDEBUG 1
#ifdef CONDEBUG
#define CON_PRINTF printf
#else
#define CON_PRINTF /\
/printf
#endif


CMemoryRegister::CMemoryRegister(CCFreeLong_LowDebug* pDebug)
{
	m_pDebug=pDebug;

	m_pMaxPoint=NULL;
	m_nUseMax=0;
	m_nPointCount=0;

	int i=0;
	for(i=0;i<FREELONG_MEMORY_REGISTER_MAX;i++)
	{
		m_RegisterArray[i].m_pPoint=NULL;
		FREELONG_CLEAN_CHAR_BUFFER(m_RegisterArray[i].m_szInfo);
	}
}

CMemoryRegister::~CMemoryRegister()
{
	int i=0;
	m_Lock.Lock();
	{
		FREELONG_DEBUG("CMemoryRegister: Max Register Point=0X%p\n",m_pMaxPoint);

		for(i=0;i<m_nUseMax;i++)
		{
			if(m_RegisterArray[i].m_pPoint)
			{
				//对于没有释放的内存，进行报警显示
				FREELONG_DEBUG("***** Memory Lost: [%p] - %s\n",m_RegisterArray[i].m_pPoint,m_RegisterArray[i].m_szInfo);
			}
		}
	}
	m_Lock.Unlock();
}

void CMemoryRegister::RegisterCopy(SFreeLongMemoryRegister* pDest,void* pPoint,char* szInfo)
{
	pDest->m_pPoint=pPoint;
	if(szInfo)
	{
		SafeStrcpy(pDest->m_szInfo,szInfo,FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE);
	}
	else
	{
		FREELONG_CLEAN_CHAR_BUFFER(szInfo);
	}
}

void CMemoryRegister::Add(void* pPoint,char* szInfo)
{
	int i=0;
	m_Lock.Lock();
	{
		if(pPoint > m_pMaxPoint)
		{
			m_pMaxPoint=pPoint;
		}

		for(i=0;i<m_nUseMax;i++)
		{
			if(!m_RegisterArray[i].m_pPoint)
			{
				m_nPointCount++;
				RegisterCopy(&(m_RegisterArray[i]),pPoint,szInfo);
				goto CMemoryRegister_Add_End_Process;
			}
		}

		if(FREELONG_MEMORY_REGISTER_MAX<= m_nUseMax)
		{
			FREELONG_DEBUG("***ERROR*** CMemoryRegister is full!\n");
			goto CMemoryRegister_Add_End_Process;
		}

		RegisterCopy(&(m_RegisterArray[m_nUseMax]),pPoint,szInfo);
		m_nPointCount++;
		m_nUseMax++;
	}

CMemoryRegister_Add_End_Process:
	m_Lock.Unlock();
}

void CMemoryRegister::Del(void* pPoint)
{
	int i=0;
	m_Lock.Lock();
	{
		for(i=0;i<m_nUseMax;i++)
		{
			if(pPoint == m_RegisterArray[i].m_pPoint)
			{
				m_nPointCount--;
				m_RegisterArray[i].m_pPoint=NULL;
				FREELONG_CLEAN_CHAR_BUFFER(m_RegisterArray[i].m_szInfo);
				goto CMemoryRegister_Del_End_Process;
			}
		}
	}
	
CMemoryRegister_Del_End_Process:
	m_Lock.Unlock();
}

void CMemoryRegister::Modeify(void* pOld,void* pNew)
{
	int i=0;
	m_Lock.Lock();
	{
		if(pOld > m_pMaxPoint)
		{
			m_pMaxPoint=pOld;
		}
		
		for(i=0;i<m_nUseMax;i++)
		{
			if(pOld==m_RegisterArray[i].m_pPoint)
			{
				m_RegisterArray[i].m_pPoint=pNew;
				goto CMemoryRegister_Add_End_Process;
			}
		}	
	}
	
	FREELONG_DEBUG("***ERROR*** CMemoryRegister::Modeify(): I can\'t find point!\n");
CMemoryRegister_Add_End_Process:
	m_Lock.Unlock();
}


void CMemoryRegister::PrintInfo()
{
	m_Lock.Lock();
	{
		CON_PRINTF("memory pool: %d / %d,biggest=%p\n",
			m_nPointCount,
			m_nUseMax,
			m_pMaxPoint);

	
	}
	m_Lock.Unlock();
}
