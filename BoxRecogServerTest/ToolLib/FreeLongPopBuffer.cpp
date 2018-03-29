// FreeLongPopBuffer.cpp: implementation of the CFreeLongPopBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongPopBuffer.h"
#include "FreeLongBuffer.h"


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


CFreeLongPopBuffer::CFreeLongPopBuffer(char* szBuffer,int nBufferSize,bool bInitFlag)
{
	m_pHead=NULL;
	m_pBuffer=NULL;
	m_nBufferSize=0;
	Set(szBuffer,nBufferSize);

	//这个类是粘合类，内部缓冲区不是它自己申请的，而是外部传入的
	if(bInitFlag)
	{
		Clean();
	}
}

CFreeLongPopBuffer::~CFreeLongPopBuffer()
{

}


bool CFreeLongPopBuffer::ICanWork()
{
	if(!m_pBuffer)
	{
		return false;
	}

	if(!m_nBufferSize)
	{
		return false;
	}

	if(!m_pHead)
	{
		return false;
	}

	return true;
}

void CFreeLongPopBuffer::Set(char* szBuffer,int nBufferSize)
{
	m_pBuffer=szBuffer;
	m_nBufferSize=nBufferSize;
	m_pHead=(FreeLongPopBufferHead*)m_pBuffer;            //定义队列头指针

}

void CFreeLongPopBuffer::Clean()
{
	if(m_pHead)
	{
		m_pHead->m_nTokenCount=0;

		//即使内部一个Token也没有，字节数起码是FreeLongPopBufferHeadSize
		m_pHead->m_nAllBytesCount=FreeLongPopBufferHeadSize;
	}
}

void CFreeLongPopBuffer::PrintInside()
{
	if(!ICanWork())
	{
		CON_PRINTF("CFreeLongPopBuffer::PrintInside(): ERROR! m_pBuffer=null\n");
		return;
	}

	//定义元素区开始的指针
	char* pTokenBegin=FREE_LONG_POP_BUFFER_FIRST_TOKEN_BEGIN(m_pBuffer);

	FreeLongPopBufferTokenHead* pTokenHead=(FreeLongPopBufferTokenHead*)pTokenBegin;

	char* pTokenData=FREE_LONG_BUFFER_TOKEN_DATA_BEGIN(pTokenBegin);

	int i=0;
	
	CON_PRINTF("CFreeLongPopBuffer::PrintInside(): Token:%d!Bytes: %d\n",
		m_pHead->m_nTokenCount,m_pHead->m_nAllBytesCount);

	for(i=0;i<m_pHead->m_nTokenCount;i++)
	{
//		CON_PRINTF("[%d] - %s\n",pTokenHead->m_nDataLength);

		pTokenBegin+=FREE_LONG_POP_BUFFER_TOKEN_LENGTH(pTokenHead->m_nDataLength);

		pTokenHead=(FreeLongPopBufferTokenHead*)pTokenBegin;
		pTokenData=FREE_LONG_BUFFER_TOKEN_DATA_BEGIN(pTokenBegin);
	}

}


int CFreeLongPopBuffer::GetTokenCount()
{
	if(!ICanWork())
	{
		return 0;
	}

	return m_pHead->m_nTokenCount;
}


int CFreeLongPopBuffer::GetAllBytes()
{
	if(!ICanWork())
	{
		return 0;
	}

	return m_pHead->m_nAllBytesCount;
	
}

bool CFreeLongPopBuffer::ICanSave(int nDataLength)
{
	int nLeaveBytes=0;
	if(!ICanWork())
	{
		return false;
	}

	nLeaveBytes=m_nBufferSize-m_pHead->m_nAllBytesCount;
	if(FREE_LONG_POP_BUFFER_TOKEN_LENGTH(nDataLength)>(ULONG)nLeaveBytes)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int CFreeLongPopBuffer::AddLast(char* szData,int nDataLength)
{
	int nRet=0;
	if(!szData)
	{
		goto CFreeLongPopBuffer_AddLast_End_Process;
	}

	if(0>=nDataLength)
	{
		goto CFreeLongPopBuffer_AddLast_End_Process;
	}

	if(!ICanWork())
	{
		goto CFreeLongPopBuffer_AddLast_End_Process;
	}

	if(!ICanSave(nDataLength))
	{
		goto CFreeLongPopBuffer_AddLast_End_Process;
	}

	{
		char* pTokenBegin=m_pBuffer+m_pHead->m_nAllBytesCount;
		FreeLongPopBufferTokenHead* pTokenHead=(FreeLongPopBufferTokenHead*)pTokenBegin;

		char* pTokenData=FREE_LONG_BUFFER_TOKEN_DATA_BEGIN(pTokenBegin);

		//具体的增加操作
		pTokenHead->m_nDataLength=nDataLength;
		memcpy(pTokenData,szData,nDataLength);
		m_pHead->m_nTokenCount++;
		m_pHead->m_nAllBytesCount+=FREE_LONG_POP_BUFFER_TOKEN_LENGTH(nDataLength);

		nRet=nDataLength;
	}


CFreeLongPopBuffer_AddLast_End_Process:
	return nRet;
}

int CFreeLongPopBuffer::AddLast(CFreeLongBuffer* pBuffer)
{
	if(!pBuffer)
	{
		return 0;
	}

	return AddLast(pBuffer->m_pData,pBuffer->m_nDataLength);
}

int CFreeLongPopBuffer::GetFirstTokenLength()
{
	if(!ICanWork())
	{
		return 0;
	}

	char* pFirstTokenBebin=FREE_LONG_POP_BUFFER_FIRST_TOKEN_BEGIN(m_pBuffer);

	FreeLongPopBufferTokenHead* pFirstTokenHead=(FreeLongPopBufferTokenHead*)pFirstTokenBebin;
	return pFirstTokenHead->m_nDataLength;
}

int CFreeLongPopBuffer::GetFirst(char* szBuffer,int nBufferSize)
{
	int nRet=0;
	if(!ICanWork())
	{
		goto CFreeLongPopBuffer_GetFirst_End_Process;
	}

	if(!GetTokenCount())
	{
		goto CFreeLongPopBuffer_GetFirst_End_Process;
	}

	if(GetFirstTokenLength()>nBufferSize)
	{
		goto CFreeLongPopBuffer_GetFirst_End_Process;
	}

	{
		char* pFirstTokenBegin=FREE_LONG_POP_BUFFER_FIRST_TOKEN_BEGIN(m_pBuffer);
		FreeLongPopBufferTokenHead* pFirstTokenHead=(FreeLongPopBufferTokenHead*)pFirstTokenBegin;
		char* pFirstTokenData=FREE_LONG_BUFFER_TOKEN_DATA_BEGIN(pFirstTokenBegin);

		memcpy(szBuffer,pFirstTokenData,pFirstTokenHead->m_nDataLength);
		nRet=pFirstTokenHead->m_nDataLength;
	}

CFreeLongPopBuffer_GetFirst_End_Process:
	return nRet;
}

int CFreeLongPopBuffer::GetFirst(CFreeLongBuffer* pBuffer)
{
	if(!ICanWork())
	{
		return 0;
	}

	if(!pBuffer->SetSize(GetFirstTokenLength()))
	{
		return 0;
	}

	if(!pBuffer->m_nDataLength)
	{
		return 0;
	}

	return GetFirst(pBuffer->m_pData,pBuffer->m_nDataLength);
	
}

bool CFreeLongPopBuffer::DeleteFirst()
{
	bool bRet=false;

	if(!ICanWork())
	{
		goto CFreeLongPopBuffer_DeleteFirst_End_Process;
	}

	if(!GetTokenCount())
	{
		goto CFreeLongPopBuffer_DeleteFirst_End_Process;
	}

	{
		char* pFirstTokenBegin=FREE_LONG_POP_BUFFER_FIRST_TOKEN_BEGIN(m_pBuffer);
		FreeLongPopBufferTokenHead* pFirstTokenHead=(FreeLongPopBufferTokenHead*)pFirstTokenBegin;

		int nFirstTokenLength=FREE_LONG_POP_BUFFER_TOKEN_LENGTH(pFirstTokenHead->m_nDataLength);
		

		char* pSecondTokenBegin=pFirstTokenBegin+nFirstTokenLength;
		int nCopyBytesCount=m_pHead->m_nAllBytesCount-FreeLongPopBufferHeadSize-nFirstTokenLength;

		memcpy(pFirstTokenBegin,pSecondTokenBegin,nCopyBytesCount);

		m_pHead->m_nAllBytesCount-=nFirstTokenLength;
		m_pHead->m_nTokenCount--;
		bRet=true;
	}

CFreeLongPopBuffer_DeleteFirst_End_Process:
	return bRet;
}

int CFreeLongPopBuffer::GetAndDeleteFirst(char* szBuffer,int nBufferSize)
{
	if(!ICanWork())
	{
		return 0;
	}

	int nRet=GetFirst(szBuffer,nBufferSize);
	DeleteFirst();
	return nRet;
}

int CFreeLongPopBuffer::GetAndDeleteFirst(CFreeLongBuffer* pBuffer)
{
	if(!ICanWork())
	{
		return 0;
	}
	int nRet=GetFirst(pBuffer);
	DeleteFirst();
	return nRet;
	
}

int CFreeLongPopBuffer::MoveAllData(_FREELONG_ENUM_DATA_CALLBACK pCallBack,PVOID pCallParam)
{
	int nMovedTokenCount=0;
	bool bCallBackRet=true;

	if(!pCallBack)
	{
		goto CFreeLongPopBuffer_MoveAllData_End_Process;
	}

	if(!ICanWork())
	{
		goto CFreeLongPopBuffer_MoveAllData_End_Process;
	}

	while (m_pHead->m_nTokenCount) 
	{
		char* pFirstTokenBegin=FREE_LONG_POP_BUFFER_FIRST_TOKEN_BEGIN(m_pBuffer);
		FreeLongPopBufferTokenHead* pFirstTokenHead=(FreeLongPopBufferTokenHead*)pFirstTokenBegin;
		
		char* pFirstTokenData=FREE_LONG_BUFFER_TOKEN_DATA_BEGIN(pFirstTokenBegin);

		bCallBackRet=pCallBack(pFirstTokenData,pFirstTokenHead->m_nDataLength,pCallParam);

		DeleteFirst();

		nMovedTokenCount++;

		if(!bCallBackRet)
		{
			break;
		}
		
	}

CFreeLongPopBuffer_MoveAllData_End_Process:
	return nMovedTokenCount;
}