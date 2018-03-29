#pragma once
#include "config.h"
#include <string>
#include <vector>

#define MSG_LOG_FROM_ETAG 				0x50
#define MSG_LOG_FROM_ICCARD 			0x51
#define MSG_LOG_FROM_WEIGHT 			0x52
#define MSG_LOG_FROM_LED 			0x53
#define MSG_LOG_FROM_BROADCAST 			0x54

#define MSG_FROM_DEV_PTHREAD			0x48

#define	MSG_LOG_FROM_DEV				MSG_LOG_FROM_BROADCAST


#define SYSTEM_SETTING_CML_FILE_NAME   	"PrinterServerConfig.xml"





#define SERVER_IP_ADDRESS_STR  			("127.0.0.1")
#define SERVER_NET_PORT 				(8888)

#define REGISTER_DEV_TAG				("PRINTER")
#define REGISTER_DEV_ID					("DEV_PRINTER_001")
#define SYS_CTRL_R_START_EVENT_ID		("EC_START_PRT")
#define SYS_CTRL_R_STOP_EVENT_ID		("EC_STOP_PRT")
#define SYS_CTRL_IC_RING_EVENT_ID               ("EG_FINISHED_PRT")

#define GATHER_DATA_EVENT_ID			("EG_FINISHED_PRT")
#define ARRIVED_DEV_EVENT_ID			("EG_ARRIVED_PRT")
#define ONLOAD_DEV_DEV_TAG				("EG_ONLOAD_PRT")

#define GATHER_DATA_DEV_TAG				("PRINTER")


typedef unsigned char	BYTE;
#pragma pack(push)
#pragma pack(1)
union BT_T080_STATUS
{
	struct struSTATUS
	{
		BYTE b0:1;
		BYTE b1:1;
		BYTE b2:1;
		BYTE b3:1;
		BYTE b4:1;
		BYTE b5:1;
		BYTE b6:1;
		BYTE b7:1;
	}status;
	BYTE byteStatus;
};
#pragma pack(pop)


void SplitString(std::string source, std::vector<std::string>& dest, const std::string &division);
void SysSleep(long ms);
int broadcast_msg_play(char *xml_buf, int xml_buf_len);
void ReceiveReaderData(const char* pXmlData, int nXmlLen, const char *pEventID);
void *pthread_dev_handle(void *arg);
void *pthread_dev_handle2(void *arg);
void *pthread_dev_handle_test(void *arg);
int RFID_upload_arrived_data(int server_fd, const char *ic_number1, const char *ic_number2);
void GetCurTime(char *pTime);
void BuildStatusString(struct struDeviceAndServiceStatus *pStatus, char *pMsg, int nCount);
void *pthread_worker_task(void *arg);
void *StartEventThread(void *arg);


