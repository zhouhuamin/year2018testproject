// MSG_Center.h: interface for the MSG_Task class.
#if !defined _MSG_CENTER_
#define _MSG_CENTER_

#include <stdint.h>
#include <list>
#include <string>
#include <vector>
#include <map>

using namespace std;

#include "ace/Signal.h"
#include "ace/streams.h"
#include "ace/Task.h"
#include "ace/Semaphore.h"
#include "SysMessage.h"

#define MAX_BUFFERD_MESSAGE_NUMBER             (100)
#define MAX_HANDLE_THREADS                      1


struct T_Server_Info
{
    int   connect_handle;
    int   last_active;
};

class CMSG_Center : public ACE_Task<ACE_SYNCH>
{
    // Instantiated with an MT synchronization trait.
public:

    enum
    {
        MAX_THREADS = 1
    };

    CMSG_Center();
    ~CMSG_Center(void);

    virtual int open();

    virtual int put(NET_PACKET_MSG* pMsg);

    virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

public:
    
    int HandleNetMessage(NET_PACKET_MSG *pPacket);
    int handleContaReco(NET_PACKET_MSG *pPacket);
    int HandleContaRecoTest(NET_PACKET_MSG *pPacket);
    int handleContaRecoResult(NET_PACKET_MSG *pPacket);
    
    int client_exit(int handle);
    
    
    int handleClientRegister(NET_PACKET_MSG *pPacket);
    int CheckRecoServer();
  
    static int init_message_buffer();
    static int handleChannelKeeplive(NET_PACKET_MSG* pPacket);
    static int get_message(NET_PACKET_MSG*& pMsg);
    static ACE_THR_FUNC_RETURN ThreadHandleMessage(void *arg);
    static ACE_THR_FUNC_RETURN ThreadCheckRecoServer(void *arg);
    std::list<NET_PACKET_MSG*> msgToHandleList_;
    ACE_Semaphore msg_semaphore_;
    ACE_Thread_Mutex msg_handle_lock_;
    bool m_bStartOk;

private:
    static bool m_bWork;
    static ACE_Thread_Mutex msg_lock_;
    static std::vector<NET_PACKET_MSG*> msgVec_;
    static int m_nMsgBufferCount;
    
    
    static ACE_Thread_Mutex server_lock_;
    static std::map<int,T_Server_Info*> m_StaServerHandleMap;
    static std::list<T_Server_Info*> m_StaServerHandleList;
    
public:
    size_t WriteAll(FILE *fd, void *buff, size_t len);
};

typedef ACE_Singleton<CMSG_Center, ACE_Null_Mutex> MSG_CENTER;

#endif // !defined(AFX_MSG_TASK_H__69346340_854F_4162_801C_94B418746B85__INCLUDED_)
