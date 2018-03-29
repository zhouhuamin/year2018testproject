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



class CMSG_Center : public ACE_Task<ACE_SYNCH>
{
    // Instantiated with an MT synchronization trait.
public:

  

    CMSG_Center();
    ~CMSG_Center(void);

    virtual int open();

    virtual int put(NET_PACKET_MSG* pMsg);

    virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);

public:
    
    int HandleNetMessage(NET_PACKET_MSG *pPacket);
  
    int handleContaRecoReq(NET_PACKET_MSG* pPacket);
  
   
    int ConnectToEventServer();

    int setEventhandle(int handle)
    {
        event_handle=handle;
    }
  
    static int init_message_buffer();
  
    static int get_message(NET_PACKET_MSG*& pMsg);
    static ACE_THR_FUNC_RETURN ThreadHandleMessage(void *arg);
	size_t	WriteAll(FILE *fd, void *buff, size_t len);
   
    std::list<NET_PACKET_MSG*> msgToHandleList_;
    ACE_Semaphore msg_semaphore_;
    ACE_Thread_Mutex msg_handle_lock_;

    bool m_bStartOk;

private:

    int         event_handle;
    
    static bool m_bWork;
    static ACE_Thread_Mutex msg_lock_;
    static std::vector<NET_PACKET_MSG*> msgVec_;
    static int m_nMsgBufferCount;


};

typedef ACE_Singleton<CMSG_Center, ACE_Null_Mutex> MSG_CENTER;

#endif // !defined(AFX_MSG_TASK_H__69346340_854F_4162_801C_94B418746B85__INCLUDED_)
