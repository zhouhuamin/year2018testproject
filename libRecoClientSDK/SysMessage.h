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
    int  is_data_full;                            //�����Ƿ�����
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
    char main_type[32];                   //�������������ͬ��ͨ�����п�����
    int  sub_type;                        //����ͬ���¼�����Ӧ��ͬ�Ľṹ
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
//��ɫ

enum Color {
    red, blue, white, gray, green, other
};


//��װ������з�ʽ

enum AlignType {
    H, T
}; //H:horizontal ����  T��tandem ����

struct Align {
    AlignType Atype; //���з�ʽ ���Ż�������
    int count; //����
};



struct ContaID {
    char ID[12]; //��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
    char Type[5]; //��װ������
    float fAccuracy;
    Region idreg; //����
    Region typereg; //����
    Color color; //��ɫ
    Align ali; //���з�ʽ
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
