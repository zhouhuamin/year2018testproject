// FreeLongTaskRun.h: interface for the CFreeLongTaskRun class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_TASK_RUN_H_
#define _FREELONG_TASK_RUN_H_

#include "ThreadManager.h"
#include "FreeLongTaskPool.h"
#include "FreeLongTaskRunInfo.h"

class CFreeLongBaseLibrary;

class CFreeLongTaskRun  
{
public:
	CFreeLongTaskRun(CFreeLongBaseLibrary* pFreeLongBaseLib);
	virtual ~CFreeLongTaskRun()
	{
		StopAll();
	}

public:
	bool StartTask(_TASKPOOL_CALLBACK pCallback,
		           void* pCallParam,
				   char* szAppName=NULL);



	bool StartTask(CFreeLongTaskRunInfo* pTaskRunInfo,
		           char* szAppName=NULL);


	bool StartTask(SFreelongTaskRunInfo* pRunInfoStruct,
		           char* szAppName=NULL);

	void StopAll();

	bool IsRunning()
	{
		return m_ThreadManager.ThreadContinue();
	}

	int GetThreadCount()
	{
		return m_ThreadManager.GetThreadCount();
	}

	void PrintInfo();


	void inline SafeStrcpy(char* pDest,char* pSource,int nCount)
	{
		int nLen=(int)strlen(pSource)+1;
		
		if(!pDest)
		{
			goto SafeStrcpy_END_PROCESS;
		}
		if(!pSource)
		{
			goto SafeStrcpy_END_PROCESS;
		}
		
		if(nLen > nCount)
		{
			nLen=nCount;
		}
		
		memcpy(pDest,pSource,nLen);
		*(pDest+nLen-1)='\0';
		
		
		
SafeStrcpy_END_PROCESS:
		return;
	}


private:
	static bool FreeLongRunTaskCallback(void* pCallParam,int& nStatus);
	CThreadManager m_ThreadManager;
	CFreeLongBaseLibrary* m_pFreeLongBaseLib;

};

#endif // !defined(AFX_FREELONGTASKRUN_H__131440FB_D4DD_4968_A313_EAF2BE521522__INCLUDED_)
