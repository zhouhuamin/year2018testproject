// ControlCmdHandler.h: interface for the CControlCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _CONTROL_CMD_HANDLER_H_
#define _CONTROL_CMD_HANDLER_H_

#include "SysMessage.h"
#include <vector>
#include <string>

using namespace std;


class CControlCmdHandler
{
public:
    int VerifyRecvPacket(char *chRecvBuffer, char* chDest, int &nRecLen, int &nOffset, int &nLen);
    int set_keep_live(int keep_alive_times, int keep_alive_interval);
    int SendHeartbeatMsg();
    int ReceiveTimer(int fd); //����Ƿ�����������
    int register_server();
    int keep_live();
    
    CControlCmdHandler(); //���캯��
    virtual ~CControlCmdHandler(); //��������


    int open(void*); //��ʼ������
    int ConnectToAlarmServer(short port, char* chIP); //���ӵ�ICS
    int svc(); //�߳���
    int recvData(); //�������ݴ���

    static bool m_bConnected; //�Ƿ�������״̬
private:
    //recv buffer to hole recv data
    char* chRecvBuffer;
    int nRecvLen ;
    char* chDest;
    
    
    //recv data pointer,maybe one recv can't recv total packet or over a packet
    unsigned long dwRecvBuffLen;
    volatile int socket_; //���Ӿ��
    bool bGoWork_; //�������б�־
    bool bThread_flag_; //�߳�������־
    
    int  m_nKeepLiveCount;

};

#endif 
