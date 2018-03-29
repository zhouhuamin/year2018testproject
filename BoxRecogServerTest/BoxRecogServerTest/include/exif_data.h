#if !defined(__exif_data_h)
#define __exif_data_h
#define MAX_COMMENT 1000 
    typedef struct tag_ExifInfo { 
        char  Version      [5];                         // 版本
        char  CameraMake   [32];                        // 相机制造商
        char  CameraModel  [40];                        // 相机型号
        char  DateTime     [20];                        // 图片创建时间
        int   Height, Width;                            // 图像宽高
        int   Orientation; 
        int   IsColor; 
        int   Process; 
        int   FlashUsed; 
        float FocalLength;                              // 焦距
        float ExposureTime;                             // 曝光时间
        float ApertureFNumber;                          // 光圈值
        float Distance;                                 // 目标距离
        float CCDWidth; 
        float ExposureBias;                             // 曝光补偿
        int   Whitebalance;                             // 白平衡
        int   MeteringMode;                             // 测光模式
        int   ExposureProgram;                          // 曝光程序
        int   ISOequivalent;                            // IOS速度
        int   CompressionLevel;                         // 水平压缩
        float FocalplaneXRes; 
        float FocalplaneYRes; 
        float FocalplaneUnits; 
        float Xresolution; 
        float Yresolution; 
        float ResolutionUnit; 
        float Brightness;                              // 亮度
        char  Comments[MAX_COMMENT];                    // 备注
        unsigned char * ThumbnailPointer;               /* Pointer at the thumbnail */ 
        unsigned ThumbnailSize;                         /* Size of thumbnail. */ 
        bool  IsExif; 
    } EXIFINFO; 
    /**********************************
     *  以下是港宇的结构体
     **********************************/
    #define PLATE_JPEG_SIZE 50*1024
    // 违章相关，WZ_TYPE_CHAR对应FRAME_HEADER中的wz_type成员
    static char WZ_TYPE_CHAR[][32]={
        "正常",
        "违停",
        "压实线或变道",
        "借道行驶或不按导向箭头行驶",
        "压双黄线",
        "逆行",
        "闯红灯或复合车道的不按灯色行驶",
        "超速",
        "严重超速",
        "长时间占用应急车道",
        "超速",
        "压双黄线或实线",
        "逆行",
        "占用公交车道"
    };
    // 违章相关，WZ_TYPE_NUM对应FRAME_HEADER中的traffic_code成员
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
    // 车道类型相关,对应 Exif_Data 中的 lane_Type成员
    static char strLaneType[][16]={
        "无方向指示",
        "直行右转",
        "左转右转",
        "直行左转",
        "左转",
        "直行",
        "右转",
        "未知"
    };
    // 车身颜色获取结构体，取car_color[0]
    typedef enum CAR_COLOR
    {
        CL_WHITE = 0,                               // 白色
        CL_GRAY,                                    // 灰色
        CL_YELLOW,                                  // 黄色
        CL_PINK,                                    // 粉色
        CL_RED,                                    // 红色
        CL_GREEN,                                   // 绿色
        CL_BLUE,                                    // 蓝色
        CL_BROWN,                                   // 棕色
        CL_BLACK                                    // 黑色
    }CAR_COLOR;
    typedef struct _Exif_Head
    {
        char shoot_dev_cam[64];                     // 拍摄设备厂商
        char shoot_dev_type[64];                    // 拍摄设备型号
        char img_vender_time[64];                   // 图像曝光时间
        char img_gain[64];                          // 图像增益
        char img_take_time[64];                     // 图像拍摄时间
        char img_rev_time[64];                      // 图像修改时间
    }Exif_Head;
    // 车牌类型
    typedef enum PLATE_TYPE{
        LT_UNKNOWN=0,                                // 未知车牌
        LT_BLUE,                                    // 蓝牌小汽车
        LT_BLACK,                                   // 黑牌小汽车
        LT_YELLOW,                                  // 单排黄牌
        LT_YELLOW2,                                 // 双排黄牌（大车尾牌，农用车
        LT_POLICE,                                  // 警车车
        LT_ARMPOL,                                  // 武警车牌
        LT_INDIVI,                                  // 个性化车牌
        LT_ARMY,                                    // 单排军车
        LT_ARMY2,                                   // 双排军车
        LT_EMBASSY,                                 // 使馆牌
        LT_HONGKONG,                                // 香港牌
        LT_TRACTOR,                                 // 拖拉机
    }PLATE_TYPE;
    typedef struct Plate_Area                        // 车牌坐标
    {
        int left;
        int top;
        int right;
        int bottom;
    }Plate_Area;
    typedef enum RECO_TYPE{
        PLATE_UNKNOWN=0,                            // 无牌车
        PLATE_GOT,                                  // 有牌车
        PLATE_OTHER,                                // 其他
    }RECO_TYPE;
    typedef struct
    {
        unsigned short w;                            // 图像宽
        unsigned short h;                            // 图像高    
        unsigned short bits;                         // 图像位数 8~16bits
        unsigned short format;                       // 图像类型代码(0--灰度，1--Bayer_RG，2--Bayer_GR，3--Bayer_BG，5--RGB，6--YUV422，7--JPEG, 8--MPEG4 9--h264)    
        unsigned short frame_type;                   // 帧类型(0--普通，1--抓拍图像，2--心跳帧)
        unsigned short frame_rev;                    // 保留    
        unsigned int  firmware_version;              // 固件程序版本
        unsigned int  device_no;                    // 设备编号    
        unsigned int  len;                          // 图像数据长度    
        unsigned int  speed;                        // 双线圈测速值(us)
        unsigned int  rs232;                        // 串口信息(1~4字节)
        unsigned short year;                        // 图像采集时间
        unsigned short month;
        unsigned short day;
        unsigned short hour;
        unsigned short minute;
        unsigned short second;    
        unsigned int ip;                            // 采集当前帧的摄像机IP
        unsigned int frame_count;                    // 总共的抓拍帧数目
        unsigned int trigger_count;                  // 总共的触发数
        unsigned int trigger_index;                  // 触发组索引
        unsigned int frame_no;                       // 帧号    
        unsigned int gain;                           // 当前抓拍参数
        unsigned int time;                          // 曝光时间
        unsigned int gain_r;                        // 红增益
        unsigned int gain_g;                        // 绿增益
        unsigned int gain_b;                        // 蓝增益
        unsigned int mode;                          // 摄像机工作模式
        unsigned int JpegQ;                        // JPEG压缩品质
        unsigned int td1;                            // 抓拍延时(单位us)
        unsigned int td2;                            // 曝光延时(单位us)
        unsigned int trig_chl;                    // 触发通道
        unsigned int msecond;                        // ms计时
        unsigned int yavg;                        // 平均亮度
        unsigned int vid_head;                    // 流媒体关键帧
        unsigned int st_usr_param;                // 软触发信息
        unsigned int red_time;                    // 红灯计时
        unsigned int grey;                        // 一个点的原始亮度
        unsigned int wz_type;                        // 违章类型
        unsigned short traffic_code;                // 违章代码
        unsigned char capture_no;                    // 本次触发抓拍张数（total）
        unsigned char rev_1;                        // 保留位
        unsigned int rev[3];                        // 保留参数
        unsigned char user_info[64];                // 用户数据
    } FRAME__HEADER;                                // 帧头
    typedef struct EXIF_Reco_Result{
        char plate[16];                            // 车牌号
        char platecolor[4];                        // 车牌颜色
        PLATE_TYPE plate_type;                     // 车牌类型
        int  car_type;                            // 车型(大车小车)
        char car_color[4];                          // 车身颜色
        int plate_brightness;                        // 车牌亮度
        Plate_Area Location;                        // 车牌坐标
        int Plate_Jpeg_Size;                        // 小图大小
        char Plate_Jpeg_Buf[PLATE_JPEG_SIZE - 2*256];        // 小图(jpeg)数据
        char curfilename[256];
        char curdir[256];
        short nCarLength;                            // 车长
        short nCarLog;                              // 车标
        int reco_resv[3];
    }EXIF_Reco_Result;
    typedef struct Exif_Data{
        int  process_type;                        // 卡口事件
        int  traffic_code;                        // 违章代码
        int  limit_speed;                            // 限速
        char  address_info[64];                    // 监控地点
        int   reco_output;                        // 有无车牌
        EXIF_Reco_Result result;                    // 车牌信息
        char direction;                            // 方向 //1东，2南，3,西，4北
        char road_num[11];                        // 路口编号
        int drive_time;                            // 绿等卡口时，停止线前行驶时间
        char lane_type;                            // 车道类型
        char grab_lane_enable;                      // 占道检测使能，1使能，0不使能
        char grab_lane_type;                        // 占道类型 0 无，1公交车车道，非机动车道
        int illicitly_stop_time;                    // 违章停车检测时间
        char rev[41];                               // 保留字段
    }Exif_Data;
    typedef  Exif_Data PLATE_STRUCT;
#endif