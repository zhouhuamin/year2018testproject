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
	int m_nDatalength;                               // �洢��ҵ�����ݳ���
	char* m_pBuffer;                                 //ָ��ҵ�����ݿ��ָ��
	struct _FREELONG_QUEUE_TOKEN_HEAD_* m_pNext;     //ָ����һ���ڵ��ָ��
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
	int m_nMaxToken;                                    //���ö��е���󳤶�
	int m_nTokenCount;                                  //��������ЧToken������


	FreeLongQueueTokenHead* m_pHead;                    //ָ�����ͷ��
	FreeLongQueueTokenHead* m_pLast;                    //ָ�����β��ָ�룬���Լӿ��������������ͷ������

	CCFreeLong_LowDebug* m_pDebug;
	CFreeLongMemoryPoolWithLock* m_pMemPool;            //�ڴ��ָ��
	char m_szAppName[FREELONG_APPLICATION_NAME_SIZE];


};

#endif // !defined(AFX_FREELONGMEMQUEUE_H__009A7D72_5013_4E77_9A9D_C314D1968A99__INCLUDED_)
