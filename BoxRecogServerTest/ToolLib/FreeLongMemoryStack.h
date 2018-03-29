// FreeLongMemoryStack.h: interface for the CFreeLongMemoryStack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_MEMORY_STACK_H_
#define _FREELONG_MEMORY_STACK_H_

#include "FreeLongMemoryStackToken.h"

class CFreeLongMemoryStack  
{
public:
	CFreeLongMemoryStack(CCFreeLong_LowDebug* pDebug);
	virtual ~CFreeLongMemoryStack();
public:
	void* ReMalloc(void* pPoint,                                //旧的指针
		           int nNewSize,                                //新的尺寸要求
				   bool bCopyOldDataFlag=true);                 //是否备份旧数据

	void* Malloc(int nSize);
	bool  Free(void* pPoint);

	//内部信息输出
	void PrintStack();
	void PrintInfo();

	void SetCloseFlag(bool bFlag=true);
	CCFreeLong_LowDebug* m_pDebug;
private:
	CFreeLongMemoryStackToken* m_pHead;
	CMRSWInt m_pMaxPoint;
	CMRSWInt m_nAllBlockCount;
	CMRSWInt m_nMemoryUse;
	CMRSWBool m_CloseFlag;
};

#endif 
