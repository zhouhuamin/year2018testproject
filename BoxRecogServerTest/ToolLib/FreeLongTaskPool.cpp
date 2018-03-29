// FreeLongTaskPool.cpp: implementation of the CFreeLongTaskPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongTaskPool.h"
#include "FreeLongBaseLibrary.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define CONDEBUG 1
#ifdef CONDEBUG
#define CON_PRINTF printf
#else
#define CON_PRINTF /\
/printf
#endif



CFreeLongTaskPool::CFreeLongTaskPool(CFreeLongBaseLibrary* pFreeLongBaseLib,int nMaxThread)
{
	m_pFreeLongBaseLib=pFreeLongBaseLib;
	m_pDebug=m_pFreeLongBaseLib->m_pDebug;

	m_nMaxThread=nMaxThread;

	m_bThreadContinue.Set(true);
	m_nThreadCount.Set(0);
	m_nThreadID.Set(0);

	m_pTaskQueue=new CFreeLongMemQueueWithLock(m_pDebug,
		                                       m_pFreeLongBaseLib->m_pMemPool,
											   "CFreeLongTaskPool");

	if(m_pTaskQueue)
	{
		m_pFreeLongBaseLib->m_pMemPool->Register(m_pTaskQueue,"CFreeLongTaskPool::m_pTaskQueue");

	}

	m_pThreadPool=new CFreeLongThreadPool(m_pDebug);
	
	if(m_pThreadPool)
	{
		m_pFreeLongBaseLib->m_pMemPool->Register(m_pThreadPool,"CFreeLongTaskPool::m_pThreadPool");
	}

	if(ICanWork())
	{
		if(!m_pThreadPool->ThreadPoolRegTask(TaskCtrlThread,this))
		{

		}
		else
		{
			m_nThreadCount.Add();
		}
	}
}

CFreeLongTaskPool::~CFreeLongTaskPool()
{
	m_bThreadContinue.Set(false);

	while(m_nThreadCount.Get())
	{
		Sleep(100);
	}

	
	m_pFreeLongBaseLib->m_pMemPool->UnRegister(m_pThreadPool);
	delete m_pThreadPool;
	m_pThreadPool=NULL;

	m_pFreeLongBaseLib->m_pMemPool->UnRegister(m_pTaskQueue);
	delete m_pTaskQueue;
	m_pTaskQueue=NULL;


	
}


bool CFreeLongTaskPool::ICanWork()
{
	if(!m_pThreadPool)
	{
		return false;
	}


	if(!m_pTaskQueue)
	{
		return false;
	}

	return true;
}

void CFreeLongTaskPool::PrintInfo()
{
	CON_PRINTF("thread pool: thread count=%d,task pool:thread count=%d,task in queue=%d\n",
		m_pThreadPool->GetAllThreadCount(),m_nThreadCount.Get(),m_pTaskQueue->GetTokenCount());

//	FREELONG_DEBUG("thread pool: thread count=%d,task pool:thread count=%d,task in queue=%d\n",
//		m_pThreadPool->GetAllThreadCount(),m_nThreadCount.Get(),m_pTaskQueue->GetTokenCount());
}


void CFreeLongTaskPool::TaskCtrlThread(void* pCallParam,
							   MBOOL& bThreadContinue)
{
	CFreeLongTaskPool* pThis=(CFreeLongTaskPool*)pCallParam;
	int i=0;

	//ͬʱע��m_nMaxThread���̣߳�������ִ��������е�����
	for(i=0;i<pThis->m_nMaxThread;i++)
	{
		if(!pThis->m_pThreadPool->ThreadPoolRegTask(TaskServiceThread,pThis))
		{
			break;
		}
		else
		{
			pThis->m_nThreadCount.Add();
		}
	}

	pThis->m_nThreadCount.Dec();          //������ɣ��Լ��˳�
}

bool CFreeLongTaskPool::TaskServiceThreadDoIt(STaskPoolToken& Task)
{
	bool bCallbackRet=Task.m_pCallback(Task.m_pUserParam,Task.m_nUserStatus);

	if(!bCallbackRet)
	{
		return bCallbackRet;            //���������ֱ�ӷ���
	}

	//������񷵻��棬��ʾ����û�н�������Ҫ�����ƻ��������
	bCallbackRet=RegisterATaskDoIt(&Task);
	if(!bCallbackRet)
	{
		FREELONG_DEBUG("CFreeLongTaskPool::TaskServiceThreadDoIt():a task need continue, \
			but add 2 queue fail!task lost!\n");

	}

	return bCallbackRet;

}

/*
 *TaskServiceThread��һ���߳��壬���Ǵ��̴߳�һ����������в��ϼ����ͬ���߲�ͬ���͵�������ִ�У�
 *�����Զ��̶߳�����ĸо�
 */
void CFreeLongTaskPool::TaskServiceThread(void* pCallParam,
		               MBOOL& bThreadContinue)
{
	int nQueueRet=0;
	STaskPoolToken Task;
	char* szTask=(char*)&Task;

	CFreeLongTaskPool* pThis=(CFreeLongTaskPool*)pCallParam;
	int nID=pThis->m_nThreadID.Add()-1;

	while (XMG(bThreadContinue)) 
	{
		if(!pThis->m_bThreadContinue.Get())
		{
			goto CFreeLongTaskPool_TaskServiceThread_End_Process;
		}

		//���Դ���������е�����һ�����񣬲�ִ��
		nQueueRet=pThis->m_pTaskQueue->GetAndDeleteFirst(szTask,STaskPoolTokenSize);
		if(STaskPoolTokenSize==nQueueRet)
		{
			pThis->TaskServiceThreadDoIt(Task);
		}

		Sleep(pThis->m_nMaxThread);    //����m_nMaxThreadΪ���������Ա�֤ÿ��1000�εĴ���
	}


CFreeLongTaskPool_TaskServiceThread_End_Process:
	pThis->m_nThreadCount.Dec();

}

bool CFreeLongTaskPool::RegisterATaskDoIt(STaskPoolToken* pToken,int nLimit)
{
	bool bRet=false;
	if(STaskPoolTokenSize == m_pTaskQueue->AddLast((char*)pToken,STaskPoolTokenSize),nLimit)
	{
		bRet=true;
	}

	return bRet;
}

bool CFreeLongTaskPool::RegisterATask(_TASKPOOL_CALLBACK pCallback,
									  void* pUserParam)
{
	STaskPoolToken Token;
	if(!ICanWork())
	{
		return false;
	}

	if(!pCallback)
	{
		return false;
	}

	Token.m_pCallback=pCallback;
	Token.m_pUserParam=pUserParam;
	Token.m_nUserStatus=0;

	return RegisterATaskDoIt(&Token,m_nMaxThread);

}

int CFreeLongTaskPool::ThreadPoolRegTask(_TPOOL_CALLBACK pCallback,
					                     void* pParam,
										 bool bWait4Success)
{
	return m_pThreadPool->ThreadPoolRegTask(pCallback,pParam);
}