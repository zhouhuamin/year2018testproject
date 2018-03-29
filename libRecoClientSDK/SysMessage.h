#if !defined _SYS_MESSAGE_H_
#define _SYS_MESSAGE_H_

#pragma warning(disable: 4200)

#include <map>
#include <vector>
#include <list>
using namespace std;

#define MAX_MSG_BODYLEN  20*1024

#define SYS_NET_MSGHEAD                "0XJZTECH"
#define SYS_NET_MSGTAIL                "NJTECHJZ"




#define SYS_MSG_SYSTEM_LOGIN                   0X01
#define SYS_MSG_SYSTEM_LOGIN_ACK               0X02

#define SYS_MSG_SYSTEM_MSG_KEEPLIVE            0X03
#define SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK        0X04

#define SYS_MSG_SYSTEM_MSG_RECO_REQ            0X05
#define SYS_MSG_SYSTEM_MSG_RECO_ACK            0X06

struct NET_PACKET_HEAD
{
    int msg_type;
    int packet_len;
	char version_no[16];
    int proxy_count;
    int net_proxy[10];
};

struct NET_PACKET_MSG
{
    NET_PACKET_HEAD msg_head;
    char msg_body[MAX_MSG_BODYLEN];
};

struct NVP_CONTARECO_REQ
{
	char                     req_sequence[100];
	int                      pic_len;
	char                     pic_buffer[];

};

struct NET_EVENT_SUBSCRIBE
{
    char     client_type;
    char     user_name[32];
    char     pass_word[32];
    char     event_serial[32];
	
};

struct NET_EVENT_SUBSCRIBE_ACK
{
    int result;
};


struct T_Upload_Customs_Data
{
    char szChannelNo[32];
    char szSequenceNo[32];
    int  is_data_full;                            //数据是否完整
    int  nCustomsDataLen;
    char szCustomsData[];
};

struct NET_EVENT
{
    char main_type[32];
    int  sub_type;
};


struct NET_EVENT_UPLOAD_CUSTOMSDATA
{
    char main_type[32];                   //用来区别各个不同的通道集中控制器
    int  sub_type;                        //区别不同的事件，对应不同的结构
    T_Upload_Customs_Data customs_data;
};


struct NET_REGATHER
{
	char szChnnl_No[16];
    char szDeviceTag[16];                  
};


struct NET_QUERY_GATHERINFO
{
	char szChnnl_No[16];
    char szBeginTime[32];  
	char szStopTime[32];  
};

struct LOG_IN
{
	int client_type;
};





struct Region {
    int x;
    int y;
    int width;
    int height;
};
//颜色

enum Color {
    red, blue, white, gray, green, other
};


//集装箱号排列方式

enum AlignType {
    H, T
}; //H:horizontal 横排  T：tandem 竖排

struct Align {
    AlignType Atype; //排列方式 横排或者竖排
    int count; //排数
};



struct ContaID {
    char ID[12]; //集装箱号：4位字母，6位数字，1位校验码
    char Type[5]; //集装箱类型
    float fAccuracy;
    Region idreg; //区域
    Region typereg; //区域
    Color color; //颜色
    Align ali; //排列方式
};

struct T_ContaRecoResult {
    char req_sequence[100];      // char req_sequence[32];
    int result;
    ContaID conta_id;
};

struct NET_KEEP_LIVE {
    int client_type;
};



#endif
