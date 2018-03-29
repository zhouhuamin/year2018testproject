// FreeLongMemoryPoolWithLock.cpp: implementation of the CFreeLongMemoryPoolWithLock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongMemoryPoolWithLock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool inline SocketIsOK(Linux_Win_SOCKET nSocket)
{
	if(Linux_Win_InvalidSOCKET == nSocket)
	{
		return false;
	}
	
	return true;
}

void inline _Linux_Win_CloseSocket(Linux_Win_SOCKET nSocket)
{
	if(!SocketIsOK(nSocket))
	{
		return;
	}
	
#ifdef WIN32
	closesocket(nSocket);
#else
	close(nSocket);
#endif
	
}




CFreeLongMemoryPoolWithLock::CFreeLongMemoryPoolWithLock(CCFreeLong_LowDebug* pDebug,bool bOpenRegisterFlag)
{
	m_pDebug=pDebug;

	m_pMemPool=new CFreeLongMemoryStack(m_pDebug);
	m_pRegister=NULL;
	m_pSocketRegister=NULL;
	
	if(bOpenRegisterFlag)
	{
		m_pRegister=new CMemoryRegister(m_pDebug);
		m_pSocketRegister=new CSocketRegister(m_pDebug);

	}

	FREELONG_DEBUG("FreeLong. Memory Pool Open,register flag=%d\n",bOpenRegisterFlag);

	
}

CFreeLongMemoryPoolWithLock::~CFreeLongMemoryPoolWithLock()
{
	if(m_pRegister)
	{
		delete m_pRegister;
		m_pRegister=NULL;
	}

	if(m_pSocketRegister)
	{
		delete m_pSocketRegister;
		m_pSocketRegister=NULL;
	}

	if(m_pMemPool)
	{
		delete m_pMemPool;
		m_pMemPool=NULL;

	}

	FREELONG_DEBUG("FreeLong. Memory Pool Close\n");
}

void CFreeLongMemoryPoolWithLock::SetCloseFlag(bool bFlag)
{
	if(m_pMemPool)
	{
		m_pMemPool->SetCloseFlag(bFlag);
	}
}

void* CFreeLongMemoryPoolWithLock::Malloc(int nSize,char* szInfo)
{
	void* pRet=NULL;
	if(m_pMemPool)
	{
		pRet=m_pMemPool->Malloc(nSize);
		if(pRet)
		{
			Register(pRet,szInfo);
		}
	}

	return pRet;
}
void CFreeLongMemoryPoolWithLock::Free(void* pBlock)
{
	if(m_pMemPool)
	{
		m_pMemPool->Free(pBlock);
		UnRegister(pBlock);
	}
}

void* CFreeLongMemoryPoolWithLock::Remalloc(void* pPoint,int nNewSize,bool bCopyOldDataFlag)
{
	void* pRet=NULL;
	if(m_pMemPool)
	{
		pRet=m_pMemPool->ReMalloc(pPoint,nNewSize,bCopyOldDataFlag);

		if(m_pRegister)
		{
			if(pRet)
			{
				m_pRegister->Modeify(pPoint,pRet);
			}
			else
			{
				m_pRegister->Del(pPoint);
			}
		}
	}

	return pRet;
}

void CFreeLongMemoryPoolWithLock::Register(void* pPoint,char* szInfo)
{
	if(m_pRegister)
	{
		m_pRegister->Add(pPoint,szInfo);
	}
}

void CFreeLongMemoryPoolWithLock::UnRegister(void* pPoint)
{
	if(m_pRegister)
	{
		m_pRegister->Del(pPoint);
	}
}

void CFreeLongMemoryPoolWithLock::RegisterSocket(Linux_Win_SOCKET s,char* szInfo)
{
	if(m_pSocketRegister)
	{
		m_pSocketRegister->Add(s,szInfo);
	}
}


void CFreeLongMemoryPoolWithLock::CloseSocket(Linux_Win_SOCKET& s)
{
	if(m_pSocketRegister)
	{
		if(!m_pSocketRegister->Del(s))
		{
			FREELONG_DEBUG("CFreeLongMemoryPoolWithLock::CloseSocket(): \
							Socket %d is not registered!but I have close it yet\n",s);
		}

		_Linux_Win_CloseSocket(s);
		s=Linux_Win_InvalidSOCKET;
	}
}


void CFreeLongMemoryPoolWithLock::PrintTree()
{
	if(m_pMemPool)
	{
		m_pMemPool->PrintStack();
	}
}

void CFreeLongMemoryPoolWithLock::PrintInfo()
{
	if(m_pSocketRegister)
	{
		m_pSocketRegister->PrintInfo();
	}

	if(m_pRegister)
	{
		m_pRegister->PrintInfo();
	}

	if(m_pMemPool)
	{
		m_pMemPool->PrintInfo();
	}
}