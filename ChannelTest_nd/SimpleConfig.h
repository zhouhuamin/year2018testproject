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
    char   device_id[16];        //�豸ID���������ֲ�ͬ���豸����
    int    is_capture_device;
    int    is_data_full;
    char   device_tag[32];
    int    is_register;           //�Ƿ��Ѿ�ע��
    int    connect_socket;       //�豸�������Ӿ��
    long   last_keeplive;          //����ʱ��
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
	std::string szLocalIP;				// ����IP��ַ
	int  nLocalPort;					// ����˿�

	std::string szUpstreamIP;			// ��λ����IP
	int  nUpstreamPort;					// ��λ����˿ں�
	std::string szUpstreamObjectCode;	// ��λ�������
	int  nServiceType;					// ����������

	std::string szObjectCode;			// �������
	std::string szObjectStatus;			// ״ֵ̬
	std::string szConnectedObjectCode;	// �ԽӶ������
	std::string szReportTime;			// ʱ���
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

