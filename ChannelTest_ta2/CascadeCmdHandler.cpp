// CascadeCmdHandler.cpp: implementation of the CCascadeCmdHandler class.
//
//////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "stdafx.h"
#include "Winsock2.h"
#else
#include <netinet/tcp.h>

#include "Log/FreeLongLog.h"
#endif

#include "CascadeCmdHandler.h"
#include "SysUtil.h"
#include "SysMessage.h"
#include "MSG_Center.h"
#include "SimpleConfig.h"
#include "MyLog.h"

#include <string>
using namespace std;

bool CCascadeCmdHandler::m_bConnected = false;
int CCascadeCmdHandler::socket_ = -1;


extern "C" {
    typedef void*(*THREADFUNC)(void*);
}

void* HandlerProc(void * pParam)
{
    if (!pParam)
    {
        return 0;
    }
    CCascadeCmdHandler* pHandler = (CCascadeCmdHandler*) pParam;
    if (!pHandler)
    {
        return 0;
    }

    pHandler->svc();

    SysUtil::SysSleep(500);

    pthread_exit(NULL);

    return 0;
}


CCascadeCmdHandler::CCascadeCmdHandler(void)
{
    dwRecvBuffLen = 0;
    bGoWork_ = true;
    socket_ = -1;
    bThread_flag_ = false;
    
    chRecvBuffer=new char[MAX_MSG_BODYLEN];
    chDest=new char[MAX_MSG_BODYLEN];
}

CCascadeCmdHandler::~CCascadeCmdHandler(void)
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

int CCascadeCmdHandler::open(void*)
{
    
    int rcvbuf = 2 * 1024 * 1024;
    int rcvbufsize = sizeof (int);
    
 
    setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (char*) &rcvbuf,rcvbufsize);

    
    
    if (!bThread_flag_)
    {
        bThread_flag_=true;
        pthread_t localThreadId;
        int nThreadErr = pthread_create(&localThreadId, NULL,
                (THREADFUNC) HandlerProc, this);
        if (nThreadErr == 0)
        {
            pthread_detach(localThreadId); //	释放线程私有数据,不必等待pthread_join();

        }

    }

    MSG_CENTER::instance()->RecordCascadeConnection(socket_);
    return 0;
}


int CCascadeCmdHandler::svc()
{
    unsigned long dwRecvBuffLen = 0;
    int nRecvLen = 0;
    int nTimeCount = 0;
  
   
    while (bGoWork_)
    {
        if (socket_ > 0)
        {
            /*阻塞接收信令*/
            if (SysUtil::ReceiveTimer(socket_) > 0)
            {
                int nRet = SysUtil::SocketRead(socket_, chRecvBuffer + nRecvLen, MAX_MSG_BODYLEN - nRecvLen);
                nRecvLen += nRet;
                
//                CMyLog::m_pLog->_XGSysLog("recv from centerchannel manager len %d......\n",nRet);
                

                if (nRet > 0)
                {
                    nTimeCount = 0; //已经接受到数据，不需要再发线路保持报文

                    int nOffset = 0;
                    int nLen = 0;
                    
                    memset(chDest, 0, MAX_MSG_BODYLEN);
                    while (VerifyRecvPacket(chRecvBuffer, chDest, nRecvLen, nOffset, nLen) == 0)
                    {
                        NET_PACKET_HEAD* pMsgHead = (NET_PACKET_HEAD*) (chDest + nOffset);
                        int nPacketLen = pMsgHead->packet_len;

                        if (nLen == nPacketLen + sizeof (NET_PACKET_HEAD))
                        {
                            if (pMsgHead->msg_type == SYS_MSG_SYSTEM_MSG_KEEPLIVE ||
                                pMsgHead->msg_type == SYS_MSG_SYSTEM_REGISTER_ACK)
                            {
                                SysUtil::SysSleep(50);
                                continue;

                            }
                            
                            CMyLog::m_pLog->_XGSysLog("parse ok...\n");

                            NET_PACKET_MSG* pMsg_ = NULL;
                            MSG_CENTER::instance()->get_message(pMsg_);
                            if (!pMsg_)
                            {
                                return -1;
                            }

                            memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
                            //拷贝初始报文
                            memcpy(pMsg_, chDest + nOffset, nPacketLen + sizeof (NET_PACKET_HEAD));
                            MSG_CENTER::instance()->put(pMsg_);
                        }
                        else
                        {
                            CMyLog::m_pLog->_XGSysLog("parse len error...\n");
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
                if (nTimeCount > 30)
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

int CCascadeCmdHandler::ConnectToCascadeServer(short port, char* chIP)
{
    if (socket_ > 0)
    {
        SysUtil::CloseSocket(socket_);
        socket_ = -1;
    }

    socket_ = SysUtil::CreateSocket();
    if (SysUtil::ConnectSocket(socket_, chIP, port) == 0)
    {
        RegisterToCascadeServer(socket_);


        int sockbufsize = 800 * 1024;
        setsockopt(socket_,
                SOL_SOCKET,
                SO_SNDBUF,
                (char *) & sockbufsize,
                sizeof ( sockbufsize));



        set_keep_live(3, 5);
        m_bConnected = true;
        open(NULL);
        return 0;
    }
    else
    {
        SysUtil::CloseSocket(socket_);
        socket_ = -1;
        m_bConnected = false;
        CMyLog::m_pLog->_XGSysLog("********connect to center server %s:%d fail......\n",chIP,port);
        return -1;
    }
}

int CCascadeCmdHandler::ReceiveTimer(int fd)
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

int CCascadeCmdHandler::SendHeartbeatMsg()
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

int CCascadeCmdHandler::set_keep_live(int keep_alive_times, int keep_alive_interval)
{                                        //linux下
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
    return 0;
}

int CCascadeCmdHandler::VerifyRecvPacket(char *chRecvBuffer, char* chDest, int &nRecLen, int &nOffset, int &nLen)
{
    /*缓冲区长度小于最小帧长度*/
    if (nRecLen < sizeof (NET_PACKET_HEAD))
    {
        return -1;
    }

    int nHeadPos = SysUtil::SearchHeadPos(chRecvBuffer, nRecLen);
    if (nHeadPos < 0)
    {
        CMyLog::m_pLog->_XGSysLog("parse Head error...\n");
        return -1;
    }

    int nTailPos = SysUtil::SearchTailPos(chRecvBuffer, nRecLen);
    if (nTailPos < 0)
    {
        CMyLog::m_pLog->_XGSysLog("parse tail error...\n");
        return -1;
    }

    if (nTailPos < nHeadPos)
    {
        char szTest[1024]={0};
        memcpy(szTest,chRecvBuffer,300);
        CMyLog::m_pLog->_XGSysLog("parse pos error...\n");
        return -1;
    }
    
    
    CMyLog::m_pLog->_XGSysLog("recv from CCascadeCmdHandler,nTailPos %d---nHeadPos %d\n",nTailPos,nHeadPos);
    
   
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

int CCascadeCmdHandler::RegisterToCascadeServer(int socket)
{
    char chReq[1024] = {0};
    memset(chReq, 1024, 0);

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_REGISTER_REQ;
    pHead->packet_len = sizeof (T_ChannelRegister_Req);

    sendLen += sizeof (NET_PACKET_HEAD);


    T_ChannelRegister_Req* pRegisterReq = (T_ChannelRegister_Req*) (chReq + sendLen);
    sprintf(pRegisterReq->channel_id,"%s%s%s",CSimpleConfig::GATHER_AREA_ID,CSimpleConfig::GATHER_CHANNE_NO,CSimpleConfig::GATHER_I_E_TYPE);
    sendLen += sizeof (T_ChannelRegister_Req);

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket, chReq, sendLen, 100);

    return nRet;
}
