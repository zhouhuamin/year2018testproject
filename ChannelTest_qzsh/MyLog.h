// MyLog.h: interface for the CMyLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MYLOG_H_)
#define _MYLOG_H_


#include "MutexGuard.h"
#include "Log/FreeLongLog.h"
#include "Log/FreeLongSourceRegister.h"
#include <string>
#include <map>
#include <list>
using namespace std;

class CMyLog
{
public:
    static void Release();
    static void Init();
    static int LogSysOperation(char* chMsg);
    static CSysMutex log_lock_;

    static CFreeLongLog* m_pLog;
private:
    static bool m_bInit;

};

#endif










































