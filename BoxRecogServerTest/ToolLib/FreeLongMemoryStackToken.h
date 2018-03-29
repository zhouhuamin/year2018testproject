// FreeLongMemoryStackToken.h: interface for the CFreeLongMemoryStackToken class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_MEMORY_STACK_TOKEN_H_
#define _FREELONG_MEMORY_STACK_TOKEN_H_


#include "CFreeLong_LowDebug.h"
#include "MRSWInt.h"

typedef struct _FREE_LONG_MEM_BLOCK_HEAD_
{
	ULONG   m_ulBlockSize;
	struct  _FREE_LONG_MEM_BLOCK_HEAD_*  m_pNext;
}SFreeLongMemoryBlockHead;

const ULONG SFreeLongMemoryBlockHeadSize=sizeof(SFreeLongMemoryBlockHead);

#define FREELONG_MEM_BLOCK_SIZE(nDataLength) (nDataLength+SFreeLongMemoryBlockHeadSize)    //������ʵ�Ĵ�С��Ҫ��������ͷ�Ĵ�С,n+8

#define FREELONG_MEM_BLOCK_DATA_SIZE(nDataLength) (nDataLength-SFreeLongMemoryBlockHeadSize)    //������ʵ�Ĵ�С��n-8
//����Ӧ�ó����ͷŵ�ָ�룬������ʵ���ڴ�ָ�룬��p0=p1-8
#define FREELONG_MEM_BLOCK_HEAD(pData) ((SFreeLongMemoryBlockHead*)(((char*)pData)-SFreeLongMemoryBlockHeadSize))    

//���ݿ����ʵָ��p1=p0+8
#define FREELONG_MEM_BLOCK_DATA(pHead) (((char*)pHead)+SFreeLongMemoryBlockHeadSize)   

//��С�ڴ���С16 BYTES
#define FREELONG_MEMORY_STACK_BLOCK_MIN   16

//����ڴ���С1M��������С�ģ�ֱ��������ͷ�
#define FREELONG_MEMORY_STACK_MAX_SAVE_BLOCK_SIZE   (1*1024*1024)


#define FREELONG_DEBUG m_pDebug->Debug2File
#define FREELONG_DEBUG_BIN m_pDebug->Debug2File4Bin


class CFreeLongMemoryStackToken  
{
public:
	CFreeLongMemoryStackToken(int nBlockSize,CCFreeLong_LowDebug* pDebug);
	virtual ~CFreeLongMemoryStackToken();
public:
	void* Malloc(int nSize,                                    //������ڴ��С
				CMRSWInt& nAllBlockCount,                      //ͳ�Ʊ��������ع�������ڴ������
				CMRSWInt& nMemoryUse);                         //ͳ�Ʊ���������ʹ�õ��ڴ������

	bool Free(void* pPoint,bool bCloseFlag);
	void PrintStack();

private:
	void DestroySon(SFreeLongMemoryBlockHead* pSon);           //ϵͳ�˳�ʱ���ݹ����������ڴ��

private:
	SFreeLongMemoryBlockHead* m_pFirstSon;                     //��һ������ָ�룬���Ǳ�ͷָ��
	CFreeLongMemoryStackToken* m_pNext;                        //ָ���ֵܽڵ㣬����֦��һ�ڵ��ָ��
	CMultiReadSingleWriteLock m_Lock;                          //��д�����������������������

	ULONG m_ulBlockSize;
	CMRSWInt  m_nBlockOutSide;                    //�����ȥ���ڴ������
	CMRSWInt  m_nBlockInSide;                     //�ڲ������Ŀ����ڴ������
	CCFreeLong_LowDebug*   m_pDebug;
	
};

#endif 
