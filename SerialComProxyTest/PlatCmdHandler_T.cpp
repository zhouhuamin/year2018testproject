// PlatCmdHandler_T.cpp: implementation of the CCmdHandler_T class.
//
//////////////////////////////////////////////////////////////////////
#include "PlatCmdHandler_T.h"
#include "Plat_Cmd_Acceptor.h"

#include "SysMessage.h"
#include "MSG_Center.h"
#include <syslog.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlatCmdHandler_T::CPlatCmdHandler_T()
{
    m_nObjID = 0;
    dwRecvBuffLen = 0;
    chRecvBuffer = new char[MAX_CMD_BUFFERLEN];

}

CPlatCmdHandler_T::~CPlatCmdHandler_T()
{
    delete [] chRecvBuffer;
    syslog(LOG_DEBUG, "......close connection!\n");
}

void CPlatCmdHandler_T::destroy()
{
    this->reactor()->remove_handler(this,
            ACE_Event_Handler::READ_MASK
            | ACE_Event_Handler::DONT_CALL);

    this->peer().close();
    CPlat_Cmd_Acceptor::free_handler(m_nObjID);
    //	inherited::destroy();
}

int CPlatCmdHandler_T::open(void *void_acceptor)
{
    dwRecvBuffLen = 0;
    //	this->activate (THR_DETACHED);
    if(this->reactor()->register_handler(this,
            ACE_Event_Handler::READ_MASK) == -1)
    {
//        ACE_ERROR_RETURN((LM_ERROR,
//                "(%P|%t) can't register with reactor\n"),
//                -1);
        syslog(LOG_DEBUG, "can't register with reactor\n");
    }

    syslog(LOG_DEBUG, "recv socket is %d\n", (int) this->peer().get_handle());

    return 0;
}

int CPlatCmdHandler_T::close(u_long flags)
{
    ACE_UNUSED_ARG(flags);
    this->destroy();
    return 0;
}

int CPlatCmdHandler_T::handle_input(ACE_HANDLE handle)
{
    ACE_UNUSED_ARG(handle);

    return this->process();
}

int CPlatCmdHandler_T::handle_close(ACE_HANDLE handle,
        ACE_Reactor_Mask mask)
{
    ACE_UNUSED_ARG(handle);
    ACE_UNUSED_ARG(mask);

    this->destroy();
    return 0;
}

int CPlatCmdHandler_T::svc(void)
{
    while (!bStopFlag_)
    {
        if(this->process() == -1)
            return -1;
    }

    bStopFlag_ = true;
    return 0;
}

int CPlatCmdHandler_T::stop(void)
{
    bStopFlag_ = true;
    return 0;
}

int CPlatCmdHandler_T::process()
{
  
    ssize_t bytes_read;
    switch ((bytes_read = this->peer().recv(chRecvBuffer + dwRecvBuffLen, MAX_CMD_BUFFERLEN - dwRecvBuffLen)))
    {
        case -1:
            RecvPlatformCmd();
            return -1;
        case 0:
            RecvPlatformCmd();
            return -1;
        default:
            dwRecvBuffLen += bytes_read;
            syslog(LOG_DEBUG, "recv gather message from centerserver len %d\n",dwRecvBuffLen);
/*
            if(this->VerifyRecvPacket(chRecvBuffer, dwRecvBuffLen) == 0)
            {

                printf("%s\n", chRecvBuffer + 38);
                {
                    NET_PACKET_MSG* pMsg_ = NULL;
                    CMSG_Center::get_message(pMsg_);
                    if(!pMsg_)
                    {
                        return -1;
                    }

                    memset(pMsg_, 0, sizeof (NET_PACKET_MSG));

                    pMsg_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS_ACK;

                    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pMsg_->msg_body;

                    //������ʼ����
                    memcpy(&pCustomsData->szCustomsData, chRecvBuffer, dwRecvBuffLen);
                    pCustomsData->nCustomsDataLen = dwRecvBuffLen;

                    pMsg_->msg_head.packet_len = dwRecvBuffLen + sizeof (T_Upload_Customs_Data);

                    //��¼���Ӿ��
                    MSG_CENTER::instance()->put(pMsg_);
                }
            }
 */
    }

    return 0;
}

int CPlatCmdHandler_T::VerifyRecvPacket(char *chRecvBuffer, int &nRecLen)
{
    /*����������С����С֡����*/
    if(nRecLen < 40)
    {
        return -1;
    }

    unsigned char* pRecvBuffer = (unsigned char*) chRecvBuffer;

    if(pRecvBuffer[0] == 0XE2 && pRecvBuffer[1] == 0X5C && pRecvBuffer[2] == 0X4B && pRecvBuffer[3] == 0X89)
    {
        if(pRecvBuffer[nRecLen - 2] == 0XFF && pRecvBuffer[nRecLen - 1] == 0XFF)
        {
            return 0;
        }
    }

    return -1;
}

int CPlatCmdHandler_T::RecvPlatformCmd()
{
//    NET_PACKET_MSG* pMsg_ = NULL;
//    CMSG_Center::get_message(pMsg_);
//    if(!pMsg_)
//    {
//        return -1;
//    }
//
//    memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
//
//    pMsg_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS_ACK;
//
//    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pMsg_->msg_body;
//
//    //������ʼ����
//    memcpy(&pCustomsData->szCustomsData, chRecvBuffer, dwRecvBuffLen);
//    pCustomsData->nCustomsDataLen = dwRecvBuffLen;
//
//    pMsg_->msg_head.packet_len = dwRecvBuffLen + sizeof (T_Upload_Customs_Data);
//
//    //��¼���Ӿ��
//    MSG_CENTER::instance()->put(pMsg_);
    if (dwRecvBuffLen <= 0)
            return 0;
    unsigned char chType = (unsigned char)chRecvBuffer[8];
    if (chType == 0x00)
        return 0;
    
    NET_PACKET_MSG* pMsg_ = NULL;
    CMSG_Center::get_message(pMsg_);
    if(!pMsg_)
    {
        return -1;
    }

    memset(pMsg_, 0, sizeof (NET_PACKET_MSG));


    switch (chType)
    {
        case 0x21:
            {
                    pMsg_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS;

                    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pMsg_->msg_body;

                    //������ʼ����
                    memcpy(&pCustomsData->szCustomsData, chRecvBuffer, dwRecvBuffLen);
                    pCustomsData->nCustomsDataLen = dwRecvBuffLen;

                    pMsg_->msg_head.packet_len = dwRecvBuffLen + sizeof (T_Upload_Customs_Data);

                    //��¼���Ӿ��
                    MSG_CENTER::instance()->put(pMsg_);
            }            
            break;

//        case 0x22:	// ���ں�̨ϵͳ�򿨿�ǰ�˼���ϵͳ�������豸��������ָ������϶�Ӧ0x32
//            {
//                    pMsg_->msg_head.msg_type = SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS_ACK;
//
//                    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pMsg_->msg_body;
//
//                    //������ʼ����
//                    memcpy(&pCustomsData->szCustomsData, chRecvBuffer, dwRecvBuffLen);
//                    pCustomsData->nCustomsDataLen = dwRecvBuffLen;
//
//                    pMsg_->msg_head.packet_len = dwRecvBuffLen + sizeof (T_Upload_Customs_Data);
//
//                    //��¼���Ӿ��
//                    MSG_CENTER::instance()->put(pMsg_);
//            }
//            break;

//        case 0x39:	// �˹�����ָ�ֱ����ƽ̨���͵�����̧��
//            {
//                    pMsg_->msg_head.msg_type = 0; // SYS_MSG_MAN_FANGXI;
//
//                    T_Upload_Customs_Data* pCustomsData = (T_Upload_Customs_Data*) pMsg_->msg_body;
//
//                    //������ʼ����
//                    memcpy(&pCustomsData->szCustomsData, chRecvBuffer, dwRecvBuffLen);
//                    pCustomsData->nCustomsDataLen = dwRecvBuffLen;
//
//                    pMsg_->msg_head.packet_len = dwRecvBuffLen + sizeof (T_Upload_Customs_Data);
//
//                    //��¼���Ӿ��
//                    MSG_CENTER::instance()->put(pMsg_);
//            }
//            break;

        default:
            break;
    }
    return 0;    
}
