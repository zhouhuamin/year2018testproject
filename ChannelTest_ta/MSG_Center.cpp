
#include <map>


#include <vector>

// MSG_Center.cpp: implementation of the MSG_Task class.
//
//////////////////////////////////////////////////////////////////////

#include "ace/Select_Reactor.h"
#include "ace/SOCK_Stream.h"

#include "tinyXML/tinyxml.h"


#include "SysMessage.h"
#include "MSG_Center.h"
#include "Cmd_Acceptor.h"
#include "SysUtil.h"
#include "CascadeCmdHandler.h"
#include "ControlCmdHandler.h"
#include <sys/time.h>
#include <syslog.h>

#include "DeviceStatus.h"

extern DeviceStatus g_ObjectStatus;

#define LELINK_MSG_QUERY_SYSKEY				                  0x1001
#define LELINK_MSG_QUERY_SYSKEY_ACK				          0x1002

ACE_Thread_Mutex CMSG_Center::msg_lock_;

std::vector<NET_PACKET_MSG*> CMSG_Center::msgVec_;
int CMSG_Center::m_nMsgBufferCount = 0;
ACE_Thread_Mutex CMSG_Center::device_lock_;

enum CHANNEL_RUN_STATE
{
    NORMAL = 0,
    WAITRESULT,
    REGATHER


};

CMSG_Center::~CMSG_Center()
{
    ACE_Reactor::instance()->cancel_timer(this);
}

CMSG_Center::CMSG_Center()
{

}

static ACE_THR_FUNC_RETURN ThreadConnectToCascadeServerFunc(void *lParam)
{
    CMSG_Center* pTask = (CMSG_Center*) lParam;
    pTask->ConnectToCenterManageServer();
    return 0;
}

static ACE_THR_FUNC_RETURN ThreadConnectToMonitorServerFunc(void *lParam)
{
    CMSG_Center* pTask = (CMSG_Center*) lParam;
    pTask->ConnectToMonitorServer();
    return 0;
}

static ACE_THR_FUNC_RETURN ThreadExceptionCheckFunc(void *lParam)
{
    CMSG_Center* pTask = (CMSG_Center*) lParam;
    pTask->CheckException();
    return 0;
}

int CMSG_Center::open()
{
    cascade_socket = -1;
    m_pDataPack = NULL;

    memset(&m_SysContaPicInfo,  0x00, sizeof (T_Sys_ContaPicInfo));
    memset(&m_SysVehiclePicInfo, 0x00, sizeof(T_Sys_VehiclePicInfo));

    m_pSendPicBuffer = new char[6 * 1024 * 1024];

    m_nRunState = NORMAL;
    memset(&m_szSequenceID, 0, 32);


    //初始化信息数据缓冲区，此缓冲区用于CCtrlCmdReceiver_T向该类传输数据
    init_message_buffer();
    // msg_semaphore_.acquire();    //2015-5-5

    m_bStartOk = false;


    {

        void* device_so = dlopen("/usr/lib/libDataPacket.so", RTLD_LAZY);
        if (!device_so)
        {

            CMyLog::m_pLog->_XGSysLog("********load library fail! dll name is  %s ,error %s %d \n", "libDataPacket.so", dlerror(), errno);
            return 1;
        } else
        {
            CMyLog::m_pLog->_XGSysLog("load library succ! dll name is  %s \n", "llibDataPacket.so");
        }



        // reset errors
        dlerror();

        // load the symbols
        create_t* create_device = (create_t*) dlsym(device_so, "create");
        const char* dlsym_error = dlerror();
        if (dlsym_error)
        {
            CMyLog::m_pLog->_XGSysLog("Cannot load symbol create:  %s \n", dlsym_error);
            return 1;
        }

        if (create_device)
        {
            CMyLog::m_pLog->_XGSysLog("get create function pointer %p succ !\n", create_device);
        }



        // create an instance of the class
        m_pDataPack = create_device();


    }

    ACE_Thread_Manager::instance()->spawn(ThreadConnectToCascadeServerFunc, this);
    ACE_Thread_Manager::instance()->spawn(ThreadExceptionCheckFunc, this);
    ACE_Thread_Manager::instance()->spawn_n(MAX_HANDLE_THREADS, ThreadHandleMessage, this);
    ACE_Thread_Manager::instance()->spawn(ThreadConnectToMonitorServerFunc, this);



    if (ACE_Reactor::instance()->schedule_timer(this, 0,
            ACE_Time_Value(1, 0),
            ACE_Time_Value(1, 0)) == -1)
    {
        return -1;
    }

    return 0;
}

int CMSG_Center::ConnectToCenterManageServer()
{
    CCascadeCmdHandler * handler = new CCascadeCmdHandler;

    while (1)
    {
        if (!CCascadeCmdHandler::m_bConnected)
        {
            if (handler->ConnectToCascadeServer(CSimpleConfig::CENTER_CONTROLER_PORT, CSimpleConfig::CENTER_CONTROLER_IP) == -1)
            {


            } else
            {
                CCascadeCmdHandler::m_bConnected = true;
            }
        }

        //select代替sleep,等待一段时间间隔5s
        SysUtil::SysSleep(2 * 1000);
    }

    delete handler;

}

int CMSG_Center::ConnectToMonitorServer()
{
    CControlCmdHandler * handler = new CControlCmdHandler;

    while (1)
    {
        if (!CControlCmdHandler::m_bConnected)
        {
            if (handler->ConnectToMonitorServer(CSimpleConfig::SERVER_MONITOR_PORT, "127.0.0.1") == -1)
            {


            } else
            {
                CControlCmdHandler::m_bConnected = true;
            }
        }

        //select代替sleep,等待一段时间间隔5s
        SysUtil::SysSleep(2 * 1000);
    }

    delete handler;
}

ACE_THR_FUNC_RETURN CMSG_Center::ThreadHandleMessage(void *arg)
{
    CMSG_Center* pTask = (CMSG_Center*) arg;
    while (1)
    {
        pTask->msg_semaphore_.acquire();

        NET_PACKET_MSG* pMsg_ = NULL;
        {
            ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, pTask->msg_handle_lock_, 0);
            //printlog("Handle count : %d ****************************\n\n", CHA_Proactive_Acceptor::GetValidHandleCount());
            if (pTask->msgToHandleList_.size() == 0)
            {

                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 100000;
                select(0, 0, NULL, NULL, &tv);

        // 2015-5-5
        if (pTask != NULL)
        {
            CMyLog::m_pLog->_XGSysLog("111111++++++++++++++++msg to handle %d!\n", pTask->msgToHandleList_.size());
        }
                continue;
            }

            pMsg_ = pTask->msgToHandleList_.front();
            pTask->msgToHandleList_.pop_front();

        }

        if (!pMsg_)
        {
                    // 2015-5-5
        if (pTask != NULL)
        {
            CMyLog::m_pLog->_XGSysLog("222222++++++++++++++++msg to handle %d!\n", pTask->msgToHandleList_.size());
        }
            continue;
        }
        
        // 2015-5-5
        if (pTask != NULL)
        {
            CMyLog::m_pLog->_XGSysLog("333333++++++++++++++++msg to handle %d!\n", pTask->msgToHandleList_.size());
        }

        //cast the pointer to Client_Handler*
        try
        {
            //printlog("+1\n");
            //CMyLog::m_pLog->_XGSysLog("begin HandleNetMessage!\n");
            pTask->HandleNetMessage(pMsg_);
            //CMyLog::m_pLog->_XGSysLog("end HandleNetMessage!\n");
            //printlog("-1\n");
        } catch (...)
        {
            CMyLog::m_pLog->_XGSysLog("crash Message : %0Xd ...\n", pMsg_->msg_head.msg_type);
            continue;
        }
    }
}

int CMSG_Center::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
    ACE_UNUSED_ARG(tv);
    ACE_UNUSED_ARG(arg);

    NET_PACKET_MSG* pMsg = NULL;
    get_message(pMsg);
    if (!pMsg)
    {
        return -1;
    }


    //    put(pMsg);
    return 0;
}

int CMSG_Center::get_message(NET_PACKET_MSG *&pMsg)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, msg_lock_, -1);

    pMsg = msgVec_[m_nMsgBufferCount];
    m_nMsgBufferCount++;
    if (m_nMsgBufferCount > MAX_BUFFERD_MESSAGE_NUMBER - 1)
    {
        m_nMsgBufferCount = 0;
    }

    return 0;
}

int CMSG_Center::put(NET_PACKET_MSG* pMsg)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, msg_handle_lock_, -1);
    msgToHandleList_.push_back(pMsg);

    if (msgToHandleList_.size() > 10)
    {
        CMyLog::m_pLog->_XGSysLog("--------msg to handle %d! \n", msgToHandleList_.size());
    }

    msg_semaphore_.release();
    return 0;
}

int CMSG_Center::init_message_buffer()
{

    msgVec_.reserve(MAX_BUFFERD_MESSAGE_NUMBER);
    int nError = errno;
    for (int i = 0; i < MAX_BUFFERD_MESSAGE_NUMBER; i++)
    {

        NET_PACKET_MSG* pMsg = new NET_PACKET_MSG;
        msgVec_.push_back(pMsg);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        select(0, 0, NULL, NULL, &tv);
    }

    return 0;
}

int CMSG_Center::HandleNetMessage(NET_PACKET_MSG *pPacket)
{

    if (!pPacket)
    {
        return -1;
    }

    switch (pPacket->msg_head.msg_type)
    {
        case SYS_MSG_SYSTEM_REGISTER_REQ:
            //CMyLog::m_pLog->_XGSysLog("begin handleDeviceRegister!\n");
            handleDeviceRegister(pPacket);
            //CMyLog::m_pLog->_XGSysLog("end handleDeviceRegister!\n");
            break;

        case SYS_MSG_SYSTEM_MSG_KEEPLIVE:
            //CMyLog::m_pLog->_XGSysLog("begin handleDeviceKeeplive!\n");
            handleDeviceKeeplive(pPacket);
            //CMyLog::m_pLog->_XGSysLog("end handleDeviceKeeplive!\n");
            break;

        case SYS_MSG_PUBLISH_EVENT:
            //CMyLog::m_pLog->_XGSysLog("begin handleEventData !\n");
            handleEventData(pPacket);
            //CMyLog::m_pLog->_XGSysLog("end handleEventData !\n");
            break;

        case SYS_MSG_SYSTEM_MSG_SEQUENCE_DISPATCH:
            //CMyLog::m_pLog->_XGSysLog("begin handleSysSequenceControl !\n");
            handleSysSequenceControl(pPacket);
            //CMyLog::m_pLog->_XGSysLog("end handleSysSequenceControl !\n");
            break;

        case SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS_ACK:
            //CMyLog::m_pLog->_XGSysLog("begin handleCustomsAck !\n");
            handleCustomsAck(pPacket);
            //CMyLog::m_pLog->_XGSysLog("end handleCustomsAck !\n");
            break;


        case SYS_MSG_TAKEOVER_DEVICE:
        case SYS_MSG_RETURN_DEVICE:
            //CMyLog::m_pLog->_XGSysLog("begin handleSystmSwitch !\n");
            handleSystmSwitch(pPacket);
            //CMyLog::m_pLog->_XGSysLog("end handleSystmSwitch !\n");
            break;


        case SYS_MSG_REGATHERDATA:
            //CMyLog::m_pLog->_XGSysLog("begin handleDeviceRegather !\n");
            handleDeviceRegather(pPacket);
            //CMyLog::m_pLog->_XGSysLog("end handleDeviceRegather !\n");
            break;


    }

    return 0;

}

bool CMSG_Center::IsUTF8(char *pBuffer, long size)
{
    bool IsUTF8 = true;
    unsigned char* start = (unsigned char*) pBuffer;
    unsigned char* end = (unsigned char*) pBuffer + size;
    while (start < end)
    {
        if (*start < 0x80)
        {
            start++;
        } else if (*start < (0xC0))
        {
            IsUTF8 = false;
            break;
        } else if (*start < (0xE0))
        {
            if (start >= end - 1)
                break;
            if ((start[1] & (0xC0)) != 0x80)
            {
                IsUTF8 = false;
                break;
            }
            start += 2;
        } else if (*start < (0xF0))
        {
            if (start >= end - 2)
                break;
            if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
            {
                IsUTF8 = false;
                break;
            }
            start += 3;
        } else
        {
            IsUTF8 = false;
            break;
        }
    }

    return IsUTF8;
}

int CMSG_Center::handleDeviceRegister(NET_PACKET_MSG *pPacket)
{
    if (!pPacket)
    {
        return -1;
    }

    int nProxyCount = pPacket->msg_head.proxy_count;
    if (nProxyCount < 1)
    {
        return -1;
    }

    int connect_handle = pPacket->msg_head.net_proxy[nProxyCount - 1];

    T_DeviceServerRegister_Req* pRegisterReq = (T_DeviceServerRegister_Req*) pPacket->msg_body;




    {

        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, device_lock_, -1);
        if (CSimpleConfig::m_StaDeviceMap[string(pRegisterReq->device_id)])
        {
            T_Gather_Device* pDevice = CSimpleConfig::m_StaDeviceMap[string(pRegisterReq->device_id)];

            if (!pDevice)
            {
                return -1;
            }

            if (strcmp(pRegisterReq->device_tag, pDevice->device_tag) != 0)
            {

                CMyLog::m_pLog->_XGSysLog("******** error!device tag is %s,config is %s\n", pRegisterReq->device_tag, pDevice->device_tag);
                return -1;
            }

            pDevice->connect_socket = connect_handle;
            pDevice->is_register = 1;
            pDevice->is_active = 1;
            pDevice->last_keeplive = time(NULL);


        }


    }


    //send ack 
    ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
    char chMsgAck[1024] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //锟斤拷头锟斤拷锟?
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_ACK;
    pHead->packet_len = sizeof (T_DeviceServerRegister_Ack);


    sendLen += sizeof (NET_PACKET_HEAD);

    T_DeviceServerRegister_Ack* pRegisterAck = (T_DeviceServerRegister_Ack*) (chMsgAck + sendLen);
    pRegisterAck->reg_result = 0;

    if (CSimpleConfig::IS_VIRTUAL == 0)
    {
        pRegisterAck->next_action = 1;
    } else
    {
        pRegisterAck->next_action = 0;
    }

    sendLen += sizeof (T_DeviceServerRegister_Ack);

    pHead->packet_len = sizeof (T_DeviceServerRegister_Ack);
    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
    int nRet = server.send_n(chMsgAck, sendLen, &expire_time);

    CMyLog::m_pLog->_XGSysLog("recv device %s register...\n", pRegisterReq->device_id);
}

int CMSG_Center::device_exit(int handle)
{

    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, device_lock_, -1);
    std::map<string, T_Gather_Device*>::iterator iter;
    for (iter = CSimpleConfig::m_StaDeviceMap.begin(); iter != CSimpleConfig::m_StaDeviceMap.end(); iter++)
    {
        T_Gather_Device* pDevice = iter->second;
        if (pDevice && pDevice->connect_socket == handle)
        {
            pDevice->last_keeplive = time(NULL);
            pDevice->is_active = 0;

            CMyLog::m_pLog->_XGSysLog("********device %s ollline...\n", pDevice->device_tag);
            break;
        }
    }


    return 0;
}

int CMSG_Center::handleDeviceKeeplive(NET_PACKET_MSG *pPacket)
{
    if (!pPacket)
    {
        return -1;
    }

    int nProxyCount = pPacket->msg_head.proxy_count;
    if (nProxyCount < 1)
    {
        return -1;
    }

    int connect_handle = pPacket->msg_head.net_proxy[nProxyCount - 1];

    if (connect_handle < 1)
    {
        return 0;
    }


    {

        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, device_lock_, -1);
        std::map<string, T_Gather_Device*>::iterator iter;
        for (iter = CSimpleConfig::m_StaDeviceMap.begin(); iter != CSimpleConfig::m_StaDeviceMap.end(); iter++)
        {
            T_Gather_Device* pDevice = iter->second;
            if (pDevice && pDevice->connect_socket == connect_handle)
            {
                pDevice->last_keeplive = time(NULL);
                pDevice->is_active = 1;
                break;
            }
        }
    }

    //send ack
    ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
    char chMsgAck[1024] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK;
    pHead->packet_len = 0;


    sendLen += sizeof (NET_PACKET_HEAD);

    pHead->packet_len = 0;
    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
    int nRet = server.send_n(chMsgAck, sendLen, &expire_time);

	{		
		struDeviceAndServiceStatus tmpStatus;
		tmpStatus.szLocalIP					= "";
		tmpStatus.nLocalPort				= 0;
		tmpStatus.szUpstreamIP				= CSimpleConfig::CENTER_CONTROLER_IP;
		tmpStatus.nUpstreamPort				= CSimpleConfig::CENTER_CONTROLER_PORT;
		tmpStatus.szUpstreamObjectCode		= "APP_CENTER_001";
		tmpStatus.nServiceType				= 1;
		tmpStatus.szObjectCode				= "APP_CHANNEL_001";
		tmpStatus.szObjectStatus			= "000";
		tmpStatus.szConnectedObjectCode		= "";
		char szNowTime[32]					= {0};
		g_ObjectStatus.GetCurTime(szNowTime);
		szNowTime[31]						= '\0';
		tmpStatus.szReportTime				= szNowTime;
		g_ObjectStatus.SetDeviceStatus(tmpStatus);
	}
}

int CMSG_Center::handleSystmSwitch(NET_PACKET_MSG *pPacket)
{
    if (!pPacket)
    {
        return -1;
    }

    if (pPacket->msg_head.msg_type == SYS_MSG_TAKEOVER_DEVICE)
    {
        CSimpleConfig::IS_VIRTUAL = 0;
    } else if (pPacket->msg_head.msg_type == SYS_MSG_RETURN_DEVICE)
    {
        CSimpleConfig::IS_VIRTUAL = 1;
    }


    CMyLog::m_pLog->_XGSysLog("channel virtual status to %d\n", CSimpleConfig::IS_VIRTUAL);


    char chMsgAck[1024] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //锟斤拷头锟斤拷锟?
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = pPacket->msg_head.msg_type;
    pHead->packet_len = 0;


    sendLen += sizeof (NET_PACKET_HEAD);

    pHead->packet_len = 0;
    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 200 * 1000);

    {
        if (CSimpleConfig::m_StaDeviceMap.size() > 0)
        {
            std::map<string, T_Gather_Device*>::iterator iter;
            for (iter = CSimpleConfig::m_StaDeviceMap.begin(); iter != CSimpleConfig::m_StaDeviceMap.end(); iter++)
            {
                T_Gather_Device* pGatherDevice = iter->second;
                if (pGatherDevice)
                {
                    SysUtil::SocketWrite(pGatherDevice->connect_socket, chMsgAck, sendLen, 50);
                }
            }
        }
    }

    return 0;
}

/*
 *接收设备服务发送的数据，但是不处理具体的事件类型，只是把数据缓存下来，继续放到事件队列，等待处理
 */
int CMSG_Center::handleEventData(NET_PACKET_MSG *pPacket)
{
    if (!pPacket)
    {
        return -1;
    }

    T_SysEventData* pEventData = (T_SysEventData*) pPacket->msg_body;


    //记录此TAG的数据是否完整
    if (CSimpleConfig::m_StaTagDeviceMap[string(pEventData->device_tag)])
    {
        // 2015-4-9 zhouhm
        if (pEventData->is_data_full == 0)
            CSimpleConfig::m_StaTagDeviceMap[string(pEventData->device_tag)]->is_data_full = 0;
        else
            CSimpleConfig::m_StaTagDeviceMap[string(pEventData->device_tag)]->is_data_full = 1;


    }


    CMyLog::m_pLog->_XGSysLog("recv event %s,%s data:\n", pEventData->event_id, pEventData->device_tag);
    CMyLog::m_pLog->_XGSysLog("%s\n", pEventData->xml_data);


    //移出异常处理列表 
    {
        CMyLog::m_pLog->_XGSysLog("erase exception list by recv eventid......\n");
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, exception_lock_, -1);
        std::vector<T_Sequence_Exception>::iterator iter;
        for (iter = m_ExceptionList.begin(); iter != m_ExceptionList.end(); iter++)
        {
            T_Sequence_Exception& exception = *iter;

            if (strcmp(exception.wait_event, pEventData->event_id) == 0)
            {
                m_ExceptionList.erase(iter);
                break;
            }
        }
    }


    if (strcmp(pEventData->event_id, "1600000002") == 0)
    {
        int nn = 0;
    }

    //更新各个时序的状态
    {
        CMyLog::m_pLog->_XGSysLog("update sequence status......\n");
        if (CSimpleConfig::m_StaSequenceControlVec.size() > 0)
        {
            for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
            {
                T_Sequence_Control* pSeqCtrl = CSimpleConfig::m_StaSequenceControlVec[i];

                for (int j = 0; j < pSeqCtrl->recv_event_num; j++)
                {
                    if (strcmp(pSeqCtrl->recv_event[j].event_id, pEventData->event_id) == 0)
                    {

                        pSeqCtrl->recv_event[j].is_recv_event = 1;
                    }
                }
            }
        }
    }





    //记录接收的数据
    {
        CMyLog::m_pLog->_XGSysLog("buffer recv data......\n");
        if (!m_CtrlDataMap[string(pEventData->device_tag)])
        {
            T_SysCtrlData* pSysCtrlData = new T_SysCtrlData;
            memset(pSysCtrlData, 0, sizeof (T_SysCtrlData));
            strcpy(pSysCtrlData->device_tag, pEventData->device_tag);
            pSysCtrlData->xml_data_len = 0;
            pSysCtrlData->has_data = 0;


            m_CtrlDataMap[string(pEventData->device_tag)] = pSysCtrlData;
        }

        T_SysCtrlData* pSysCtrlData = m_CtrlDataMap[string(pEventData->device_tag)];
        if (strlen(pEventData->xml_data) > 0)
        {



            pSysCtrlData->xml_data_len = strlen(pEventData->xml_data);
            strcpy(pSysCtrlData->xml_data, pEventData->xml_data);
            pSysCtrlData->has_data = 1;

            CMyLog::m_pLog->_XGSysLog("buffer %s:\n %s...\n", pEventData->device_tag, pEventData->xml_data);
        }



    }


    if (m_nRunState == REGATHER)
    {
        CMyLog::m_pLog->_XGSysLog("recv device %s regather data!......\n", pEventData->device_tag);
        handle_packet_data();
        return 0;
    }

    if (strcmp("OPTCAR", pEventData->device_tag) == 0)
    {
        handleVehiclePic(pEventData->xml_data);
    }    
    
    //查看某个事件的前置事件是否发生，
    if (strcmp("CONTA", pEventData->device_tag) == 0)
    {
        handleContaPic(pEventData->xml_data);
    }
  

    //查找当前的事件在时序中的位置
    int index_ = 0;
    if (CSimpleConfig::m_StaSequenceControlVec.size() > 0)
    {
        for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
        {
            T_Sequence_Control* pSeqCtrl = CSimpleConfig::m_StaSequenceControlVec[i];

            for (int j = 0; j < pSeqCtrl->recv_event_num; j++)
            {
                if (strcmp(pSeqCtrl->recv_event[j].event_id, pEventData->event_id) == 0)
                {
                    index_ = i;
                    goto INDEX_STEP;
                }
            }
        }
    }



INDEX_STEP:


    if (index_ > 0)
    {
        //        for (int i = 0; i < index_; i++)
        {
            T_Sequence_Control* pSeqCtrl = CSimpleConfig::m_StaSequenceControlVec[index_];

            if (pSeqCtrl->event_assoc == 1)
            {
                for (int j = 0; j < pSeqCtrl->recv_event_num; j++)
                {
                    if (pSeqCtrl->recv_event[j].is_recv_event == 0)
                    {
                        CMyLog::m_pLog->_XGSysLog("pre condition not occur!......\n");
                        return -1;
                    }

                }
            }

            if (pSeqCtrl->event_assoc == 0)
            {
                int recv_event = 0;
                for (int j = 0; j < pSeqCtrl->recv_event_num; j++)
                {
                    recv_event += pSeqCtrl->recv_event[j].is_recv_event;
                }

                if (recv_event == 0)
                {
                    CMyLog::m_pLog->_XGSysLog("pre condition not occur!......\n");
                    return -1;
                }
            }

        }
    }






    int send_event = 1;
    if (CSimpleConfig::m_StaSequenceControlVec.size() > 0)
    {
        for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
        {
            T_Sequence_Control* pSequence = CSimpleConfig::m_StaSequenceControlVec[i];

            int nEventIndex = -1;

            //根据接收的事件ID，找到对应的数据控制项
            for (int j = 0; j < pSequence->recv_event_num; j++)
            {
                if (strcmp(pSequence->recv_event[j].event_id, pEventData->event_id) == 0) //根据接收到的事件ID，查找对应的发送事件ID
                {
                    nEventIndex = j;
                    break;
                }
            }

            if (nEventIndex == -1)
            {
                continue;
            }

            //判断此控制项是否已经接收所有需要的事件,事件之间是或的关系,满足一个就可以
            if (pSequence->event_assoc == 0)
            {
                CMyLog::m_pLog->_XGSysLog("check event assoc -and | or ......\n", pEventData->event_id);

                int nHasRecv = 0;
                for (int m = 0; m < pSequence->recv_event_num; m++)
                {
                    if (pSequence->recv_event[m].is_recv_event == 1)
                    {
                        nHasRecv++;


                    }
                }

                if (nHasRecv == 0 || nHasRecv > 1)
                {
                    send_event = 0;
                    goto SEND_STEP;
                }
            }


            if (pSequence->event_assoc == 1)
            {
                CMyLog::m_pLog->_XGSysLog("check event assoc -and | or ......\n", pEventData->event_id);

                int nHasRecv = 0;
                for (int m = 0; m < pSequence->recv_event_num; m++)
                {
                    if (pSequence->recv_event[m].is_recv_event == 1)
                    {
                        nHasRecv++;


                    }
                }

                if (nHasRecv < pSequence->recv_event_num)
                {
                    send_event = 0;
                    goto SEND_STEP;
                }
            }



        }
    }


SEND_STEP:

    /*
    //更新各个时序的状态
    {
CMyLog::m_pLog->_XGSysLog("update sequence status......\n");
if (CSimpleConfig::m_StaSequenceControlVec.size() > 0)
{
    for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
    {
        T_Sequence_Control* pSeqCtrl = CSimpleConfig::m_StaSequenceControlVec[i];

        for (int j = 0; j < pSeqCtrl->recv_event_num; j++)
        {
            if (strcmp(pSeqCtrl->recv_event[j].event_id, pEventData->event_id) == 0)
            {

                pSeqCtrl->recv_event[j].is_recv_event = 1;
            }
        }
    }
}
    }
     */

    if (send_event == 0)
    {
        CMyLog::m_pLog->_XGSysLog("not put event %s to list......\n", pEventData->event_id);
        return 0;
    }




    NET_PACKET_MSG* pPacket_ = NULL;
    CMSG_Center::get_message(pPacket_);
    if (!pPacket_)
    {
        CMyLog::m_pLog->_XGSysLog("********can not get buffer ......\n");
        return -1;
    }

    memset(pPacket_, 0, sizeof (NET_PACKET_MSG));

    pPacket_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_SEQUENCE_DISPATCH; //将接收的数据，put到事件队列继续处理


    //保存接收的数据
    T_Sequence_ControlData* pSequenceData = (T_Sequence_ControlData*) pPacket_->msg_body;
    strcpy(pSequenceData->event_id, pEventData->event_id);
    put(pPacket_);

    CMyLog::m_pLog->_XGSysLog("put event %s to list......\n", pEventData->event_id);

    return 0;
}

int CMSG_Center::handleSysSequenceControl(NET_PACKET_MSG *pPacket)
{
    //syslog(LOG_DEBUG, "Enter handleSysSequenceControl\n");
    T_Sequence_ControlData* pSequenceData = (T_Sequence_ControlData*) pPacket->msg_body;
    pSequenceData->xml_len = 0;

    CMyLog::m_pLog->_XGSysLog("handleSysSequenceControl,event id %s...\n", pSequenceData->event_id);

    {
        std::vector<T_Sequence_Exception*>::iterator iter;
        for (iter = CSimpleConfig::m_StaSequenceExceptionVec.begin(); iter != CSimpleConfig::m_StaSequenceExceptionVec.end(); iter++)
        {
            T_Sequence_Exception* pException = *iter;

            if (strcmp(pException->recv_event, pSequenceData->event_id) == 0)
            {
                ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, exception_lock_, -1);
                T_Sequence_Exception exception;
                memcpy(&exception, pException, sizeof (T_Sequence_Exception));
                m_ExceptionList.push_back(exception);
                CMyLog::m_pLog->_XGSysLog("put exception event id %s to list...\n", pException->recv_event);
                break;
            }
        }
    }

    if (strcmp(pSequenceData->event_id, SYS_EVENT_INIT_STATUS) == 0)
    {
        init();
        //syslog(LOG_DEBUG, "2Leave handleSysSequenceControl\n");
        return 0;
    }

    //打包事件，对数据打包发送
    if (strcmp(pSequenceData->event_id, SYS_EVENT_PACKET_DATA) == 0)
    {
        CMyLog::m_pLog->_XGSysLog("handle event %s,packet data...\n", SYS_EVENT_PACKET_DATA);
        handle_packet_data();
        //syslog(LOG_DEBUG, "3Leave handleSysSequenceControl\n");
        return 0;
    }

    //根据时序配置处理
    handle_event(pSequenceData);
    //syslog(LOG_DEBUG, "Leave handleSysSequenceControl\n");
    return 0;


}

int CMSG_Center::handle_event(T_Sequence_ControlData* pSequenceData)
{
   // syslog(LOG_DEBUG, "Enter handle_event\n");
    if (CSimpleConfig::m_StaSequenceControlVec.size() > 0)
    {
        //syslog(LOG_DEBUG, "m_StaSequenceControlVec size=%d\n", CSimpleConfig::m_StaSequenceControlVec.size());
        for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
        {
            T_Sequence_Control* pSequence = CSimpleConfig::m_StaSequenceControlVec[i];

            int nEventIndex = -1;

            //根据接收的事件ID，找到对应的数据控制项
            for (int j = 0; j < pSequence->recv_event_num; j++)
            {
                if (strcmp(pSequence->recv_event[j].event_id, pSequenceData->event_id) == 0) //根据接收到的事件ID，查找对应的发送事件ID
                {
                    nEventIndex = j;
                    break;
                }
            }

            if (nEventIndex == -1)
            {
                continue;
            }


            //判断此控制项是否已经接收所有需要的事件
            int isAllRecv = 1;

            if (pSequence->event_assoc == 1) //事件之间是与的关系,必须全部满足
            {
                for (int m = 0; m < pSequence->recv_event_num; m++)
                {
                    if (pSequence->recv_event[m].is_recv_event == 0)
                    {
                        isAllRecv = 0;
                    }
                }
            } else //事件之间是或的关系,满足一个就可以
            {
                isAllRecv = 0;
                for (int m = 0; m < pSequence->recv_event_num; m++)
                {
                    if (pSequence->recv_event[m].is_recv_event == 1)
                    {
                        isAllRecv = 1;
                        break;
                    }
                }
            }

            if (isAllRecv != 1)
            {
                continue;
            }


            //已经接收到所有的控制事件，根据配置继续下一步的处理
            int nPostEventNum = pSequence->post_event_num;
            //syslog(LOG_DEBUG, "nPostEventNum=%d\n", nPostEventNum);
            for (int n = 0; n < nPostEventNum; n++)
            {
                if (strcmp(pSequence->post_event[n].post_tag, "CTRL") == 0)
                {
                    NET_PACKET_MSG* pPacket_ = NULL;
                    CMSG_Center::get_message(pPacket_);
                    if (!pPacket_)
                    {
                        //syslog(LOG_DEBUG, "1Leave handle_event\n");
                        return -1;
                    }

                    memset(pPacket_, 0, sizeof (NET_PACKET_MSG));

                    pPacket_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_SEQUENCE_DISPATCH;

                    T_Sequence_ControlData* pSequenceDataContinue = (T_Sequence_ControlData*) pPacket_->msg_body;
                    memcpy(pSequenceDataContinue, pSequenceData, sizeof (T_Sequence_ControlData) + pSequenceData->xml_len);

                    strcpy(pSequenceDataContinue->event_id, pSequence->post_event[n].event_id);

                    put(pPacket_);
                    //这里没有break，因为一个事件可能会引起几个动作，不仅仅是一个


                } else
                {

                    std::string strEventID = pSequence->post_event[n].event_id;
//                    if (strEventID == "EC_SHOW_LED")
//                    {
//                        int a = 10;
//                    }
                    if (pSequenceData->xml_len > 0)
                    {
                        post_event_to_device(pSequence->post_event[n].post_tag, pSequence->post_event[n].event_id, pSequenceData->xml_data, pSequenceData->xml_len);
                        CMyLog::m_pLog->_XGSysLog("post event to %s:%s\n", pSequence->post_event[n].post_tag, pSequence->post_event[n].event_id);

                    } else
                    {
                        post_event_to_device(pSequence->post_event[n].post_tag, pSequence->post_event[n].event_id);
                        CMyLog::m_pLog->_XGSysLog("post event to %s:%s\n", pSequence->post_event[n].post_tag, pSequence->post_event[n].event_id);
                    }

                    //syslog(LOG_DEBUG, "post event to %s:%s\n",pSequence->post_event[n].post_tag, pSequence->post_event[n].event_id);


                    {
                        std::vector<T_Sequence_Exception*>::iterator iter;
                        for (iter = CSimpleConfig::m_StaSequenceExceptionVec.begin(); iter != CSimpleConfig::m_StaSequenceExceptionVec.end(); iter++)
                        {
                            T_Sequence_Exception* pException = *iter;

                            if (strcmp(pException->recv_event, pSequence->post_event[n].event_id) == 0)
                            {
                                ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, exception_lock_, -1);
                                T_Sequence_Exception exception;
                                memcpy(&exception, pException, sizeof (T_Sequence_Exception));
                                m_ExceptionList.push_back(exception);
                                CMyLog::m_pLog->_XGSysLog("put exception event id %s to list...\n", pException->recv_event);
                                break;
                            }
                        }
                    }

                }


            }
        }
    }
    //syslog(LOG_DEBUG, "Leave handle_event\n");
}

int CMSG_Center::handle_device_ctrlevent(char* szEventID, char* szCtrlData)
{
    //syslog(LOG_DEBUG, "Enter handle_device_ctrlevent\n");
    if (CSimpleConfig::m_StaSequenceControlVec.size() > 0)
    {
        //syslog(LOG_DEBUG, "CSimpleConfig::m_StaSequenceControlVec.size():%d\n", CSimpleConfig::m_StaSequenceControlVec.size());
        for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
        {
            T_Sequence_Control* pSequence = CSimpleConfig::m_StaSequenceControlVec[i];

            int nEventIndex = -1;

            //根据接收的事件ID，找到对应的数据控制项
            for (int j = 0; j < pSequence->recv_event_num; j++)
            {
                if (strcmp(pSequence->recv_event[j].event_id, szEventID) == 0) //根据接收到的事件ID，查找对应的发送事件ID
                {
                    nEventIndex = j;
                    break;
                }
            }

            if (nEventIndex == -1)
            {
                continue;
            }

            //已经接收到所有的控制事件，根据配置继续下一步的处理
            int nPostEventNum = pSequence->post_event_num;
            //syslog(LOG_DEBUG, "in handle_device_ctrlevent nPostEventNum:%d\n", nPostEventNum);
            //for (int n = 0; n < nPostEventNum; n++)
            {
                //                post_event_to_device(pSequence->post_event[n].post_tag, pSequence->post_event[n].event_id, szCtrlData, c);
                //                CMyLog::m_pLog->_XGSysLog("post event to %s:%s\n", pSequence->post_event[n].post_tag, pSequence->post_event[n].event_id);


                char szSeqCtrl[20 * 1024] = {0};
                T_Sequence_ControlData* sequenceData = (T_Sequence_ControlData*) szSeqCtrl;


                strcpy(sequenceData->event_id, szEventID);
                strcpy(sequenceData->xml_data, szCtrlData);
                sequenceData->xml_len = strlen(szCtrlData);



                {

                    if (CSimpleConfig::m_StaSequenceControlVec.size() > 0)
                    {
                        for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
                        {
                            T_Sequence_Control* pSequence = CSimpleConfig::m_StaSequenceControlVec[i];

                            int nEventIndex = -1;

                            //根据接收的事件ID，找到对应的数据控制项
                            for (int j = 0; j < pSequence->recv_event_num; j++)
                            {
                                if (strcmp(pSequence->recv_event[j].event_id, sequenceData->event_id) == 0) //根据接收到的事件ID，查找对应的发送事件ID
                                {

                                    pSequence->recv_event[j].is_recv_event = 1;
                                    nEventIndex = j;
                                    break;
                                }
                            }

                            if (nEventIndex == -1)
                            {
                                continue;
                            }

                        }

                    }



                    handle_event(sequenceData);

                    SysUtil::SysSleep(100);


                }
            }
        }
    }
    //syslog(LOG_DEBUG, "Leave handle_device_ctrlevent\n");
}

int CMSG_Center::GetATimeStamp(char* szBuf, int nMaxLength)
{

    if (!szBuf)
    {
        return -1;
    }

    strcpy(szBuf, "");


    struct tm* tmNow;
    struct timeval tv;
    ::gettimeofday(&tv, NULL);

    tmNow = localtime(&tv.tv_sec);
    int nLen = sprintf(szBuf, "%04d%02d%02d%02d%02d%02d%03d",
            tmNow->tm_year + 1900,
            tmNow->tm_mon + 1,
            tmNow->tm_mday,
            tmNow->tm_hour,
            tmNow->tm_min,
            tmNow->tm_sec,
            tv.tv_usec / 1000
            );
    if (nLen > nMaxLength)
    {
        nLen = nMaxLength;
        szBuf[nLen - 1] = '\0';
    }

    return 0;
}

int CMSG_Center::send_packet(char* szSequenceNo, char* szXMLGatherInfo)
{
    CMyLog::m_pLog->_XGSysLog("send data to platform %s...\n", szSequenceNo);
    char szSendPacket[10 * 1024] = {0};
    szSendPacket[0] = 0XE2;
    szSendPacket[1] = 0X5C;
    szSendPacket[2] = 0X4B;
    szSendPacket[3] = 0X89;
    int nLen = 4;

    T_Len* pTotalLen = (T_Len*) (szSendPacket + nLen);
    nLen += sizeof (int);

    szSendPacket[nLen] = 0X21;
    nLen += sizeof (char);

    memcpy(szSendPacket + nLen, CSimpleConfig::GATHER_AREA_ID, 10);
    nLen += 10;

    memcpy(szSendPacket + nLen, CSimpleConfig::GATHER_CHANNE_NO, 10);
    nLen += 10;

    if (strcmp(CSimpleConfig::GATHER_I_E_TYPE, "I") == 0)
    {
        szSendPacket[nLen] = 'I';
    } else
    {
        szSendPacket[nLen] = 'E';
    }
    nLen += 1;


    szSendPacket[nLen] = 0X00;
    nLen += 1;

    szSendPacket[nLen] = 0X00;
    nLen += 1;

    szSendPacket[nLen] = 0X00;
    nLen += 1;

    szSendPacket[nLen] = 0X00;
    nLen += 1;


    int nXMLLen = strlen(szXMLGatherInfo) + 1;
    T_Len* pXMLLen = (T_Len*) (szSendPacket + nLen);
    pXMLLen->length = nXMLLen;
    pTotalLen->length = 40 + nXMLLen;

    nLen += 4;

    strcpy(szSendPacket + nLen, szXMLGatherInfo);
    nLen += nXMLLen;

    szSendPacket[nLen] = 0XFF;
    nLen += 1;
    szSendPacket[nLen] = 0XFF;
    nLen += 1;


    if (cascade_socket < 1)
    {
        return -1;
    }

    int send_socket = cascade_socket;


    NET_PACKET_MSG* pPacket_ = NULL;
    CMSG_Center::get_message(pPacket_);
    if (!pPacket_)
    {
        return -1;
    }

    memset(pPacket_, 0, sizeof (NET_PACKET_MSG));
    pPacket_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS;
    pPacket_->msg_head.packet_len = sizeof (T_Upload_Customs_Data) + nLen;

    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pPacket_->msg_body;
    sprintf(pCustomsData->szChannelNo, "%s%s%s", CSimpleConfig::GATHER_AREA_ID, CSimpleConfig::GATHER_CHANNE_NO, CSimpleConfig::GATHER_I_E_TYPE);
    strcpy(pCustomsData->szSequenceNo, szSequenceNo);
    pCustomsData->nCustomsDataLen = nLen;
    memcpy(&pCustomsData->szCustomsData, szSendPacket, nLen);

    //是否接收到所有的tag的数据,以及是否存在数据不完整
    pCustomsData->is_data_full = 0;


    {
        std::map<string, T_Gather_Device*>::iterator iter;
        for (iter = CSimpleConfig::m_StaDeviceMap.begin(); iter != CSimpleConfig::m_StaDeviceMap.end(); iter++)
        {
            T_Gather_Device* pGatherDevice = iter->second;
            if (pGatherDevice && pGatherDevice->is_capture_device == 1)
            {
                if (pGatherDevice->is_data_full == 1)
                {
                    //pCustomsData->is_data_full = 1; // 20150924
                    break;
                }
            }
        }
    }



    //--------------------------------
    char chBuf[32] = {0};
    memcpy(chBuf, SYS_NET_MSGHEAD, 8);
    
    struct timeval timeout = {5, 0};
    setsockopt(send_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));

    int nRet = SysUtil::SocketWrite(send_socket, chBuf, 8, 50);

    nRet += SysUtil::SocketWrite(send_socket, (char*) pPacket_, sizeof (NET_PACKET_HEAD) + pPacket_->msg_head.packet_len, 50);

    memcpy(chBuf, SYS_NET_MSGTAIL, 8);
    nRet += SysUtil::SocketWrite(send_socket, chBuf, 8, 50);

    if (nRet == 8 + 8 + sizeof (NET_PACKET_HEAD) + pPacket_->msg_head.packet_len)
    {
        CMyLog::m_pLog->_XGSysLog("send to centerchannelmanager server %s succ...\n", szSequenceNo);
    } else //数据发送不成功
    {
        CMyLog::m_pLog->_XGSysLog("********send  fail...\n");
        CCascadeCmdHandler::m_bConnected = false;
    }


    m_nRunState = WAITRESULT;

    return 0;
}


int CMSG_Center::send_packet_test(const char* szSequenceNo, const char* szXMLGatherInfo)
{
    CMyLog::m_pLog->_XGSysLog("send data to platform %s...\n", szSequenceNo);
    char szSendPacket[10 * 1024] = {0};
    szSendPacket[0] = 0XE2;
    szSendPacket[1] = 0X5C;
    szSendPacket[2] = 0X4B;
    szSendPacket[3] = 0X89;
    int nLen = 4;

    T_Len* pTotalLen = (T_Len*) (szSendPacket + nLen);
    nLen += sizeof (int);

    szSendPacket[nLen] = 0X21;
    nLen += sizeof (char);

    memcpy(szSendPacket + nLen, CSimpleConfig::GATHER_AREA_ID, 10);
    nLen += 10;

    memcpy(szSendPacket + nLen, CSimpleConfig::GATHER_CHANNE_NO, 10);
    nLen += 10;

    if (strcmp(CSimpleConfig::GATHER_I_E_TYPE, "I") == 0)
    {
        szSendPacket[nLen] = 'I';
    } else
    {
        szSendPacket[nLen] = 'E';
    }
    nLen += 1;


    szSendPacket[nLen] = 0X00;
    nLen += 1;

    szSendPacket[nLen] = 0X00;
    nLen += 1;

    szSendPacket[nLen] = 0X00;
    nLen += 1;

    szSendPacket[nLen] = 0X00;
    nLen += 1;


    int nXMLLen = strlen(szXMLGatherInfo) + 1;
    T_Len* pXMLLen = (T_Len*) (szSendPacket + nLen);
    pXMLLen->length = nXMLLen;
    pTotalLen->length = 40 + nXMLLen;

    nLen += 4;

    strcpy(szSendPacket + nLen, szXMLGatherInfo);
    nLen += nXMLLen;

    szSendPacket[nLen] = 0x00; // 0XFF;
    nLen += 1;
    szSendPacket[nLen] = 0x00; // 0XFF;
    nLen += 1;


    if (cascade_socket < 1)
    {
        return -1;
    }

    int send_socket = cascade_socket;


    NET_PACKET_MSG* pPacket_ = NULL;
    CMSG_Center::get_message(pPacket_);
    if (!pPacket_)
    {
        return -1;
    }

    memset(pPacket_, 0, sizeof (NET_PACKET_MSG));
    pPacket_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS;
    pPacket_->msg_head.packet_len = sizeof (T_Upload_Customs_Data) + nLen;

    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pPacket_->msg_body;
    sprintf(pCustomsData->szChannelNo, "%s%s%s", CSimpleConfig::GATHER_AREA_ID, CSimpleConfig::GATHER_CHANNE_NO, CSimpleConfig::GATHER_I_E_TYPE);
    strcpy(pCustomsData->szSequenceNo, szSequenceNo);
    pCustomsData->nCustomsDataLen = nLen;
    memcpy(&pCustomsData->szCustomsData, szSendPacket, nLen);

    //是否接收到所有的tag的数据,以及是否存在数据不完整
    pCustomsData->is_data_full = 0;


    {
        std::map<string, T_Gather_Device*>::iterator iter;
        for (iter = CSimpleConfig::m_StaDeviceMap.begin(); iter != CSimpleConfig::m_StaDeviceMap.end(); iter++)
        {
            T_Gather_Device* pGatherDevice = iter->second;
            if (pGatherDevice && pGatherDevice->is_capture_device == 1)
            {
                if (pGatherDevice->is_data_full == 1)
                {
                    //pCustomsData->is_data_full = 1; // 20150924
                    break;
                }
            }
        }
    }



    //--------------------------------
    char chBuf[32] = {0};
    memcpy(chBuf, SYS_NET_MSGHEAD, 8);
    
    struct timeval timeout = {5, 0};
    setsockopt(send_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));

    int nRet = SysUtil::SocketWrite(send_socket, chBuf, 8, 50);

    nRet += SysUtil::SocketWrite(send_socket, (char*) pPacket_, sizeof (NET_PACKET_HEAD) + pPacket_->msg_head.packet_len, 50);

    memcpy(chBuf, SYS_NET_MSGTAIL, 8);
    nRet += SysUtil::SocketWrite(send_socket, chBuf, 8, 50);

    if (nRet == 8 + 8 + sizeof (NET_PACKET_HEAD) + pPacket_->msg_head.packet_len)
    {
        CMyLog::m_pLog->_XGSysLog("send to centerchannelmanager server %s succ...\n", szSequenceNo);
    } else //数据发送不成功
    {
        CMyLog::m_pLog->_XGSysLog("********send  fail...\n");
        CCascadeCmdHandler::m_bConnected = false;
    }


    m_nRunState = WAITRESULT;

    return 0;
}

int CMSG_Center::packet_gather_data(char* szXMLGatherInfo)
{
    //不是补采的数据，生成一个新的ID
    if (strcmp(m_szSequenceID, "") == 0)
    {
        GetATimeStamp(m_szSequenceID, 32);

        srand(time(0));
        int nRand = rand() % 1000 + 1;
        sprintf(m_szSequenceID, "%s%03d", m_szSequenceID, nRand);

    }
    
    std::string GATHER_CHNL_TYPE = "";
    if (CSimpleConfig::GATHER_CHNL_TYPE[0] != '\0')
        GATHER_CHNL_TYPE = CSimpleConfig::GATHER_CHNL_TYPE;


    sprintf(szXMLGatherInfo, "<GATHER_INFO AREA_ID=\"%s\" CHNL_NO=\"%s\" I_E_TYPE=\"%s\" SEQ_NO=\"%s\" CHNL_TYPE=\"%s\">",
            CSimpleConfig::GATHER_AREA_ID, CSimpleConfig::GATHER_CHANNE_NO, CSimpleConfig::GATHER_I_E_TYPE, m_szSequenceID, GATHER_CHNL_TYPE.c_str());

    if (m_pDataPack)
    {
        m_pDataPack->SetHeadData(szXMLGatherInfo);
    }



    std::map<string, T_SysCtrlData*>::iterator iter;
    for (iter = m_CtrlDataMap.begin(); iter != m_CtrlDataMap.end(); iter++)
    {
        T_SysCtrlData* pSysCtrlData = iter->second;

        if (pSysCtrlData && pSysCtrlData->has_data == 1)
        {
            if (m_pDataPack)
            {
                m_pDataPack->SetTagData(pSysCtrlData->device_tag, m_CtrlDataMap[string(pSysCtrlData->device_tag)]->xml_data);

                CMyLog::m_pLog->_XGSysLog("prepare packet data %s\n %s\n", pSysCtrlData->device_tag, m_CtrlDataMap[string(pSysCtrlData->device_tag)]->xml_data);
            }
        }

    }

    if (m_pDataPack)
    {
        CMyLog::m_pLog->_XGSysLog("XML data is \n %s\n", szXMLGatherInfo);
        m_pDataPack->PacketData(szXMLGatherInfo);
    }

    return 0;
}

int CMSG_Center::handle_packet_data()
{
    char szXMLGatherInfo[10 * 1024] = {0};

    packet_gather_data(szXMLGatherInfo);
    send_packet(m_szSequenceID, szXMLGatherInfo);


    SysUtil::SysSleep(500);

    if (m_SysContaPicInfo.pic_num > 0)
    {
        send_conta_pic();
    }

}

int CMSG_Center::handleCustomsAck(NET_PACKET_MSG * pPacket)
{
    //syslog(LOG_DEBUG, "Enter handleCustomsAck\n");
    if (!pPacket)
    {
        //syslog(LOG_DEBUG, "1Leave handleCustomsAck\n");
        return -1;
    }

    {
        CMyLog::m_pLog->_XGSysLog("sizeof m_ExceptionList %d\n", m_ExceptionList.size());
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, exception_lock_, -1);
        m_ExceptionList.clear();

        CMyLog::m_pLog->_XGSysLog("sizeof m_ExceptionList %d\n", m_ExceptionList.size());
    }


    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pPacket->msg_body;
    int nXMLLen = pCustomsData->nCustomsDataLen;

    CMyLog::m_pLog->_XGSysLog("handle ack from platform %s,packet data...\n", pCustomsData->szSequenceNo);

    CMyLog::m_pLog->_XGSysLog("%s\n", pCustomsData->szCustomsData);

    TiXmlDocument xml;
    xml.Parse(pCustomsData->szCustomsData);
    TiXmlElement *root = xml.RootElement();
    if (root == NULL)
    {
        init();
        CMyLog::m_pLog->_XGSysLog("********parse customs ack root node  fail...\n");
        //syslog(LOG_DEBUG, "2Leave handleCustomsAck\n");
        return -1;
    }

    TiXmlElement *item_event = root->FirstChildElement("EVENT");
    if (!item_event)
    {
        init();
        //syslog(LOG_DEBUG, "3Leave handleCustomsAck\n");
        return -1;
    }

    char* pEventType = (char*) item_event->Attribute("type");
    char* pHint = (char*) item_event->Attribute("hint");
    char* pEventID = (char*) item_event->Attribute("eventid");
    char* pCtrlData = (char*) item_event->GetText();

    char szEventID[32] = {0};
    char szCtrlData[4096] = {0};

    if (pEventID)
    {
        strcpy(szEventID, pEventID);
    }

    if (pCtrlData)
    {
        strcpy(szCtrlData, pCtrlData);
    }

    //syslog(LOG_DEBUG, "1Process handle_device_ctrlevent begin\n");
    handle_device_ctrlevent(szEventID, szCtrlData);
    //syslog(LOG_DEBUG, "1Process handle_device_ctrlevent end\n");

    while ((item_event = item_event->NextSiblingElement()) != NULL)
    {
        char* pEventType = (char*) item_event->Attribute("type");
        char* pHint = (char*) item_event->Attribute("hint");
        char* pEventID = (char*) item_event->Attribute("eventid");
        char* pCtrlData = (char*) item_event->GetText();

        char szEventID[32] = {0};
        char szCtrlData[4096] = {0};

        if (pEventID)
        {
            strcpy(szEventID, pEventID);
        }

        if (pCtrlData)
        {
            strcpy(szCtrlData, pCtrlData);
        }

        //syslog(LOG_DEBUG, "2Process handle_device_ctrlevent begin\n");
        handle_device_ctrlevent(szEventID, szCtrlData);
        //syslog(LOG_DEBUG, "2Process handle_device_ctrlevent end\n");

    }

    init();

}

//事件发送处理，根据设备的TAG，发送到相应的设备

int CMSG_Center::post_event_to_device(char* device_tag, char* event_id)
{
    if (strcmp(device_tag, "CTRL") == 0) //CTRL默认为本地控制处理
    {
        return 0;
    }

    int device_connect_handle = -1;

    {
        //ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, device_lock_, -1);
        T_Gather_Device* pDevice = NULL;
        CMSG_Center::device_lock_.acquire();
        pDevice = CSimpleConfig::m_StaTagDeviceMap[string(device_tag)];
        CMSG_Center::device_lock_.release();
        if (!pDevice)
        {
            CMyLog::m_pLog->_XGSysLog("********error!cannot find device connect handle for tag %s\n", device_tag);
            return -1;
        }


        if (pDevice->is_active == 0)
        {
            CMyLog::m_pLog->_XGSysLog("********device  %s offline......\n", device_tag);

			{		
				struDeviceAndServiceStatus tmpStatus;
				tmpStatus.szLocalIP					= "";
				tmpStatus.nLocalPort				= 0;
				tmpStatus.szUpstreamIP				= CSimpleConfig::CENTER_CONTROLER_IP;
				tmpStatus.nUpstreamPort				= CSimpleConfig::CENTER_CONTROLER_PORT;
				tmpStatus.szUpstreamObjectCode		= "APP_CENTER_001";
				tmpStatus.nServiceType				= 1;
				tmpStatus.szObjectCode				= "APP_CHANNEL_001";
				tmpStatus.szObjectStatus			= "000";
				tmpStatus.szConnectedObjectCode		= string(device_tag);
				char szNowTime[32]					= {0};
				g_ObjectStatus.GetCurTime(szNowTime);
				szNowTime[31]						= '\0';
				tmpStatus.szReportTime				= szNowTime;
				g_ObjectStatus.SetDeviceStatus(tmpStatus);
			}
            return -1;
        }

        device_connect_handle = pDevice->connect_socket;


    }



    ACE_SOCK_Stream server((ACE_HANDLE) device_connect_handle);
    char chMsgAck[1024] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_CTRL;
    pHead->packet_len = sizeof (T_Sys_Ctrl);


    sendLen += sizeof (NET_PACKET_HEAD);

    T_Sys_Ctrl* pCtrlData = (T_Sys_Ctrl*) (chMsgAck + sendLen);
    strcpy(pCtrlData->event_id, event_id);
    pCtrlData->xml_len = 0;
    sendLen += sizeof (T_Sys_Ctrl);

    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
    int nRet = server.send_n(chMsgAck, sendLen, &expire_time);


    return 0;
}

int CMSG_Center::post_event_to_device(char* device_tag, char* event_id, char* xml_data, int nLen)
{
    if (strcmp(device_tag, "CTRL") == 0) //CTRL默认为本地控制处理
    {
        return 0;
    }


    int device_connect_handle = -1;

    {
        //ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, device_lock_, -1);
        T_Gather_Device* pDevice = NULL;
        CMSG_Center::device_lock_.acquire();
        pDevice = CSimpleConfig::m_StaTagDeviceMap[string(device_tag)];
        CMSG_Center::device_lock_.release();
        if (!pDevice)
        {
            CMyLog::m_pLog->_XGSysLog("********error!cannot find device connect handle for tag %s\n", device_tag);
            return -1;
        }

        if (pDevice->is_active == 0)
        {
            CMyLog::m_pLog->_XGSysLog("********device  %s offline......\n", device_tag);

			{		
				struDeviceAndServiceStatus tmpStatus;
				tmpStatus.szLocalIP					= "";
				tmpStatus.nLocalPort				= 0;
				tmpStatus.szUpstreamIP				= CSimpleConfig::CENTER_CONTROLER_IP;
				tmpStatus.nUpstreamPort				= CSimpleConfig::CENTER_CONTROLER_PORT;
				tmpStatus.szUpstreamObjectCode		= "APP_CENTER_001";
				tmpStatus.nServiceType				= 1;
				tmpStatus.szObjectCode				= "APP_CHANNEL_001";
				tmpStatus.szObjectStatus			= "000";
				tmpStatus.szConnectedObjectCode		= string(device_tag);
				char szNowTime[32]					= {0};
				g_ObjectStatus.GetCurTime(szNowTime);
				szNowTime[31]						= '\0';
				tmpStatus.szReportTime				= szNowTime;
				g_ObjectStatus.SetDeviceStatus(tmpStatus);
			}

            return -1;
        }

        device_connect_handle = pDevice->connect_socket;
    }



    ACE_SOCK_Stream server((ACE_HANDLE) device_connect_handle);
    char chMsgAck[1024 * 10] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_CTRL;
    pHead->packet_len = sizeof (T_Sys_Ctrl) + nLen;


    sendLen += sizeof (NET_PACKET_HEAD);

    T_Sys_Ctrl* pCtrlData = (T_Sys_Ctrl*) (chMsgAck + sendLen);
    strcpy(pCtrlData->event_id, event_id);
    pCtrlData->xml_len = nLen;
    strcpy(pCtrlData->xml_data, xml_data);
    sendLen += sizeof (T_Sys_Ctrl) + nLen;

    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
    int nRet = server.send_n(chMsgAck, sendLen, &expire_time);


    return 0;

}

int CMSG_Center::init()
{

    m_nRunState = NORMAL;
    memset(&m_szSequenceID, 0, 32);

    {
        std::map<string, T_Gather_Device*>::iterator iter;
        for (iter = CSimpleConfig::m_StaTagDeviceMap.begin(); iter != CSimpleConfig::m_StaTagDeviceMap.end(); iter++)
        {
            T_Gather_Device* pGatherDevice = iter->second;
            if (pGatherDevice && pGatherDevice->is_capture_device == 1)
            {
                pGatherDevice->is_data_full == 1;
            }
        }
    }


    {
        std::map<string, T_Gather_Device*>::iterator iter;
        for (iter = CSimpleConfig::m_StaDeviceMap.begin(); iter != CSimpleConfig::m_StaDeviceMap.end(); iter++)
        {
            T_Gather_Device* pGatherDevice = iter->second;
            if (pGatherDevice)
            {
                pGatherDevice->is_data_full = 1;

            }
        }
    }


    {
        for (int i = 0; i < CSimpleConfig::m_StaSequenceControlVec.size(); i++)
        {
            T_Sequence_Control* pSequence = CSimpleConfig::m_StaSequenceControlVec[i];

            for (int m = 0; m < pSequence->recv_event_num; m++)
            {
                pSequence->recv_event[m].is_recv_event = 0;
            }
        }
    }






    {

        std::map<string, T_SysCtrlData*>::iterator iter;
        for (iter = m_CtrlDataMap.begin(); iter != m_CtrlDataMap.end(); iter++)
        {
            T_SysCtrlData* pSysCtrlData = iter->second;

            if (pSysCtrlData)
            {

                pSysCtrlData->has_data = 0;
                pSysCtrlData->xml_data_len = 0;
                strcpy(pSysCtrlData->xml_data, "");

            }

        }
    }



    return 0;
}

int CMSG_Center::CheckException()
{

    while (1)
    {
        if (m_ExceptionList.size() > 0)
        {
            ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, exception_lock_, -1);

            std::vector<T_Sequence_Exception>::iterator iter;
            for (iter = m_ExceptionList.begin(); iter != m_ExceptionList.end(); iter++)
            {
                T_Sequence_Exception& exception = *iter;
                exception.wait_time -= 20;
                if (exception.wait_time <= 0)
                {

                    publish_exception_event(exception.post_event);

                    m_ExceptionList.erase(iter);
                    break;
                }
            }
        }

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 20000;
        select(0, 0, NULL, NULL, &tv);

    }


    return 0;
}

int CMSG_Center::publish_exception_event(char* event_id)
{
    NET_PACKET_MSG* pPacket_ = NULL;
    CMSG_Center::get_message(pPacket_);
    if (!pPacket_)
    {
        return -1;
    }

    memset(pPacket_, 0, sizeof (NET_PACKET_MSG));

    pPacket_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_SEQUENCE_DISPATCH; //将接收的数据，put到事件队列继续处理

    //保存接收的数据
    T_Sequence_ControlData* pSequenceData = (T_Sequence_ControlData*) pPacket_->msg_body;
    strcpy(pSequenceData->event_id, event_id);
    put(pPacket_);


    CMyLog::m_pLog->_XGSysLog("********publish exception event  %s\n", event_id);


}

int CMSG_Center::handleDeviceRegather(NET_PACKET_MSG * pPacket)
{

    if (!pPacket)
    {
        return -1;
    }

    NET_REGATHER* pRegatherData = (NET_REGATHER*) pPacket->msg_body;
    post_event_to_device(pRegatherData->szDeviceTag, "regather00");

    m_nRunState = REGATHER;

    return 0;
}

int CMSG_Center::handleContaPic(char* szXML)
{
    if (!szXML)
    {
        return -1;
    }


    memset(&m_SysContaPicInfo, 0, sizeof (T_Sys_ContaPicInfo));

    int nPicNum = 0;


    TiXmlDocument xml;
    xml.Parse(szXML);

    TiXmlElement *root = xml.RootElement();
    if (root == NULL)
    {
        return -1;
    }

    TiXmlElement *item_pic = root->NextSiblingElement();

    if (!item_pic)
    {
        return -1;
    }


    TiXmlElement *item_f_pic = item_pic->FirstChildElement("CONTA_PIC_F");

    if (item_f_pic)
    {

        char* pText = (char*) item_f_pic->GetText();
        if (pText)
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, pText);
        } else
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, "");
        }

        if (m_SysContaPicInfo.pic_info[nPicNum].pic_path[0] != '\0')
        {
            m_SysContaPicInfo.pic_info[nPicNum].pic_type = 0;

            nPicNum++;
        }

    }


    TiXmlElement *item_b_pic = item_pic->FirstChildElement("CONTA_PIC_B");

    if (item_b_pic)
    {

        char* pText = (char*) item_b_pic->GetText();
        if (pText)
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, pText);
        } else
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, "");
        }

        if (m_SysContaPicInfo.pic_info[nPicNum].pic_path[0] != '\0')
        {
            m_SysContaPicInfo.pic_info[nPicNum].pic_type = 3;

            nPicNum++;
        }

    }



    TiXmlElement *item_lf_pic = item_pic->FirstChildElement("CONTA_PIC_LF");

    if (item_lf_pic)
    {

        char* pText = (char*) item_lf_pic->GetText();
        if (pText)
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, pText);
        } else
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, "");
        }

        if (m_SysContaPicInfo.pic_info[nPicNum].pic_path[0] != '\0')
        {
            m_SysContaPicInfo.pic_info[nPicNum].pic_type = 1;

            nPicNum++;
        }

    }



    TiXmlElement *item_rf_pic = item_pic->FirstChildElement("CONTA_PIC_RF");

    if (item_rf_pic)
    {

        char* pText = (char*) item_rf_pic->GetText();
        if (pText)
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, pText);
        } else
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, "");
        }

        if (m_SysContaPicInfo.pic_info[nPicNum].pic_path[0] != '\0')
        {
            m_SysContaPicInfo.pic_info[nPicNum].pic_type = 2;

            nPicNum++;
        }

    }




    TiXmlElement *item_lb_pic = item_pic->FirstChildElement("CONTA_PIC_LB");

    if (item_lb_pic)
    {

        char* pText = (char*) item_lb_pic->GetText();
        if (pText)
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, pText);
        } else
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, "");
        }

        if (m_SysContaPicInfo.pic_info[nPicNum].pic_path[0] != '\0')
        {
            m_SysContaPicInfo.pic_info[nPicNum].pic_type = 4;

            nPicNum++;
        }

    }



    TiXmlElement *item_rb_pic = item_pic->FirstChildElement("CONTA_PIC_RB");

    if (item_rb_pic)
    {

        char* pText = (char*) item_rb_pic->GetText();
        if (pText)
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, pText);
        } else
        {
            strcpy(m_SysContaPicInfo.pic_info[nPicNum].pic_path, "");
        }

        if (m_SysContaPicInfo.pic_info[nPicNum].pic_path[0] != '\0')
        {
            m_SysContaPicInfo.pic_info[nPicNum].pic_type = 5;

            nPicNum++;
        }
    }


    m_SysContaPicInfo.pic_num = nPicNum;


    return 0;
}

int CMSG_Center::handleVehiclePic(char* szXML)
{
    if (!szXML)
    {
        return -1;
    }


    memset(&m_SysVehiclePicInfo, 0, sizeof (m_SysVehiclePicInfo));

    int nPicNum = 0;


    TiXmlDocument xml;
    xml.Parse(szXML);

    TiXmlElement *root = xml.RootElement();
    if (root == NULL)
    {
        return -1;
    }

    TiXmlElement *item_pic = root->NextSiblingElement();

    if (!item_pic)
    {
        return -1;
    }


    TiXmlElement *item_vehicle_pic = item_pic->FirstChildElement("PCAR_PICNAME");

    if (item_vehicle_pic)
    {

        char* pText = (char*) item_vehicle_pic->GetText();
        if (pText)
        {
            strcpy(m_SysVehiclePicInfo.vehicle_pic_info[nPicNum].pic_path, pText);
        } 
        else
        {
            strcpy(m_SysVehiclePicInfo.vehicle_pic_info[nPicNum].pic_path, "");
        }

        if (m_SysVehiclePicInfo.vehicle_pic_info[nPicNum].pic_path[0] != '\0')
        {
            m_SysVehiclePicInfo.vehicle_pic_info[nPicNum].pic_type = 6;

            nPicNum++;
        }

    }
    
    m_SysVehiclePicInfo.pic_num = nPicNum;


    return 0;
}

int CMSG_Center::send_conta_pic()
{

    if (cascade_socket < 1)
    {
        return -1;
    }


    ACE_SOCK_Stream server((ACE_HANDLE) cascade_socket);
    char* chMsgAck = m_pSendPicBuffer;

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8);
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);
    pHead->msg_type = SYS_MSG_SEND_PICDATA;
    pHead->packet_len = sizeof (T_Upload_Pic_Data);
    sendLen += sizeof (NET_PACKET_HEAD);

    T_Upload_Pic_Data* pUploadPicData = (T_Upload_Pic_Data*) (chMsgAck + sendLen);

    pUploadPicData->nPicNum = m_SysContaPicInfo.pic_num + m_SysVehiclePicInfo.pic_num;
    strncpy(pUploadPicData->szSequenceNo, m_szSequenceID, strlen(m_szSequenceID));
    strncpy(pUploadPicData->szChannelNo, CSimpleConfig::GATHER_CHANNE_NO, strlen(CSimpleConfig::GATHER_CHANNE_NO));
    strncpy(pUploadPicData->szAreaNo, CSimpleConfig::GATHER_AREA_ID, strlen(CSimpleConfig::GATHER_AREA_ID));
    strncpy(pUploadPicData->szIEFlag, CSimpleConfig::GATHER_I_E_TYPE, strlen(CSimpleConfig::GATHER_I_E_TYPE));
    



    int nOffSet = 0;
    int i = 0;
    for (i = 0; i < m_SysContaPicInfo.pic_num; i++)
    {
        pUploadPicData->picInfo[i].nOffSet = nOffSet;
        pUploadPicData->picInfo[i].nPicType = m_SysContaPicInfo.pic_info[i].pic_type;


        int nFileLen = 0;
        FILE* pImageFile = fopen(m_SysContaPicInfo.pic_info[i].pic_path, "rb");
        if (!pImageFile)
        {
            continue;
        }

        fseek(pImageFile, 0, SEEK_END);
        nFileLen = ftell(pImageFile);
        fseek(pImageFile, 0, SEEK_SET);

        if (nFileLen < 1)
        {
            continue;
        }

        //fread(pUploadPicData->szPicData + nOffSet, 1, nFileLen, pImageFile);
        ReadAll(pImageFile, pUploadPicData->szPicData + nOffSet, nFileLen);

        nOffSet += nFileLen;

        pUploadPicData->picInfo[i].nPicLen = nFileLen;

        fclose(pImageFile);
    }
    
    // 2015-9-19
    for (int j = 0, k = i; j < m_SysVehiclePicInfo.pic_num; j++, k++)
    {
        pUploadPicData->picInfo[k].nOffSet  = nOffSet;
        pUploadPicData->picInfo[k].nPicType = m_SysVehiclePicInfo.vehicle_pic_info[j].pic_type;


        int nFileLen = 0;
        FILE* pImageFile = fopen(m_SysVehiclePicInfo.vehicle_pic_info[j].pic_path, "rb");
        if (!pImageFile)
        {
            continue;
        }

        fseek(pImageFile, 0, SEEK_END);
        nFileLen = ftell(pImageFile);
        fseek(pImageFile, 0, SEEK_SET);

        if (nFileLen < 1)
        {
            continue;
        }

        //fread(pUploadPicData->szPicData + nOffSet, 1, nFileLen, pImageFile);
        //size_t CMSG_Center::ReadAll(FILE *fd, void *buff, size_t len)
        ReadAll(pImageFile, pUploadPicData->szPicData + nOffSet, nFileLen);

        nOffSet += nFileLen;

        pUploadPicData->picInfo[k].nPicLen = nFileLen;

        fclose(pImageFile);
    }
    
    pHead->packet_len = sizeof (T_Upload_Pic_Data) + nOffSet;
    sendLen += sizeof (T_Upload_Pic_Data) + nOffSet;



    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;






    //   ACE_Time_Value expire_time = ACE_Time_Value(5, 0);
    //   int nRet = server.send_n(chMsgAck, sendLen, &expire_time);
    int nRet = SysUtil::SocketWrite(cascade_socket, chMsgAck, sendLen, 50);

    if (nRet == sendLen)
    {
        CMyLog::m_pLog->_XGSysLog("send conta pic data succ %s,%d,packetlen %d......\n", m_szSequenceID, nRet, pHead->packet_len);
//        std::string strFileNameNew = "/root/zhouhm/";
//        strFileNameNew += "ch_";
//        strFileNameNew += m_szSequenceID;
//        strFileNameNew += ".dat";
//        FILE* pImageFile = fopen(strFileNameNew.c_str(), "wb");
//        if (!pImageFile)
//        {
//            return -1;
//        }     
//        WriteAll(pImageFile, chMsgAck, sendLen);
//        fclose(pImageFile);
    }


    if (m_SysContaPicInfo.pic_num > 0)
    {
        for (int i = 0; i < m_SysContaPicInfo.pic_num; i++)
        {
            char szCMD[256] = {0};
            sprintf(szCMD, "rm -rf %s", m_SysContaPicInfo.pic_info[i].pic_path);
            //system(szCMD);
        }
        // 2015-9-27
        memset(&m_SysContaPicInfo,  0x00, sizeof (T_Sys_ContaPicInfo));
    
    }
    
    if (m_SysVehiclePicInfo.pic_num > 0)
    {
        for (int i = 0; i < m_SysVehiclePicInfo.pic_num; i++)
        {
            char szCMD[256] = {0};
            sprintf(szCMD, "rm -rf %s", m_SysVehiclePicInfo.vehicle_pic_info[i].pic_path);
            //system(szCMD);
        }
        // 2015-9-27
        memset(&m_SysVehiclePicInfo, 0x00, sizeof(T_Sys_VehiclePicInfo));
    }

    return 0;
}


size_t CMSG_Center::ReadAll(FILE *fd, void *buff, size_t len)
{
	size_t n	= 0;
	size_t sum	= 0;
	do 
	{
		n = fread((char*)buff + sum, 1, len - sum, fd);
		sum += n;
	} while (sum < len && n != 0);
	if (n == 0 && ferror(fd))
		return 0;
	if (n == 0)
		return 1;
	return 1;
}


size_t CMSG_Center::WriteAll(FILE *fd, void *buff, size_t len)
{
	size_t n	= 0;
	size_t sum	= 0;
	do 
	{
		n = fwrite((char*)buff + sum, 1, len - sum, fd);
		sum += n;
	} while (sum < len && n != 0);
	if (n == 0 && ferror(fd))
		return 0;
	if (n == 0)
		return 1;
	return 1;
}
