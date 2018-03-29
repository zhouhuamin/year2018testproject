// MutexLock.h: interface for the CMutexLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _MUTEX_LOCK_H_
#define _MUTEX_LOCK_H_
#include "SysInclude.h"
#ifdef WIN32                                            //windows�£���C�������ķ���
	#define LKMUTEX CRITICAL_SECTION                      //�����ٽ���ʵ�ֵ�������
    #define MUTEXINIT(m)    InitializeCriticalSection(m)
    #define MUTEXLOCK(m)    EnterCriticalSection(m)
	#define MUTEXUNLOCK(m)  LeaveCriticalSection(m)
	#define MUTEXDESTROY(m) DeleteCriticalSection(m)
#else
	#define LKMUTEX pthread_mutex_t                       //linux�¶���    
	#define MUTEXINIT(m)    pthread_mutex_init(m,NULL)    //check error
	#define MUTEXLOCK(m)    pthread_mutex_lock(m)
	#define MUTEXUNLOCK(m)  pthread_mutex_unlock(m)
	#define MUTEXDESTROY(m) pthread_mutex_destroy(m)
#endif


class CMutexLock  
{
public:
	CMutexLock()
	{
		MUTEXINIT(&m_Lock);
	}
	virtual ~CMutexLock()
	{
		MUTEXDESTROY(&m_Lock);
	}

public:
	void Lock()
	{
		MUTEXLOCK(&m_Lock);
	}

	void Unlock()
	{
		MUTEXUNLOCK(&m_Lock);
	}

private:
	LKMUTEX m_Lock;
};

#endif 
