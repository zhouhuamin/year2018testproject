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

#define FREELONG_MEM_BLOCK_SIZE(nDataLength) (nDataLength+SFreeLongMemoryBlockHeadSize)    //计算真实的大小，要加上数据头的大小,n+8

#define FREELONG_MEM_BLOCK_DATA_SIZE(nDataLength) (nDataLength-SFreeLongMemoryBlockHeadSize)    //计算真实的大小，n-8
//根据应用程序释放的指针，逆求真实的内存指针，即p0=p1-8
#define FREELONG_MEM_BLOCK_HEAD(pData) ((SFreeLongMemoryBlockHead*)(((char*)pData)-SFreeLongMemoryBlockHeadSize))    

//数据块的真实指针p1=p0+8
#define FREELONG_MEM_BLOCK_DATA(pHead) (((char*)pHead)+SFreeLongMemoryBlockHeadSize)   

//最小内存块大小16 BYTES
#define FREELONG_MEMORY_STACK_BLOCK_MIN   16

//最大内存块大小1M，超过大小的，直接申请和释放
#define FREELONG_MEMORY_STACK_MAX_SAVE_BLOCK_SIZE   (1*1024*1024)


#define FREELONG_DEBUG m_pDebug->Debug2File
#define FREELONG_DEBUG_BIN m_pDebug->Debug2File4Bin


class CFreeLongMemoryStackToken  
{
public:
	CFreeLongMemoryStackToken(int nBlockSize,CCFreeLong_LowDebug* pDebug);
	virtual ~CFreeLongMemoryStackToken();
public:
	void* Malloc(int nSize,                                    //申请的内存大小
				CMRSWInt& nAllBlockCount,                      //统计变量，返回管理过的内存块总数
				CMRSWInt& nMemoryUse);                         //统计变量，返回使用的内存块数量

	bool Free(void* pPoint,bool bCloseFlag);
	void PrintStack();

private:
	void DestroySon(SFreeLongMemoryBlockHead* pSon);           //系统退出时，递归销毁所有内存块

private:
	SFreeLongMemoryBlockHead* m_pFirstSon;                     //第一个儿子指针，就是表头指针
	CFreeLongMemoryStackToken* m_pNext;                        //指向兄弟节点，即左枝下一节点的指针
	CMultiReadSingleWriteLock m_Lock;                          //单写多读锁，保护上面两个变量

	ULONG m_ulBlockSize;
	CMRSWInt  m_nBlockOutSide;                    //分配出去的内存块数量
	CMRSWInt  m_nBlockInSide;                     //内部保留的空闲内存块数量
	CCFreeLong_LowDebug*   m_pDebug;
	
};

#endif 
