#ifndef _SIMPLE_CONFIG_H_
#define	_SIMPLE_CONFIG_H_

#pragma warning(disable : 4786)	
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "tinyXML/xml_config.h"

struct T_Gather_Device
{
    char   device_id[16];        //设备ID，用来区分不同的设备类型
    int    is_capture_device;
    int    is_data_full;
    char   device_tag[32];
    int    is_register;           //是否已经注册
    int    connect_socket;       //设备进程连接句柄
    long   last_keeplive;          //心跳时间
    int    is_active;
};

struct T_RecvEvent
{
    int        is_recv_event;
    char       event_id[32];
};


struct T_PostEvent
{
    char       post_tag[32];
    char       event_id[32];
};

struct T_Sequence_Control
{
    char            event_desc[128];
    int             event_assoc;
    int             recv_event_num;
    T_RecvEvent    recv_event[8];
    int             post_event_num;
    T_PostEvent    post_event[8];
};

struct T_Sequence_Exception
{
    char   recv_event[16];
    char   wait_event[16];
    int    wait_time;
    char   post_event[16];
    char   exception_desc[128];
};

// 2015-5-19
struct struDeviceAndServiceStatus
{
	std::string szLocalIP;				// 服务IP地址
	int  nLocalPort;					// 服务端口

	std::string szUpstreamIP;			// 上位服务IP
	int  nUpstreamPort;					// 上位服务端口号
	std::string szUpstreamObjectCode;	// 上位服务代号
	int  nServiceType;					// 服务类别代码

	std::string szObjectCode;			// 对象代号
	std::string szObjectStatus;			// 状态值
	std::string szConnectedObjectCode;	// 对接对象代号
	std::string szReportTime;			// 时间戳
};

class CSimpleConfig
{
public:
    CSimpleConfig();

    virtual ~CSimpleConfig();

    void get_config();
    static int GetProcessPath(char* cpPath);

    static int IS_VIRTUAL;
    static int LOCAL_LISTEN_PORT;

    static int SERVER_MONITOR_PORT;

    static char CENTER_CONTROLER_IP[32];
    static int  CENTER_CONTROLER_PORT;

    static char GATHER_AREA_ID[32];
    static char GATHER_CHANNE_NO[32];
    static char GATHER_I_E_TYPE[8];
    static char GATHER_CHNL_TYPE[8];

    static char EXE_FULL_PATH[256];
    static char GATHERGATE_TYPE[8];


    static std::map<string,T_Gather_Device*> m_StaDeviceMap;
    static std::map<string,T_Gather_Device*> m_StaTagDeviceMap;
    static std::vector<T_Sequence_Control*>    m_StaSequenceControlVec;
    static std::vector<T_Sequence_Exception*>  m_StaSequenceExceptionVec;

   
	static std::string  m_publisher_server_ip;
	static int			m_publisher_server_port;

};

#endif

