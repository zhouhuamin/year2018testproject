// FreeLongBuffer.h: interface for the CFreeLongBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREElONG_BUFFER_H_
#define _FREElONG_BUFFER_H_

#include "FreeLongMemoryPoolWithLock.h"

class CFreeLongBuffer  
{
public:
	CFreeLongBuffer(CFreeLongMemoryPoolWithLock* pMemPool);
	~CFreeLongBuffer();

public: //典型的工具类，所有的内部变量全部公有，方便应用层调用
	CFreeLongMemoryPoolWithLock* m_pMemPool;
	char* m_pData;
	int   m_nDataLength;

public:
	//----------------------------------------------------
	//尺寸设置
	bool SetSize(int nSize);
	bool InsertSpace2Head(int nAddBytes);
	bool AddSpace2Tail(int nAddBytes);
	void CutHead(int nBytes);
	void CutTail(int nBytes);

	//-----------------------------------------------------
	//数值转换
	bool SetInt(int n);   //将整数以二进制方式拷贝到缓冲区，带网络字节顺序
	int  GetInt();

	bool SetShort(short n);
	short GetShort();

	bool SetChar(char n);
	char GetChar();


	//---------------------------------------------------- 
	//追加二进制数据到最后，返回新的数据长度
	int AddData(char* szData,int nDataLength);

	//插入数据到最前面，返回新的数据长度
	int InsertData2Head(char* szData,int nDataLength);


	//---------------------------------
	//拷贝到一块目标缓冲区
	int BinCopyTo(char* szBuffer,int nBufferSize);

	//从一块来源缓冲区拷贝数据到本对象中
	int BinCopyFrom(char* szData,int nDataLength);

	//从另外一个Buffer对象拷贝数据到本对象
	int BinCopyFrom(CFreeLongBuffer* pBuffer);

	//文本数据拷贝
	int StrCopyFrom(char* szString);

	int Printf(char* szFormat,...);


	int memcmp(char* szData,int nDataLength);
	int strcmp(char* szString);
	
};

#endif 
