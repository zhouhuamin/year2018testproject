#include "ace/Signal.h"
#include "SimpleConfig.h"
#include "ace/streams.h"
#include "ace/Thread_Manager.h"
#include "ace/Select_Reactor.h"
#include "server_acceptor.h"
#include "MyLog.h"
#include "MSG_Center.h"
#include "Cmd_Acceptor.h"
#include <syslog.h>
#include <zmq.h>

#include "DeviceStatus.h"

DeviceStatus g_ObjectStatus;

int GlobalStopFlag  = 0;
int g_nTestFlag     = 0;
pthread_mutex_t g_ImageDataMutex;


static ACE_THR_FUNC_RETURN pthread_worker_task(void *arg)
{
	char szPublisherIp[64] = {0};
	signal(SIGPIPE,SIG_IGN);

	void * pCtx = NULL;
	void * pSock = NULL;
	//ä½¿ç??tcp??è®?è¿?è¡???ä¿¡ï???è¦?è¿??¥ç???????ºå??IP?°å??ä¸?92.168.1.2
	//??ä¿¡ä½¿?¨ç??ç½?ç»?ç«???ä¸?766
	snprintf(szPublisherIp, sizeof(szPublisherIp), "tcp://%s:%d", CSimpleConfig::m_publisher_server_ip.c_str(), CSimpleConfig::m_publisher_server_port);
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
	//è¿??¥ç????IP192.168.1.2ï¼?ç«???766
	if(zmq_connect(pSock, pAddr) < 0)
	{
		zmq_close(pSock);
		zmq_ctx_destroy(pCtx);
		return 0;
	}
	//å¾???????æ¶???   
	while(1)
	{
		static int i = 0;
		char szMsg[1024] = {0};
		g_ObjectStatus.BuildStatusString(szMsg, i++);

		//_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
		//printf("Enter to send...\n");
		//syslog(LOG_DEBUG, "Enter to send...\n");

		if (szMsg[0] != '\0')
		{
			if(zmq_send(pSock, szMsg, sizeof(szMsg), 0) < 0)
			{
				//fprintf(stderr, "send message faild\n");
				syslog(LOG_ERR, "send message : [%s] faild\n", szMsg);
				sleep (10);
				continue;
			}
			syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
			// getchar();
		}
		sleep (10);
	}

	return 0;
}

static ACE_THR_FUNC_RETURN pthread_channel_test(void *arg)
{
    // "<VE_NAME>»ÆÃöD66681</VE_NAME>\n"
    // <PCAR_NO>»ÆÃöD66681</PCAR_NO>
    // test code
    while (!GlobalStopFlag)
    {
            int ch = getchar();
            
            if (ch == 's')
            {    
                
                for (int i = 0; i < 1; ++i)
                {
                    char szBuffer[6 + 1] = {0};
                std::string strSeqNo = "20161125173118";   
                sprintf(szBuffer, "%06d", i);
                strSeqNo += szBuffer;
                
                    g_nTestFlag	= 1;
std::string strXml = "<GATHER_INFO AREA_ID=\"7208100000\" CHNL_NO=\"7208100001\" I_E_TYPE=\"E\"";
         strXml += " SEQ_NO=\"";
         strXml += strSeqNo;
         strXml += "\"";
         strXml += "CHNL_TYPE=\"0\">\n"
"\n"
"<IC>\n"
"<DR_IC_NO></DR_IC_NO>\n"
"<IC_DR_CUSTOMS_NO></IC_DR_CUSTOMS_NO>\n"
"<IC_CO_CUSTOMS_NO></IC_CO_CUSTOMS_NO>\n"
"<IC_BILL_NO></IC_BILL_NO>\n"
"<IC_GROSS_WT></IC_GROSS_WT>\n"
"<IC_VE_CUSTOMS_NO></IC_VE_CUSTOMS_NO>\n"
"<IC_VE_NAME></IC_VE_NAME>\n"
"<IC_CONTA_ID></IC_CONTA_ID>\n"
"<IC_ESEAL_ID></IC_ESEAL_ID>\n"
"<IC_EX_DATA>123</IC_EX_DATA>\n"                 
"<IC_REG_DATETIME></IC_REG_DATETIME>\n"
"<IC_PER_DAY_DUE></IC_PER_DAY_DUE>\n"
"<IC_FORM_TYPE></IC_FORM_TYPE>\n"
"</IC>\n"
"\n"
"\n"
"<WEIGHT>\n"
"<GROSS_WT></GROSS_WT>\n"
"</WEIGHT>\n"
"\n"
"\n"
"<CAR>\n"
"<VE_NAME>»ÆÃöD66681</VE_NAME>\n"
"<CAR_EC_NO>E0040000F4F0BE05</CAR_EC_NO>\n"
"<CAR_EC_NO2></CAR_EC_NO2>\n"
"<VE_CUSTOMS_NO></VE_CUSTOMS_NO>\n"
"<VE_WT></VE_WT>\n"
"</CAR>\n"
"\n"
"\n"
"<CONTA><CONTA_NUM>1</CONTA_NUM><CONTA_RECO>1</CONTA_RECO><CONTA_ID_F>FSCU9390315</CONTA_ID_F><CONTA_ID_B></CONTA_ID_B><CONTA_MODEL_F>45G1</CONTA_MODEL_F><CONTA_MODEL_B></CONTA_MODEL_B></CONTA><CONTA_PIC><CONTA_PIC_F>/dev/shm/676c0f1d-b3bf-4fb4-8d4b-dfc04752ef3a.jpg</CONTA_PIC_F><CONTA_PIC_B>/dev/shm/cd7d858c-94a9-4c62-8a3b-ead4dca6ed25.jpg</CONTA_PIC_B><CONTA_PIC_RF>/dev/shm/57771b65-7600-468a-99af-332ff970ee92.jpg</CONTA_PIC_RF><CONTA_PIC_LB>/dev/shm/09cf4890-8295-4df8-a4c0-ba36b588aaa6.jpg</CONTA_PIC_LB><CONTA_PIC_LF>/dev/shm/630c7f42-97b9-42b9-89e9-02f0ab6bff7d.jpg</CONTA_PIC_LF><CONTA_PIC_RB>/dev/shm/ff45ce69-91d0-4f4b-84c4-381d355010db.jpg</CONTA_PIC_RB></CONTA_PIC>\n"
"\n"
"<SEAL>\n"
"<ESEAL_ID></ESEAL_ID>\n"
"<SEAL_KEY></SEAL_KEY>\n"
"</SEAL>\n"
"\n"
"\n"
"<OPTCAR><PCAR_NO>»ÆÃöD66681</PCAR_NO><PCAR_NO_PICNAME></PCAR_NO_PICNAME><PCAR_PICNAME>/root/recvpic/20151014173015336_big.jpg</PCAR_PICNAME></OPTCAR>\n"
"\n"
"<BARCODE>\n"
"<DATA>1232455</DATA>\n"
"</BARCODE>\n"
"</GATHER_INFO>\n";


                    
               CMSG_Center *tmpCenter = MSG_CENTER::instance();
               
               tmpCenter->send_packet_test(strSeqNo.c_str(), strXml.c_str()); 
                }
                    
            }
            else if (ch == 'q')
            {
                    g_nTestFlag		= 0;
            }

            sleep(3);
    }   
    
    
    return 0;
}

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor *) arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

int main(int argc, char *argv[])
{

    int xxxx = sizeof (T_Upload_Pic_Data);
    
    openlog("ChannelControlServer", LOG_PID, LOG_LOCAL6);
    /* Ignore signals generated when a connection is broken unexpectedly. */

    ACE_Sig_Action sig((ACE_SignalHandler) SIG_IGN, SIGPIPE);
    ACE_UNUSED_ARG(sig);

    ACE_UNUSED_ARG(argc);
    ACE_UNUSED_ARG(argv);

    CMyLog::Init();
    CSimpleConfig cg;
    cg.get_config();


    //////////////////////////////////////////////////////////////////////////////////////////////
    //do timeout event

    /* get default instance of ACE_Reactor  */
    ACE_Select_Reactor *select_reactor;
    ACE_NEW_RETURN(select_reactor, ACE_Select_Reactor, 1);
    ACE_Reactor *reactor;
    ACE_NEW_RETURN(reactor, ACE_Reactor(select_reactor, 1), 1);
    ACE_Reactor::close_singleton();
    ACE_Reactor::instance(reactor, 1);


    CMyLog::m_pLog->_XGSysLog("starting up channel control server......\n");

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    select(0, 0, NULL, NULL, &tv);

    MSG_CENTER::instance()->open();

    CCmd_Acceptor::InitHandler();

    CCmd_Acceptor peer_acceptor;
    if (peer_acceptor.open(ACE_INET_Addr(CSimpleConfig::LOCAL_LISTEN_PORT), reactor, 1) == -1)
    {
        CMyLog::m_pLog->_XGSysLog("open listen port %d fail...\n",CSimpleConfig::LOCAL_LISTEN_PORT);
		{		
			struDeviceAndServiceStatus tmpStatus;
			tmpStatus.szLocalIP					= "";
			tmpStatus.nLocalPort				= 0;
			tmpStatus.szUpstreamIP				= CSimpleConfig::CENTER_CONTROLER_IP;
			tmpStatus.nUpstreamPort				= CSimpleConfig::CENTER_CONTROLER_PORT;
			tmpStatus.szUpstreamObjectCode		= "APP_CENTER_001";
			tmpStatus.nServiceType				= 1;
			tmpStatus.szObjectCode				= "APP_CHANNEL_001";
			tmpStatus.szObjectStatus			= "031";
			tmpStatus.szConnectedObjectCode		= "";
			char szNowTime[32]					= {0};
			g_ObjectStatus.GetCurTime(szNowTime);
			szNowTime[31]						= '\0';
			tmpStatus.szReportTime				= szNowTime;
			g_ObjectStatus.SetDeviceStatus(tmpStatus);
		}
    }
    else
    {
        CMyLog::m_pLog->_XGSysLog("open listen port %d ok...\n", CSimpleConfig::LOCAL_LISTEN_PORT);

		{		
			struDeviceAndServiceStatus tmpStatus;
			tmpStatus.szLocalIP					= "";
			tmpStatus.nLocalPort				= 0;
			tmpStatus.szUpstreamIP				= CSimpleConfig::CENTER_CONTROLER_IP;
			tmpStatus.nUpstreamPort				= CSimpleConfig::CENTER_CONTROLER_PORT;
			tmpStatus.szUpstreamObjectCode		= "APP_CENTER_001";
			tmpStatus.nServiceType				= 1;
			tmpStatus.szObjectCode				= "APP_CHANNEL_001";
			tmpStatus.szObjectStatus			= "030";
			tmpStatus.szConnectedObjectCode		= "";
			char szNowTime[32]					= {0};
			g_ObjectStatus.GetCurTime(szNowTime);
			szNowTime[31]						= '\0';
			tmpStatus.szReportTime				= szNowTime;
			g_ObjectStatus.SetDeviceStatus(tmpStatus);
		}
    }

    ACE_Thread_Manager::instance()->spawn(event_loop,			reactor);
//	ACE_Thread_Manager::instance()->spawn(pthread_worker_task,	reactor);
    ACE_Thread_Manager::instance()->spawn(pthread_channel_test,	reactor);
    
    MSG_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();

    ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down controlcenter daemon\n"));

    return 0;
}
