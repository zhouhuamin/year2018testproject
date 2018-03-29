// FreeLongMemQueue.cpp: implementation of the CFreeLongMemQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongMemQueue.h"
#include "SafePrint.h"
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


CFreeLongMemQueue::CFreeLongMemQueue(CCFreeLong_LowDebug* pDebug,
									 CFreeLongMemoryPoolWithLock* pMemPool,
									 char* szAppName,
									 int nMaxToken)
{
	m_nMaxToken=nMaxToken;
	m_pHead=NULL;
	m_pLast=NULL;
	m_pDebug=pDebug;
	m_pMemPool=pMemPool;
	SafeStrcpy(m_szAppName,szAppName,FREELONG_APPLICATION_NAME_SIZE);
	m_nTokenCount=0;
	

}

CFreeLongMemQueue::~CFreeLongMemQueue()
{
	if(m_pHead)
	{
		DeleteTokenAndHisNext(m_pHead);
		m_pHead=NULL;
	}
}

bool CFreeLongMemQueue::ICanWork()
{
	if(!m_pDebug)
	{
		return false;
	}

	if(!m_pMemPool)
	{
		return false;
	}

	return true;
}

void CFreeLongMemQueue::CleanAll()
{
	if(!ICanWork())
	{
		return;
	}

	while (DeleteFirst()) 
	{
	}
	
}

int CFreeLongMemQueue::GetFirstLength()
{
	int nRet=0;
	if(m_pHead)
	{
		nRet=m_pHead->m_nDatalength;
	}

	return nRet;
}

int CFreeLongMemQueue::GetTokenCount()
{
	return m_nTokenCount;
}

void CFreeLongMemQueue::PrintAToken(FreeLongQueueTokenHead* pToken)
{
	if(!pToken)
	{
		return;
	}

	CON_PRINTF("Queue Token: pToken:%p, \
				Buffer=%p,Length=%d,m_pNext=%p\n",
				pToken,
				pToken->m_pBuffer,
				pToken->m_nDatalength,
				pToken->m_pNext);

	if(pToken->m_pNext)
	{
		PrintAToken(pToken->m_pNext);
	}

}

void CFreeLongMemQueue::PrintInside()
{
	if(!ICanWork())
	{
		return;
	}


	CON_PRINTF("Queue: TokenCount=%d,Head=0X%p,Last=0X%p\n",
		m_nTokenCount,
		m_pHead,
		m_pLast);

	if(m_pHead)
	{
		PrintAToken(m_pHead);
	}
}

bool CFreeLongMemQueue::DeleteTokenAndHisNext(FreeLongQueueTokenHead* pToken)
{
	bool bRet=false;
	if(!ICanWork())
	{
		goto CFreeLongMemQueue_DeleteTokenAndHisNext_End_Process;
	}

	if(pToken->m_pNext)
	{
		DeleteTokenAndHisNext(pToken->m_pNext);
		pToken->m_pNext=NULL;
	}

	if(pToken->m_pBuffer)
	{
		m_pMemPool->Free(pToken->m_pBuffer);
		pToken->m_nDatalength=0;
		pToken->m_pBuffer=NULL;
	}

	m_pMemPool->Free((void*)pToken);
	m_nTokenCount--;
	bRet=true;

CFreeLongMemQueue_DeleteTokenAndHisNext_End_Process:
	m_pLast=NULL;
	return bRet;
}


FreeLongQueueTokenHead* CFreeLongMemQueue::GetAToken()
{
	FreeLongQueueTokenHead* pToken=NULL;
	char* pTokenBuffer=NULL;
	char szNameBuffer[256];
	if(!ICanWork())
	{
		goto CFreeLongMemQueue_GetAToken_End_Process;
	}

	SafePrint(szNameBuffer,256,"%s::Token_Head",m_szAppName);

	pTokenBuffer=(char*)m_pMemPool->Malloc(FreeLongQueueTokenHeadSize,szNameBuffer);

	if(!pTokenBuffer)
	{
		FREELONG_DEBUG("%s::GetAToken():malloc new token fail!\n",m_szAppName);
		goto CFreeLongMemQueue_GetAToken_End_Process;
	}

	pToken=(FreeLongQueueTokenHead*)pTokenBuffer;
	pToken->m_nDatalength=0;
	pToken->m_pNext=NULL;
	pToken->m_pBuffer=NULL;
	m_nTokenCount++;

CFreeLongMemQueue_GetAToken_End_Process:
	return pToken;

}

int CFreeLongMemQueue::AddLast2ThisToken(FreeLongQueueTokenHead* pToken,char* szData,int nDataLength)
{
	int nRet=0;
	char szNameBUffer[256];

	if(!ICanWork())
	{
		goto CFreeLongMemQueue_AddLast2ThisToken_End_Process;
	}

	if(!pToken->m_pBuffer)
	{
		SafePrint(szNameBUffer,256,"%s::pToken->m_pBuffer",m_szAppName);

		pToken->m_pBuffer=(char*)m_pMemPool->Malloc(nDataLength,szNameBUffer);
		if(!pToken->m_pBuffer)
		{
			FREELONG_DEBUG("%s::AddLast2ThisToken():malloc pToken->m_pBuffer fail!\n",m_szAppName);
			goto CFreeLongMemQueue_AddLast2ThisToken_End_Process;
		}

		memcpy(pToken->m_pBuffer,szData,nDataLength);
		pToken->m_nDatalength=nDataLength;
		nRet=nDataLength;
		m_pLast=pToken;
		goto CFreeLongMemQueue_AddLast2ThisToken_End_Process;	
	}
	else
	{
		if(!pToken->m_pNext)
		{
			pToken->m_pNext=GetAToken();
			if(!pToken->m_pNext)
			{
				FREELONG_DEBUG("%s::AddLast2ThisToken():malloc pToken->m_pNext fail!\n",m_szAppName);
				goto CFreeLongMemQueue_AddLast2ThisToken_End_Process;
			}
		}

		if(pToken->m_pNext)
		{
			nRet=AddLast2ThisToken(pToken->m_pNext,szData,nDataLength);
		}
	}
CFreeLongMemQueue_AddLast2ThisToken_End_Process:
	return nRet;
	
}

int CFreeLongMemQueue::AddLast(char* szData,
								int nDataLenght,
								int nLimit)
{
	int nRet=0;
	if(!ICanWork())
	{
		goto CFreeLongMemQueue_AddLast_End_Process;
	}

	if(0>=nLimit)
	{
		if(m_nMaxToken<=m_nTokenCount)
		{
			goto CFreeLongMemQueue_AddLast_End_Process;
		}
	}
	else
	{
		if(nLimit<=m_nTokenCount)
		{
			goto CFreeLongMemQueue_AddLast_End_Process;
		}
	}

	if(!m_pHead)
	{
		m_pHead=GetAToken();
		if(!m_pHead)
		{
			FREELONG_DEBUG("%s::AddLast():malloc m_pHead fail!\n",m_szAppName);
			goto CFreeLongMemQueue_AddLast_End_Process;
		}
	}

	if(m_pLast)
	{
		nRet=AddLast2ThisToken(m_pLast,szData,nDataLenght);
	}
	else if(m_pHead)
	{
		nRet=AddLast2ThisToken(m_pHead,szData,nDataLenght);
	}

CFreeLongMemQueue_AddLast_End_Process:
	return nRet;
}


int CFreeLongMemQueue::GetFirst(char* szBuffer,int nBufferSize)
{
	int nRet=0;

	if(!ICanWork())
	{
		goto CFreeLongMemQueue_GetFirst_End_Process;
	}

	if(!m_pHead)
	{
		goto CFreeLongMemQueue_GetFirst_End_Process;
	}

	if(!m_pHead->m_pBuffer)
	{
		FREELONG_DEBUG("%s::GetFirst():m_pHead->m_pBuffer=null!\n",m_szAppName);
		goto CFreeLongMemQueue_GetFirst_End_Process;
	}

	if(m_pHead->m_nDatalength>nBufferSize)
	{
		FREELONG_DEBUG("%s::GetFirst():m_pHead->m_nDatalength > nBufferSize!\n",m_szAppName);
		goto CFreeLongMemQueue_GetFirst_End_Process;
	}

	memcpy(szBuffer,m_pHead->m_pBuffer,m_pHead->m_nDatalength);
	nRet=m_pHead->m_nDatalength;


CFreeLongMemQueue_GetFirst_End_Process:
	return nRet;
}


int CFreeLongMemQueue::GetFirst(CFreeLongBuffer* pBuffer)
{
	if(!ICanWork())
	{
		return 0;
	}

	if(!pBuffer)
	{
		return 0;
	}

	return pBuffer->BinCopyFrom(m_pHead->m_pBuffer,m_pHead->m_nDatalength);
}


bool CFreeLongMemQueue::DeleteFirst()
{
	bool bRet=false;
	FreeLongQueueTokenHead* pSecond=NULL;
	if(!ICanWork())
	{
		goto CFreeLongMemQueue_DeleteFirst_End_Process;
	}

	if(!m_pHead)
	{
		goto CFreeLongMemQueue_DeleteFirst_End_Process;
	}

	pSecond=m_pHead->m_pNext;
	m_pHead->m_pNext=NULL;

	bRet=DeleteTokenAndHisNext(m_pHead);

	if(bRet)
	{
		m_pHead=pSecond;
		if(!m_pHead)
		{
			m_pLast=NULL;
		}
	}
	else
	{
		FREELONG_DEBUG("%s::DeleteFirst():delete m_pHead fail!\n",m_szAppName);
		m_pHead->m_pNext=pSecond;
	}


CFreeLongMemQueue_DeleteFirst_End_Process:
	return bRet;
}


int CFreeLongMemQueue::GetAndDeleteFirst(char* szBuffer,int nBufferSize)
{
	int nRet=GetFirst(szBuffer,nBufferSize);
	if(nRet)
	{
		DeleteFirst();
	}
	
	return nRet;

}
int CFreeLongMemQueue::GetAndDeleteFirst(CFreeLongBuffer* pBuffer)
{
	int nRet=GetFirst(pBuffer);
	if(nRet)
	{
		DeleteFirst();
	}

	return nRet;
}


int CFreeLongMemQueue::PopFromFirst4FreeLongPopBuffer(CFreeLongPopBuffer* pPopBuffer)
{
	int nRet=0;
	if(!ICanWork())
	{
		goto CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process;
	}

	if(!m_pHead)
	{
		goto CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process;
	}

	if(!m_pHead->m_pBuffer)
	{
		FREELONG_DEBUG("%s::PopFromFirst4FreeLongPopBuffer():m_pHead->m_pBuffer=null!\n",m_szAppName);
		goto CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process;
	}

	if(!m_pHead->m_nDatalength)
	{
		FREELONG_DEBUG("%s::PopFromFirst4FreeLongPopBuffer():m_pHead->m_nDatalength=0!\n",m_szAppName);
		goto CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process;
	}

	if(!pPopBuffer)
	{
		FREELONG_DEBUG("%s::PopFromFirst4FreeLongPopBuffer():pPopBuffer=null!\n",m_szAppName);
		goto CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process;
	}

	nRet=pPopBuffer->AddLast(m_pHead->m_pBuffer,m_pHead->m_nDatalength);
	if(m_pHead->m_nDatalength!=nRet)
	{
		goto CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process;
	}
	if(!DeleteFirst())
	{
		FREELONG_DEBUG("%s::PopFromFirst4FreeLongPopBuffer():DeleteFirst fail!\n",m_szAppName);
		goto CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process;
	}

	if(m_pHead)
	{
		nRet+=PopFromFirst4FreeLongPopBuffer(pPopBuffer);
	}
CFreeLongMemQueue_PopFromFirst4FreeLongPopBuffer_End_Process:
	return nRet;
}


int CFreeLongMemQueue::PopFromFirst(char* szBuffer,int nBufferSize)
{
	int nCopyBytes=0;

	CFreeLongPopBuffer popBuffer(szBuffer,nBufferSize);

	if(!ICanWork())
	{
		goto CFreeLongMemQueue_PopFromFirst_End_Process;
	}

	if(m_pHead)
	{
		nCopyBytes=PopFromFirst4FreeLongPopBuffer(&popBuffer);
		if(nCopyBytes)
		{
			nCopyBytes=popBuffer.GetAllBytes();
		}
	}

	
CFreeLongMemQueue_PopFromFirst_End_Process:
	return nCopyBytes;
}

int CFreeLongMemQueue::Push2Last(char* szData,int nDataLenght)
{
	int nRet=0;

	CFreeLongPopBuffer popBuffer(szData,nDataLenght);
	if(!ICanWork())
	{
		goto CFreeLongMemQueue_Push2Last_End_Process;
	}

	popBuffer.MoveAllData(PushDataCallback,this);

CFreeLongMemQueue_Push2Last_End_Process:
	return nRet;
}

bool CFreeLongMemQueue::PushDataCallback(char* szData,int nDataLenght,void* pCallparam)
{
	CFreeLongMemQueue* pThis=(CFreeLongMemQueue*)pCallparam;
	int nRet=pThis->AddLast(szData,nDataLenght);

	if(nDataLenght==nRet)
	{
		return true;
	}
	else
	{
		pThis->FREELONG_DEBUG("%s::PushDataCallback():I am full!\n",pThis->m_szAppName);
		return false;
	}
}

void CFreeLongMemQueue::WriteAToken2File(FreeLongQueueTokenHead* pToken,FILE* fp)
{
	if(!ICanWork())
	{
		return;
	}

	if(!fp)
	{
		return;
	}

	if(!pToken)
	{
		return;
	}

	if(!pToken->m_pBuffer)
	{
		return;
	}

	if(!pToken->m_nDatalength)
	{
		return;
	}

	fwrite((void*)&(pToken->m_nDatalength),sizeof(int),1,fp);

	fwrite((pToken->m_pBuffer),sizeof(char),pToken->m_nDatalength,fp);

	if(pToken->m_pNext)
	{
		WriteAToken2File(pToken->m_pNext,fp);
	}
}

void CFreeLongMemQueue::Write2File(char* szFileName)
{
	FILE* fp=NULL;
	if(!ICanWork())
	{
		return;
	}

	if(!m_pHead)
	{
		return;
	}

	if(!GetTokenCount())
	{
		return;
	}

	/*
	 *	这里是以递归的形式写入文件，每一次写入都将以前的记录覆盖，记录列表里的新的文件
	 */
	fp=fopen(szFileName,"wb");    //wb的形式打开文件，都将以前的记录覆盖
	if(fp)
	{
		fwrite((void*)&m_nTokenCount,sizeof(char),4,fp);
		WriteAToken2File(m_pHead,fp);
		fclose(fp);
	}
}

int CFreeLongMemQueue::ReadFromFile(char* szFileName)
{
	FILE* fp=NULL;
	int n=0;
	int i=0;
	int nReadTokenCount=0;
	int nDataLength=0;

	char* pTempBuffer=NULL;
	char szNameBuffer[256];
	if(!ICanWork())
	{
		goto CFreeLongMemQueue_ReadFromFile_End_Process;
	}

	SafePrint(szNameBuffer,256,"%s::ReadFromFile::pTempBuffer",m_szAppName);
	pTempBuffer=(char*)m_pMemPool->Malloc(1,szNameBuffer);

	fp=fopen(szFileName,"rb");
	if(!fp)
	{
		goto CFreeLongMemQueue_ReadFromFile_End_Process;
	}

	n=fread((void*)&nReadTokenCount,sizeof(char),4,fp);
	if(4>n)
	{
		FREELONG_DEBUG("%s::ReadFromFile():read token count fail!\n",m_szAppName);
		goto CFreeLongMemQueue_ReadFromFile_End_Process;
	}

	for(i=0;i<nReadTokenCount;i++)
	{
		n=fread((void*)&nDataLength,sizeof(char),4,fp);
		if(4>n)
		{
			FREELONG_DEBUG("%s::ReadFromFile():%d/%d,read data length fail!\n",m_szAppName,i,nReadTokenCount);
			goto CFreeLongMemQueue_ReadFromFile_End_Process;
		}

		if(0>nDataLength)
		{
			FREELONG_DEBUG("%s::ReadFromFile():%d/%d,nDataLength=%d!\n",m_szAppName,i,nReadTokenCount,nDataLength);
			goto CFreeLongMemQueue_ReadFromFile_End_Process;
		}

		pTempBuffer=(char*)m_pMemPool->Remalloc(pTempBuffer,
												nDataLength,
												false);


		if(!pTempBuffer)
		{
			FREELONG_DEBUG("%s::ReadFromFile():remalloc pTempBuffer fsil!\n",m_szAppName);
			goto CFreeLongMemQueue_ReadFromFile_End_Process;
		}

		n=fread((void*)pTempBuffer,sizeof(char),nDataLength,fp);
		if(nDataLength>n)
		{
			FREELONG_DEBUG("%s::ReadFromFile():read data fail!\n",m_szAppName);
			goto CFreeLongMemQueue_ReadFromFile_End_Process;
		}

		if(!AddLast(pTempBuffer,nDataLength))
		{
			break;
		}
	}

CFreeLongMemQueue_ReadFromFile_End_Process:
	if(pTempBuffer)
	{
		m_pMemPool->Free(pTempBuffer);
		pTempBuffer=NULL;

	}

	if(fp)
	{
		fclose(fp);
		fp=NULL;
	}

	return nReadTokenCount;
}