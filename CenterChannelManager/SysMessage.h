#if !defined _SYS_MESSAGE_H_
#define _SYS_MESSAGE_H_

#define MAX_MSG_BODYLEN  6*1024*1024

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

#define SYS_MSG_SYSTEM_MSG_SEQUENCE_DISPATCH  SYS_SYSTEM+5


#define SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS            SYS_SYSTEM+7
#define SYS_MSG_SYSTEM_MSG_UPLOADDATA_TO_CUSTOMS_ACK        SYS_SYSTEM+8


#define SYS_MSG_EVENT_SUBSCRIBE               SYS_SYSTEM+0X11                                        //�¼�����
#define SYS_MSG_EVENT_SUBSCRIBE_ACK           SYS_SYSTEM+0X12                                        //�¼����ķ���

#define SYS_MSG_EVENT_EXIT                    SYS_SYSTEM+0X16


#define SYS_MSG_SYSEVENT_PUBLISH                  SYS_SYSTEM+0X20                                        //�¼�����
#define SYS_EVENT_TYPE_UPLOAD_CUSTOMS_DATA     SYS_SYSTEM+0X01
#define SYS_EVENT_TYPE_CUSTOMS_DATA_ACK        SYS_SYSTEM+0X02

#define SYS_MSG_ICCARDREADER_READ            SYS_SUB_SYSTEM_ICCARDREADER+3
#define SYS_MSG_ICCARDREADER_READ_ACK       SYS_SUB_SYSTEM_ICCARDREADER+4


#define SYS_MSG_LEVERCONTROLER_ON            SYS_SUB_SYSTEM_ICCARDREADER+5
#define SYS_MSG_LEVERCONTROLER_ON_ACK       SYS_SUB_SYSTEM_ICCARDREADER+6

#define SYS_MSG_LEVERCONTROLER_OFF         SYS_SUB_SYSTEM_ICCARDREADER+7
#define SYS_MSG_LEVERCONTROLER_OFF_ACK    SYS_SUB_SYSTEM_ICCARDREADER+8


#define SYS_MSG_LCD_DISPLAY              SYS_SUB_SYSTEM_ICCARDREADER+9
#define SYS_MSG_LCD_DISPLAY_ACK              SYS_SUB_SYSTEM_ICCARDREADER+10

#define SYS_MSG_REGATHERDATA                 0X13

#define SYS_MSG_DEVICE_CTRL                 0X17
#define CLIENT_EXCEPTIION_FREE				0X18

#define SYS_MSG_SEND_PICDATA              0X0A
#define SYS_MSG_SEND_PICDATA_ACK          0X0B



#define SYS_EVENT_ICREADER_READ_COMPLETE   "1000000000"      //ic�����������
#define SYS_EVENT_PACKET_DATA                "C000000001"     //�������


#define EVENT_SERVICE_TYPE_CLIENT                 0X00
#define EVENT_SERVICE_TYPE_CONTROLER            0X01


#define SYS_MSG_SYSTEM_CTRL_CMD                       SYS_SYSTEM+30



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
    char device_id[32]; //����ģ��ID��Ψһֵ
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

struct T_Sequence_ControlData {
    char event_id[32];
    char ic_card_data[32];
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



struct NET_PIC_INFO
{
    int  nPicType;                 //����ͼƬ��Ӧ�����ӵ�λ�ã�ǰ�����ҵ�
    int  nPicLen;                  //ͼƬ����
    int  nOffSet;                  //�ڻ�������λ�� 
};

struct T_Upload_Pic_Data {
    char szSequenceNo[32];
    char szChannelNo[32];
    char szAreaNo[32];
    char szIEFlag[32];
    int  nPicNum;
    NET_PIC_INFO  picInfo[6 + 1];       // 2015-9-19 NET_PIC_INFO  picInfo[6];
    char szPicData[];              //ͼƬ����
    
};


struct NET_EVENT_SUBSCRIBE {
    char client_type;
    char user_name[32];
    char pass_word[32];
    char event_serial[32];

};

struct NET_EVENT_SUBSCRIBE_ACK {
    int result;
};

struct NET_EVENT {
    char main_type[32];
    int sub_type;
};

struct NET_EVENT_UPLOAD_CUSTOMSDATA {
    char main_type[32];
    int sub_type;
    T_Upload_Customs_Data customs_data;
};


struct NET_REGATHERDATA_REQ
{
    int   nRegatherType; //1,�ֹ�¼��  2������ǰ���豸��ʽ
    char  channel_no[32];
    char  device_tag[16];
    T_Upload_Customs_Data customs_data;  //�ֹ���ʽ��Ч��������Ч
    
};



struct NET_REGATHER//���ɽṹ��
{
    int iArrange_Type; //�������1-�ֹ�¼�����ݷ�ʽ���ɣ�2-����ǰ���豸��ʽ����; 
    char szChnnl_No[32]; //��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szDeviceTag[16]; // ����ǰ���豸����ʱ���ݵ��豸��ʶ��
    T_Upload_Customs_Data customs_data; //�ֹ���ʽ��������ŵı��ģ�����Ϊ��

};


struct CLIENT_COMMAND_INFO//�����౨��
{
    int iArrange_Type; //�������0-���б��ģ�1-�˹�����;2-�豸����
    char szChnnl_No[32]; // ��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szSequence_No[32]; // ��վ��+ͨ����+������־��0Ϊ����1Ϊ����
    char szDEAL_TYPE[4]; //�˹�������������ͣ�����Ϊ�գ�00��̧��01�����02�������ɣÿ���д	03�������ذ�04���������ӳ���05: ����������
    char szMEM[256]; //��ע˵�������������ݿ��й����ڲ�ѯ��
    int nCustomsDataLen; //����XML��С
    char szCustomsData[]; //���XML�ɼ����ĺͷ��б���	
};

struct CLIENT_EXCEPTION_FREE//�쳣���п��Ʊ���
{
	int iArrange_Type; //�������3-�쳣���п���
	char szChnnl_No[32]; // ��վ��+ͨ����+������־��IΪ����EΪ����
	char szSequence_No[32]; // ��վ��+ͨ����+������־��0Ϊ����1Ϊ����
	char szDEAL_TYPE[4]; //�쳣���б�ǣ�ֵΪ00
	char szMEM[256]; //��ע˵�������������ݿ��й����ڲ�ѯ��
	int nExceptionFreeDataLen; //�쳣���б���XML��С
	char szExceptionFreeData[]; //����쳣����XML���Ʊ��ģ����д��control_info	
};

#endif
