// ControlCmdHandler.cpp: implementation of the CControlCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include <netinet/tcp.h>

#include "ControlCmdHandler.h"
#include "SysUtil.h"
#include "SysMessage.h"
#include "MSG_Center.h"
#include "MyLog.h"

#include <string>
using namespace std;

bool CControlCmdHandler::m_bConnected = false;

#ifdef WIN32

void HandlerProc(void * pParam)
{
    if (!pParam)
    {
        return;
    }
    CControlCmdHandler* pHandler = (CControlCmdHandler*) pParam;
    if (!pHandler)
    {
        return;
    }

    pHandler->svc();

    SysUtil::SysSleep(500);

    return;
}
#else

static ACE_THR_FUNC_RETURN HandlerProc(void *pParam)
{
    if (!pParam)
    {
        return 0;
    }
    CControlCmdHandler* pHandler = (CControlCmdHandler*) pParam;
    if (!pHandler)
    {
        return 0;
    }

    pHandler->svc();

    SysUtil::SysSleep(500);

    return 0;
}

#endif

CControlCmdHandler::CControlCmdHandler(void)
{
    dwRecvBuffLen = 0;
    bGoWork_ = true;
    socket_ = -1;
    bThread_flag_ = false;

    chRecvBuffer = new char[MAX_MSG_BODYLEN];

    nRecvLen = 0;

    chDest = new char[MAX_MSG_BODYLEN];


}

CControlCmdHandler::~CControlCmdHandler(void)
{
    if (bGoWork_)
    {
        bGoWork_ = false;
        SysUtil::SysSleep(1000);
    }

    m_bConnected = false;
    if (socket_ > 0)
    {
        SysUtil::CloseSocket(socket_);
    }
}

int CControlCmdHandler::open(void*)
{
    m_nKeepLiveCount=0;
    int socket_buf_size=2 * 1024*1024;
    setsockopt(socket_,SOL_SOCKET,SO_RCVBUF,(char*)&socket_buf_size,sizeof(int));
    
    register_server();

    MSG_CENTER::instance()->setEventhandle(socket_);

    if (!bThread_flag_)
    {
        bThread_flag_=true;
        ACE_Thread_Manager::instance()->spawn(HandlerProc, this);
    }

    return 0;
}

int CControlCmdHandler::svc()
{

    while (bGoWork_)
    {
        if (socket_ > 0)
        {
            /*阻塞接收信令*/
            if (SysUtil::ReceiveTimer(socket_) > 0)
            {
                int nRet = SysUtil::SocketRead(socket_, chRecvBuffer + nRecvLen, MAX_MSG_BODYLEN - nRecvLen);
                nRecvLen += nRet;
                
                printf("recv len %d\n",nRecvLen);

                if (nRet > 0)
                {

                    int nOffset = 8;
                    int nLen = 0;
                    while (VerifyRecvPacket(chRecvBuffer, chDest, nRecvLen, nOffset, nLen) == 0)
                    {
                        NET_PACKET_HEAD* pMsgHead = (NET_PACKET_HEAD*) (chDest + nOffset);


                        if (pMsgHead->msg_type == SYS_MSG_SYSTEM_REGISTER_ACK)
                        {
                            printf("recv server register succ......\n");
                            continue;
                        }

                        if (pMsgHead->msg_type == SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK)
                        {
                            printf("recv server keeplive succ......\n");
                            m_nKeepLiveCount=0;
                            continue;
                        }



                        int nPacketLen = pMsgHead->packet_len;

                        if (nLen == nPacketLen + sizeof (NET_PACKET_HEAD))
                        {
                            NET_PACKET_MSG* pMsg_ = NULL;
                            CMSG_Center::get_message(pMsg_);
                            if (!pMsg_)
                            {
                                return -1;
                            }

                            memset(pMsg_, 0, sizeof (NET_PACKET_MSG));

                            //拷贝初始报文
                            memcpy(pMsg_, chDest + nOffset, nPacketLen + sizeof (NET_PACKET_HEAD));

                            CMyLog::m_pLog->_XGSysLog("pMsg_->msg_head.proxy_count %d! \n", pMsg_->msg_head.proxy_count);
                            pMsg_->msg_head.net_proxy[pMsg_->msg_head.proxy_count] = socket_;
                            pMsg_->msg_head.proxy_count++;




                            MSG_CENTER::instance()->put(pMsg_);

                        }
                    }
                } else
                {
                    SysUtil::CloseSocket(socket_);
                    socket_ = -1;
                    m_bConnected = false;

                    nRecvLen = 0;

                    SysUtil::SysSleep(10);
                }
            }
        } else
        {
            SysUtil::SysSleep(500);
        }
    }

    SysUtil::SysSleep(50);
    return 0;
}

int CControlCmdHandler::ConnectToAlarmServer(short port, char* chIP)
{
    if (socket_ > 0)
    {
        SysUtil::CloseSocket(socket_);
        socket_ = -1;
    }

    socket_ = SysUtil::CreateSocket();
    if (SysUtil::ConnectSocket(socket_, chIP, port) == 0)
    {

        set_keep_live(3, 5);

        m_bConnected = true;
        open(NULL);

        printf("connect to %s:%d succ......\n", chIP, port);
        return 0;
    } else
    {
        SysUtil::CloseSocket(socket_);
        socket_ = -1;

        printf("connect to %s:%d fail......\n", chIP, port);
        return -1;
    }
}

int CControlCmdHandler::ReceiveTimer(int fd)
{
    int iret = 0;
    fd_set rset;
    struct timeval tv;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    tv.tv_sec = 0;
    tv.tv_usec = 20 * 1000;

    iret = select(fd + 1, &rset, NULL, NULL, &tv);

    return iret;
}

int CControlCmdHandler::SendHeartbeatMsg()
{
    char chReq[1024] = {0};
    memset(chReq, 1024, 0);

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_MSG_KEEPLIVE;
    pHead->packet_len = 0;

    sendLen += sizeof (NET_PACKET_HEAD);

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);

    return 0;
}

int CControlCmdHandler::set_keep_live(int keep_alive_times, int keep_alive_interval)
{

#ifdef WIN32                                //WIndows下
    TCP_KEEPALIVE inKeepAlive = {0}; //输入参数
    unsigned long ulInLen = sizeof (TCP_KEEPALIVE);

    TCP_KEEPALIVE outKeepAlive = {0}; //输出参数
    unsigned long ulOutLen = sizeof (TCP_KEEPALIVE);

    unsigned long ulBytesReturn = 0;

    //设置socket的keep alive为5秒，并且发送次数为3次
    inKeepAlive.on_off = keep_alive_times;
    inKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //两次KeepAlive探测间的时间间隔
    inKeepAlive.keep_alive_time = keep_alive_interval * 1000; //开始首次KeepAlive探测前的TCP空闭时间


    outKeepAlive.on_off = keep_alive_times;
    outKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //两次KeepAlive探测间的时间间隔
    outKeepAlive.keep_alive_time = keep_alive_interval * 1000; //开始首次KeepAlive探测前的TCP空闭时间


    if (WSAIoctl((unsigned int) socket_, SIO_KEEPALIVE_VALS,
            (LPVOID) & inKeepAlive, ulInLen,
            (LPVOID) & outKeepAlive, ulOutLen,
            &ulBytesReturn, NULL, NULL) == SOCKET_ERROR)
    {
        ACE_DEBUG((LM_INFO,
                ACE_TEXT("(%P|%t) WSAIoctl failed. error code(%d)!\n"), WSAGetLastError()));
    }

#else                                        //linux下
    int keepAlive = 1; //设定KeepAlive
    int keepIdle = keep_alive_interval; //开始首次KeepAlive探测前的TCP空闭时间
    int keepInterval = keep_alive_interval; //两次KeepAlive探测间的时间间隔
    int keepCount = keep_alive_times; //判定断开前的KeepAlive探测次数
    if (setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE, (const char*) & keepAlive, sizeof (keepAlive)) == -1)
    {
        ACE_DEBUG((LM_INFO,
                ACE_TEXT("(%P|%t) setsockopt SO_KEEPALIVE error!\n")));
    }
    if (setsockopt(socket_, SOL_TCP, TCP_KEEPIDLE, (const char *) & keepIdle, sizeof (keepIdle)) == -1)
    {
        ACE_DEBUG((LM_INFO,
                ACE_TEXT("(%P|%t) setsockopt TCP_KEEPIDLE error!\n")));
    }
    if (setsockopt(socket_, SOL_TCP, TCP_KEEPINTVL, (const char *) & keepInterval, sizeof (keepInterval)) == -1)
    {
        ACE_DEBUG((LM_INFO,
                ACE_TEXT("(%P|%t) setsockopt TCP_KEEPINTVL error!\n")));
    }
    if (setsockopt(socket_, SOL_TCP, TCP_KEEPCNT, (const char *) & keepCount, sizeof (keepCount)) == -1)
    {
        ACE_DEBUG((LM_INFO,
                ACE_TEXT("(%P|%t)setsockopt TCP_KEEPCNT error!\n")));
    }

#endif

    return 0;

}

int CControlCmdHandler::VerifyRecvPacket(char *chRecvBuffer, char* chDest, int &nRecLen, int &nOffset, int &nLen)
{
    /*缓冲区长度小于最小帧长度*/
    if (nRecLen < sizeof (NET_PACKET_HEAD))
    {
        return -1;
    }

    int nHeadPos = SysUtil::SearchHeadPos(chRecvBuffer, nRecLen);
    if (nHeadPos < 0)
    {
        return -1;
    }

    int nTailPos = SysUtil::SearchTailPos(chRecvBuffer, nRecLen);
    if (nTailPos < 0 || nTailPos < nHeadPos)
    {
        return -1;
    }

    memcpy(chDest, chRecvBuffer + nHeadPos, (nTailPos - nHeadPos) + 8);

    if (nRecLen - nTailPos - 8 > 0)
    {
        //接收的数据可能是连续的包
        //memcpy(chRecvBuffer, chRecvBuffer + nTailPos + 8, nRecLen - nTailPos - 8);
        memmove(chRecvBuffer, chRecvBuffer + nTailPos + 8, nRecLen - nTailPos - 8);
    }

    nOffset = nHeadPos = 8;
    nLen = nTailPos - nHeadPos;
    nRecLen = nRecLen - nTailPos - 8;

    return 0;
}

int CControlCmdHandler::register_server()
{
    if (socket_ < 1)
    {
        return -1;
    }


    char chReq[1024] = {0};
    memset(chReq, 1024, 0);

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_REQ;
    pHead->packet_len = sizeof (T_Register_Req);

    sendLen += sizeof (NET_PACKET_HEAD);


    T_Register_Req* pRegisterReq = (T_Register_Req*) (chReq + sendLen);
    pRegisterReq->client_type = 9;
    sendLen += sizeof (T_Register_Req);


    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);

    if (nRet == sendLen)
    {
        CMyLog::m_pLog->_XGSysLog("register succ...\n");
    }

    return 0;
}

int CControlCmdHandler::keep_live()
{
    
    if(m_nKeepLiveCount >35)
    {
        SysUtil::CloseSocket(socket_);
        socket_=-1;
        m_bConnected=false;
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        return -1;
        
    }
    
    if (socket_ < 1)
    {
        return -1;
    }


    char chReq[1024] = {0};
    memset(chReq, 1024, 0);

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_MSG_KEEPLIVE;
    pHead->packet_len = sizeof(NET_KEEP_LIVE);
    sendLen += sizeof (NET_PACKET_HEAD);
    
    
    NET_KEEP_LIVE* pKeeplive=(NET_KEEP_LIVE*)(chReq + sendLen);
    pKeeplive->client_type=9;
    sendLen += sizeof (NET_KEEP_LIVE);
    
    
    

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);

    if (nRet != sendLen)
    {
        CMyLog::m_pLog->_XGSysLog("********send keeplive msg fail,to send is %d  real send is %d ...\n",sendLen,nRet);
    }
    
    
    m_nKeepLiveCount++;

    return 0;
}