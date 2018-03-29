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
	void* ReMalloc(void* pPoint,                                //�ɵ�ָ��
		           int nNewSize,                                //�µĳߴ�Ҫ��
				   bool bCopyOldDataFlag=true);                 //�Ƿ񱸷ݾ�����

	void* Malloc(int nSize);
	bool  Free(void* pPoint);

	//�ڲ���Ϣ���
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
