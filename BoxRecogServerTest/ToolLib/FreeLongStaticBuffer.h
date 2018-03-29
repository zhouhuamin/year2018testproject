// FreeLongStaticBuffer.h: interface for the CFreeLongStaticBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_STATIC_BUFFER_H_
#define _FREELONG_STATIC_BUFFER_H_

#include "FreeLongMemoryPoolWithLock.h"
#define  FREELONG_SAFE_BUFFER_MAX_SIZE (132*1024)
class CFreeLongStaticBuffer  
{
public:
	CFreeLongStaticBuffer(CFreeLongMemoryPoolWithLock* pMemPool);
	virtual ~CFreeLongStaticBuffer();
	
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
	int BinCopyFrom(CFreeLongStaticBuffer* pBuffer);
	
	//�ı����ݿ���
	int StrCopyFrom(char* szString);
	
	int Printf(char* szFormat,...);
	
	
	int memcmp(char* szData,int nDataLength);
	int strcmp(char* szString);

	bool IHaveData();
};

#endif 