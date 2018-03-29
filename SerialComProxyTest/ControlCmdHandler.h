// ControlCmdHandler.h: interface for the CControlCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _CONTROL_CMD_HANDLER_H_
#define _CONTROL_CMD_HANDLER_H_

#include "SysMessage.h"
#include <vector>
#include <string>

using namespace std;

class CMSG_Center;
class CControlCmdHandler
{
public:
    int VerifyRecvPacket(char *chRecvBuffer, char* chDest, int &nRecLen, int &nOffset, int &nLen);
    int set_keep_live(int keep_alive_times, int keep_alive_interval);
    int SendHeartbeatMsg();
    int ReceiveTimer(int fd); //����Ƿ�����������
    CControlCmdHandler(); //���캯��
    virtual ~CControlCmdHandler(); //��������


    int open(void*); //��ʼ������
    int ConnectToAlarmServer(short port, const char* chIP, CMSG_Center *pCenter); //���ӵ�ICS
    int svc(); //�߳���
    int recvData(); //�������ݴ���
    
    int registerChannel(char* channel_id, int socket);
     size_t	WriteAll(FILE *fd, void *buff, size_t len);

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
