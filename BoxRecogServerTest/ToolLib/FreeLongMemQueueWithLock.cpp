// FreeLongMemQueueWithLock.cpp: implementation of the CFreeLongMemQueueWithLock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongMemQueueWithLock.h"
#include "SafePrint.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFreeLongMemQueueWithLock::CFreeLongMemQueueWithLock(CCFreeLong_LowDebug* pDebug,
													 CFreeLongMemoryPoolWithLock* pMemPool,
													 char* szAppName,
													 int nMaxToken)
{
	SafeStrcpy(m_szAppName,szAppName,FREELONG_APPLICATION_NAME_SIZE);
	m_pMemPool=pMemPool;

	m_pQueue=new CFreeLongMemQueue(pDebug,pMemPool,m_szAppName,nMaxToken);
	if(m_pQueue)
	{
		char szNameBuffer[256];
		SafePrint(szNameBuffer,256,"%s::m_pQueue",m_szAppName);

		//new成功，在内存池注册指针，以实现指针管理
		m_pMemPool->Register(m_pQueue,szNameBuffer);
	}
}

CFreeLongMemQueueWithLock::~CFreeLongMemQueueWithLock()
{
	m_Lock.EnableWrite();
	{
		if(m_pQueue)
		{
			m_pMemPool->UnRegister(m_pQueue); //否则内存池会报警
			delete m_pQueue;
			m_pQueue=NULL;
		}
	}
	m_Lock.DisableWrite();
}


bool CFreeLongMemQueueWithLock::ICanWork()
{
	if(!m_pMemPool)
	{
		return false;
	}

	if(!m_pQueue)
	{
		return false;
	}

	bool bRet=false;
	m_Lock.AddRead();
	{
		bRet=m_pQueue->ICanWork();
	}
	m_Lock.DecRead();
	return bRet;
}

int CFreeLongMemQueueWithLock::GetFirst(char* szBuffer,int nBufferSize)
{
	int nRet=0;
	m_Lock.AddRead();
	{
		nRet=m_pQueue->GetFirst(szBuffer,nBufferSize);
	}
	m_Lock.DecRead();

	return nRet;
}

int CFreeLongMemQueueWithLock::GetFirst(CFreeLongBuffer* pBuffer)
{
	int nRet=0;
	m_Lock.AddRead();
	{
		nRet=m_pQueue->GetFirst(pBuffer);
	}
	m_Lock.DecRead();
	
	return nRet;
	
}

int CFreeLongMemQueueWithLock::GetFirstLength()
{
	int nRet=0;
	m_Lock.AddRead();
	{
		nRet=m_pQueue->GetFirstLength();
	}
	m_Lock.DecRead();
	
	return nRet;
}

int CFreeLongMemQueueWithLock::GetTokenCount()
{

	int nRet=0;
	m_Lock.AddRead();
	{
		nRet=m_pQueue->GetTokenCount();
	}
	m_Lock.DecRead();
	
	return nRet;
}

void CFreeLongMemQueueWithLock::Write2File(char* szFileName)
{
	m_Lock.AddRead();
	{
		m_pQueue->Write2File(szFileName);
	}
	m_Lock.DecRead();
}

void CFreeLongMemQueueWithLock::PrintInside()
{
	m_Lock.AddRead();
	{
		m_pQueue->PrintInside();
	}
	m_Lock.DecRead();
}

int CFreeLongMemQueueWithLock::AddLast(char* szData,int nDataLenght)
{
	int nRet=0;
	m_Lock.EnableWrite();
	{
		nRet=m_pQueue->AddLast(szData,nDataLenght);
	}
	m_Lock.DisableWrite();
	
	return nRet;
}

bool CFreeLongMemQueueWithLock::DeleteFirst()
{
	bool bRet=false;
	m_Lock.EnableWrite();
	{
		bRet=m_pQueue->DeleteFirst();
	}
	m_Lock.DisableWrite();
	
	return bRet;
}

int CFreeLongMemQueueWithLock::GetAndDeleteFirst(char* szBuffer,int nBufferSize)
{
	int nRet=0;
	m_Lock.EnableWrite();
	{
		nRet=m_pQueue->GetAndDeleteFirst(szBuffer,nBufferSize);
	}
	m_Lock.DisableWrite();
	
	return nRet;
}

int CFreeLongMemQueueWithLock::GetAndDeleteFirst(CFreeLongBuffer* pBuffer)
{
	int nRet=false;
	m_Lock.EnableWrite();
	{
		nRet=m_pQueue->GetAndDeleteFirst(pBuffer);
	}
	m_Lock.DisableWrite();
	
	return nRet;
}

int CFreeLongMemQueueWithLock::PopFromFirst(char* szBuffer,int nBufferSize)
{
	int  nRet=0;
	m_Lock.EnableWrite();
	{
		nRet=m_pQueue->PopFromFirst(szBuffer,nBufferSize);
	}
	m_Lock.DisableWrite();
	
	return nRet;
}

int CFreeLongMemQueueWithLock::Push2Last(char* szData,int nDataLenght)
{
	int nRet=false;
	m_Lock.EnableWrite();
	{
		nRet=m_pQueue->Push2Last(szData,nDataLenght);
	}
	m_Lock.DisableWrite();
	
	return nRet;
}

void CFreeLongMemQueueWithLock::CleanAll()
{
	m_Lock.EnableWrite();
	{
		m_pQueue->CleanAll();
	}
	m_Lock.DisableWrite();
	
}


int CFreeLongMemQueueWithLock::ReadFromFile(char* szFileName)
{
	int nRet=false;
	m_Lock.EnableWrite();
	{
		nRet=m_pQueue->ReadFromFile(szFileName);
	}
	m_Lock.DisableWrite();
	
	return nRet;
}