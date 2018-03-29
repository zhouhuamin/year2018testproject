// SysMutex.cpp: implementation of the CSysMutex class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "stdafx.h"
#endif

#include "SysMutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSysMutex::CSysMutex()
{
    MUTEXINIT(&m_crtRealDataList);
}

CSysMutex::~CSysMutex()
{
    MUTEXDESTROY(&m_crtRealDataList);
}

void CSysMutex::Lock()
{
    MUTEXLOCK(&m_crtRealDataList);
}

void CSysMutex::UnLock()
{
    MUTEXUNLOCK(&m_crtRealDataList);
}

void CSysSemaphore::release()
{

    sem_post(&sem);


}

void CSysSemaphore::acquire()
{
    sem_wait(&sem);
}

CSysSemaphore::CSysSemaphore()
{
    sem_init(&sem, 0, 0);
}

CSysSemaphore::~CSysSemaphore()
{
    sem_destroy(&sem);
}

