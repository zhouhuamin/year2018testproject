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


CSysMutex CMyLog::log_lock_;

bool CMyLog::m_bInit = false;

int CMyLog::LogSysOperation(char* chMsg)
{
    CMutexGuard guard(log_lock_);
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
            GetModuleFileName(NULL, szFullPath, 256); //得到程序模块名称，全路径
#else
            int iCount = 0;
            iCount = readlink("/proc/self/exe", szFullPath, 256);
            szFullPath[iCount] = '\0';

#endif


            string strCurrentpath(szFullPath);
            int nLast = strCurrentpath.find_last_of("/");

            strCurrentpath = strCurrentpath.substr(0, nLast + 1);
            m_pLog = new CFreeLongLog((char*) strCurrentpath.c_str(), "ChannelControlServer", 4);
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




