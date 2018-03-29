/* 
 * File:   DeviceStatus.cpp
 * Author: root
 * 
 * Created on 2015年6月29日, 上午10:01
 */

#include "DeviceStatus.h"
#include "ace/Guard_T.h"
#include "ace/Global_Macros.h"

#include <time.h>

DeviceStatus::DeviceStatus()
{
}

DeviceStatus::DeviceStatus(const DeviceStatus& orig)
{
}

DeviceStatus::~DeviceStatus()
{
}

int DeviceStatus::SetDeviceStatus(const struDeviceAndServiceStatus &status)
{
	//ACE_WRITE_GUARD_RETURN(ACE_RW_Thread_Mutex, guard, m_statusLock, -1);
	m_status	= status;
	return 0;
}

void DeviceStatus::GetCurTime(char *pTime)
{
	time_t t 			= time(0);
	struct tm *ld		= NULL;
	char tmp[32] 		= {0};
	ld					= localtime(&t);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", ld);
	memcpy(pTime, tmp, 64);
	return;
}

int DeviceStatus::BuildStatusString(char *pMsg, int nCount)
{
	char szMsg[512] = {0};
	//ACE_READ_GUARD_RETURN(ACE_RW_Thread_Mutex, guard, m_statusLock, -1);
	snprintf(szMsg, sizeof(szMsg), "NJJZTECH : |%s|%d|%s|%d|%s|%d|%s|%s|%s|%s|%d", \
		m_status.szLocalIP.c_str(), \
		m_status.nLocalPort, \
		m_status.szUpstreamIP.c_str(), \
		m_status.nUpstreamPort, \
		m_status.szUpstreamObjectCode.c_str(), \
		m_status.nServiceType, \
		m_status.szObjectCode.c_str(), \
		m_status.szObjectStatus.c_str(), \
		m_status.szConnectedObjectCode.c_str(), \
		m_status.szReportTime.c_str(), \
		nCount);
	memcpy(pMsg, szMsg, sizeof(szMsg));
	szMsg[511] = '\0';
	return 0;
}
