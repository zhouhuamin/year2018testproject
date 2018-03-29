// ControlCmdHandler.h: interface for the CControlCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _CONTROL_CMD_HANDLER_H_
#define _CONTROL_CMD_HANDLER_H_

#include "SysMessage.h"
#include <vector>
#include <string>

using namespace std;

#define MAX_RECV_LENGTH (MAX_MSG_BODYLEN*2)

class CControlCmdHandler
{
public:
    int registerToServer();
    int VerifyRecvPacket(char *chRecvBuffer, char* chDest, int &nRecLen, int &nOffset, int &nLen);
    int set_keep_live(int keep_alive_times, int keep_alive_interval);
    int SendHeartbeatMsg();
    int ReceiveTimer(int fd); //����Ƿ�����������
    CControlCmdHandler(); //���캯��
    virtual ~CControlCmdHandler(); //��������


    int open(void*); //��ʼ������
    int ConnectToMonitorServer(short port, char* chIP); //���ӵ�ICS
    int svc(); //�߳���
    int recvData(); //�������ݴ���

    
    static bool m_bConnected; //�Ƿ�������״̬
private:
    //recv buffer to hole recv data
    char* chRecvBuffer;
    char* chDest;
    //recv data pointer,maybe one recv can't recv total packet or over a packet
    unsigned long dwRecvBuffLen;
    int socket_; //���Ӿ��
    bool bGoWork_; //�������б�־
    bool bThread_flag_; //�߳�������־

};

#endif 
