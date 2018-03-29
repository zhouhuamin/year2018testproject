// FreeLongBuffer.cpp: implementation of the CFreeLongBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongBuffer.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define FREELONG_BUFFER_STRING_MAX 1024

#ifdef WIN32
#define Linux_Win_vsnprintf _vsnprintf
#else
#define Linux_Win_vsnprintf vsnprintf
#endif


CFreeLongBuffer::CFreeLongBuffer(CFreeLongMemoryPoolWithLock* pMemPool)
{
	m_pMemPool=pMemPool;
	m_pData=NULL;
	m_nDataLength=0;
}

CFreeLongBuffer::~CFreeLongBuffer()
{
	SetSize(0);

}

bool CFreeLongBuffer::SetSize(int nSize)
{

	if(!m_pMemPool)
	{
		//内存池指针不存在，直接返回
		return false;
	}

	m_nDataLength=nSize;
	if(m_nDataLength==0)
	{
		if(m_pData)
		{
			m_pMemPool->Free(m_pData);              //释放
			m_pData=NULL;
		}

		return true;                                 //操作成功，返回
	}

	//重新设置

	if(!m_pData)
	{
		m_pData=(char*)m_pMemPool->Malloc(m_nDataLength,"CFreeLongBuffer::SetSize():m_pData");
		if(!m_pData)
		{
			m_nDataLength=0;                  //把长度设置为0
			return false;                     //返回false
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pData=(char*)m_pMemPool->Remalloc(m_pData,m_nDataLength);
		if(!m_pData)
		{
			m_nDataLength=0;                  //把长度设置为0
			return false;                     //返回false
		}
		else
		{
			return true;
		}

	}
	
}

int CFreeLongBuffer::InsertData2Head(char* szData,int nDataLength)
{
	if(InsertSpace2Head(nDataLength))
	{
		memcpy(m_pData,szData,nDataLength);
		return m_nDataLength;
	}

	return m_nDataLength;
}

bool CFreeLongBuffer::InsertSpace2Head(int nAddBytes)
{
	bool bRet=false;
	int nNewSize=0;
	char* pBuffer=NULL;

	if(!m_pMemPool)
	{
		goto CFreeLongBuffer_InsertSpace2Head_End_Process;
	}

	nNewSize=m_nDataLength+nAddBytes;
	pBuffer=(char*)m_pMemPool->Malloc(nNewSize,"CFreeLongBuffer::InsertSpace2Head():pBuffer");

	if(!pBuffer)
	{
		goto CFreeLongBuffer_InsertSpace2Head_End_Process;
	}

	//下面为防御性检测
	if((m_pData) && (m_nDataLength))
	{
		memcpy(pBuffer+nAddBytes,m_pData,m_nDataLength);
	}

	bRet=BinCopyFrom(pBuffer,nNewSize);

CFreeLongBuffer_InsertSpace2Head_End_Process:
	if(pBuffer)
	{
		m_pMemPool->Free(pBuffer);
		pBuffer=NULL;
	}

	return bRet;
}


bool CFreeLongBuffer::AddSpace2Tail(int nAddBytes)
{
	return SetSize(m_nDataLength+nAddBytes);
}

void CFreeLongBuffer::CutHead(int nBytes)
{
	if(m_nDataLength<=nBytes)
	{
		SetSize(0);
	}
	else
	{
		//从后向前MOVE
		memcpy(m_pData,m_pData+nBytes,m_nDataLength-nBytes);

		//由于内存池的原因，申请一个比较小的空间，一般直接返回原来的指针
		m_nDataLength-=nBytes;
	}
}

void CFreeLongBuffer::CutTail(int nBytes)
{
	if(m_nDataLength<=nBytes)
	{
		SetSize(0);
	}
	else
	{	
		//由于内存池的原因，申请一个比较小的空间，一般直接返回原来的指针
		m_nDataLength-=nBytes;
	}
}

int CFreeLongBuffer::BinCopyTo(char* szBuffer,int nBufferSize)
{
	if(!m_pData)
	{
		return 0;
	}

	if(!m_nDataLength)
	{
		return 0;
	}

	if(nBufferSize < m_nDataLength)
	{
		return 0;
	}

	memcpy(szBuffer,m_pData,m_nDataLength);
	return m_nDataLength;
}


int CFreeLongBuffer::BinCopyFrom(char* szData,int nDataLength)
{
	if((!szData) || (0>=nDataLength))
	{
		SetSize(0);
		return 0;
	}

	if(!SetSize(nDataLength))
	{
		return 0;
	}
	memcpy(m_pData,szData,nDataLength);
	return nDataLength;
}

int CFreeLongBuffer::BinCopyFrom(CFreeLongBuffer* pBuffer)
{
	return BinCopyFrom(pBuffer->m_pData,pBuffer->m_nDataLength);
}

bool CFreeLongBuffer::SetInt(int n)
{
	int nSave=htonl(n);
	return BinCopyFrom((char*)&nSave,sizeof(int));
}

int CFreeLongBuffer::GetInt()
{
	if(!m_pData)
	{
		return 0;
	}

	if(!sizeof(int)>m_nDataLength)
	{
		return 0;
	}

	return ntohl(*(int*)m_pData); 
}


bool CFreeLongBuffer::SetShort(short n)
{
	short sSave=htons(n);
	return BinCopyFrom((char*)&sSave,sizeof(short));
}

short CFreeLongBuffer::GetShort()
{
	if(!m_pData)
	{
		return 0;
	}
	
	if(!sizeof(short)>m_nDataLength)
	{
		return 0;
	}
	
	return ntohl(*(short*)m_pData); 
}


bool CFreeLongBuffer::SetChar(char n)
{
	return BinCopyFrom(&n,sizeof(char));
}

char CFreeLongBuffer::GetChar()
{
	if(!m_pData)
	{
		return 0;
	}
	
	if(!sizeof(char)>m_nDataLength)
	{
		return 0;
	}
	
	return *(char*)m_pData; 
}

int CFreeLongBuffer::AddData(char* szData,int nDataLength)
{
	if((!m_pData) || (0>=m_nDataLength))
	{
		return BinCopyFrom(szData,nDataLength);
	}

	if(!InsertSpace2Head(nDataLength))
	{
		return 0;
	}

	//拷贝新数据到开始
	memcpy(m_pData,szData,nDataLength);
	return m_nDataLength;
}


int CFreeLongBuffer::StrCopyFrom(char* szString)
{
	int n=strlen(szString);
	n++;
	return BinCopyFrom(szString,n);
}

int CFreeLongBuffer::Printf(char* szFormat,...)
{
	char szBuf[FREELONG_BUFFER_STRING_MAX];

	int nListCount=0;
	va_list pArgList;

	va_start(pArgList,szFormat);
	nListCount+=Linux_Win_vsnprintf(szBuf+nListCount,FREELONG_BUFFER_STRING_MAX-nListCount,szFormat,pArgList);
	va_end(pArgList);
	
	//进行缓冲区越界检测
	if(nListCount>(FREELONG_BUFFER_STRING_MAX-1))
	{
		nListCount=FREELONG_BUFFER_STRING_MAX-1;
	}
	
	*(szBuf+nListCount)='\0';
				
	return StrCopyFrom(szBuf);
}


int CFreeLongBuffer::memcmp(char* szData,int nDataLength)
{
	if(!m_pData)
	{
		return -1;
	}

	if(!m_nDataLength)
	{
		return -1;
	}

	if(!szData)
	{
		return -1;
	}

	if(m_nDataLength<nDataLength)
	{
		return -1;
	}

	int nRet=::memcmp(m_pData,szData,nDataLength);
	return nRet;
}

int CFreeLongBuffer::strcmp(char* szString)
{
	if(!m_pData)
	{
		return -1;
	}
	
	if(!m_nDataLength)
	{
		return -1;
	}
	
	if(!szString)
	{
		return -1;
	}

	int nRet=::strcmp(m_pData,szString);
	return nRet;
}