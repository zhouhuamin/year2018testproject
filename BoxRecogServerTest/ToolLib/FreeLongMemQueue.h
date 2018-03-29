// FreeLongMemQueue.h: interface for the CFreeLongMemQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_MEM_QUEUE_H_
#define _FREELONG_MEM_QUEUE_H_

#include "CFreeLong_LowDebug.h"
#include "FreeLongMemoryPoolWithLock.h"
#include "FreeLongBuffer.h"
#include "FreeLongPopBuffer.h"

typedef struct _FREELONG_QUEUE_TOKEN_HEAD_ 
{
	int m_nDatalength;                               // 存储的业务数据长度
	char* m_pBuffer;                                 //指向业务数据块的指针
	struct _FREELONG_QUEUE_TOKEN_HEAD_* m_pNext;     //指向下一个节点的指针
}FreeLongQueueTokenHead;

const ULONG FreeLongQueueTokenHeadSize=sizeof(FreeLongQueueTokenHead);

#define  FREELONG_APPLICATION_NAME_SIZE  256
#define  FREELONG_CHAIN_TOKEN_MAX        1024
class CFreeLongMemQueue  
{
public:
	CFreeLongMemQueue(CCFreeLong_LowDebug* pDebug,
						CFreeLongMemoryPoolWithLock* pMemPool,
						char* szAppName,
						int nMaxToken=FREELONG_CHAIN_TOKEN_MAX);

	~CFreeLongMemQueue();
public:
	bool ICanWork();
	void CleanAll();
	int GetFirstLength();
	int GetTokenCount();
	void PrintInside();

public:
	int AddLast(char* szData,
				int nDataLenght,
				int nLimit=-1);
	int GetFirst(char* szBuffer,int nBufferSize);
	int GetFirst(CFreeLongBuffer* pBuffer);

	bool DeleteFirst();
	int GetAndDeleteFirst(char* szBuffer,int nBufferSize);
	int GetAndDeleteFirst(CFreeLongBuffer* pBuffer);

public:
	int PopFromFirst(char* szBuffer,int nBufferSize);
	int Push2Last(char* szData,int nDataLenght);

public:
	void Write2File(char* szFileName);
	int ReadFromFile(char* szFileName);

private:
	void PrintAToken(FreeLongQueueTokenHead* pToken);
	void WriteAToken2File(FreeLongQueueTokenHead* pToken,FILE* fp);

	int PopFromFirst4FreeLongPopBuffer(CFreeLongPopBuffer* pPopBuffer);
	int AddLast2ThisToken(FreeLongQueueTokenHead* pToken,char* szData,int nDataLenght);
	
	FreeLongQueueTokenHead* GetAToken();
	bool DeleteTokenAndHisNext(FreeLongQueueTokenHead* pToken);

	static bool PushDataCallback(char* szData,int nDataLenght,void* pCallparam);
	


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
	int m_nMaxToken;                                    //设置队列的最大长度
	int m_nTokenCount;                                  //队列中有效Token的数量


	FreeLongQueueTokenHead* m_pHead;                    //指向队列头部
	FreeLongQueueTokenHead* m_pLast;                    //指向队列尾的指针，可以加快插入操作，避免从头部遍历

	CCFreeLong_LowDebug* m_pDebug;
	CFreeLongMemoryPoolWithLock* m_pMemPool;            //内存池指针
	char m_szAppName[FREELONG_APPLICATION_NAME_SIZE];


};

#endif // !defined(AFX_FREELONGMEMQUEUE_H__009A7D72_5013_4E77_9A9D_C314D1968A99__INCLUDED_)
