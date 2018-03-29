// FreeLongMemQueueWithLock.h: interface for the CFreeLongMemQueueWithLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_MEMORY_QUEUE_WITH_LOCK_
#define _FREELONG_MEMORY_QUEUE_WITH_LOCK_

#include "FreeLongMemQueue.h"
#include "MultiReadSingleWriteLock.h"

class CFreeLongMemQueueWithLock  
{
public:
	CFreeLongMemQueueWithLock(CCFreeLong_LowDebug* pDebug,
		CFreeLongMemoryPoolWithLock* pMemPool,
		char* szAppName,
		int nMaxToken=FREELONG_CHAIN_TOKEN_MAX);
	~CFreeLongMemQueueWithLock();

public:
	bool ICanWork();
	int AddLast(char* szData,int nDataLenght);
	

	int GetFirst(char* szBuffer,int nBufferSize);
	int GetFirst(CFreeLongBuffer* pBuffer);
	
	int GetFirstLength();
	int GetTokenCount();
	
	int GetAndDeleteFirst(char* szBuffer,int nBufferSize);
	int GetAndDeleteFirst(CFreeLongBuffer* pBuffer);

	int PopFromFirst(char* szBuffer,int nBufferSize);
	int Push2Last(char* szData,int nDataLenght);


	void CleanAll();
	bool DeleteFirst();
	

	void Write2File(char* szFileName);
	int ReadFromFile(char* szFileName);
	void PrintInside();
	

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
private:
	CFreeLongMemoryPoolWithLock* m_pMemPool;
	CFreeLongMemQueue* m_pQueue;
	CMultiReadSingleWriteLock m_Lock;

	char m_szAppName[FREELONG_APPLICATION_NAME_SIZE];
	

	
};

#endif 
