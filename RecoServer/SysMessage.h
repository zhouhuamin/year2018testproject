#if !defined _SYS_MESSAGE_H_
#define _SYS_MESSAGE_H_

#define MAX_MSG_BODYLEN  (2*1024*1024)

#define SYS_NET_MSGHEAD                          "0XJZTECH"
#define SYS_NET_MSGTAIL                           "NJTECHJZ"


#define SYS_MSG_SYSTEM_REGISTER_REQ            0X01
#define SYS_MSG_SYSTEM_REGISTER_ACK            0X02

#define SYS_MSG_SYSTEM_MSG_KEEPLIVE            0X03
#define SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK        0X04

#define SYS_MSG_SYSTEM_MSG_RECO_REQ            0X05
#define SYS_MSG_SYSTEM_MSG_RECO_ACK            0X06

struct NET_PACKET_HEAD {
    int msg_type;
    int packet_len;
    char version_no[16];
    int proxy_count;
    int net_proxy[10];
};

struct NET_PACKET_MSG {
    NET_PACKET_HEAD msg_head;
    char msg_body[MAX_MSG_BODYLEN];
};

struct T_Register_Req {
    int client_type;
};

struct T_Register_Ack {
    int reg_result; //ע�������ɹ�����ʧ��
};

struct T_KeepliveAck {
    int nResult;
};



struct T_ContaRecoReq {
    char req_sequence[100];      // 2015-9-16 char req_sequence[32];
    int pic_len;
    char pic_buffer[];
};

struct NET_RecoRegion {
    int x;
    int y;
    int width;
    int height;
};
//��ɫ

enum NET_RecoColor {
    RED, YELLOW,BLUE, WHITE, GREY, GREEN, OTHER
};


//��װ������з�ʽ

enum NET_RecoAlignType {
    H_, T_
}; //H:horizontal ����  T��tandem ����

struct NET_RecoAlign {
    NET_RecoAlignType Atype; //���з�ʽ ���Ż�������
    int count; //����
};


struct NET_RecoContaID {
    char ID[12]; //��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
    char Type[5]; //��װ������
    float fAccuracy;
    NET_RecoRegion idreg; //����
    NET_RecoRegion typereg; //����
    NET_RecoColor color; //��ɫ
    NET_RecoAlign ali; //���з�ʽ
};


struct T_ContaRecoResult {
    char req_sequence[100];          // 2015-9-16 char req_sequence[32];
    int result;
    NET_RecoContaID conta_id;
};

struct NET_KEEP_LIVE {
    int client_type;
};


#endif
