// MSG_Center.h: interface for the MSG_Task class.
#if !defined _MSG_CENTER_
#define _MSG_CENTER_

#include "SysMessage.h"
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
#include "DataPacket.h"
#include "SimpleConfig.h"


#define MAX_BUFFERD_MESSAGE_NUMBER             (200)
#define MAX_HANDLE_THREADS                        1

struct T_SysCtrlData
{
    int   has_data;
    char  device_tag[32];
    int   xml_data_len;
    char  xml_data[5*1024];
};


struct T_ContaPicInfo
{
    int   pic_type;
    char  pic_path[256];
};

struct T_VehiclePicInfo
{
    int   pic_type;
    char  pic_path[256];
};


struct T_Sys_ContaPicInfo
{
    int             pic_num;
    T_ContaPicInfo  pic_info[6];
};

struct T_Sys_VehiclePicInfo
{
    int                 pic_num;
    T_VehiclePicInfo    vehicle_pic_info[2];
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
    int  RecordCascadeConnection(int cascadesocket)
    {
        cascade_socket = cascadesocket;
    }
    
    int GetCascadeSocket()
    {
        return cascade_socket;
    }

    int handleDoHeart(NET_PACKET_MSG *pPacket);
    bool IsUTF8(char *pBuffer, long size);
    int HandleNetMessage(NET_PACKET_MSG *pPacket);

    int handleDeviceRegister(NET_PACKET_MSG* pPacket);
    int handleDeviceKeeplive(NET_PACKET_MSG* pPacket);

    int CheckException();
    
    int handleEventData(NET_PACKET_MSG *pPacket);
    int handleSysSequenceControl(NET_PACKET_MSG *pPacket);

    int GetATimeStamp(char* szBuf, int nMaxLength);
    int handle_icreader_complete_event(T_Sequence_ControlData* pSequenceData);
    int handle_packet_data();

    int packet_gather_data(char* szXMLGatherInfo);
    int send_packet(char* szSequenceNo,char* szXMLGatherInfo);
    int send_packet_test(const char* szSequenceNo,const char* szXMLGatherInfo);
    int handleCustomsAck(NET_PACKET_MSG* pPacket);
    int handleSystmSwitch(NET_PACKET_MSG *pPacket);
    int handleDeviceRegather(NET_PACKET_MSG *pPacket);
    
    int send_conta_pic();

    int init();

    int handle_event(T_Sequence_ControlData* pSequenceData);
    int handle_device_ctrlevent(char* szEventID,char* szCtrlData);
    
    int post_event_to_device(char* device_tag,char* event_id);
    int post_event_to_device(char* device_tag,char* event_id,char* xml_data,int nLen);
    int publish_exception_event(char* event_id);
    
    int handleContaPic(char* szXML);
    int handleVehiclePic(char* szXML);
    
    int device_exit(int handle);

    int ConnectToCenterManageServer();
    int ConnectToMonitorServer();
 
    static int init_message_buffer();
    static int get_message(NET_PACKET_MSG*& pMsg);

    static ACE_THR_FUNC_RETURN ThreadHandleMessage(void *arg);
    static ACE_THR_FUNC_RETURN ThreadSuicide(void *arg);
    size_t	ReadAll(FILE *fd, void *buff, size_t len);
    size_t	WriteAll(FILE *fd, void *buff, size_t len);
    std::list<NET_PACKET_MSG*> msgToHandleList_;
    ACE_Semaphore msg_semaphore_;
    ACE_Thread_Mutex msg_handle_lock_;

    bool m_bStartOk;

private:
    int m_nUserReqCount;
    int m_nDevReqCount;

    void * mchandle_;

    static bool m_bWork;
    static ACE_Thread_Mutex msg_lock_;
    static std::vector<NET_PACKET_MSG*> msgVec_;
    static int m_nMsgBufferCount;

    std::map<string,T_SysCtrlData*> m_CtrlDataMap;

    CDataPacket* m_pDataPack;
 
    ACE_Thread_Mutex exception_lock_;
    std::vector<T_Sequence_Exception> m_ExceptionList;
    
    static ACE_Thread_Mutex device_lock_;

    int m_nUserIndex;
    int m_nDeviceIndex;

    int   cascade_socket;

    int   m_nRunState;
    char  m_szSequenceID[32];
    
    char* m_pSendPicBuffer;
    
    
    T_Sys_ContaPicInfo     m_SysContaPicInfo;
    T_Sys_VehiclePicInfo   m_SysVehiclePicInfo;
    
};

typedef ACE_Singleton<CMSG_Center, ACE_Null_Mutex> MSG_CENTER;

#endif 
