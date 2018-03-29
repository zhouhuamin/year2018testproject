#if !defined(__exif_data_h)
#define __exif_data_h
#define MAX_COMMENT 1000 
    typedef struct tag_ExifInfo { 
        char  Version      [5];                         // �汾
        char  CameraMake   [32];                        // ���������
        char  CameraModel  [40];                        // ����ͺ�
        char  DateTime     [20];                        // ͼƬ����ʱ��
        int   Height, Width;                            // ͼ����
        int   Orientation; 
        int   IsColor; 
        int   Process; 
        int   FlashUsed; 
        float FocalLength;                              // ����
        float ExposureTime;                             // �ع�ʱ��
        float ApertureFNumber;                          // ��Ȧֵ
        float Distance;                                 // Ŀ�����
        float CCDWidth; 
        float ExposureBias;                             // �عⲹ��
        int   Whitebalance;                             // ��ƽ��
        int   MeteringMode;                             // ���ģʽ
        int   ExposureProgram;                          // �ع����
        int   ISOequivalent;                            // IOS�ٶ�
        int   CompressionLevel;                         // ˮƽѹ��
        float FocalplaneXRes; 
        float FocalplaneYRes; 
        float FocalplaneUnits; 
        float Xresolution; 
        float Yresolution; 
        float ResolutionUnit; 
        float Brightness;                              // ����
        char  Comments[MAX_COMMENT];                    // ��ע
        unsigned char * ThumbnailPointer;               /* Pointer at the thumbnail */ 
        unsigned ThumbnailSize;                         /* Size of thumbnail. */ 
        bool  IsExif; 
    } EXIFINFO; 
    /**********************************
     *  �����Ǹ���Ľṹ��
     **********************************/
    #define PLATE_JPEG_SIZE 50*1024
    // Υ����أ�WZ_TYPE_CHAR��ӦFRAME_HEADER�е�wz_type��Ա
    static char WZ_TYPE_CHAR[][32]={
        "����",
        "Υͣ",
        "ѹʵ�߻���",
        "�����ʻ�򲻰������ͷ��ʻ",
        "ѹ˫����",
        "����",
        "����ƻ򸴺ϳ����Ĳ�����ɫ��ʻ",
        "����",
        "���س���",
        "��ʱ��ռ��Ӧ������",
        "����",
        "ѹ˫���߻�ʵ��",
        "����",
        "ռ�ù�������"
    };
    // Υ����أ�WZ_TYPE_NUM��ӦFRAME_HEADER�е�traffic_code��Ա
    static char WZ_TYPE_NUM[][5]={
        "0",
        "1039",
        "1042",
        "1208",
        "1230",
        "1301",
        "1302",
        "1303",
        "1603",
        "4007",
        "4305",
        "4308",
        "4602",
        "1019"
    };
    // �����������,��Ӧ Exif_Data �е� lane_Type��Ա
    static char strLaneType[][16]={
        "�޷���ָʾ",
        "ֱ����ת",
        "��ת��ת",
        "ֱ����ת",
        "��ת",
        "ֱ��",
        "��ת",
        "δ֪"
    };
    // ������ɫ��ȡ�ṹ�壬ȡcar_color[0]
    typedef enum CAR_COLOR
    {
        CL_WHITE = 0,                               // ��ɫ
        CL_GRAY,                                    // ��ɫ
        CL_YELLOW,                                  // ��ɫ
        CL_PINK,                                    // ��ɫ
        CL_RED,                                    // ��ɫ
        CL_GREEN,                                   // ��ɫ
        CL_BLUE,                                    // ��ɫ
        CL_BROWN,                                   // ��ɫ
        CL_BLACK                                    // ��ɫ
    }CAR_COLOR;
    typedef struct _Exif_Head
    {
        char shoot_dev_cam[64];                     // �����豸����
        char shoot_dev_type[64];                    // �����豸�ͺ�
        char img_vender_time[64];                   // ͼ���ع�ʱ��
        char img_gain[64];                          // ͼ������
        char img_take_time[64];                     // ͼ������ʱ��
        char img_rev_time[64];                      // ͼ���޸�ʱ��
    }Exif_Head;
    // ��������
    typedef enum PLATE_TYPE{
        LT_UNKNOWN=0,                                // δ֪����
        LT_BLUE,                                    // ����С����
        LT_BLACK,                                   // ����С����
        LT_YELLOW,                                  // ���Ż���
        LT_YELLOW2,                                 // ˫�Ż��ƣ���β�ƣ�ũ�ó�
        LT_POLICE,                                  // ������
        LT_ARMPOL,                                  // �侯����
        LT_INDIVI,                                  // ���Ի�����
        LT_ARMY,                                    // ���ž���
        LT_ARMY2,                                   // ˫�ž���
        LT_EMBASSY,                                 // ʹ����
        LT_HONGKONG,                                // �����
        LT_TRACTOR,                                 // ������
    }PLATE_TYPE;
    typedef struct Plate_Area                        // ��������
    {
        int left;
        int top;
        int right;
        int bottom;
    }Plate_Area;
    typedef enum RECO_TYPE{
        PLATE_UNKNOWN=0,                            // ���Ƴ�
        PLATE_GOT,                                  // ���Ƴ�
        PLATE_OTHER,                                // ����
    }RECO_TYPE;
    typedef struct
    {
        unsigned short w;                            // ͼ���
        unsigned short h;                            // ͼ���    
        unsigned short bits;                         // ͼ��λ�� 8~16bits
        unsigned short format;                       // ͼ�����ʹ���(0--�Ҷȣ�1--Bayer_RG��2--Bayer_GR��3--Bayer_BG��5--RGB��6--YUV422��7--JPEG, 8--MPEG4 9--h264)    
        unsigned short frame_type;                   // ֡����(0--��ͨ��1--ץ��ͼ��2--����֡)
        unsigned short frame_rev;                    // ����    
        unsigned int  firmware_version;              // �̼�����汾
        unsigned int  device_no;                    // �豸���    
        unsigned int  len;                          // ͼ�����ݳ���    
        unsigned int  speed;                        // ˫��Ȧ����ֵ(us)
        unsigned int  rs232;                        // ������Ϣ(1~4�ֽ�)
        unsigned short year;                        // ͼ��ɼ�ʱ��
        unsigned short month;
        unsigned short day;
        unsigned short hour;
        unsigned short minute;
        unsigned short second;    
        unsigned int ip;                            // �ɼ���ǰ֡�������IP
        unsigned int frame_count;                    // �ܹ���ץ��֡��Ŀ
        unsigned int trigger_count;                  // �ܹ��Ĵ�����
        unsigned int trigger_index;                  // ����������
        unsigned int frame_no;                       // ֡��    
        unsigned int gain;                           // ��ǰץ�Ĳ���
        unsigned int time;                          // �ع�ʱ��
        unsigned int gain_r;                        // ������
        unsigned int gain_g;                        // ������
        unsigned int gain_b;                        // ������
        unsigned int mode;                          // ���������ģʽ
        unsigned int JpegQ;                        // JPEGѹ��Ʒ��
        unsigned int td1;                            // ץ����ʱ(��λus)
        unsigned int td2;                            // �ع���ʱ(��λus)
        unsigned int trig_chl;                    // ����ͨ��
        unsigned int msecond;                        // ms��ʱ
        unsigned int yavg;                        // ƽ������
        unsigned int vid_head;                    // ��ý��ؼ�֡
        unsigned int st_usr_param;                // ������Ϣ
        unsigned int red_time;                    // ��Ƽ�ʱ
        unsigned int grey;                        // һ�����ԭʼ����
        unsigned int wz_type;                        // Υ������
        unsigned short traffic_code;                // Υ�´���
        unsigned char capture_no;                    // ���δ���ץ��������total��
        unsigned char rev_1;                        // ����λ
        unsigned int rev[3];                        // ��������
        unsigned char user_info[64];                // �û�����
    } FRAME__HEADER;                                // ֡ͷ
    typedef struct EXIF_Reco_Result{
        char plate[16];                            // ���ƺ�
        char platecolor[4];                        // ������ɫ
        PLATE_TYPE plate_type;                     // ��������
        int  car_type;                            // ����(��С��)
        char car_color[4];                          // ������ɫ
        int plate_brightness;                        // ��������
        Plate_Area Location;                        // ��������
        int Plate_Jpeg_Size;                        // Сͼ��С
        char Plate_Jpeg_Buf[PLATE_JPEG_SIZE - 2*256];        // Сͼ(jpeg)����
        char curfilename[256];
        char curdir[256];
        short nCarLength;                            // ����
        short nCarLog;                              // ����
        int reco_resv[3];
    }EXIF_Reco_Result;
    typedef struct Exif_Data{
        int  process_type;                        // �����¼�
        int  traffic_code;                        // Υ�´���
        int  limit_speed;                            // ����
        char  address_info[64];                    // ��صص�
        int   reco_output;                        // ���޳���
        EXIF_Reco_Result result;                    // ������Ϣ
        char direction;                            // ���� //1����2�ϣ�3,����4��
        char road_num[11];                        // ·�ڱ��
        int drive_time;                            // �̵ȿ���ʱ��ֹͣ��ǰ��ʻʱ��
        char lane_type;                            // ��������
        char grab_lane_enable;                      // ռ�����ʹ�ܣ�1ʹ�ܣ�0��ʹ��
        char grab_lane_type;                        // ռ������ 0 �ޣ�1�������������ǻ�������
        int illicitly_stop_time;                    // Υ��ͣ�����ʱ��
        char rev[41];                               // �����ֶ�
    }Exif_Data;
    typedef  Exif_Data PLATE_STRUCT;
#endif