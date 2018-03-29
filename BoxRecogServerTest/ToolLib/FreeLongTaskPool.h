// FreeLongTaskPool.h: interface for the CFreeLongTaskPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_TASK_POOL_H_
#define  _FREELONG_TASK_POOL_H_


#include "CFreeLong_LowDebug.h"
#include "FreeLongMemoryPoolWithLock.h"
#include "FreeLongMemQueueWithLock.h"
#include "FreeLongThreadPool.h"
#include "MRSWInt.h"


typedef bool(*_TASKPOOL_CALLBACK)(void* pCallParam,           //参数指针
								  int& nStatus);              //程序状态控制机制


#define TASK_POOL_TOKEN_MAX   (16*1024)                       //并发任务数
#define DEFAULT_THREAD_MAX    (30)                            //默认线程数

typedef struct _TASK_POOL_TOKEN_
{
	_TASKPOOL_CALLBACK     m_pCallback;                       //回调函数指针
	void*                   m_pUserParam;                      //回调参数
	int                     m_nUserStatus;                     //代替用户传递的状态值，初始为0
}STaskPoolToken;

const int STaskPoolTokenSize=sizeof(STaskPoolToken);

class CFreeLongBaseLibrary;
class CFreeLongThreadPool;
class CFreeLongMemQueueWithLock;
class CFreeLongMemoryPoolWithLock;
class CCFreeLong_LowDebug;

class CFreeLongTaskPool  
{
public:
	CFreeLongTaskPool(CFreeLongBaseLibrary* pFreeLongBaseLib,int nMaxThread=DEFAULT_THREAD_MAX);
	~CFreeLongTaskPool();

public:
	bool ICanWork();
	void PrintInfo();

public:
	//注册一个任务，任务可以理解为一个一个的小的执行片段，可以很快的返回。通过任务的方式，不需要为了一个简单的
	//处理，启动一个单独的线程
	bool RegisterATask(_TASKPOOL_CALLBACK pCallback,
				       void* pUserParam=NULL);


	//注册一个线程，始终执行某个处理，直到线程退出，将控制权返还给线程池。
	int ThreadPoolRegTask(_TPOOL_CALLBACK pCallback,
						  void* pParam,
						  bool bWait4Success=TRUE);


private:
	bool RegisterATaskDoIt(STaskPoolToken* pToken,int nLimit=-1);

	//服务者线程
	bool TaskServiceThreadDoIt(STaskPoolToken& Task);

	static void TaskServiceThread(void* pCallParam,
		                          MBOOL& bThreadContinue);

	//管理者线程
	static void TaskCtrlThread(void* pCallParam,
							   MBOOL& bThreadContinue);

private:
	CMRSWBool m_bThreadContinue;
	CMRSWInt m_nThreadCount;
	CFreeLongMemQueueWithLock* m_pTaskQueue;                    //核心任务队列
	CFreeLongThreadPool* m_pThreadPool;

private:
	int m_nMaxThread;
	CMRSWInt m_nThreadID;

	CFreeLongBaseLibrary* m_pFreeLongBaseLib;
	CCFreeLong_LowDebug* m_pDebug;

	
};

#endif 
