// SocketRegister.cpp: implementation of the CSocketRegister class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketRegister.h"
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



CSocketRegister::CSocketRegister(CCFreeLong_LowDebug* pDebug)
{
	m_pDebug=pDebug;

	m_nMaxSocket=Linux_Win_InvalidSOCKET;
	m_nSocketUseCount=0;
	m_nUseMax=0;
	int i=0;
	for(i=0;i<FREELONG_MEMORY_REGISTER_MAX;i++)
	{
		m_RegisterArray[i].m_nSocket=Linux_Win_InvalidSOCKET;
		FREELONG_CLEAN_CHAR_BUFFER(m_RegisterArray[i].m_szInfo);
	}
}

CSocketRegister::~CSocketRegister()
{

	int i=0;
	m_Lock.Lock();
	{
		FREELONG_DEBUG("CSocketRegister: Max Socket Count=%d,max Socket =%d\n",m_nUseMax,m_nMaxSocket);
		
		for(i=0;i<m_nUseMax;i++)
		{
			if(SocketIsOK(m_RegisterArray[i].m_nSocket))
			{
				//对于没有释放的句柄，进行报警显示，并关闭
				FREELONG_DEBUG("***** Socket Lost: [%d] - %s\n",m_RegisterArray[i].m_nSocket,m_RegisterArray[i].m_szInfo);
				_Linux_Win_CloseSocket(m_RegisterArray[i].m_nSocket);
			}
		}
	}
	m_Lock.Unlock();

}

void CSocketRegister::Add(Linux_Win_SOCKET s,char* szInfo)
{

	int i=0;
	m_Lock.Lock();
	{
		if(!SocketIsOK(m_nMaxSocket))
		{
			m_nMaxSocket=s;
		} 
		else if(s>m_nMaxSocket)
		{
			m_nMaxSocket=s;
		}
		
		for(i=0;i<m_nUseMax;i++)
		{
			if(m_RegisterArray[i].m_nSocket==s)
			{
				if(szInfo)
				{
					SafeStrcpy(m_RegisterArray[i].m_szInfo,szInfo,FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE);
				}
				goto CSocketRegister_Add_End_Process;
			}
		}


		for(i=0;i<m_nUseMax;i++)
		{
			if(!SocketIsOK(m_RegisterArray[i].m_nSocket))
			{
				m_RegisterArray[i].m_nSocket=s;
				if(szInfo)
				{
					SafeStrcpy(m_RegisterArray[i].m_szInfo,szInfo,FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE);
				}

				m_nSocketUseCount++;

				goto CSocketRegister_Add_End_Process;
			}
		}
		
		
		//
		if(FREELONG_MEMORY_REGISTER_MAX > m_nUseMax)
		{
			m_RegisterArray[i].m_nSocket=s;
			if(szInfo)
			{
				SafeStrcpy(m_RegisterArray[i].m_szInfo,szInfo,FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE);
			}
			
			m_nUseMax++;
			m_nSocketUseCount++;
		}
		else
		{
			FREELONG_DEBUG("***ERROR*** CSocketRegister::Add:pool is full!\n");
		}
	}
CSocketRegister_Add_End_Process:
	m_Lock.Unlock();
}

bool CSocketRegister::Del(Linux_Win_SOCKET s)
{
	bool bRet=false;
	int i=0;
	m_Lock.Lock();
	{
		for(i=0;i<m_nUseMax;i++)
		{
			if(s == m_RegisterArray[i].m_nSocket)
			{
				m_RegisterArray[i].m_nSocket=Linux_Win_InvalidSOCKET;
				FREELONG_CLEAN_CHAR_BUFFER(m_RegisterArray[i].m_szInfo);
				m_nSocketUseCount--;
				bRet=true;
				goto CSocketRegister_Del_End_Process;
			}
		}
	}
	
CSocketRegister_Del_End_Process:
	m_Lock.Unlock();
	return bRet;
}

void CSocketRegister::PrintInfo()
{
	m_Lock.Lock();
	{
		CON_PRINTF("socket pool: %d / %d,biggest=%d\n",
			m_nSocketUseCount,
			m_nUseMax+1,
			m_nMaxSocket);

	}
	m_Lock.Unlock();
}