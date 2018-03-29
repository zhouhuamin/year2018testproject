// FreeLongBaseLibrary.cpp: implementation of the CFreeLongBaseLibrary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FreeLongBaseLibrary.h"
#include "SafePrint.h"
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

CFreeLongBaseLibrary::CFreeLongBaseLibrary(char* szAppName,                                  //应用名
										   char* szLogPath,
										   char* szTempPath,
										   int nTaskPoolThreadMax,                           //任务池最大数目
										   bool bDebug2TTYFlag,                              //输出到屏幕开关
										   _BASE_LIBRARY_PRINT_INFO_CALLBACK pPrintInfoCallback, //info输出回调
										   void* pPrintInfoCallbackParam,
										   _APP_INFO_OUT_CALLBACK pInfoOutCallback,           //应用程序输出回调
										   void* pInfoOutCallbackParam)
{
	//各个指针变量初始化为NULL，防止某个动作失败后跳转，成为野指针
	m_pDebug=NULL;
	m_pTaskPool=NULL;
	m_pMemPool=NULL;
	m_pLog=NULL;

	//保存各个字符串
	SafeStrcpy(m_szAppName,szAppName,FREELONG_APPLICATION_NAME_SIZE);
	SafeStrcpy(m_szLogPathName,szLogPath,FREELONG_APPLICATION_NAME_SIZE);
	SafeStrcpy(m_szTempPathName,szTempPath,FREELONG_APPLICATION_NAME_SIZE);
	
	m_pPrintInfoCallback=pPrintInfoCallback;
	m_pPrintInfoCallbackParam=pPrintInfoCallbackParam;

	srand((unsigned int)time(NULL));


	//万事从debug开始
	m_pMemPool=new CFreeLongMemoryPoolWithLock(m_pDebug);
	if(!m_pMemPool)
	{
		FREELONG_DEBUG("CFreeLongBaseLibrary():m_pMemPool new fail!\n");
		return;
	}

	//初始化日志系统
	m_pLog=new CFreeLongLog(m_szLogPathName,m_szAppName);

	FREELONG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
}

CFreeLongBaseLibrary::~CFreeLongBaseLibrary()
{
	FREELONG_DEBUG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");


	CON_PRINTF("1\n");

	m_pMemPool->SetCloseFlag();


	CON_PRINTF("2\n");

	CON_PRINTF("3\n");
	if(m_pTaskRun)
	{
		m_pMemPool->UnRegister(m_pTaskRun);
		delete m_pTaskRun;
		m_pTaskRun=NULL;

	}

	CON_PRINTF("4\n");

	CON_PRINTF("5\n");
	if(m_pTaskPool)
	{
		m_pMemPool->UnRegister(m_pTaskPool);
		delete m_pTaskPool;
		m_pTaskPool=NULL;
	}

	CON_PRINTF("6\n");
	if(m_pLog)
	{
		m_pMemPool->UnRegister(m_pLog);
		delete m_pLog;
		m_pLog=NULL;

	}

	CON_PRINTF("7\n");
	if(m_pMemPool)
	{
		delete m_pMemPool;
		m_pMemPool=NULL;
	}

	CON_PRINTF("8\n");

	CON_PRINTF("Bye World");

	CON_PRINTF("-------------------------------------------\n");

	if(m_pDebug)
	{
		delete m_pDebug;
		m_pDebug=NULL;
	}

	CON_PRINTF("9\n");
}


bool CFreeLongBaseLibrary::InfoPrintTaskCallback(void* pCallParam,int& nStatus)
{
	CFreeLongBaseLibrary* pThis=(CFreeLongBaseLibrary*)pCallParam;

	if(pThis->TimeIsUp(pThis->m_tLastPrint,MAIN_LOOP_DELAY))
	{
		TimeSetNow(pThis->m_tLastPrint);    //更新时间
		
		CON_PRINTF("***************************************\n");

		//这里增加m_pDebug，是因为debug可以方便的把调试的信息回调或者打印

		pThis->m_pTaskPool->PrintInfo();
		pThis->m_pTaskRun->PrintInfo();
		pThis->m_pMemPool->PrintInfo();
		if(pThis->m_pPrintInfoCallback)
		{
			pThis->m_pPrintInfoCallback(pThis->m_pPrintInfoCallbackParam);
		}
	
	
		CON_PRINTF("***************************************\n");
		CON_PRINTF("\n");
	}

	//这里返回true，则此函数将会被周期性的执行，如果不希望周期性被执行，则返回false
	return true;
}


