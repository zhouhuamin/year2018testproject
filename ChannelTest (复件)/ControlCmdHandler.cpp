// ControlCmdHandler.cpp: implementation of the CControlCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include <netinet/tcp.h>

#include "ControlCmdHandler.h"
#include "MyLog.h"
#include "SysUtil.h"
#include "SysMessage.h"
#include "MSG_Center.h"
#include "SimpleConfig.h"


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
    
    chRecvBuffer=new char[MAX_MSG_BODYLEN];
    chDest=new char[MAX_MSG_BODYLEN];
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
    
    delete [] chRecvBuffer;
    delete [] chDest;
}

int CControlCmdHandler::open(void*)
{
    registerToServer();

    if (!bThread_flag_)
    {
#ifdef WIN32
        _beginthread(HandlerProc, NULL, this);
#else
        ACE_Thread_Manager::instance()->spawn(HandlerProc, this);
#endif
    }
    
 
    return 0;
}

int CControlCmdHandler::svc()
{
    unsigned long dwRecvBuffLen = 0;
 
    int nRecvLen = 0;

  
    

    int nTimeCount=0;

    while (bGoWork_)
    {
        if (socket_ > 0)
        {
            /*������������*/
            if (SysUtil::ReceiveTimer(socket_) > 0)
            {
                int nRet = SysUtil::SocketRead(socket_, chRecvBuffer + nRecvLen, MAX_MSG_BODYLEN - nRecvLen);
                nRecvLen += nRet;

                if (nRet > 0)
                {

                    int nOffset = 0;
                    int nLen = 0;
                    
                    memset(chDest, 0, MAX_MSG_BODYLEN);
                    while (VerifyRecvPacket(chRecvBuffer, chDest, nRecvLen, nOffset, nLen) == 0)
                    {
                        NET_PACKET_HEAD* pMsgHead = (NET_PACKET_HEAD*) (chDest + nOffset);
                        int nPacketLen = pMsgHead->packet_len;

                        if (nLen == nPacketLen + sizeof (NET_PACKET_HEAD))
                        {

                            if (pMsgHead->msg_type == SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK)
                            {
                                SysUtil::SysSleep(50);
                                continue;

                            }
                            
                            if(pMsgHead->msg_type==SYS_MSG_SYSTEM_REGISTER_ACK)
                            {
                                T_DeviceServerRegister_Ack* pRegisterAck=(T_DeviceServerRegister_Ack*)(chDest + nOffset+sizeof (NET_PACKET_HEAD));
                                CSimpleConfig::IS_VIRTUAL=pRegisterAck->next_action;

                                CMyLog::m_pLog->_XGSysLog("recv register ack ,virtual status %d\n",CSimpleConfig::IS_VIRTUAL);
                                continue;
                            }

                            NET_PACKET_MSG* pMsg_ = NULL;
                            CMSG_Center::get_message(pMsg_);
                            if (!pMsg_)
                            {
                                return -1;
                            }

                            memset(pMsg_, 0, sizeof (NET_PACKET_MSG));

                            //������ʼ����
                            memcpy(pMsg_, chDest + nOffset, nPacketLen + sizeof (NET_PACKET_HEAD));

                            MSG_CENTER::instance()->put(pMsg_);

                        }
                    }
                }
                else
                {
                    SysUtil::CloseSocket(socket_);
                    socket_ = -1;
                    m_bConnected = false;

                    nRecvLen = 0;

                    SysUtil::SysSleep(50);
                }
            }
            else
            {
                nTimeCount++;
                if (nTimeCount > 300)
                {
                    nTimeCount = 0;
                    SendHeartbeatMsg();
                }
            }
        }
        else
        {
            SysUtil::SysSleep(500);
        }
    }

    SysUtil::SysSleep(50);

   

    return 0;
}


int CControlCmdHandler::ConnectToMonitorServer(short port, char* chIP)
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

        CMyLog::m_pLog->_XGSysLog("connect to %s:%d succ......\n",chIP,port);
        return 0;
    }
    else
    {
        SysUtil::CloseSocket(socket_);
        socket_ = -1;

        CMyLog::m_pLog->_XGSysLog("connect to %s:%d fail......\n",chIP,port);
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
    tv.tv_usec = 100 * 1000;

    iret = select(fd + 1, &rset, NULL, NULL, &tv);

    return iret;
}

int CControlCmdHandler::SendHeartbeatMsg()
{
    char chReq[1024] = {0};
    memset(chReq, 1024, 0);

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //��ͷ����
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_MSG_KEEPLIVE;
    pHead->packet_len = 0;

    sendLen += sizeof (NET_PACKET_HEAD);

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //��ͷ����
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);

    return 0;
}

int CControlCmdHandler::set_keep_live(int keep_alive_times, int keep_alive_interval)
{

#ifdef WIN32                                //WIndows��
    TCP_KEEPALIVE inKeepAlive = {0}; //�������
    unsigned long ulInLen = sizeof (TCP_KEEPALIVE);

    TCP_KEEPALIVE outKeepAlive = {0}; //�������
    unsigned long ulOutLen = sizeof (TCP_KEEPALIVE);

    unsigned long ulBytesReturn = 0;

    //����socket��keep aliveΪ5�룬���ҷ��ʹ���Ϊ3��
    inKeepAlive.on_off = keep_alive_times;
    inKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //����KeepAlive̽����ʱ����
    inKeepAlive.keep_alive_time = keep_alive_interval * 1000; //��ʼ�״�KeepAlive̽��ǰ��TCP�ձ�ʱ��


    outKeepAlive.on_off = keep_alive_times;
    outKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //����KeepAlive̽����ʱ����
    outKeepAlive.keep_alive_time = keep_alive_interval * 1000; //��ʼ�״�KeepAlive̽��ǰ��TCP�ձ�ʱ��


    if (WSAIoctl((unsigned int) socket_, SIO_KEEPALIVE_VALS,
        (LPVOID) & inKeepAlive, ulInLen,
        (LPVOID) & outKeepAlive, ulOutLen,
        &ulBytesReturn, NULL, NULL) == SOCKET_ERROR)
    {
        ACE_DEBUG((LM_INFO,
                ACE_TEXT("(%P|%t) WSAIoctl failed. error code(%d)!\n"), WSAGetLastError()));
    }

#else                                        //linux��
    int keepAlive = 1; //�趨KeepAlive
    int keepIdle = keep_alive_interval; //��ʼ�״�KeepAlive̽��ǰ��TCP�ձ�ʱ��
    int keepInterval = keep_alive_interval; //����KeepAlive̽����ʱ����
    int keepCount = keep_alive_times; //�ж��Ͽ�ǰ��KeepAlive̽�����
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
    /*����������С����С֡����*/
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
    if (nTailPos < 0)
    {
        return -1;
    }

    memcpy(chDest, chRecvBuffer + nHeadPos, (nTailPos - nHeadPos) + 8);

    if (nRecLen - nTailPos - 8 > 0)
    {
        //���յ����ݿ����������İ�
        //memcpy(chRecvBuffer, chRecvBuffer + nTailPos + 8, nRecLen - nTailPos - 8);
        memmove(chRecvBuffer, chRecvBuffer + nTailPos + 8, nRecLen - nTailPos - 8);
    }

    nOffset = nHeadPos = 8;
    nLen = nTailPos - nHeadPos;
    nRecLen = nRecLen - nTailPos - 8;

    return 0;
}

int CControlCmdHandler::registerToServer()
{
    char chReq[1024] = {0};
    memset(chReq, 1024, 0);

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //��ͷ����
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_REQ;
    pHead->packet_len =0;

    sendLen += sizeof (NET_PACKET_HEAD);

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //��ͷ����
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);

    return nRet;
}
