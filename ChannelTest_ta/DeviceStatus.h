/* 
 * File:   DeviceStatus.h
 * Author: root
 *
 * Created on 2015年6月29日, 上午10:01
 */

#ifndef DEVICESTATUS_H
#define	DEVICESTATUS_H
#include "SimpleConfig.h"
#include "ace/RW_Thread_Mutex.h"


class DeviceStatus {
public:
    DeviceStatus();
    DeviceStatus(const DeviceStatus& orig);
    virtual ~DeviceStatus();

	int SetDeviceStatus(const struDeviceAndServiceStatus &status);
	void GetCurTime(char *pTime);
	int BuildStatusString(char *pMsg, int nCount);

private:
	ACE_RW_Thread_Mutex			m_statusLock;
	struDeviceAndServiceStatus	m_status;
};

#endif	/* DEVICESTATUS_H */

