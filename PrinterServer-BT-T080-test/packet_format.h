#pragma once
#include <string>
#include <vector>

#define MAX_MSG_BODY_LEN  (0x200)

#define MSG_TYPE_DEV_REGISTER		0x01
#define MSG_TYPE_DEV_REGISTER_ACK	0x02
#define MSG_TYPE_KEEP_LIVE			0x03
#define MSG_TYPE_KEEP_LIVE_ACK		0x04
#define MSG_TYPE_CTRL_EVENT			0x09
#define MSG_TYPE_GATHER_DATA		0x05

#define NET_PACKET_HEAD_STRING      "0XJZTECH"
#define NET_PACKET_HEAD_STRING_LEN  8

#define NET_PACKET_TAIL_STRING      "NJTECHJZ"
#define NET_PACKET_TAIL_STRING_LEN  8

typedef struct
{
	int msg_type;
	int packet_len;
	char szVersion[16];  //packet version
	int proxy_count;
	int net_proxy[10];
} net_packet_head_t;

typedef struct
{
	char dev_tag[32];
	char dev_id[32];
} NVP_REGISTER;

typedef struct
{
	int reg_result;
	int next_action;
} NVP_REGISTER_ACK;

typedef struct
{
	char event_id[32];
	int xml_data_len;
	char xml_data[MAX_MSG_BODY_LEN - 36];
} NET_SYS_CTRL;

typedef struct
{
	char event_id[32];
	int is_data_full;
	char dev_tag[32];
	int xml_data_len;
	char xml_data[MAX_MSG_BODY_LEN - 72];
} NET_gather_data;

typedef struct
{
	char str_head[8];
	net_packet_head_t msg_head;
	u8 msg_body[MAX_MSG_BODY_LEN];
	char str_tail[8];
} net_packet_t;

struct structLedMsg
{
    u32         id;
    std::string strMsg;
    structLedMsg()
    {
        id      = 0;
        strMsg  = "";
    }
};

struct structPrinterMsg
{
    std::string SinglePound;
    std::string CertificateNo;
    std::string PassNo;
    structPrinterMsg()
    {
        SinglePound     = "";
        CertificateNo   = "";
        PassNo          = "";
    }
};




struct T_BUYS_WEIGHT_INFO
{
    std::string ID; 
    std::string CARD_TYPE;
    std::string PRINT_DATE; 
    std::string DEPARTMENT; 
    std::string CAR_NUMBER; 
    std::string GOODS_NAME; 
    std::string DELIVERY_UNITS; 
    std::string GROSS; 
    std::string TARE; 
    std::string NET_WEIGHT; 
    std::string KTARE; 
    std::string KTARE2; 
    std::string ACTUAL_WEIGHT; 
    std::string GROSS_MEMBER; 
    std::string TARE_MEMBER; 
    std::string GROSS_TIME; 
    std::string TARE_TIME;     
    std::string ORDER_NUMBER1; 
    std::string ORDER_NUMBER2;     
};

struct T_CERTIFICATE_INFO
{
    std::string STANDARDS; 
    std::string LICENSE_NUMBER; 
    std::string STRENGTH_GRADE; 
    std::string SERIAL_NUMBER; 
    std::string CARRY_NUMBER; 
    std::string FACTORY_NUMBER; 
    std::string MANUFACTURE_DATE; 
    std::string ISSUE_MAN; 
    std::string LABORATORY_CERT; 
    std::string TIPS; 
};

struct T_SALES_WEIGHT_INFO
{
    std::string ID; 
    std::string CARD_TYPE;
    std::string PRINT_DATE; 
    std::string DEPARTMENT; 
    std::string CAR_NUMBER; 
    std::string GOODS_NAME; 
    std::string RECEIVE_UNITS; 
    std::string GROSS; 
    std::string TARE; 
    std::string NET_WEIGHT; 
    
    std::string GROSS_MEMBER; 
    std::string TARE_MEMBER; 
    std::string GROSS_TIME; 
    std::string TARE_TIME; 
    
    
    std::string ORDER_NUMBER1; 
    std::string ORDER_NUMBER2;
    std::string ORDER_NUMBER3;
    std::string ORDER_NUMBER4;
    std::string ORDER_NUMBER5;
    
    std::string ORDER_NUMBER6; 
    std::string ORDER_NUMBER7;
    std::string ORDER_NUMBER8;
    std::string ORDER_NUMBER9;
    std::string ORDER_NUMBER10;    
    
    std::string NOTE; 
    T_CERTIFICATE_INFO cert;
};


struct structPrinterMsg2
{
    std::string         strCount;
    std::vector<std::string> itemVect;
    
    
    std::string         messageType;
    T_BUYS_WEIGHT_INFO  buysInfo;
    T_SALES_WEIGHT_INFO salesInfo;
    
};


int net_dev_register_handle(int server_fd);
int net_dev_keeplive_handle(int server_fd);
int server_read_timeout(int server_fd, u8 *buf, int len, int time_out);
int connect_server(const char *host_name, int port_id);
int upload_data_handle(int server_fd);
