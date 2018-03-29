// FreeLongBaseLibrary.h: interface for the CFreeLongBaseLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_BASE_LIBRARY_H_
#define _FREELONG_BASE_LIBRARY_H_
#include "CFreeLong_LowDebug.h"
#include "FreeLongMemoryPoolWithLock.h"
#include "FreeLongLog.h"
#include "FreeLongTaskPool.h"
#include "FreeLongTaskRun.h"

#define MAIN_LOOP_DELAY 10             //����һ��
#define TimeSetNow(t) time(&t)


typedef void(*_BASE_LIBRARY_PRINT_INFO_CALLBACK)(void* pCallParam);

class CFreeLongBaseLibrary  
{
public:
	CFreeLongBaseLibrary(char* szAppName,
		                 char* szLogPath,
						 char* szTempPath,
		                 int nTaskPoolThreadMax=DEFAULT_THREAD_MAX,        //����������Ŀ
	 	                 bool bDebug2TTYFlag=true,                         //�������Ļ����
		                 _BASE_LIBRARY_PRINT_INFO_CALLBACK pPrintInfoCallback=NULL, //info����ص�
		                 void* pPrintInfoCallbackParam=NULL,
		                 _APP_INFO_OUT_CALLBACK pInfoOutCallback=NULL,     //Ӧ�ó�������ص�
		                 void* pInfoOutCallbackParam=NULL);

	 ~CFreeLongBaseLibrary();

public:
	char m_szAppName[FREELONG_APPLICATION_NAME_SIZE];

	char m_szLogPathName[FREELONG_APPLICATION_NAME_SIZE];

	char m_szTempPathName[FREELONG_APPLICATION_NAME_SIZE];

	CFreeLongLog* m_pLog;                                     //��־ģ��
	CFreeLongMemoryPoolWithLock* m_pMemPool;
	CFreeLongTaskPool* m_pTaskPool;                           //�����
	CFreeLongTaskRun* m_pTaskRun;                             //�����������

	CCFreeLong_LowDebug* m_pDebug;                            //�ں˼�Debug��ÿ������дһ���ļ��������ϴε�

private:
	static bool InfoPrintTaskCallback(void* pCallParam,int& nStatus);

	time_t m_tLastPrint;

	_BASE_LIBRARY_PRINT_INFO_CALLBACK m_pPrintInfoCallback;

	void* m_pPrintInfoCallbackParam;

public:
	inline bool TimeIsUp(time_t tLast,long lMax)
	{
		time_t tNow;
		TimeSetNow(tNow);
		long lDeltaT=(long)tNow-(long)tLast;
		if(lMax<=lDeltaT)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif 
