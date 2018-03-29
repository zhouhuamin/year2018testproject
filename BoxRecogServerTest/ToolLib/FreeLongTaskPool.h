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


typedef bool(*_TASKPOOL_CALLBACK)(void* pCallParam,           //����ָ��
								  int& nStatus);              //����״̬���ƻ���


#define TASK_POOL_TOKEN_MAX   (16*1024)                       //����������
#define DEFAULT_THREAD_MAX    (30)                            //Ĭ���߳���

typedef struct _TASK_POOL_TOKEN_
{
	_TASKPOOL_CALLBACK     m_pCallback;                       //�ص�����ָ��
	void*                   m_pUserParam;                      //�ص�����
	int                     m_nUserStatus;                     //�����û����ݵ�״ֵ̬����ʼΪ0
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
	//ע��һ����������������Ϊһ��һ����С��ִ��Ƭ�Σ����Ժܿ�ķ��ء�ͨ������ķ�ʽ������ҪΪ��һ���򵥵�
	//��������һ���������߳�
	bool RegisterATask(_TASKPOOL_CALLBACK pCallback,
				       void* pUserParam=NULL);


	//ע��һ���̣߳�ʼ��ִ��ĳ������ֱ���߳��˳���������Ȩ�������̳߳ء�
	int ThreadPoolRegTask(_TPOOL_CALLBACK pCallback,
						  void* pParam,
						  bool bWait4Success=TRUE);


private:
	bool RegisterATaskDoIt(STaskPoolToken* pToken,int nLimit=-1);

	//�������߳�
	bool TaskServiceThreadDoIt(STaskPoolToken& Task);

	static void TaskServiceThread(void* pCallParam,
		                          MBOOL& bThreadContinue);

	//�������߳�
	static void TaskCtrlThread(void* pCallParam,
							   MBOOL& bThreadContinue);

private:
	CMRSWBool m_bThreadContinue;
	CMRSWInt m_nThreadCount;
	CFreeLongMemQueueWithLock* m_pTaskQueue;                    //�����������
	CFreeLongThreadPool* m_pThreadPool;

private:
	int m_nMaxThread;
	CMRSWInt m_nThreadID;

	CFreeLongBaseLibrary* m_pFreeLongBaseLib;
	CCFreeLong_LowDebug* m_pDebug;

	
};

#endif 
