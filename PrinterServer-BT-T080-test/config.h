#pragma once
#include "includes.h"


typedef struct
{
	char server_ip[20];
	int server_port;

	char com_ip[20];
	int com_port;

	int keeplive_gap;
	int DEV_scan_gap;

	int DEV_scan_duration_time;
	char register_DEV_TAG[32];
	char register_DEV_ID[32];
	char SYS_CTRL_R_start_event_ID[32];
	char SYS_CTRL_R_stop_event_ID[32];
        char szIC_RING_EVENT[32];
	char gather_event_id[32];
	char gather_DEV_TAG[32];
	char arrived_DEV_event_id[32];
	char onload_event_id[32];

	int RFID_label_type;  //??ç­¾ç±»?? 0: iso-180006C  1:iso-180006B
	int IC_read_block_NO;

	u8 WT_protocol_head[64];
	int WT_protocol_head_len;
	int WT_protcol_value_bytes;
	int WT_protocol_tail_len;
	u8 WT_protocol_tail[64];

	int WT_threshold_value;
	int WT_average_diff_value;
        char publisher_server_ip[20];
	int  publisher_server_port;
        
        char GATHER_AREA_ID[32];
        char GATHER_CHANNE_NO[32];
        char GATHER_I_E_TYPE[8];
} sSystem_setting;

typedef struct
{
	int dev_open_flag;
	int dev_connect_flag;
	int server_connect_flag;
	int server_register_flag;
	int dev_register_flag;   //device register flag
	int work_flag;           //0:wait 1:working
}sRealdata_system;

typedef struct
{
	u32 dev_scan_count;
	int dev_scan_ok_start_systick; //?????????°æ??ç­¾æ?¶ç???¶é??
	u32 dev_keeplive_cnt;
	u32 dev_keeplive_ACK_cnt;
	u32 request_clean_history_cnt;
	u32 read_request_respond_flag;
} DEV_UNIT_t;

// 2015-5-19
struct struDeviceAndServiceStatus
{
	char szLocalIP[32];					// ????IP?°å??
	int  nLocalPort;						// ???¡ç????

	char szUpstreamIP[32];				// ä¸?ä½?????IP
	int	 nUpstreamPort;				// ä¸?ä½????¡ç???£å??
	char szUpstreamObjectCode[32];	// ä¸?ä½????¡ä»£??
	int  nServiceType;					// ???¡ç±»??ä»£ç??

	char szObjectCode[32];				// å¯¹è±¡ä»£å??
	char szObjectStatus[3 + 1];		// ?¶æ????
	char szConnectedObjectCode[32];	// å¯¹æ?¥å?¹è±¡ä»£å??
	char szReportTime[32];				// ?¶é?´æ??
};

int parame_init(void);
int set_parame_default(void);
int load_setting_from_xml(sSystem_setting *p_setting);
int save_setting_to_xml(sSystem_setting *p_setting);
int load_setting_from_xml2(sSystem_setting *p_setting);

extern sSystem_setting gSetting_system;
extern sRealdata_system gReal_system;
extern DEV_UNIT_t dev_unit;

