// MyLog.cpp: implementation of the CMyLog class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "stdafx.h"
#endif

#include "MyLog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFreeLongLog* CMyLog::m_pLog = NULL;

ACE_Thread_Mutex CMyLog::log_lock_;

bool CMyLog::m_bInit = false;

int CMyLog::LogSysOperation(char* chMsg)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, log_lock_, -1);
    m_pLog->_XGSysLog(chMsg);
    return 0;
}

void CMyLog::Init()
{
    if (!m_bInit)
    {
        m_bInit = true;

        if (!m_pLog)
        {

            /*
             *	获得当前绝对路径
             */
            char szFullPath[256]; // MAX_PATH
#ifdef WIN32
            GetModuleFileName(NULL, szFullPath, MAX_PATH); //得到程序模块名称，全路径
#else
            int iCount;

            iCount = readlink("/proc/self/exe", szFullPath, 256);

            if (iCount < 0 || iCount >= 256)
            {
                m_pLog->_XGSysLog("********get process absolute path failed,errno:%d !\n", errno);
                return;
            }

            szFullPath[iCount] = '\0';

#endif


            string strCurrentpath(szFullPath);
            int nLast = strCurrentpath.find_last_of("\\");

            strCurrentpath = strCurrentpath.substr(0, nLast + 1);
            m_pLog = new CFreeLongLog((char*) strCurrentpath.c_str(), "ChannelDeviceServer", 4);
        }


    }

}

void CMyLog::Release()
{
    if (m_bInit)
    {



        if (m_pLog)
        {
            delete m_pLog;
            m_pLog = NULL;
        }
    }

}


