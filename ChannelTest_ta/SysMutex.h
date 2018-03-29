// SysMutex.h: interface for the CSysMutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _SYS_MUTEX_H_
#define _SYS_MUTEX_H_

//ͨ�ð���
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

#ifdef WIN32                                //WIndows��
#include <conio.h>
#include <windows.h>
#include <process.h>
#include <winsock.h>
#else                                        //linux��
#include <pthread.h>
#include <sys/time.h>
#endif



#ifdef WIN32                                            //windows�£���C�������ķ���
#define MUTEX CRITICAL_SECTION                      //�����ٽ���ʵ�ֵ�������
#define MUTEXINIT(m)    InitializeCriticalSection(m)
#define MUTEXLOCK(m)    EnterCriticalSection(m)
#define MUTEXUNLOCK(m)  LeaveCriticalSection(m)
#define MUTEXDESTROY(m) DeleteCriticalSection(m)
#else
#define MUTEX pthread_mutex_t                       //linux�¶���
#define MUTEXINIT(m)    pthread_mutex_init(m,NULL)    //check error
#define MUTEXLOCK(m)    pthread_mutex_lock(m)
#define MUTEXUNLOCK(m)  pthread_mutex_unlock(m)
#define MUTEXDESTROY(m) pthread_mutex_destroy(m)
#endif

class CSysMutex
{
public:
    void UnLock();
    void Lock();
    CSysMutex();
    virtual ~CSysMutex();

private:
    MUTEX m_crtRealDataList;


};

#endif 
