// FreeLongMemoryPoolWithLock.h: interface for the CFreeLongMemoryPoolWithLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_MEMORY_POOL_WITH_LOCK_
#define _FREELONG_MEMORY_POOL_WITH_LOCK_
#include "CFreeLong_LowDebug.h"
#include "MemoryRegister.h"
#include "sockinclude.h"
#include "SocketRegister.h"
#include "FreeLongMemoryStack.h"

class CFreeLongMemoryPoolWithLock  
{
public:
	CFreeLongMemoryPoolWithLock(CCFreeLong_LowDebug* pDebug,bool bOpenRegisterFlag=true);
	virtual ~CFreeLongMemoryPoolWithLock();
public:
	void Register(void* pPoint,char* szInfo);
	void UnRegister(void* pPoint);

private:
	CMemoryRegister* m_pRegister;

public:
	void RegisterSocket(Linux_Win_SOCKET s,char* szInfo);
	void CloseSocket(Linux_Win_SOCKET& s);
private:
	CSocketRegister* m_pSocketRegister;	

public:
	void SetCloseFlag(bool bFlag=true);
	void* Remalloc(void* pPoint,int nNewSize,bool bCopyOldDataFlag=true);
	void* Malloc(int nSize,char* szInfo=NULL);

	void Free(void* pBlock);
	void PrintTree();
	void PrintInfo();

	CFreeLongMemoryStack* m_pMemPool;
	CCFreeLong_LowDebug*  m_pDebug;
};

#endif 
