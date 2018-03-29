// FreeLongThreadPool.cpp: implementation of the CFreeLongThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongThreadPool.h"
#include <process.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define _THREADPOOL_CAN_NOT_USE                  2          //还没有初始化，无法工作
#define _THREADPOOL_OVERFLOW                     -1         //溢出，不能注册
#define _THREADPOOL_PLEASE_WAIT                  0          //没有备用线程，请等待
#define _THREADPOOL_OK                           1          //注册成功

#define FREELONG_DEBUG m_pDebug->Debug2File


CFreeLongThreadPool::CFreeLongThreadPool(CCFreeLong_LowDebug* pDebug)
{
	m_pDebug=pDebug;
	FREELONG_DEBUG("CFreeLongThreadPool Start!\n");

	THREADID id;
	THREAD t;

	MUTEXINIT(&m_RegisterLock);
	XMI(m_bThreadContinue,true);
	XMI(m_nThreadPoolIdleThreadCount,0);

	m_nThreadPoolThreadCount.Set(0);

	int i=0;
	for(i=0;i<THIS_POOLTHREAD_MAX;i++)
	{
		m_Token[i].m_hThread=0;
		m_Token[i].m_hThreadID=0;
		m_Token[i].m_nExitCode=THREAD_POOL_EXIT_CODE+i;

		m_Token[i].m_pCallback=NULL;
		m_Token[i].m_pCallParam=NULL;
		m_Token[i].m_pThreadPoolObject=this;

		XMI(m_Token[i].m_nState,TPOOL_THREAD_STATE_NOT_RUN);
	}

	id=0;
	t=0;

	THREADCREATE(ThreadPoolCtrlThread,this,t,id);
	Sleep(OPEN_THREAD_DELAY);
}

CFreeLongThreadPool::~CFreeLongThreadPool()
{
	XMS(m_bThreadContinue,false);
	while(m_nThreadPoolThreadCount.Get())
	{
		Sleep(MIN_SLEEP);
	}

	int i=0;
	for(i=0;i<THIS_POOLTHREAD_MAX;i++)
	{
		XME(m_Token[i].m_nState);
	}

	XME(m_bThreadContinue);
	XME(m_nThreadPoolIdleThreadCount);
	MUTEXDESTROY(&m_RegisterLock);

	FREELONG_DEBUG("CFreeLongThreadPool Stop!\n");

}

int CFreeLongThreadPool::Search4NotUseToken()
{
	int i=0;
	for(i=0;i<THIS_POOLTHREAD_MAX;i++)
	{
		if(TPOOL_THREAD_STATE_NOT_RUN == XMG(m_Token[i].m_nState))
		{
			return i;
		}
	}

	return -1;
}


THREADFUNCDECL(CFreeLongThreadPool::ThreadPoolCtrlThread,pParam)
{
	CFreeLongThreadPool* pThis=(CFreeLongThreadPool*)pParam;

	pThis->m_nThreadPoolThreadCount.Add();
	int nIdleThread=0;
	int nNotRunThread=0;

	while (XMG(pThis->m_bThreadContinue)) 
	{
		//始终保持空闲的线程数量在10左右
		nIdleThread=XMG(pThis->m_nThreadPoolIdleThreadCount);
		if(WHILE_THREAD_COUNT>nIdleThread)
		{
			nNotRunThread=pThis->Search4NotUseToken();
			if(-1!=nNotRunThread)
			{
				THREADCREATE(ThreadPoolThread,
					         &(pThis->m_Token[nNotRunThread]),
							 pThis->m_Token[nNotRunThread].m_hThread,
							 pThis->m_Token[nNotRunThread].m_hThreadID);

				
			}
		}

		Sleep(OPEN_THREAD_DELAY);

	}

	pThis->m_nThreadPoolThreadCount.Dec();

#ifdef WIN32
	return THREAD_POOL_EXIT_CODE-1;
#else
	return NULL;
#endif
}

THREADFUNCDECL(CFreeLongThreadPool::ThreadPoolThread,pParam)
{
	SThreadToken* pThreadToken=(SThreadToken*)pParam;

	XMS(pThreadToken->m_nState,TPOOL_THREAD_STATE_IDLE);

	pThreadToken->m_pThreadPoolObject->m_nThreadPoolThreadCount.Add();

	XMA(pThreadToken->m_pThreadPoolObject->m_nThreadPoolIdleThreadCount);

	while (XMG(pThreadToken->m_pThreadPoolObject->m_bThreadContinue))
	{
		switch(XMG(pThreadToken->m_nState))
		{
		case TPOOL_THREAD_STATE_NOT_RUN:
			XMS(pThreadToken->m_nState,TPOOL_THREAD_STATE_IDLE);
		case TPOOL_THREAD_STATE_IDLE:
		default:
			break;
		case TPOOL_THREAD_STATE_BUSY:
			if(pThreadToken->m_pCallback)
			{
				pThreadToken->m_pCallback(pThreadToken->m_pCallParam,
					                      pThreadToken->m_pThreadPoolObject->m_bThreadContinue);
				XMA(pThreadToken->m_pThreadPoolObject->m_nThreadPoolIdleThreadCount);
			}

			break;
		}

		if(WHILE_THREAD_COUNT<pThreadToken->m_pThreadPoolObject->m_nThreadPoolThreadCount.Get())
		{
			break;       //备用线程数量超出限额，跳出循环，退出自己
		}

		if(TPOOL_THREAD_STATE_IDLE != XMG(pThreadToken->m_nState))
		{
			XMS(pThreadToken->m_nState,TPOOL_THREAD_STATE_IDLE);
		}

		Sleep(DEFAULT_THREAD_SLEEP);    //等待下次任务
	}

	XMD(pThreadToken->m_pThreadPoolObject->m_nThreadPoolIdleThreadCount);
	pThreadToken->m_pThreadPoolObject->m_nThreadPoolThreadCount.Dec();
	XMS(pThreadToken->m_nState,TPOOL_THREAD_STATE_NOT_RUN);

#ifdef WIN32
	return  pThreadToken->m_nExitCode;
#else
	return NULL;
#endif


}

int CFreeLongThreadPool::GetAIdleThread()
{
	int nRet=-1;
	int i=0;
	for(i=0;i<THIS_POOLTHREAD_MAX;i++)
	{
		if(TPOOL_THREAD_STATE_IDLE==XMG(m_Token[i].m_nState))
		{
			nRet=i;
			break;
		}
	}

	return nRet;
}

int CFreeLongThreadPool::ThreadPoolRegisterANewThread(_TPOOL_CALLBACK pCallback,
									 void* pParam)
{
	int nRet=_THREADPOOL_PLEASE_WAIT;
	MUTEXLOCK(&m_RegisterLock);
	int nIdleThread=GetAIdleThread();
	if(0>nIdleThread)
	{
		if(THIS_POOLTHREAD_MAX == m_nThreadPoolThreadCount.Get())
		{
			nRet=_THREADPOOL_OVERFLOW;
		}
		else
		{
			nRet=_THREADPOOL_PLEASE_WAIT;
		}

	}
	else
	{
		m_Token[nIdleThread].m_pCallback=pCallback;
		m_Token[nIdleThread].m_pCallParam=pParam;
		XMS(m_Token[nIdleThread].m_nState,TPOOL_THREAD_STATE_BUSY);
		XMD(m_nThreadPoolIdleThreadCount);
		nRet=_THREADPOOL_OK;

	}

	MUTEXUNLOCK(&m_RegisterLock);

	return nRet;

}

int CFreeLongThreadPool::ThreadPoolRegisterANewThreadWhile(_TPOOL_CALLBACK pCallback,
		                              void* pParam)
{
	int nRet=_THREADPOOL_PLEASE_WAIT;

	while (1) 
	{
		nRet=ThreadPoolRegisterANewThread(pCallback,pParam);
		if(_THREADPOOL_PLEASE_WAIT != nRet)
		{
			break;
		}

		Sleep(OPEN_THREAD_DELAY);
	}

	return nRet;
	

}

int CFreeLongThreadPool::ThreadPoolRegTask(_TPOOL_CALLBACK pCallback,
										   void* pParam,
						                   bool bWait4Success)
{
	if(bWait4Success)
	{
		return ThreadPoolRegisterANewThreadWhile(pCallback,pParam);
	}
	else
	{
		return ThreadPoolRegisterANewThread(pCallback,pParam);
	}
}

int CFreeLongThreadPool::GetAllThreadCount()
{
	return m_nThreadPoolThreadCount.Get();
}
