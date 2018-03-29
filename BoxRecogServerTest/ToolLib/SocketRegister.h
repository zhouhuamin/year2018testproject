// SocketRegister.h: interface for the CSocketRegister class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _SOCKET_REGISTER_H_
#define _SOCKET_REGISTER_H_
#include "CFreeLong_LowDebug.h"
#include "MemoryRegister.h"
#include "sockinclude.h"

#pragma comment(lib,"ws2_32.lib")	


typedef struct _FREELONG_SOCKET_REGISTER_
{
	Linux_Win_SOCKET m_nSocket;
	char             m_szInfo[FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE];
}SFreeLongSocketRegister;

const ULONG SFreeLongSocketRegisterSize=sizeof(SFreeLongSocketRegister);


class CSocketRegister  
{
public:
	CSocketRegister(CCFreeLong_LowDebug* pDebug);
	virtual ~CSocketRegister();

public:
	void PrintInfo();
public:
	void Add(Linux_Win_SOCKET s,char* szInfo=NULL);

	bool Del(Linux_Win_SOCKET s);
private:
	void inline SafeStrcpy(char* pDest,char* pSource,int nCount)
	{
		int nLen=(int)strlen(pSource)+1;
		
		if(!pDest)
		{
			goto SafeStrcpy_END_PROCESS;
		}
		if(!pSource)
		{
			goto SafeStrcpy_END_PROCESS;
		}
		
		if(nLen > nCount)
		{
			nLen=nCount;
		}
		
		memcpy(pDest,pSource,nLen);
		*(pDest+nLen-1)='\0';
		
		
		
SafeStrcpy_END_PROCESS:
		return;
	}

	
	CCFreeLong_LowDebug* m_pDebug;
	CMutexLock           m_Lock;
	SFreeLongSocketRegister           m_RegisterArray[FREELONG_MEMORY_REGISTER_MAX];
	int                               m_nUseMax;
	Linux_Win_SOCKET                  m_nMaxSocket;
	int                               m_nSocketUseCount;

};

#endif 
