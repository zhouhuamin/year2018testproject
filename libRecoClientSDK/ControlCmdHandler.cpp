// ControlCmdHandler.cpp: implementation of the CControlCmdHandler class.
//
//////////////////////////////////////////////////////////////////////
#include <netinet/tcp.h>
#include "ControlCmdHandler.h"
#include "SysUtil.h"

#include <string>
using namespace std;

bool CControlCmdHandler::m_bConnected = false;
bool CControlCmdHandler::m_bInit = false;

extern "C"
{
    typedef void*(*THREADFUNC)(void*);
}

void HandlerContaProc(void * pParam)
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

    pthread_exit(NULL);

    return;
}

void ConnectToRecoServetrFunc(void * pParam)
{

    CControlCmdHandler* pHandler = (CControlCmdHandler*) pParam;
    if (!pHandler)
    {
        return;
    }

    int nTimerCount = 0;
    while (pHandler->bGoWork_)
    {
        if (!CControlCmdHandler::m_bConnected)
        {

            if (pHandler->ConnectToRecoServer(pHandler->m_nServerPoer, pHandler->m_szServerIP) == -1)
            {


            } else
            {
                CControlCmdHandler::m_bConnected = true;
                pHandler->Register();
            }
        } 
        else
        {
            nTimerCount++;
            if (nTimerCount % 2 == 0)
            {
                pHandler->SendHeartbeatMsg();
            }


            if (nTimerCount > 10000)
            {
                nTimerCount = 0;
            }
        }

        //select代替sleep,等待一段时间间隔5s
        for (int i = 0; i < 100; i++)
        {
            SysUtil::SysSleep(50);

            if (!pHandler->bGoWork_)
            {


                pthread_exit(NULL);
                return;
            }

        }

    }

    pthread_exit(NULL);
    return;
}

CControlCmdHandler::CControlCmdHandler(void)
{
    //    dwRecvBuffLen = 0;
    bGoWork_ = true;
    socket_ = -1;
    bThread_flag_ = false;

    m_pRecoResultCallback = NULL;

    m_pRecoBuffer = new char[1024 * 1024 * 6];


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

int CControlCmdHandler::Init(char* szRecoServerIP, int nRecoServerPort)
{
    strcpy(m_szServerIP, szRecoServerIP);
    m_nServerPoer = nRecoServerPort;


    pthread_t localThreadId;
    int nThreadErr = pthread_create(&localThreadId, NULL,
            (THREADFUNC) ConnectToRecoServetrFunc, this);

    if (nThreadErr == 0)
    {
        pthread_detach(localThreadId); //	锟酵凤拷锟竭筹拷私锟斤拷锟斤拷锟?锟斤拷锟截等达拷pthread_join();

    }


    return 0;
}

int CControlCmdHandler::Stop()
{
    bGoWork_ = false;
    SysUtil::SysSleep(500);

}

int CControlCmdHandler::open(void*)
{

    int rcvbuf = 2 * 1024 * 1024;
    int rcvbufsize = sizeof (int);
    //setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (char*) &rcvbuf, rcvbufsize);
    setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (char*) &rcvbuf, rcvbufsize);

    Register();

    if (!bThread_flag_)
    {
        bThread_flag_ = true;


        pthread_t localThreadId;
        int nThreadErr = pthread_create(&localThreadId, NULL,
                (THREADFUNC) HandlerContaProc, this);

        if (nThreadErr == 0)
        {
            pthread_detach(localThreadId); //	锟酵凤拷锟竭筹拷私锟斤拷锟斤拷锟?锟斤拷锟截等达拷pthread_join();

        }
    }

    return 0;
}

int CControlCmdHandler::SetRecoResultCallback(_RECO_RESULT_CALLBACK pRecoResultCallback)
{
    m_pRecoResultCallback = pRecoResultCallback;

}

int CControlCmdHandler::svc()
{
    char* chRecvBuffer = new char[MAX_MSG_BODYLEN * 5];
    int nRecvLen = 0;
    char* chDest = new char[MAX_MSG_BODYLEN * 5];

    while (bGoWork_)
    {
        if (socket_ > 0)
        {
            /*阻塞接收信令*/
            if (SysUtil::ReceiveTimer(socket_) > 0)
            {
                int nRet = SysUtil::SocketRead(socket_, chRecvBuffer + nRecvLen, MAX_MSG_BODYLEN * 5 - nRecvLen);

                if (nRet > 0)
                {
                    nRecvLen += nRet;
                    printf("recv len %d\n",nRecvLen);
                    
                    
                    int nOffset = 8;
                    int nLen = 0;

                    memset(chDest, 0, MAX_MSG_BODYLEN * 5);
                    while (VerifyRecvPacket(chRecvBuffer, chDest, nRecvLen, nOffset, nLen) == 0)
                    {
                        NET_PACKET_HEAD* pMsgHead = (NET_PACKET_HEAD*) (chDest + nOffset);
                        int nPacketLen = pMsgHead->packet_len;

                        if (pMsgHead->msg_type == SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK ||
                                pMsgHead->msg_type == SYS_MSG_SYSTEM_LOGIN_ACK)
                        {

                            continue;
                        }

                        if (nLen == nPacketLen + sizeof (NET_PACKET_HEAD))
                        {

                            if (pMsgHead->msg_type == SYS_MSG_SYSTEM_MSG_RECO_ACK)
                            {
                                T_ContaRecoResult* pRecoResult = (T_ContaRecoResult*) (chDest + nOffset + sizeof (NET_PACKET_HEAD));

                                JZ_RecoContaID reco_result;
                                memset(&reco_result, 0, sizeof (JZ_RecoContaID));

                                strcpy(reco_result.ContaID, pRecoResult->conta_id.ID);
                                strcpy(reco_result.Type, pRecoResult->conta_id.Type);
                                
                                reco_result.fAccuracy=pRecoResult->conta_id.fAccuracy;

                                reco_result.ali.Atype = (JZ_AlignType)pRecoResult->conta_id.ali.Atype;
                                reco_result.ali.count = pRecoResult->conta_id.ali.count;
                                reco_result.color =(JZ_Color) pRecoResult->conta_id.color;
                                
                                reco_result.IDReg.x = pRecoResult->conta_id.idreg.x;
                                reco_result.IDReg.y = pRecoResult->conta_id.idreg.y;
                                reco_result.IDReg.width = pRecoResult->conta_id.idreg.width;
                                reco_result.IDReg.height = pRecoResult->conta_id.idreg.height;
                                
                                
                                
                                reco_result.TypeReg.x = pRecoResult->conta_id.typereg.x;
                                reco_result.TypeReg.y = pRecoResult->conta_id.typereg.y;
                                reco_result.TypeReg.width = pRecoResult->conta_id.typereg.width;
                                reco_result.TypeReg.height = pRecoResult->conta_id.typereg.height;
                                
                                

                                reco_result.nResult = pRecoResult->result;


                                if (m_pRecoResultCallback)
                                {
                                    m_pRecoResultCallback(pRecoResult->req_sequence, &reco_result);
                                }

                            }
                         
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
        } 
        else
        {
            SysUtil::SysSleep(500);
        }
    }


    delete [] chRecvBuffer;
    delete [] chDest;
    SysUtil::SysSleep(50);

    return 0;
}

void CControlCmdHandler::RecoConta(char* szRecoID, char* szFileName)
{
    int nFileLen = 0;
    FILE* pImageFile = fopen(szFileName, "rb");
    if (!pImageFile)
    {
        printf("RecoConta fopen failed!\n");
        return;
    }

    fseek(pImageFile, 0, SEEK_END);
    nFileLen = ftell(pImageFile);
    fseek(pImageFile, 0, SEEK_SET);

    if (nFileLen < 1)
    {
        printf("RecoConta nFileLen < 1!\n");
        return;
    }




    char* chReq = m_pRecoBuffer;

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据
    sendLen += 8;

    NET_PACKET_MSG* pMsg = (NET_PACKET_MSG*) (chReq + sendLen);

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    memset(pHead, 0, sizeof (NET_PACKET_HEAD));

    pHead->msg_type = SYS_MSG_SYSTEM_MSG_RECO_REQ;
    pHead->packet_len = sizeof (NVP_CONTARECO_REQ);

    sendLen += sizeof (NET_PACKET_HEAD);

    NVP_CONTARECO_REQ* pContaReco = (NVP_CONTARECO_REQ*) pMsg->msg_body;
    strcpy(pContaReco->req_sequence, szRecoID);
    pContaReco->pic_len = nFileLen;

    //int nRead = fread((char*) pContaReco->pic_buffer, 1, nFileLen, pImageFile);
    //ReadAll(FILE *fd, void *buff, size_t len)
    int nRead = ReadAll(pImageFile, (char*) pContaReco->pic_buffer, nFileLen);

    if (nRead != nFileLen)
    {
        printf("RecoConta nRead:%d != nFileLen:%d!\n", nRead, nFileLen);
        fclose(pImageFile);
        return;
    }
    fclose(pImageFile);

    sendLen += sizeof (NVP_CONTARECO_REQ) + pContaReco->pic_len;

    pHead->packet_len = sizeof (NVP_CONTARECO_REQ) + pContaReco->pic_len;

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);
    printf("SysUtil::SocketWrite:%d,%d!\n", nRet, socket_);

    usleep(10*1000);
    int nnn = 0;
}

int CControlCmdHandler::ConnectToRecoServer(short port, char* chIP)
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
        return 0;
    } else
    {
        SysUtil::CloseSocket(socket_);
        socket_ = -1;
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

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据
    sendLen += 8;

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_MSG_KEEPLIVE;
    pHead->packet_len = sizeof(NET_KEEP_LIVE);
    sendLen += sizeof (NET_PACKET_HEAD);
    
    NET_KEEP_LIVE* pKeeplive=(NET_KEEP_LIVE*)(chReq + sendLen);
    pKeeplive->client_type=0;
    sendLen += sizeof (NET_KEEP_LIVE);

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);

    return 0;
}

int CControlCmdHandler::set_keep_live(int keep_alive_times, int keep_alive_interval)
{


    int keepAlive = 1; //设定KeepAlive
    int keepIdle = keep_alive_interval; //开始首次KeepAlive探测前的TCP空闭时间
    int keepInterval = keep_alive_interval; //两次KeepAlive探测间的时间间隔
    int keepCount = keep_alive_times; //判定断开前的KeepAlive探测次数
    if (setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE, (const char*) & keepAlive, sizeof (keepAlive)) == -1)
    {

    }
    if (setsockopt(socket_, SOL_TCP, TCP_KEEPIDLE, (const char *) & keepIdle, sizeof (keepIdle)) == -1)
    {

    }
    if (setsockopt(socket_, SOL_TCP, TCP_KEEPINTVL, (const char *) & keepInterval, sizeof (keepInterval)) == -1)
    {

    }
    if (setsockopt(socket_, SOL_TCP, TCP_KEEPCNT, (const char *) & keepCount, sizeof (keepCount)) == -1)
    {

    }


    return 0;

}

int CControlCmdHandler::VerifyRecvPacket(char *chRecvBuffer, char *chDest, int &nRecLen, int &nOffset, int &nLen)
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
    if (nTailPos < 0)
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

int CControlCmdHandler::Register()
{

    char chReq[1024] = {0};
    memset(chReq, 1024, 0);

    int sendLen = 0;
    memcpy(chReq, SYS_NET_MSGHEAD, 8); //包头数据
    sendLen += 8;

    NET_PACKET_MSG* pMsg = (NET_PACKET_MSG*) (chReq + sendLen);

    NET_PACKET_HEAD* pHead = (NET_PACKET_HEAD*) (chReq + sendLen);
    pHead->msg_type = SYS_MSG_SYSTEM_LOGIN;
    pHead->packet_len = sizeof (LOG_IN);

    sendLen += sizeof (NET_PACKET_HEAD);


    LOG_IN* pLogIn = (LOG_IN*) pMsg->msg_body;
    pLogIn->client_type = 0;
    sendLen += sizeof (LOG_IN);

    memcpy(chReq + sendLen, SYS_NET_MSGTAIL, 8); //包头数据
    sendLen += 8;
    int nRet = SysUtil::SocketWrite(socket_, chReq, sendLen, 100);

    return 0;

}


size_t CControlCmdHandler::ReadAll(FILE *fd, void *buff, size_t len)
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
		return sum;
	return sum;
}
