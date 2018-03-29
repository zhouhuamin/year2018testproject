// FreeLongTaskRunInfo.h: interface for the CFreeLongTaskRunInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_TASK_RUN_INFO_H_
#define _FREELONG_TASK_RUN_INFO_H_
#include "FreeLongTaskPool.h"


#define FREELONG_TASK_RUN_MAX_TASK 16             //最多步动作
typedef struct _FREELONG_TASK_RUN_INFO_ 
{
	int m_nTaskCount;
	void* m_pCallParam;
	_TASKPOOL_CALLBACK m_CallbackArray[FREELONG_TASK_RUN_MAX_TASK];

}SFreelongTaskRunInfo;

const ULONG SFreelongTaskRunInfoSize=sizeof(SFreelongTaskRunInfo);

class CFreeLongTaskRun;

typedef struct _FreeLongRunTaskCallback_Param_
{
	SFreelongTaskRunInfo m_Info;
	CFreeLongBaseLibrary* m_pFreeLongBaseLib;
	CFreeLongTaskRun* m_pThis;
	int m_nRunIndex;
	char m_szAppName[FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE];

}SFreeLongRunTaskCallbackParam;

const ULONG SFreeLongRunTaskCallbackParamSize=sizeof(SFreeLongRunTaskCallbackParam);

class CFreeLongTaskRunInfo  
{
public:
	CFreeLongTaskRunInfo(SFreeLongRunTaskCallbackParam* pParam)
	{
		m_pInfo=&(pParam->m_Info);
		Init(m_pInfo);
	}

	CFreeLongTaskRunInfo(SFreelongTaskRunInfo* pInfo)
	{
		m_pInfo=pInfo;
		Init(m_pInfo);
	}

	CFreeLongTaskRunInfo()
	{
		m_pInfo=NULL;
		Init(&m_Info);
	}

	
	 ~CFreeLongTaskRunInfo()
	 {
	 }
public:
	SFreelongTaskRunInfo m_Info;
	SFreelongTaskRunInfo* m_pInfo;

private:
	static void Init(SFreelongTaskRunInfo* pInfo)
	{
		pInfo->m_nTaskCount=0;
		pInfo->m_pCallParam=NULL;
		
		int i=0;
		for(i=0;i<FREELONG_TASK_RUN_MAX_TASK;i++)
		{
			pInfo->m_CallbackArray[i]=NULL;
		}
	}

public:
	SFreelongTaskRunInfo* GetInfopoint()
	{
		if(m_pInfo)
		{
			return m_pInfo;
		}
		else
		{
			return &m_Info;
		}
	}

public:
	void SetCallbackparam(void* pCallParam)
	{
		if(m_pInfo)
		{
			m_pInfo->m_pCallParam=pCallParam;

		}
		else
		{
			m_Info.m_pCallParam=pCallParam;
		}
	}


	bool AddTask(_TASKPOOL_CALLBACK pCallback,void* pCallParam)
	{
		if(pCallParam)
		{
			SetCallbackparam(pCallParam);
			return AddTask(pCallback);
		}

		return false;
	}


	bool AddTask(_TASKPOOL_CALLBACK pCallback)
	{
		if(m_pInfo)
		{
			if(FREELONG_TASK_RUN_MAX_TASK<=m_pInfo->m_nTaskCount)
			{
				return false;
			}

			m_pInfo->m_CallbackArray[m_pInfo->m_nTaskCount]=pCallback;
			m_pInfo->m_nTaskCount++;
			return true;
		}
		else
		{
			if(FREELONG_TASK_RUN_MAX_TASK<=m_Info.m_nTaskCount)
			{
				return false;
			}
			
			m_Info.m_CallbackArray[m_Info.m_nTaskCount]=pCallback;
			m_Info.m_nTaskCount++;
			return true;
		}
	}

	void CopyFrom(SFreelongTaskRunInfo* pInfo)
	{
		char* pMyInfo=NULL;
		if(m_pInfo)
		{
			pMyInfo=(char*)m_pInfo;
		}
		else
		{
			pMyInfo=(char*)&m_Info;
		}

		memcpy(pMyInfo,(char*)pInfo,SFreelongTaskRunInfoSize);
	}
};


#endif // !defined(AFX_FREELONGTASKRUNINFO_H__D322E3F6_6FD9_424F_8D59_5D203A9F8B58__INCLUDED_)
