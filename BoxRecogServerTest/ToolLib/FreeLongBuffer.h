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

public: //���͵Ĺ����࣬���е��ڲ�����ȫ�����У�����Ӧ�ò����
	CFreeLongMemoryPoolWithLock* m_pMemPool;
	char* m_pData;
	int   m_nDataLength;

public:
	//----------------------------------------------------
	//�ߴ�����
	bool SetSize(int nSize);
	bool InsertSpace2Head(int nAddBytes);
	bool AddSpace2Tail(int nAddBytes);
	void CutHead(int nBytes);
	void CutTail(int nBytes);

	//-----------------------------------------------------
	//��ֵת��
	bool SetInt(int n);   //�������Զ����Ʒ�ʽ���������������������ֽ�˳��
	int  GetInt();

	bool SetShort(short n);
	short GetShort();

	bool SetChar(char n);
	char GetChar();


	//---------------------------------------------------- 
	//׷�Ӷ��������ݵ���󣬷����µ����ݳ���
	int AddData(char* szData,int nDataLength);

	//�������ݵ���ǰ�棬�����µ����ݳ���
	int InsertData2Head(char* szData,int nDataLength);


	//---------------------------------
	//������һ��Ŀ�껺����
	int BinCopyTo(char* szBuffer,int nBufferSize);

	//��һ����Դ�������������ݵ���������
	int BinCopyFrom(char* szData,int nDataLength);

	//������һ��Buffer���󿽱����ݵ�������
	int BinCopyFrom(CFreeLongBuffer* pBuffer);

	//�ı����ݿ���
	int StrCopyFrom(char* szString);

	int Printf(char* szFormat,...);


	int memcmp(char* szData,int nDataLength);
	int strcmp(char* szString);
	
};

#endif 
