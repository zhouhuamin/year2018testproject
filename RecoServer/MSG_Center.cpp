// MSG_Center.cpp: implementation of the MSG_Task class.
//
//////////////////////////////////////////////////////////////////////

#include "ace/Select_Reactor.h"
#include "ace/SOCK_Stream.h"

#include "MSG_Center.h"
#include "SysUtil.h"
#include "ControlCmdHandler.h"
#include "MyLog.h"
#include "IDDetect.h"//动态库头文件


ACE_Thread_Mutex CMSG_Center::msg_lock_;

std::vector<NET_PACKET_MSG*> CMSG_Center::msgVec_;
int CMSG_Center::m_nMsgBufferCount = 0;

struct T_CtrlCmd
{
    char szSequenceNo[64];
    int nLen;
    char szCtrlData[];
};

CMSG_Center::~CMSG_Center()
{
    ACE_Reactor::instance()->cancel_timer(this);
}

CMSG_Center::CMSG_Center()
{


}

static ACE_THR_FUNC_RETURN ThreadConnectToEventServerFunc(void *lParam)
{
    CMSG_Center* pTask = (CMSG_Center*) lParam;
    pTask->ConnectToEventServer();
    return 0;
}

int CMSG_Center::open()
{

    event_handle = -1;
    //初始化信息数据缓冲区，此缓冲区用于CCtrlCmdReceiver_T向该类传输数据
    init_message_buffer();
    msg_semaphore_.acquire();


    m_bStartOk = false;


    ACE_Thread_Manager::instance()->spawn_n(MAX_HANDLE_THREADS, ThreadHandleMessage, this);
    ACE_Thread_Manager::instance()->spawn(ThreadConnectToEventServerFunc, this);

//    if (ACE_Reactor::instance()->schedule_timer(this, 0,
//            ACE_Time_Value(2, 0),
//            ACE_Time_Value(2, 0)) == -1)
//    {
//        return -1;
//    }

    return 0;
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

            if (pTask->msgToHandleList_.size() == 0)
            {

                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 100000;
                select(0, 0, NULL, NULL, &tv);


                continue;
            }

            pMsg_ = pTask->msgToHandleList_.front();
            pTask->msgToHandleList_.pop_front();

        }

        if (!pMsg_)
        {
            continue;
        }


        try
        {
            pTask->HandleNetMessage(pMsg_);

        } catch (...)
        {
            CMyLog::m_pLog->_XGSysLog("crash Message : %d ...\n", pMsg_->msg_head.msg_type);
            continue;
        }
    }
}

int CMSG_Center::ConnectToEventServer()
{
    CControlCmdHandler * handler = new CControlCmdHandler;

    int count = 0;

    while (1)
    {
        if (!CControlCmdHandler::m_bConnected)
        {
            if (handler->ConnectToAlarmServer(19000, "127.0.0.1") == -1)
            {


            } else
            {
                CControlCmdHandler::m_bConnected = true;
            }
        } else
        {

            handler->keep_live();

        }

        //select代替sleep,等待一段时间间隔5s
        //SysUtil::SysSleep(3 * 1000);
        sleep(10);

    }

    delete handler;

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
        tv.tv_usec = 100;
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
        case SYS_MSG_SYSTEM_MSG_RECO_REQ:
            handleContaRecoReq(pPacket);
            break;


    }

    return 0;
}

int CMSG_Center::handleContaRecoReq(NET_PACKET_MSG* pPacket)
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


    T_ContaRecoReq* pContaReq = (T_ContaRecoReq*) pPacket->msg_body;


    CMyLog::m_pLog->_XGSysLog("recv conta req %s! \n", pContaReq->req_sequence);

    char szRecoFile[256] = {0};
//   sprintf(szRecoFile, "/dev/shm/1.jpg");
    
    long reco_pid = getpid();
    static int nSequence = 2;
    sprintf(szRecoFile, "/dev/shm/%d_%d.jpg", reco_pid, nSequence);
    ++nSequence;
//    sprintf(szRecoFile, "/dev/shm/%s.jpg", pContaReq->req_sequence);

    printf("szRecoFile:%s,pic_len:%d, buffer:%p\n", szRecoFile, pContaReq->pic_len, pContaReq->pic_buffer);
    FILE* pImageFileToReco = fopen(szRecoFile, "wb");
    //fwrite(pContaReq->pic_buffer, 1, pContaReq->pic_len, pImageFileToReco);
	WriteAll(pImageFileToReco, pContaReq->pic_buffer, pContaReq->pic_len);
    fclose(pImageFileToReco);


    ContainerID code_id;
    memset(&code_id, 0, sizeof (ContainerID));

    
    if (access(szRecoFile, F_OK) == -1)
    {
        CMyLog::m_pLog->_XGSysLog("%s is not existed!\n", szRecoFile);
        return -1;
    }
    
    int ret = 0;
    ret = PathReadCode(szRecoFile, &code_id); //调用函数 该函数在IDDetect.cpp中


    CMyLog::m_pLog->_XGSysLog("%s,%s %s\n", pContaReq->req_sequence, code_id.ID, code_id.Type);


    //send ack to reco manager

    ACE_SOCK_Stream server((ACE_HANDLE) connect_handle);
    char chMsgAck[10240] = {0};

    int sendLen = 0;
    memcpy(chMsgAck, SYS_NET_MSGHEAD, 8); //锟斤拷头锟斤拷锟?
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chMsgAck + sendLen);

    pHead->proxy_count = nProxyCount - 1;

    CMyLog::m_pLog->_XGSysLog("........proxy count %d...\n", pHead->proxy_count);

    for (int i = 0; i < nProxyCount - 1; i++)
    {
        pHead->net_proxy[i] = pPacket->msg_head.net_proxy[i];
    }


    pHead->msg_type = SYS_MSG_SYSTEM_MSG_RECO_ACK;
    pHead->packet_len = sizeof (T_ContaRecoResult);


    sendLen += sizeof (NET_PACKET_HEAD);

    T_ContaRecoResult* pRecoResultAck = (T_ContaRecoResult*) (chMsgAck + sendLen);
    pRecoResultAck->result = ret;
    strcpy(pRecoResultAck->conta_id.ID, (char*) code_id.ID);
    strcpy(pRecoResultAck->conta_id.Type, (char*) code_id.Type);
    pRecoResultAck->conta_id.fAccuracy = code_id.accuracy;
    pRecoResultAck->conta_id.ali.Atype = (NET_RecoAlignType)code_id.ali.Atype;
    pRecoResultAck->conta_id.ali.count = code_id.ali.count;

    pRecoResultAck->conta_id.idreg.x = code_id.IDreg.x;
    pRecoResultAck->conta_id.idreg.y = code_id.IDreg.y;
    pRecoResultAck->conta_id.idreg.width = code_id.IDreg.width;
    pRecoResultAck->conta_id.idreg.height = code_id.IDreg.height;
    
    
    pRecoResultAck->conta_id.typereg.x = code_id.Typereg.x;
    pRecoResultAck->conta_id.typereg.y = code_id.Typereg.y;
    pRecoResultAck->conta_id.typereg.width = code_id.Typereg.width;
    pRecoResultAck->conta_id.typereg.height = code_id.Typereg.height;

    pRecoResultAck->conta_id.color =(NET_RecoColor) code_id.color;
    strcpy(pRecoResultAck->req_sequence, pContaReq->req_sequence);


    sendLen += sizeof (T_ContaRecoResult);

    pHead->packet_len = sizeof (T_ContaRecoResult);
    memcpy(chMsgAck + sendLen, SYS_NET_MSGTAIL, 8);
    sendLen += 8;

    ACE_Time_Value expire_time = ACE_Time_Value(0, 100 * 1000);
    int nRet = server.send_n(chMsgAck, sendLen, &expire_time);



    return 0;

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



