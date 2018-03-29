#include "includes.h"
#include "reader1000api.h"
#include "dev.h"
#include "packet_format.h"
#include "starup_handle.h"
#include "RFID_label.h"
#include "signal.h"
#include <time.h>
#include <zmq.h>
#include "ace/Log_Priority.h"
#include "ace/Log_Msg.h"
#include "ace/SOCK_Connector.h"
#include "ace/Signal.h"
#include "SysMessage.h"
#include "MSGHandleCenter.h"
#include "jzLocker.h"
#include <map>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/std/utility.hpp>
#include <boost/lexical_cast.hpp>

extern int gFd_dev;
static net_packet_head_t packet_head;
static u8 data_buf[0x800];
static char tmp_str[150];
int g_nArrivedFlag = 0;	// 0Îªï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×´Ì¬, 1Îªï¿½ï¿½ï¿½ï¿½×´Ì¬
locker g_nArrivedFlagLock;
extern struct struDeviceAndServiceStatus g_ObjectStatus;
extern pthread_mutex_t g_StatusMutex;
extern int dev_retry_connect_handle(int *pFd_dev);
extern std::map<std::string, strutLabelModifiers> g_LabelMap;
extern std::string                         g_strLabel;
extern locker       		  g_LabelMapLock;
extern sem	g_StartEventSem;
extern sem	g_StoppedEventSem;
extern sem      g_ClearEventSem;
using namespace boost;

void ReceiveReaderData(char* pXmlData, int nXmlLen, char *pEventID) 
{
	NET_PACKET_MSG* pMsg_ = NULL;
	CMSG_Handle_Center::get_message(pMsg_);
	if (!pMsg_) 
	{
		return;
	}
	memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
	pMsg_->msg_head.msg_type = SYS_MSG_PUBLISH_EVENT;
	pMsg_->msg_head.packet_len = sizeof (T_SysEventData);
	T_SysEventData* pReadData = (T_SysEventData*) pMsg_->msg_body;

	strcpy(pReadData->event_id, pEventID);
	pReadData->event_id[31]	= '\0';
	strcpy(pReadData->device_tag, gSetting_system.gather_DEV_TAG);

	pReadData->is_data_full	= 0; // 1;
	char szXMLGatherInfo[10 * 1024] = {0};

        if (nXmlLen > 0 && pXmlData != NULL)
        {
            strncpy(szXMLGatherInfo, pXmlData, nXmlLen);
            strcpy(pReadData->xml_data, szXMLGatherInfo);
            pReadData->xml_data_len = strlen(szXMLGatherInfo) + 1;
            syslog(LOG_DEBUG, "%s\n", szXMLGatherInfo);
        }
        else
        {
            //strcpy(pReadData->xml_data, szXMLGatherInfo);
            pReadData->xml_data_len = 0;
        }

	//syslog(LOG_DEBUG, "%s\n", szXMLGatherInfo);
	pMsg_->msg_head.packet_len = sizeof (T_SysEventData) + pReadData->xml_data_len;
	syslog(LOG_DEBUG, "pack len:%d\n", pMsg_->msg_head.packet_len);
	MSG_HANDLE_CENTER::instance()->put(pMsg_);
	return;
}

void *pthread_dev_handle(void *arg)
{
	int ret;
	int n_count;
	int r_len;
	u8 EPCID[200];
	int i;
	u8 *p_start;
	int retry_cnt = 0;

	signal(SIGPIPE,SIG_IGN);

	while (1)
	{
		n_count = 0;

		if (gSetting_system.RFID_label_type)
		{
			ret = ISO6B_ListID(gFd_dev, EPCID, &n_count, 0);
			ret = ISO6B_ListID(gFd_dev, EPCID, &n_count, 0);

			n_count = 0;
			ret = ISO6B_ListID(gFd_dev, EPCID, &n_count, 0);
		}
		else
		{
			ret = EPC1G2_ListTagID(gFd_dev, 1, 0, 0, NULL, EPCID, &n_count,
					&r_len, 0);
			ret = EPC1G2_ListTagID(gFd_dev, 1, 0, 0, NULL, EPCID, &n_count,
					&r_len, 0);

			n_count = 0;
			ret = EPC1G2_ListTagID(gFd_dev, 1, 0, 0, NULL, EPCID, &n_count,
					&r_len, 0);
		}
		
		//syslog(LOG_DEBUG, "pthread_dev_handle:ret-%d, n_count-%d,gSetting_system.RFID_label_type-%d\n", ret, n_count, gSetting_system.RFID_label_type);

		if (0 == ret)
		{
			if (n_count > 0)
			{
				p_start = EPCID;
				for (i = 0; i < n_count; i++)
				{
					if (gSetting_system.RFID_label_type)
					{
						add_label_to_list(&p_start[0 + 8 * i], 8);
					}
					else
					{
						add_label_to_list(&p_start[1], p_start[0] * 2);
						p_start += (p_start[0] * 2 + 1);
					}
				}
			}
			//send_msg_to_main_pthread();
			retry_cnt = 0;
		}
		else if (-99 == ret)
		{
			retry_cnt++;
			if (retry_cnt >= 3)
			{
				retry_cnt = 0;
				dev_retry_connect_handle(&gFd_dev);
			}
		}
		else  //??è®??¥æ??äº¤ä??ºä???é¢?
		{
//			LOG("the reader no respond! ret = %d\n", ret);
		}

		usleep(gSetting_system.DEV_scan_gap * 1000);
		usleep(1000);
	}

	return 0;
}

int RFID_upload_gather_data(int server_fd, const char *ic_number1, const char *ic_number2)
{
	syslog(LOG_DEBUG, "Enter RFID_upload_gather_data\n");
	NET_gather_data *p_gather_data;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	int w_len;
	xmlChar * xml_buf;
	char *p_send;
	int xml_len;
	int ret;

	packet_head.msg_type = MSG_TYPE_GATHER_DATA;
	strcpy(packet_head.szVersion, "1.0");

	p_gather_data = (NET_gather_data *) (data_buf + sizeof(net_packet_head_t)
			+ 8);

	strcpy(p_gather_data->event_id, gSetting_system.gather_event_id);
	sprintf(tmp_str, "RE:\t%s\t%s\n", ic_number1, ic_number2);

	ret = 0;
	memcpy(&(p_gather_data->is_data_full), &ret, sizeof(int));
	strcpy(p_gather_data->dev_tag, gSetting_system.gather_DEV_TAG);

	/**********************************XML*********************************/
	doc = xmlNewDoc(NULL);

	root_node = xmlNewNode(NULL, BAD_CAST "CAR");

	xmlDocSetRootElement(doc, root_node);

	xmlNewChild(root_node, NULL, BAD_CAST "VE_NAME", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO",
	BAD_CAST (ic_number1));

	xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO2", BAD_CAST (ic_number2));
	xmlNewChild(root_node, NULL, BAD_CAST "VE_CUSTOMS_NO", BAD_CAST "");

	xmlNewChild(root_node, NULL, BAD_CAST "VE_WT", BAD_CAST "");

	xmlDocDumpFormatMemoryEnc(doc, &xml_buf, &xml_len, "GB2312", 1);

	p_send = strstr((char*) xml_buf, "<CAR>");

	if (p_send == NULL)
	{
		return -1;
	}

	xml_len -= (p_send - (char*) xml_buf);

	p_gather_data->xml_data_len = xml_len + 1;
	memcpy(p_gather_data->xml_data, p_send, xml_len);
	p_gather_data->xml_data[xml_len] = 0;

	xmlFree(xml_buf);
	xmlFreeDoc(doc);

	// xmlCleanupParser(); 2015-8-1

	xmlMemoryDump();
	/**********************************XML END*********************************/

	w_len = ((u8*) p_gather_data->xml_data - data_buf) + xml_len + 1;
	packet_head.packet_len = w_len - sizeof(net_packet_head_t) - 8;
	strcpy((char *) data_buf, "0XJZTECH");

	memcpy(data_buf + 8, &packet_head, sizeof(net_packet_head_t));
	strcpy((char*) (data_buf + w_len), "NJTECHJZ");
	w_len += 8;

	//	LOG("upload gather data w_len = %d\n", w_len);
	//ret = write(server_fd, data_buf, w_len);
	//if (ret != w_len)
	//{
	//	//LOG_M("upload gather data error! ret =%d\n", ret);
	//	syslog(LOG_DEBUG, "upload gather data error! ret =%d\n", ret);
	//}

	//log_send(tmp_str);
	syslog(LOG_DEBUG, "upload gather data w_len = %d\n", w_len);
	ReceiveReaderData(p_gather_data->xml_data, xml_len, p_gather_data->event_id);

	return 0;
}

int RFID_upload_arrived_data(int server_fd, const char *ic_number1, const char *ic_number2)
{
	syslog(LOG_DEBUG, "Enter RFID_upload_arrived_data\n");
	NET_gather_data *p_gather_data;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	int w_len;
	xmlChar * xml_buf;
	char *p_send;
	int xml_len;
	int ret;

	packet_head.msg_type = MSG_TYPE_GATHER_DATA;
	strcpy(packet_head.szVersion, "1.0");

	p_gather_data = (NET_gather_data *) (data_buf + sizeof(net_packet_head_t)
			+ 8);

	strcpy(p_gather_data->event_id, gSetting_system.arrived_DEV_event_id);
	sprintf(tmp_str, "ARRIVED:\t%s\t%s\n", ic_number1, ic_number2);

	ret = 0;
	memcpy(&(p_gather_data->is_data_full), &ret, sizeof(int));
	strcpy(p_gather_data->dev_tag, gSetting_system.gather_DEV_TAG);

	/**********************************XML*********************************/
	doc = xmlNewDoc(NULL);

	root_node = xmlNewNode(NULL, BAD_CAST "CAR");

	xmlDocSetRootElement(doc, root_node);

	xmlNewChild(root_node, NULL, BAD_CAST "VE_NAME", BAD_CAST "");
	xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO",
	BAD_CAST (ic_number1));

	xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO2", BAD_CAST (ic_number2));
	xmlNewChild(root_node, NULL, BAD_CAST "VE_CUSTOMS_NO", BAD_CAST "");

	xmlNewChild(root_node, NULL, BAD_CAST "VE_WT", BAD_CAST "");

	xmlDocDumpFormatMemoryEnc(doc, &xml_buf, &xml_len, "GB2312", 1);

	p_send = strstr((char*) xml_buf, "<CAR>");

	if (p_send == NULL)
	{
		return -1;
	}

	xml_len -= (p_send - (char*) xml_buf);

	p_gather_data->xml_data_len = xml_len + 1;
	memcpy(p_gather_data->xml_data, p_send, xml_len);
	p_gather_data->xml_data[xml_len] = 0;

	xmlFree(xml_buf);
	xmlFreeDoc(doc);

	xmlCleanupParser();

	xmlMemoryDump();
	/**********************************XML END*********************************/

	w_len = ((u8*) p_gather_data->xml_data - data_buf) + xml_len + 1;
	packet_head.packet_len = w_len - sizeof(net_packet_head_t) - 8;
	strcpy((char *) data_buf, "0XJZTECH");

	memcpy(data_buf + 8, &packet_head, sizeof(net_packet_head_t));
	strcpy((char*) (data_buf + w_len), "NJTECHJZ");
	w_len += 8;

	syslog(LOG_DEBUG, "upload arrived data w_len = %d\n", w_len);
	//ReceiveReaderData(p_gather_data->xml_data, xml_len, p_gather_data->event_id);
        ReceiveReaderData(0, 0, p_gather_data->event_id);


	//ret = write(server_fd, data_buf, w_len);
	//if (ret != w_len)
	//{
	//	//LOG_M("upload gather data error! ret =%d\n", ret);
	//	syslog(LOG_DEBUG, "upload gather data error! ret =%d\n", ret);
	//}

	//log_send(tmp_str);

	return 0;
}

void GetCurTime(char *pTime)
{
	time_t t 		= time(0);
	struct tm *ld	= NULL;
	char tmp[32] 	= {0};
	ld					= localtime(&t);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", ld);
	memcpy(pTime, tmp, 64);
}


void BuildStatusString(struct struDeviceAndServiceStatus *pStatus, char *pMsg, int nCount)
{
        char szMsg[512] = {0};
		snprintf(szMsg, sizeof(szMsg), "NJJZTECH : |%s|%d|%s|%d|%s|%d|%s|%s|%s|%s|%d", \
pStatus->szLocalIP, \
pStatus->nLocalPort, \
pStatus->szUpstreamIP, \
pStatus->nUpstreamPort, \
pStatus->szUpstreamObjectCode, \
pStatus->nServiceType, \
pStatus->szObjectCode, \
pStatus->szObjectStatus, \
pStatus->szConnectedObjectCode, \
pStatus->szReportTime, \
nCount);
                
//	using boost::property_tree::ptree;
//	ptree pt;
//	ptree pattr1;
//        std::string AREA_ID = gSetting_system.GATHER_AREA_ID;
//        std::string CHNL_NO = gSetting_system.GATHER_CHANNE_NO;
//        std::string I_E_TYPE = gSetting_system.GATHER_I_E_TYPE;
//        std::string strID   = gSetting_system.register_DEV_ID;
//        std::string NAME    = "µç×Ó³µÅÆ";
//        std::string STATUS  = "1";
//        std::string REASON  = "Õý³£";
//
//	pattr1.add<std::string>("<xmlattr>.AREA_ID",    AREA_ID);
//	pattr1.add<std::string>("<xmlattr>.CHNL_NO",    CHNL_NO);
//	pattr1.add<std::string>("<xmlattr>.I_E_TYPE",   I_E_TYPE);
//
//	pt.add_child("DEVICE_STATUS_INFO",          pattr1);
//	pt.put("DEVICE_STATUS_INFO.DEVICES.ID",     strID);
//	pt.put("DEVICE_STATUS_INFO.DEVICES.NAME",   NAME);
//	pt.put("DEVICE_STATUS_INFO.DEVICES.STATUS", STATUS);
//	pt.put("DEVICE_STATUS_INFO.DEVICES.REASON", REASON);
//
//	// ?¼å???è¾??ºï???å®?ç¼???ï¼?é»?è®?utf-8ï¼?
//	boost::property_tree::xml_writer_settings<char> settings('\t', 1,  "GB2312");
//	//write_xml(filename, pt, std::locale(), settings);
//        
//        ostringstream oss;
//        write_xml(oss, pt, settings);
//        
//        string strGatherXml = oss.str();        
//          
//	memcpy(pMsg, strGatherXml.c_str(), strGatherXml.size());                
}

void *pthread_worker_task(void *arg)
{
	char szPublisherIp[64] = {0};
	signal(SIGPIPE,SIG_IGN);

    void * pCtx = NULL;
    void * pSock = NULL;
    //ä½¿ç??tcp??è®?è¿?è¡???ä¿¡ï???è¦?è¿??¥ç???????ºå??IP?°å??ï¿??92.168.1.2
    //??ä¿¡ä½¿?¨ç??ç½?ç»?ç«?ï¿??ï¿??766
    snprintf(szPublisherIp, sizeof(szPublisherIp), "tcp://%s:%d", gSetting_system.publisher_server_ip, gSetting_system.publisher_server_port);
    szPublisherIp[63] = '\0';
    const char * pAddr = szPublisherIp; // "tcp://192.168.1.101:7766";

    //??å»?context
   if((pCtx = zmq_ctx_new()) == NULL)
    {
        return 0;
    }
    //??å»?socket
    if((pSock = zmq_socket(pCtx, ZMQ_DEALER)) == NULL)
    {
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    int iSndTimeout = 5000;// millsecond
    //è®¾ç½®?¥æ?¶è???
    if(zmq_setsockopt(pSock, ZMQ_RCVTIMEO, &iSndTimeout, sizeof(iSndTimeout)) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    //è¿??¥ç????IP192.168.1.2ï¼?ç«?ï¿??766
    if(zmq_connect(pSock, pAddr) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    //å¾???????æ¶?ï¿??   
    while(1)
    {
      static int i = 0;
      //struct struDeviceAndServiceStatus tmpStatus;
      //memset(&tmpStatus, 0x00, sizeof(tmpStatus));
//		strcpy(g_ObjectStatus.szLocalIP,				"127.0.0.1");
//		g_ObjectStatus.nLocalPort	= 9002;
//
//		strcpy(g_ObjectStatus.szUpstreamIP,		"127.0.0.1");
//		g_ObjectStatus.nUpstreamPort	= 9003;
//		strcpy(g_ObjectStatus.szUpstreamObjectCode,		"APP_CENTER");
//
//		g_ObjectStatus.nServiceType	= 0;
//
//		strcpy(g_ObjectStatus.szObjectCode,			"DEV_ETAG_001");
//		strcpy(g_ObjectStatus.szObjectStatus,	"000");
//		strcpy(g_ObjectStatus.szConnectedObjectCode,	"APP_CHANNEL");
//		strcpy(g_ObjectStatus.szReportTime,			"2015-03-29 16:38:25");

		char szMsg[1024] = {0};
		pthread_mutex_lock(&g_StatusMutex);
		BuildStatusString(&g_ObjectStatus, szMsg, i++);
		pthread_mutex_unlock(&g_StatusMutex);




      //_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
      //printf("Enter to send...\n");
      syslog(LOG_DEBUG, "Enter to send...\n");
      if(zmq_send(pSock, szMsg, sizeof(szMsg), 0) < 0)
        {
            //fprintf(stderr, "send message faild\n");
            syslog(LOG_ERR, "send message faild\n");
            continue;
        }
      //syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
      syslog(LOG_DEBUG, "send message : succeed\n");
      // getchar();
      usleep (5000 * 1000);
    }



//	while (1)
//	{
//		usleep(gSetting_system.DEV_scan_gap * 1000);
//		usleep(1000);
//	}

	return 0;
}

void *ArrivedEventThread(void *arg)
{
	signal(SIGPIPE,SIG_IGN);
	while (1)
	{
		int nArrivedFlag = 0;
		g_nArrivedFlagLock.lock();
		nArrivedFlag = g_nArrivedFlag;
		g_nArrivedFlagLock.unlock();
		if (nArrivedFlag == 1)
		{
			syslog(LOG_DEBUG, "nArrivedFlag:%d\n", nArrivedFlag);
			sleep(3);
			continue;
		}

		int nFlag = 0;
		g_LabelMapLock.lock();
		if (g_LabelMap.size() > 0)
		{
			nFlag = 1;
		}
		g_LabelMapLock.unlock();
		syslog(LOG_DEBUG, "nFlag:%d\n", nFlag);

		// ï¿½ï¿½ï¿?-ï¿½ï¿½ï¿½ï¿½	
		if (nFlag == 1)
		{
			int nIterator = 0;
			std::string str1 = "";
			std::string str2 = "";
			int nCountValue1 = 0;
			int nCountValue2 = 0;
			int nTimeValue1  = 0;
			int nTimeValue2  = 0;
			g_LabelMapLock.lock();
			for (std::map<std::string, strutLabelModifiers>::iterator it = g_LabelMap.begin(); it != g_LabelMap.end(); ++it)
			{
				if ((*it).second.nSendFlag == 0  &&  nIterator == 0)
				{
					str1					= (*it).first;
					nCountValue1			= (*it).second.nCount;
					nTimeValue1             = (*it).second.nTime;
					//(*it).second.nSendFlag	= 1;
					++nIterator;
				}
				else if ((*it).second.nSendFlag == 0  &&  nIterator == 1)
				{
					str2	= (*it).first;
					nCountValue2 = (*it).second.nCount;
					nTimeValue2             = (*it).second.nTime;
					//(*it).second.nSendFlag	= 1;
					++nIterator;
				}
			}
			g_LabelMapLock.unlock();

			//if (nCountValue2 != 0 && nCountValue1 > nCountValue2 && abs(nCountValue1 - nCountValue2) >= 1)
			//{
			//	str2 = "";
			//}

			//if (nCountValue2 != 0 && nCountValue2 > nCountValue1 && abs(nCountValue2 - nCountValue1) >= 1)
			//{
			//	str1 = str2;
			//	str2 = "";
			//}

			if (nCountValue2 != 0 && nTimeValue1 > nTimeValue2 && abs(nTimeValue1 - nTimeValue2) > 2)
			{
				str2 = "";
			}

			if (nCountValue2 != 0 && nTimeValue2 > nTimeValue1 && abs(nTimeValue2 - nTimeValue1) > 2)
			{
				str1 = str2;
				str2 = "";
			}

			if (str1.empty() && str2.empty())
			{

			}
			else
			{
				syslog(LOG_DEBUG, "Arrived Read Count1=%d, Count2=%d\n", nCountValue1, nCountValue2);
				RFID_upload_arrived_data(0, str1.c_str(), str2.c_str());
			}
		}

		//usleep(gSetting_system.DEV_scan_gap * 1000);
		sleep(3);
	}
	return 0;
}

void *StartEventThread(void *arg)
{
	signal(SIGPIPE,SIG_IGN);

	int nResult = 0;
	while (1)
	{
		nResult = g_StartEventSem.wait();
		g_nArrivedFlagLock.lock();
		g_nArrivedFlag = 1;
		g_nArrivedFlagLock.unlock();


		int nFlag = 0;
		g_LabelMapLock.lock();
		if (g_LabelMap.size() > 0)
		{
			nFlag = 1;
		}
		g_LabelMapLock.unlock();
                sleep(2);

		if (nFlag == 0)
		{
			int nCount = 5;
			int nSpan  = 5000 / 5;
			if (nSpan > 0)
			{
				while (nCount)
				{
					sleep(1);
					nCount--;
					g_LabelMapLock.lock();
					if (g_LabelMap.size() > 0)
					{
						g_LabelMapLock.unlock();
						nFlag = 1;
						break;
					}
					g_LabelMapLock.unlock();
				}

				if (nFlag == 0)
					continue;
			}			
		}

		// ï¿½ï¿½ï¿?-ï¿½ï¿½ï¿½ï¿½
		if (nFlag == 1)
		{
			int nIterator = 0;
			std::string str1 = "";
			std::string str2 = "";
			int nCountValue1 = 0;
			int nCountValue2 = 0;
			int nTimeValue1  = 0;
			int nTimeValue2  = 0;
                        
                        std::map<int, std::string> tmpMap;
                        
			g_LabelMapLock.lock();
			for (std::map<std::string, strutLabelModifiers>::iterator it = g_LabelMap.begin(); it != g_LabelMap.end(); ++it)
			{
				if ((*it).second.nSendFlag == 0  &&  nIterator == 0)
				{
					str1					= (*it).first;
					nCountValue1			= (*it).second.nCount;
					nTimeValue1             = (*it).second.nTime;
					(*it).second.nSendFlag	= 1;
					++nIterator;
                                        tmpMap.insert(make_pair(nTimeValue1, str1));
				}
				else if ((*it).second.nSendFlag == 0  &&  nIterator == 1)
				{
					str2	= (*it).first;
					nCountValue2 = (*it).second.nCount;
					nTimeValue2             = (*it).second.nTime;
					(*it).second.nSendFlag	= 1;
					++nIterator;
                                        tmpMap.insert(make_pair(nTimeValue2, str2));
				}
                                else if ((*it).second.nSendFlag == 0  &&  nIterator > 1)
                                {
					str1					= (*it).first;
					nCountValue1			= (*it).second.nCount;
					nTimeValue1             = (*it).second.nTime;
					(*it).second.nSendFlag	= 1;
					++nIterator;
                                        tmpMap.insert(make_pair(nTimeValue1, str1));                                    
                                }
                                    
			}
			g_LabelMapLock.unlock();
                        
                        std::map<int, std::string>::reverse_iterator iterX = tmpMap.rbegin();
                        if (iterX != tmpMap.rend())
                        {
                            str1 = (*iterX).second;
                            str2 = "";
                        }
                        else
                        {
                            str1 = "";
                            str2 = "";
                        }

			//if (nCountValue2 != 0 && nCountValue1 > nCountValue2 && abs(nCountValue1 - nCountValue2) >= 1)
			//{
			//	str2 = "";
			//}

			//if (nCountValue2 != 0 && nCountValue2 > nCountValue1 && abs(nCountValue2 - nCountValue1) >= 1)
			//{
			//	str1 = str2;
			//	str2 = "";
			//}
                        

//			if (nCountValue2 != 0 && nTimeValue1 > nTimeValue2 && abs(nTimeValue1 - nTimeValue2) > 3)
//			{
//				str2 = "";
//			}
//
//			if (nCountValue2 != 0 && nTimeValue2 > nTimeValue1 && abs(nTimeValue2 - nTimeValue1) > 3)
//			{
//				str1 = str2;
//				str2 = "";
//			}


//			if (str1.empty() && str2.empty())
//			{
//
//			}
//			else
//			{
//				syslog(LOG_DEBUG, "Started Read Count1=%d, Count2=%d\n", nCountValue1, nCountValue2);
//				RFID_upload_gather_data(0, str1.c_str(), str2.c_str());
//			}
                        

                        g_LabelMapLock.lock();
                        str1 = g_strLabel;
                        str2 = "";
                        g_LabelMapLock.unlock();
			{
				syslog(LOG_DEBUG, "Started Read Count1=%d, Count2=%d\n", nCountValue1, nCountValue2);
				RFID_upload_gather_data(0, str1.c_str(), str2.c_str());
			}                        
                        
                        
		}		
		// finished

		////
		//g_LabelMapLock.lock();
		//if (g_LabelMap.size() > 0)
		//	g_LabelMap.clear();
		//g_LabelMapLock.unlock();



		usleep(1000);
	}
	return 0;
}

void *StopEventThread(void *arg)
{
	signal(SIGPIPE,SIG_IGN);

	int nResult = 0;
	while (1)
	{
		nResult = g_StoppedEventSem.wait();

		////
		//g_LabelMapLock.lock();
		//if (g_LabelMap.size() > 0)
		//	g_LabelMap.clear();
		//g_LabelMapLock.unlock();
		//usleep(1000);

		g_nArrivedFlagLock.lock();
		g_nArrivedFlag = 0;
		g_nArrivedFlagLock.unlock();
                
                g_LabelMapLock.lock();
                g_strLabel = "";
                g_LabelMapLock.unlock();                
                


		usleep(1000);
	}
	return 0;
}


void *TimerEventThread(void *arg)
{
	signal(SIGPIPE,SIG_IGN);

	while (1)
	{
		//syslog(LOG_DEBUG, "9999999999999999999999999999999999999999999999\n");
		time_t t 		= time(0);
		
		////
		//g_LabelMapLock.lock();
		//if (g_LabelMap.size() > 0)
		//	g_LabelMap.clear();
		//g_LabelMapLock.unlock();
		//usleep(1000);
		g_LabelMapLock.lock();
		for (std::map<std::string, strutLabelModifiers>::iterator it = g_LabelMap.begin(); it != g_LabelMap.end();)
		{
			if ((*it).second.nSendFlag == 1  &&  t - (*it).second.nTime > 60 * 2)
			{
				g_LabelMap.erase(it++);
			}
			else
			{
				++it;
			}
		}
		g_LabelMapLock.unlock();


		sleep(2);
	}
	return 0;
}

