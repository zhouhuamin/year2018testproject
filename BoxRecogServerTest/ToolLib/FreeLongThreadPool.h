// FreeLongThreadPool.h: interface for the CFreeLongThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_THREAD_POOTL_H_
#define _FREELONG_THREAD_POOTL_H_
#include <process.h>

#include "CFreeLong_LowDebug.h"

#include "MInt.h"
#include "MRSWInt.h"

#ifdef WIN32 
	#define MIN_SLEEP 10
	#define THREAD     HANDLE
	#define THREADID   unsigned
	#define THREAD_CREATE_ERROR 0
	#define THREADFUNCDECL(func,args)unsigned __stdcall func(PVOID args)
	#define THREADCREATE(func,args,thread,id) thread=(HANDLE)_beginthreadex(NULL,0,func,(PVOID)args,0,&id);
#else
	#define MIN_SLEEP 1
	#define THREAD   pthread_t
	#define THREADID   unsigned
	#define THREAD_CREATE_ERROR -1
	#define THREADFUNCDECL(func,args) void* func(PVOID args)
	#define THREADCREATE(func,args,thread,id) \
		pthread_create(&thread,NULL,func,args);
#endif




typedef void(*_TPOOL_CALLBACK)(
				void* pCallparam,
				MBOOL& bThreadContinue);

//static void ThreadPoolCallback(void* pCallParam,MBOOL& bThreadContinue);

#define OPEN_THREAD_DELAY  250    //�߳��ӳ�ʱ����С250ms��������������߳�
#define WHILE_THREAD_COUNT 3     //���n���߳̿��еȴ�����
#define DEFAULT_THREAD_SLEEP  10    //��������߲���
#define THREAD_POOL_EXIT_CODE 10000    //�ӿ�ʼ�����˳���

//���߳�������
#ifdef _ARM_
	#define THIS_POOLTHREAD_MAX   30
#else
	#ifdef WIN32 
		#define THIS_POOLTHREAD_MAX   2000
	#else
		#define THIS_POOLTHREAD_MAX   300
	#endif
#endif


#define TPOOL_THREAD_STATE_NOT_RUN   0
#define TPOOL_THREAD_STATE_IDLE      1
#define TPOOL_THREAD_STATE_BUSY      2


class CFreeLongThreadPool;
typedef struct  _THREAD_TOKEN_ 
{
	int                  m_nExitCode;
	MINT                 m_nState;
	THREAD               m_hThread;
	THREADID             m_hThreadID;
	_TPOOL_CALLBACK      m_pCallback;
	void*                m_pCallParam;
	CFreeLongThreadPool* m_pThreadPoolObject;

}SThreadToken;

const unsigned long SThreadTokenSize=sizeof(SThreadToken);



class CFreeLongThreadPool  
{
public:
	CFreeLongThreadPool(CCFreeLong_LowDebug* pDebug);
	~CFreeLongThreadPool();

public:
	int GetAllThreadCount();
	//ע��һ�����̣߳�����һ��״ֵ̬
	int ThreadPoolRegTask(_TPOOL_CALLBACK pCallback,
						  void* pParam,
						  bool bWait4Success=TRUE);


	bool TPAllThreadIsIdle();                       //��������̳߳��Ƿ����
	bool ThreadPoolsContinue();                     //����̳߳�����״̬

private:
	static THREADFUNCDECL(ThreadPoolThread,pParam);           //�̳߳ط����߳�
	static THREADFUNCDECL(ThreadPoolCtrlThread,pParam);       //�̳߳ع����߳�

private:
	int Search4NotUseToken();                  //�������е�token
	int GetAIdleThread();

	int ThreadPoolRegisterANewThread(_TPOOL_CALLBACK pCallback,
									 void* pParam);

	int ThreadPoolRegisterANewThreadWhile(_TPOOL_CALLBACK pCallback,
		                                  void* pParam);

private:
	SThreadToken         m_Token[THIS_POOLTHREAD_MAX];
	MBOOL                m_bThreadContinue;
	CMRSWInt             m_nThreadPoolThreadCount;

	MINT                 m_nThreadPoolIdleThreadCount;
	MUTEX                m_RegisterLock;
	CCFreeLong_LowDebug* m_pDebug;
	
};

#endif 
