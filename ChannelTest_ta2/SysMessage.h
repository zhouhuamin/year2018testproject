#if !defined _SYS_MESSAGE_H_
#define _SYS_MESSAGE_H_

#define MAX_MSG_BODYLEN  6*1024*1024 // 50*1024

#define SYS_NET_MSGHEAD                          "0XJZTECH"
#define SYS_NET_MSGTAIL                           "NJTECHJZ"


#define SYS_SYSTEM                             0X00
#define SYS_SUB_SYSTEM_ICCARDREADER         1000000
#define SYS_SUB_SYSTEM_LEVERCONTROLER      2000000
#define SYS_SUB_SYSTEM_LCDCONTROLER        3000000


#define SYS_MSG_SYSTEM_REGISTER_REQ            SYS_SYSTEM+1
#define SYS_MSG_SYSTEM_REGISTER_ACK           SYS_SYSTEM+2

#define SYS_MSG_SYSTEM_MSG_KEEPLIVE            SYS_SYSTEM+3
#define SYS_MSG_SYSTEM_MSG_KEEPLIVE_ACK        SYS_SYSTEM+4

#define SYS_MSG_PUBLISH_EVENT                SYS_SYSTEM+5
#define SYS_MSG_PUBLISH_EVENT_ACK           SYS_SYSTEM+6


#define SYS_MSG_REGATHERDATA                 0X13

#define SYS_MSG_SYSTEM_MSG_SEQUENCE_DISPATCH  SYS_SYSTEM+1000


#define SYS_MSG_TAKEOVER_DEVICE                 SYS_SYSTEM+1001
#define SYS_MSG_RETURN_DEVICE                   SYS_SYSTEM+1002



#define SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS            SYS_SYSTEM+7
#define SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS_ACK        SYS_SYSTEM+8

#define SYS_MSG_SYSTEM_CTRL                                       SYS_SYSTEM+9


#define SYS_MSG_ICCARDREADER_READ            SYS_SUB_SYSTEM_ICCARDREADER+3
#define SYS_MSG_ICCARDREADER_READ_ACK       SYS_SUB_SYSTEM_ICCARDREADER+4


#define SYS_MSG_LEVERCONTROLER_ON            SYS_SUB_SYSTEM_ICCARDREADER+5
#define SYS_MSG_LEVERCONTROLER_ON_ACK       SYS_SUB_SYSTEM_ICCARDREADER+6

#define SYS_MSG_LEVERCONTROLER_OFF         SYS_SUB_SYSTEM_ICCARDREADER+7
#define SYS_MSG_LEVERCONTROLER_OFF_ACK    SYS_SUB_SYSTEM_ICCARDREADER+8


#define SYS_MSG_LCD_DISPLAY                     SYS_SUB_SYSTEM_ICCARDREADER+9
#define SYS_MSG_LCD_DISPLAY_ACK              SYS_SUB_SYSTEM_ICCARDREADER+10

#define SYS_MSG_SEND_PICDATA              0X0A
#define SYS_MSG_SEND_PICDATA_ACK          0X0B


#define SYS_EVENT_INIT_STATUS                  "EC_INIT_CHAN"     //��ʼ������
#define SYS_EVENT_PACKET_DATA                  "EC_UPLOAD_CHAN"     //�������
#define SYS_EVENT_CUSTOMS_DATA_ACK             "EG_RETURN_CHAN"      //ƽ̨���ݷ���

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

struct T_DeviceServerRegister_Req {
    char device_tag[32];
    char device_id[32]; //����ģ��ID��Ψһֵ
};

struct T_ChannelRegister_Req {
    char channel_id[32]; //����ģ��ID��Ψһֵ
};

struct T_DeviceServerRegister_Ack {
    int reg_result; //ע�������ɹ�����ʧ��
    int next_action; //������������������ʼ�������ǵȴ�����ָ��
};

struct T_KeepliveAck {
    int nResult;
};

struct T_ICReader_Data {
    char event_id[32];
    char reader_data[64];
};

struct T_SysEventData {
    char event_id[32];
    int is_data_full;
    char device_tag[32];
    int xml_data_len;
    char xml_data[];
};

struct T_Sequence_ControlData {
    char event_id[32];
    int has_ic;
    int xml_len;
    char xml_data[];

};

struct T_Len {
    int length;
};

struct T_Upload_Customs_Data {
    char szChannelNo[32];
    char szSequenceNo[32];
    int is_data_full; //�����Ƿ�����
    int nCustomsDataLen;
    char szCustomsData[];
};

struct T_Sys_Ctrl {
    char event_id[32];
    int xml_len;
    char xml_data[];
};

struct NET_REGATHER//���ɽṹ��
{
    int iArrange_Type; //�������1-�ֹ�¼�����ݷ�ʽ���ɣ�2-����ǰ���豸��ʽ����; 
    char szChnnl_No[32]; //��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szDeviceTag[16]; // ����ǰ���豸����ʱ���ݵ��豸��ʶ��
    T_Upload_Customs_Data customs_data; //�ֹ���ʽ��������ŵı��ģ�����Ϊ��

};

struct NET_PIC_INFO {
    int nPicType; //����ͼƬ��Ӧ�����ӵ�λ�ã�ǰ�����ҵ�
    int nPicLen; //ͼƬ����
    int nOffSet; //�ڻ�������λ�� 
};

struct T_Upload_Pic_Data {
    char szSequenceNo[32];
    char szChannelNo[32];
    char szAreaNo[32];
    char szIEFlag[32];
    int nPicNum;
    NET_PIC_INFO picInfo[6+1];        // 2015-9-19 NET_PIC_INFO picInfo[6];
    char szPicData[]; //ͼƬ����

};

#endif
