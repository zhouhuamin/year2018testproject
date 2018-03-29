// FreeLongStaticBuffer.cpp: implementation of the CFreeLongStaticBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongStaticBuffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define FREELONG_BUFFER_STRING_MAX 1024

#ifdef WIN32
#define Linux_Win_vsnprintf _vsnprintf
#else
#define Linux_Win_vsnprintf vsnprintf
#endif


CFreeLongStaticBuffer::CFreeLongStaticBuffer(CFreeLongMemoryPoolWithLock* pMemPool)
{
	m_pMemPool=pMemPool;                       //记录内存池对象指针
	m_nDataLength=0;                           //初始化数据长度为0
}

CFreeLongStaticBuffer::~CFreeLongStaticBuffer()
{

}

bool CFreeLongStaticBuffer::IHaveData()
{
	if(0>=m_nDataLength)
	{
		return false;
	}

	return true;
}

/*
 *	因为是静态分配的内存，不再变化，所以这里仅仅是检查大小
 */
bool CFreeLongStaticBuffer::SetSize(int nSize)
{
	if(FREELONG_SAFE_BUFFER_MAX_SIZE < nSize)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::SetSize():Error!nSize=%d\n",nSize);
		return false;
	}

	m_nDataLength=nSize;       //这里其实是把m_nDataLength作为缓冲大小提示
	
	return true;
}


int CFreeLongStaticBuffer::InsertData2Head(char* szData,int nDataLength)
{
	if(!InsertSpace2Head(nDataLength))
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::InsertData2Head():Error!m_nDataLength=%d,\
			nDataLength=%d,too big!\n",m_nDataLength,nDataLength);
		return 0;	
	}
	
	memcpy(m_pData,szData,nDataLength);
	return m_nDataLength;
}


bool CFreeLongStaticBuffer::InsertSpace2Head(int nAddBytes)
{

	if(0>=m_nDataLength)
	{
		//如果没有原始数据，则设置缓冲大小
		m_nDataLength=nAddBytes;
		return true;
	}

	if(nAddBytes > FREELONG_SAFE_BUFFER_MAX_SIZE - m_nDataLength)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::InsertSpace2Head():Error!nAddBytes=%d,m_nDataLength=%d,too big!\n",
			nAddBytes,m_nDataLength);
		return false;
	}

	{
		//这里限定变量szBuffer的范围
		char szBuffer[FREELONG_SAFE_BUFFER_MAX_SIZE];
		memset(szBuffer,0,FREELONG_SAFE_BUFFER_MAX_SIZE);
		//第一次，把原有数据拷贝到新缓冲区，已经偏移了nAddBytes
		memcpy(szBuffer+nAddBytes,m_pData,m_nDataLength);
		m_nDataLength+=nAddBytes;
		memcpy(m_pData,szBuffer,m_nDataLength);

		//先将数据拷贝到中间缓冲，这样做是为了保险起见
	}

	
	return true;
}


bool CFreeLongStaticBuffer::AddSpace2Tail(int nAddBytes)
{
	if(0>=m_nDataLength)
	{
		m_nDataLength=nAddBytes;
		return true;

	}

	if(nAddBytes>(FREELONG_SAFE_BUFFER_MAX_SIZE-m_nDataLength))
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::AddSpace2Tail():Error!nAddBytes=%d,m_nDataLength=%d,too big!\n",
			nAddBytes,m_nDataLength);
		return false;
	}

	m_nDataLength+=nAddBytes;

	return true;
}

void CFreeLongStaticBuffer::CutHead(int nBytes)
{
	if(0>=m_nDataLength)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::CutHead():Error!m_nDataLength=%d,too small!\n",m_nDataLength);
		return;
	}

	if(nBytes>m_nDataLength)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::CutHead():Error!m_nDataLength=%d,\
			nBytes=%d,too small!\n",m_nDataLength,nBytes);
		m_nDataLength=0;
		return;
	}

	m_nDataLength=nBytes;
	memcpy(m_pData,m_pData+nBytes,m_nDataLength);

	return;
}



void CFreeLongStaticBuffer::CutTail(int nBytes)
{
	if(0>=m_nDataLength)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::CutTail():Error!m_nDataLength=%d,too small!\n",m_nDataLength);
		m_nDataLength=0;
		return;
	}


	if(nBytes>m_nDataLength)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::CutTail():Error!m_nDataLength=%d,\
			nBytes=%d,too small!\n",m_nDataLength,nBytes);
		m_nDataLength=0;
		return;
	}
	
	m_nDataLength=nBytes;
	return;
}

int CFreeLongStaticBuffer::BinCopyTo(char* szBuffer,int nBufferSize)
{
	if(m_nDataLength>nBufferSize)
	{

		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::BinCopyTo():Error!nBufferSize=%d\n",nBufferSize);
		return 0;
	}


	if(!szBuffer)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::BinCopyTo():Error!szBuffer=null\n");
		return 0;
	}

	
	memcpy(szBuffer,m_pData,m_nDataLength);
	return m_nDataLength;
}


int CFreeLongStaticBuffer::BinCopyFrom(char* szData,int nDataLength)
{

	if(FREELONG_SAFE_BUFFER_MAX_SIZE<nDataLength)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::BinCopyFrom():Error! \
			nDataLength=%d,too big!\n",nDataLength);
		return 0;
	}

	if(0>=nDataLength)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::BinCopyFrom():Error! \
			nDataLength=%d,too small!\n",nDataLength);
		return 0;
	}

	if(!szData)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::BinCopyFrom():Error! \
			szData=null!\n");
		return 0;
	}


	memcpy(m_pData,szData,nDataLength);
	m_nDataLength=nDataLength;
	return m_nDataLength;
}

int CFreeLongStaticBuffer::BinCopyFrom(CFreeLongStaticBuffer* pBuffer)
{
	return BinCopyFrom(pBuffer->m_pData,pBuffer->m_nDataLength);
}

bool CFreeLongStaticBuffer::SetInt(int n)
{
	int nSave=htonl(n);
	return BinCopyFrom((char*)&nSave,sizeof(int));
}

int CFreeLongStaticBuffer::GetInt()
{
	if(!m_pData)
	{
		return 0;
	}

	int* pData=(int*)m_pData;
	int nRet=*pData;
	return ntohl(nRet); 
}


bool CFreeLongStaticBuffer::SetShort(short n)
{
	short sSave=htons(n);
	return BinCopyFrom((char*)&sSave,sizeof(short));
}

short CFreeLongStaticBuffer::GetShort()
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


bool CFreeLongStaticBuffer::SetChar(char n)
{
	return BinCopyFrom(&n,sizeof(char));
}

char CFreeLongStaticBuffer::GetChar()
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

int CFreeLongStaticBuffer::AddData(char* szData,int nDataLength)
{

	int nNewSize=m_nDataLength+nDataLength;
	if(FREELONG_SAFE_BUFFER_MAX_SIZE<nNewSize)
	{
		m_pMemPool->m_pDebug->Debug2File("CFreeLongStaticBuffer::BinCopyFrom():Error! \
			m_nDataLength=%d,nDataLength=%d,too big!\n",m_nDataLength,nDataLength);

		return 0;
	}
	
	//拷贝新数据到开始
	memcpy(m_pData+m_nDataLength,szData,nDataLength);
	m_nDataLength=nNewSize;
	return m_nDataLength;
}


int CFreeLongStaticBuffer::StrCopyFrom(char* szString)
{
	int nDataLength=strlen(szString)+1;
	return BinCopyFrom(szString,nDataLength);
}

int CFreeLongStaticBuffer::Printf(char* szFormat,...)
{
	char szBuf[FREELONG_SAFE_BUFFER_MAX_SIZE];

	int nListCount=0;
	va_list pArgList;

	va_start(pArgList,szFormat);
	nListCount+=Linux_Win_vsnprintf(szBuf+nListCount,FREELONG_SAFE_BUFFER_MAX_SIZE-nListCount,szFormat,pArgList);
	va_end(pArgList);
	
	//进行缓冲区越界检测
	if(nListCount>(FREELONG_SAFE_BUFFER_MAX_SIZE-1))
	{
		nListCount=FREELONG_SAFE_BUFFER_MAX_SIZE-1;
	}
	
	*(szBuf+nListCount)='\0';
				
	return StrCopyFrom(szBuf);
}


int CFreeLongStaticBuffer::memcmp(char* szData,int nDataLength)
{
	

	if(0>=m_nDataLength)
	{
		return -1;
	}

	if(!szData)
	{
		return -1;
	}

	if(m_nDataLength!=nDataLength)
	{
		return -1;
	}

	int nRet=::memcmp(m_pData,szData,nDataLength);
	return nRet;
}

int CFreeLongStaticBuffer::strcmp(char* szString)
{

	
	if(0>=m_nDataLength)
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
