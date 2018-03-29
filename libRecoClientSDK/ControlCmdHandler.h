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
    int ReceiveTimer(int fd); //检测是否有网络数据
    CControlCmdHandler(); //构造函数
    virtual ~CControlCmdHandler(); //析构函数


    int open(void*); //初始化连接
    int ConnectToRecoServer(short port, char* chIP); //连接到ICS
    int svc(); //线程体
    int recvData(); //接收数据处理
    size_t	ReadAll(FILE *fd, void *buff, size_t len);
    

    static bool m_bConnected; //是否处于连接状态      
    
    static bool m_bInit;
    bool bGoWork_; //继续运行标志
    
    
    char   m_szServerIP[32];
    int    m_nServerPoer;
  
private:
    
    _RECO_RESULT_CALLBACK m_pRecoResultCallback;
   
    int socket_; //连接句柄
    
    bool bThread_flag_; //线程启动标志
 

    char* m_pRecoBuffer;

};

#endif 
