// ControlCmdHandler.h: interface for the CControlCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _CONTROL_CMD_HANDLER_H_
#define _CONTROL_CMD_HANDLER_H_

#include "SysMessage.h"
#include <vector>
#include <string>
#include "RecoClientSDK.h"

using namespace std;

#define MAX_RECV_LENGTH (MAX_MSG_BODYLEN*2)

class CControlCmdHandler {
public:
    int Init(char* szRecoServerIP,int nRecoServerPort);
    void RecoConta(char* szRecoID, char* szFileName);
    int Stop();
    int SetRecoResultCallback(_RECO_RESULT_CALLBACK pRecoResultCallback);
    
    int Register();
    int VerifyRecvPacket(char *chRecvBuffer, char* chDest, int &nRecLen, int &nOffset, int &nLen);
    int set_keep_live(int keep_alive_times, int keep_alive_interval);
    int SendHeartbeatMsg();
    int ReceiveTimer(int fd); //����Ƿ�����������
    CControlCmdHandler(); //���캯��
    virtual ~CControlCmdHandler(); //��������


    int open(void*); //��ʼ������
    int ConnectToRecoServer(short port, char* chIP); //���ӵ�ICS
    int svc(); //�߳���
    int recvData(); //�������ݴ���
    size_t	ReadAll(FILE *fd, void *buff, size_t len);
    

    static bool m_bConnected; //�Ƿ�������״̬      
    
    static bool m_bInit;
    bool bGoWork_; //�������б�־
    
    
    char   m_szServerIP[32];
    int    m_nServerPoer;
  
private:
    
    _RECO_RESULT_CALLBACK m_pRecoResultCallback;
   
    int socket_; //���Ӿ��
    
    bool bThread_flag_; //�߳�������־
 

    char* m_pRecoBuffer;

};

#endif 
