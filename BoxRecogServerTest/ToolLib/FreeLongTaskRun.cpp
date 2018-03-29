// FreeLongTaskRun.cpp: implementation of the CFreeLongTaskRun class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongTaskRun.h"
#include "SafePrint.h"
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


CFreeLongTaskRun::CFreeLongTaskRun(CFreeLongBaseLibrary* pFreeLongBaseLib)
{
	m_pFreeLongBaseLib=pFreeLongBaseLib;

}

bool CFreeLongTaskRun::StartTask(_TASKPOOL_CALLBACK pCallback,
			                     void* pCallParam,
				                 char* szAppName)
{
	CFreeLongTaskRunInfo InfoObj;
	InfoObj.AddTask(pCallback,pCallParam);
	return StartTask(&InfoObj,szAppName);

}

bool CFreeLongTaskRun::StartTask(CFreeLongTaskRunInfo* pTaskRunInfo,
							     char* szAppName)
{
	return StartTask(pTaskRunInfo->GetInfopoint(),szAppName);
	
}

bool CFreeLongTaskRun::StartTask(SFreelongTaskRunInfo* pRunInfoStruct,
		                         char* szAppName)
{
	bool bRet=false;
	if(!m_ThreadManager.ThreadContinue())
	{
		m_ThreadManager.Open();
	}

	SFreeLongRunTaskCallbackParam* pParam=(SFreeLongRunTaskCallbackParam*)m_pFreeLongBaseLib->m_pMemPool->Malloc(SFreeLongRunTaskCallbackParamSize,
		                                                                                     "CFreeLongTaskRun::pParam");


	if(pParam)
	{
		pParam->m_pThis=this;
		pParam->m_nRunIndex=0;
		if(szAppName)
		{
			SafeStrcpy(pParam->m_szAppName,
				      szAppName,
					  FREELONG_MEMORY_BLOCK_INFO_MAX_SIZE);

		}
		else
		{
			FREELONG_CLEAN_CHAR_BUFFER(pParam->m_szAppName);
		}

		CFreeLongTaskRunInfo InfoObj(&(pParam->m_Info));
		InfoObj.CopyFrom(pRunInfoStruct);
			
		bRet=m_pFreeLongBaseLib->m_pTaskPool->RegisterATask(FreeLongRunTaskCallback,
			                            pParam);

		if(bRet)
		{
			m_ThreadManager.AddAThread();
			if(szAppName)
			{
				
			}
		}


	}

	return bRet;
}

void CFreeLongTaskRun::StopAll()
{
	m_ThreadManager.CloseAll();
}

void CFreeLongTaskRun::PrintInfo()
{
	CON_PRINTF("task run:task count=%d\n",m_ThreadManager.GetThreadCount());
//	m_pFreeLongBaseLib->m_pDebug->Debug2File("task run:task count=%d\n",m_ThreadManager.GetThreadCount());
}

bool CFreeLongTaskRun::FreeLongRunTaskCallback(void* pCallParam,int& nStatus)
{
	bool bCallbackRet=false;
	bool bGoToNextStatus=true;

	SFreeLongRunTaskCallbackParam* pParam=
		(SFreeLongRunTaskCallbackParam*)pCallParam;

	if(!pParam)
	{
		return false;
	}

	CFreeLongTaskRun* pThis=pParam->m_pThis;
	switch(nStatus) 
	{
	case 0:
		if(pParam->m_Info.m_nTaskCount>pParam->m_nRunIndex)
		{
			bGoToNextStatus=false;  //只有应用层任务片段没有被执行完毕，一直在本段执行
			bCallbackRet=pParam->m_Info.m_CallbackArray[pParam->m_nRunIndex](pParam->m_Info.m_pCallParam,
				                                                             pParam->m_nRunIndex);

			if(!bCallbackRet)
			{
				pParam->m_nRunIndex++;
			}

			if(!pThis->m_ThreadManager.ThreadContinue())
			{
				pParam->m_nRunIndex++;
			}
		}
		else
		{
			bGoToNextStatus=true;
		}
		break;
	default:
		if(0<strlen(pParam->m_szAppName))
		{

		}

		pThis->m_ThreadManager.DecAThread();
		pThis->m_pFreeLongBaseLib->m_pMemPool->Free(pParam);
		return false;

	}

	if(bGoToNextStatus)
	{
		nStatus++;
	}

	return true;

}