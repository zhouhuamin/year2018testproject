// MemoryRegister.h: interface for the CMemoryRegister class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _MEMORY_REGISTER_H_
#define _MEMORY_REGISTER_H_
#include "CFreeLong_LowDebug.h"


#ifndef FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE
	#define FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE 124
#endif

#ifndef FREELONG_MEMORY_REGISTER_MAX
	#define  FREELONG_MEMORY_REGISTER_MAX 10000
#endif
typedef struct _FREE_LONG_MEMORY_REGISTER_
{
	void* m_pPoint;
	char  m_szInfo[FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE];
}SFreeLongMemoryRegister;
const ULONG SFreeLongMemoryRegisterSize=sizeof(SFreeLongMemoryRegister);

#define FREELONG_CLEAN_CHAR_BUFFER(p) (*((char*)p)='\0')

class CMemoryRegister  
{
public:
	CMemoryRegister(CCFreeLong_LowDebug* pDebug);
	virtual ~CMemoryRegister();

public:
	//添加一个指针以及说明
	void Add(void* pPoint,char* szInfo);

	//删除一个指针（此内存块被释放）
	void Del(void* pPoint);

	//更新指针
	void Modeify(void* pOld,void* pNew);

	//打印信息函数
	void PrintInfo();

private:
	void RegisterCopy(SFreeLongMemoryRegister* pDest,void* pPoint,char* szInfo);

	void inline SafeStrcpy(char* pDest,char* pSource,int nCount)
	{
		int nLen=(int)strlen(pSource)+1;
		
		if(!pDest)
		{
			goto SafeStrcpy_END_PROCESS;
		}
		if(!pSource)
		{
			goto SafeStrcpy_END_PROCESS;
		}
		
		if(nLen > nCount)
		{
			nLen=nCount;
		}
		
		memcpy(pDest,pSource,nLen);
		*(pDest+nLen-1)='\0';
		
		
		
SafeStrcpy_END_PROCESS:
		return;
	}

private:
	CCFreeLong_LowDebug*              m_pDebug;
	CMutexLock                        m_Lock;
	SFreeLongMemoryRegister           m_RegisterArray[FREELONG_MEMORY_REGISTER_MAX];
	int                               m_nUseMax;
	void*                             m_pMaxPoint;
	int                               m_nPointCount;

};

#endif 
