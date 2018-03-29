// FreeLongPopBuffer.h: interface for the CFreeLongPopBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_POP_BUFFER_H_
#define _FREELONG_POP_BUFFER_H_
#include "FreeLongBuffer.h"

typedef struct _FREELONG_POP_BUFFER_HEAD_
{
	int m_nTokenCount;
	int m_nAllBytesCount;
}FreeLongPopBufferHead;

const ULONG FreeLongPopBufferHeadSize=sizeof(FreeLongPopBufferHead);

#define  FREE_LONG_BUFFER_TOKEN_DATA_BEGIN(p) (((char*)p)+FreeLongPopBufferHeadSize)

typedef struct _FREE_LONG_POP_BUFFER_TOKEN_HEAD_ 
{
	int m_nDataLength;
}FreeLongPopBufferTokenHead;

const ULONG FreeLongPopBufferTokenHeadSize=sizeof(FreeLongPopBufferTokenHead);
#define FREE_LONG_POP_BUFFER_TOKEN_LENGTH(n) (n+FreeLongPopBufferTokenHeadSize)

#define FREE_LONG_POP_BUFFER_FIRST_TOKEN_BEGIN(p) (((char*)p)+FreeLongPopBufferTokenHeadSize)

typedef bool (*_FREELONG_ENUM_DATA_CALLBACK)(char* szData,
												int nDataLenght,
												void* pCallparam);

//static bool EnumDataCallback(char* szData,int nDataLenght,void* pCallparam);
class CFreeLongPopBuffer  
{
public:
	CFreeLongPopBuffer(char* szBuffer,int nBufferSize,bool bInitFlag=true);
	~CFreeLongPopBuffer();

public:
	void Set(char* szBuffer,int nBufferSize);

	void Clean();

	void PrintInside();

	bool ICanWork();

public:
	int AddLast(char* szData,int nDataLength);
	int AddLast(CFreeLongBuffer* pBuffer);

public:
	int GetTokenCount();                              //获得当前内部元素个数
	int GetAllBytes();                                //获得所有字节数
	bool ICanSave(int nDataLength);                   //判断内存空间是否够用

public:
	int GetFirstTokenLength();

	int GetFirst(char* szBuffer,int nBufferSize);
	int GetFirst(CFreeLongBuffer* pBuffer);

	bool DeleteFirst();
	int GetAndDeleteFirst(char* szBuffer,int nBufferSize);
	int GetAndDeleteFirst(CFreeLongBuffer* pBuffer);

public:
	int MoveAllData(_FREELONG_ENUM_DATA_CALLBACK pCallBack,PVOID pCallParam);
public:
	char* m_pBuffer;
	int m_nBufferSize;

private:
	//这个是队列头的指针，注意，并不是动态申请的内存块，而是指向缓冲区m_pBuffer头
	FreeLongPopBufferHead* m_pHead;
};

#endif 
