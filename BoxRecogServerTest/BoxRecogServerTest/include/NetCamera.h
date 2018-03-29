/*! \file 
  \brief NetCamera头文件
  \author <Camyu>
  \date <2014-04-10>
  \version <V2.3.5>
  \detailed description for NetCamera.cpp
*/
#ifndef _NET_CAMERA_H_
#define _NET_CAMERA_H_

////////////////////////////////////////////////////////////////////////////
//SYSTEM DEFINE
#ifndef _WIN32                //linux

#define NC_EXT  
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define LW_THREAD pthread_t

#ifndef _CRI_SEC_
#define _CRI_SEC_
#define CRITICAL_SECTION pthread_mutex_t
#define InitializeCriticalSection(x) pthread_mutex_init(x,0)
#define DeleteCriticalSection(x) pthread_mutex_destroy(x)
#define EnterCriticalSection(x) pthread_mutex_lock(x)
#define LeaveCriticalSection(x) pthread_mutex_unlock(x)
#endif

#define LAST_ERR errno
#define SOCK_RETRY EAGAIN
#define closesocket(x) close(x)
#define Sleep(X) usleep(1000*X)

#else                    //windows

#include <windows.h>
#include <assert.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable : 4996)

#define LAST_ERR WSAGetLastError() 
#define SOCK_RETRY WSAEWOULDBLOCK
#define LW_THREAD HANDLE

#ifndef    NETCAMERA_EXPORTS
#define NC_EXT    __declspec(dllimport)
#else    
#define    NC_EXT    __declspec(dllexport)
#endif

#endif

#include <set>
#include <string>
#include <time.h>
////////////////////////////////////////////////////////////////////////////////
class CY_lock
{
    CRITICAL_SECTION CS;
public:
    CY_lock(){InitializeCriticalSection(&CS);}
    ~CY_lock(){DeleteCriticalSection(&CS);}
    void Lock(void){EnterCriticalSection(&CS);}
    void Unlock(void){LeaveCriticalSection(&CS);}
};
//STRUCT DEFINE
#ifndef _FRAME_HEADER_
#define _FRAME_HEADER_

/*!
  \brief 结构体：相机交互与控制库结构体
*/
struct FRAME_HEADER
{
	unsigned short w;								///< 图像高
	unsigned short h;								///< 图像高
	unsigned short bits;							///< 图像位数 8~16bits
	unsigned short format;                    	///< 图像类型代码(0--灰度，1--Bayer_RG，2--Bayer_GR，3--Bayer_BG，5--RGB，6--YUV422，7--JPEG, 8--MPEG4 9--h264)    
	unsigned short frame_type;              ///< 帧类型(0--普通，1--抓拍图像，2--心跳帧)
	unsigned short frame_rev;                ///< 保留    
	unsigned int  firmware_version;        ///< 固件程序版本
	unsigned int  device_no;                   ///< 设备编号    
	unsigned int  len;                        		///< 图像数据长度    
	unsigned int  speed;							///< 双线圈测速值(us)
	unsigned int  rs232;							///< 串口信息(1~4字节)
	unsigned short year;							///< 图像采集时间(年)
	unsigned short month;						///< 图像采集时间(月)
	unsigned short day;							///< 图像采集时间(日)
	unsigned short hour;						///< 图像采集时间(时)
	unsigned short minute;						///< 图像采集时间(分)
	unsigned short second;    					///< 图像采集时间(秒)
	unsigned int ip;									///< 采集当前帧的摄像机IP
	unsigned int frame_count;                ///< 总共的抓拍帧数目
	unsigned int trigger_count;               ///< 总共的触发数
	unsigned int trigger_index;				///< 触发组索引
	unsigned int frame_no;						///< 帧号    
	unsigned int gain;								///< 当前抓拍参数
	unsigned int time;								///< 曝光时间
	unsigned int gain_r;							///< 红增益
	unsigned int gain_g;							///< 绿增益
	unsigned int gain_b;							///< 蓝增益
	unsigned int mode;                        	///< 摄像机工作模式
	unsigned int JpegQ;                        	///< JPEG压缩品质
	unsigned int td1;								///< 抓拍延时(单位us)
	unsigned int td2;								///< 曝光延时(单位us)
	unsigned int trig_chl;                    	///< 触发通道
	unsigned int msecond;                     ///< ms计时
	unsigned int yavg;                        	///< 平均亮度
	unsigned int vid_head;                    	///< 流媒体关键帧
	unsigned int st_usr_param;               ///< 软触发信息
	unsigned int red_time;                    	///< 红灯计时
	//unsigned long rev[6];                    	///< 保留参数
	unsigned int grey;								///< 一个点的原始亮度
	unsigned int wz_type;                       ///< 违章类型，d0-超速,d1-闯红灯
	unsigned short traffic_code;             ///< 违章代码
	unsigned char capture_no;               ///< 本次触发抓拍张数（total）
	unsigned char rev_1;                        ///< 保留位
	unsigned int rev[3];							///< 保留参数
	unsigned char user_info[64];				///< 用户数据
};
#endif
/*!
  \brief 枚举：枚举变量
*/
typedef enum {
	DEFAULT_CITY_SET = 600,                    	///< 设置默认省份
	DEFAULT_CHAR_SET,								///< 设置默认城市字符
	LANE_0_SET,                                			///< 设置车道0的识别区域
	LANE_1_SET,											///< 设置车道1的识别区域
	LANE_2_SET,                                			///< 设置车道2的识别区域
	LANE_3_SET,                                			///< 设置车道3的识别区域
	PLATE_RECO_ENABLE,                        	///< 车牌识别使能(0－禁用，1－使能)
	CAR_TYPE_ENABLE,								///< 车型识别使能(0－禁用，1－使能)
	CAR_COLOR_ENABLE,                          	///< 车身颜色识别使能(0－禁用，1－使能)
	RECO_REGION_SET,								///< 设置识别区域
	SAVE_RECO_CONFIG,                         	///< 保存识别参数
	VIDEO_DETECT_ENABLE,                       ///< 视频检测使能
	DETECT_REGION_SET,                        	///< 设置视频检测区域
	SAVE_DETECT_CONFIG,                        	///< 频检测逻辑设置
	TRIG_CHARACTER_INFO,                       ///< 读取抓拍文字设置
	DSP_ID_REQ,                                			///< 
	DSP_KEY_FILE,										///< 
	TRAFFIC_ENABLE,                            		///< 红绿灯使能
	TRAFFIC_REGION_SET,                        	///< 红灯区域设置
	SAVE_TRAFFIC_CONFIG,                        ///< 保存红绿灯配置
	IO_SER_CHECK_EANABLE,                     	///< 触发源自动切换使能
	TRAFFIC_MODE,                              		///< 相机检测模式(0－卡口电警，1－标准电警，2－标准卡口)
	TRIG_JPRG_QT,                              		///< 抓拍图片品质因数
	ESPU_STATE = 650,                          		///< state of ESPU0608
	AT88_STATE,                                			///< state of AT88SC0104C
	DS28_STATE,                                			///< state of DS28CN01
	DEVICE_NO_TMP = 700,                            ///<设备编号
	ROAD_NO_TMP,                       ///<卡口编号
	CHL_NO_TMP ,                        ///<车道编号
	VIDEO_LANE_ENABLE,                  ///<video detect lane_num enable
	IO_SER_CHEKCK_HEART_EANBLE,                ///<外触发，视频触发，心跳切换 704
	PLATE_JPEG_GET_ENABLE ,             ///<车牌小图获取使能 705
	CAR_LOGO_ENABLE                       ///<车标识别706
}DSP_CMD;
/* 号牌识别和视频检测参数结构 */                                                                

/*!
  \brief 结构体：单个线圈结构
*/                                                                    
typedef struct Single_Reco_Region {                                                
	unsigned int  show_enable;                  ///< 使能状态                                                 
	unsigned int  x0;									///< x0                                    
	unsigned int  y0;                        			///< y0                                    
	unsigned int  x1;									///< x1                                 
	unsigned int  y1;									///< y1                                    
	unsigned int  x2;									///< x2                                          
	unsigned int  y2;									///< y2                                        
	unsigned int  x3;									///< x3                                    
	unsigned int  y3;									///< y3                                                                     
} Single_Reco_Region;                                                              

/*!
  \brief 结构体：识别区域结构
*/                                                                        
typedef struct Reco_Region                                                         
{                                                                                  
    unsigned int Reco_Region_Count;			///< 数量                                            
    Single_Reco_Region  Region[4];				///< 存放区域数组                                       
}Reco_Region;                                                                      


/*!
  \brief 结构体：视频检测结构
*/                                                                                       
typedef struct video_info{                                                         
	unsigned int  show_enable;  					///< 该视频线圈区域是否显示                                        
	int    total;               								///< 该视频线圈抓拍总张数                                         
	int    inv_12;              							///< 该视频线圈抓拍多张图片时第1张和第2张的抓拍时间间隔（单位ms）                     
	int    inv_23;              							///< 该视频线圈抓拍多张图片时第2张和第3张的抓拍时间间隔（单位ms）                     
	unsigned int  x0;    								///< 该视频线圈坐标点值x0 
	unsigned int  y0;									///< 该视频线圈坐标点值y0 
	unsigned int  x1;         							///< 该视频线圈坐标点值x1 
	unsigned int  y1;           						///< 该视频线圈坐标点值y1                                     
	unsigned int  x2;									///< 该视频线圈坐标点值x2 
	unsigned int  y2;									///< 该视频线圈坐标点值y2 
	unsigned int  x3;									///< 该视频线圈坐标点值x3 
	unsigned int  y3;									///< 该视频线圈坐标点值y3 
}video_info;

/*!
  \brief 结构体：视频检测结构
*/  
typedef struct Single_Detect_Config {                                              
	unsigned int mode;          						///< 为单线圈模式，1为双线圈模式                                    
	int          flash_ring;       						///< 闪光灯是否轮闪                                           
	int          flash_chan;    							///< 视频触发时闪光灯通道                                         
	int          distance_ab;   						///< 组中第一个视频线圈到第二个的距离                                   
	video_info   info[2];								///< 视频信息                                    
	char         resv[20];								///< resv信息                                                       
}Single_Detect_Config;                                                             

/*!
  \brief 结构体：视频检测参数结构
*/                                                                    
typedef struct Detect_Config{                                                      
    unsigned int         Detect_Config_Count;       ///< 数量                                  
    Single_Detect_Config Region[4];                   ///< 范围数组                               
}Detect_Config;                                                                    


/*!
  \brief 结构体：红绿灯参数结构
*/                                                                      
typedef struct lane_info{                                                          
	unsigned int  show_enable;  						///< 该红绿灯区域是否显示                                           
	unsigned int  x0;										///< 该红绿灯区域坐标点值x0                                       
	unsigned int  y0;										///< 该红绿灯区域坐标点值y0                                    
	unsigned int  x1;										///< 该红绿灯区域坐标点值x1                                      
	unsigned int  y1;           							///< 该红绿灯区域坐标点值y1                                         
	unsigned int  x2;										///< 该红绿灯区域坐标点值x2                                       
	unsigned int  y2;										///< 该红绿灯区域坐标点值y2                                    
	unsigned int  x3;										///< 该红绿灯区域坐标点值x3                                     
	unsigned int  y3;										///< 该红绿灯区域坐标点值y3                                                
}lane_info;                                                                        
/*!
  \brief 结构体：交通参数结构
*/  
typedef struct traffic_info {                                                      
    unsigned int total;									///< 总数                          
    lane_info redarea[4];									///< 红绿灯信息                           
}traffic_info;                                                                     

/*!
  \brief 结构体：文字叠加
*/   
#define CHARACTER_INFO_SIZE_MAX 	256
typedef struct _CHARACTER_INFO_ {
	int ver;
	char    info[CHARACTER_INFO_SIZE_MAX];
	char	road[32];
	char	orient[32];
	int	speed_limit;
	int jpegx;
	int jpegy;
	int jpeg_trigx;
	int jpeg_trigy;
	int streamx;
	int streamy;
	int color;
	int mode;
	int rev[3];
}character_info;

/*!
  \brief 结构体：串口抓拍
*/ 
typedef struct ser_groupinfo
{
	int flash_chl;			///< 闪光灯通道
	int flash_ring;		///< 轮闪模式使能
	int timeout;			///< 超时设置
	int delay;				///< 延迟触发时间(保留)
	int mode;				///< 触发模式:0--5字节简单模式1--带校验的字节模式
	int total;				///< 抓拍总张数
	int inv_12;				///< 当抓拍多张时第一张和第二张的抓拍时间间隔
	int inv_23;				///< 当抓拍多张时第二张和第三张的抓拍时间间隔
	char sync[4];			///< 同步字头
	int info_fffec_len;	///< 用户数据有效长度(0~INFO_MAX_LEN)
	int red_state;			///< 红灯状态
	timeval red_start;	///< 红灯开始时间
	int option;				///< 选项
}ser_groupinfo;

/* io抓拍设置 */  
typedef struct giogroupinfo
{
	int flash_chl;			///< 闪光灯通道
	int flash_ring;	   	///< 闪光灯轮闪模式(0--禁用1--使能)
	int timeout;			///< 超时(保留)
	int delay;				///< 延迟触发时间(保留)
	int io_a;				///< 组中的第个IO口(必须是~8)
	int io_b;				///< 组中的第个IO口(必须是~8)
	int io_c;					///< 组中的第个IO口(1~8,为表示当前这个触发组只有两个IO)

	int total_a;			///< 组中的第个IO抓拍几张,为时不抓拍
	int total_b;			///< 组中的第个IO抓拍几张,为时不抓拍
	int total_c;			///< 组中的第个IO抓拍几张,为时不抓拍
	int distance_ab;	///< 组中第个IO到第个IO的线圈距离(单位:cm)
	int distance_bc;		///< 组中第个IO到第个IO的线圈距离(单位:cm)

	int inv_a_12;			///< 当第个IO口抓拍多张图片时第张和第张的抓拍时间间隔(单位:ms)
	int inv_a_23;			///< 当第个IO口抓拍多张图片时第张和第张的抓拍时间间隔(单位:ms)
	int inv_b_12;			///< 当第个IO口抓拍多张图片时第张和第张的抓拍时间间隔(单位:ms)
	int inv_b_23;			///< 当第个IO口抓拍多张图片时第张和第张的抓拍时间间隔(单位:ms)
	int inv_c_12;			///< 当第个IO口抓拍多张图片时第张和第张的抓拍时间间隔(单位:ms)
	int inv_c_23;			///< 当第个IO口抓拍多张图片时第张和第张的抓拍时间间隔(单位:ms)

	int red_state;			///< 红灯状态标识(1--红灯,0--非红灯)
	timeval red_start;	///< 红灯开始时间
	int option;				///< 选项
}giogroupinfo;
/* io抓拍设置 */  
typedef struct pc_giogroupinfo
{
	int rev[5];				///< 保留个,置,已备扩展
	int valid;				///< 有效性(0无效,1有效)
	int gio_type;			///< gio类型(0--双线圈测速1--三线圈测试)
	giogroupinfo gioinfo;
}pc_giogroupinfo;


//////////////////////////////////////////////////////////////////////////
//CAMERA TYPE DEFINE
#ifndef _NET_CAMERA_TYPE_DEFINE_
#define _NET_CAMERA_TYPE_DEFINE_
#define NC_TYPE_UNKOWN          (0x00000000)			///< 未知相机类型 
#define NC_TYPE_J               (0x00010000)					///< J型相机 
#define NC_TYPE_N               (0x00020000)					///< NA型相机 
#define NC_TYPE_G               (0x00040000)					///< GA型相机 
#define NC_TYPE_SPV             (0x00080000)					///< SPV相机 
#define NC_TYPE_SMV             (0x00090000)				///< SMV相机 
#define NC_TYPE_TKV             (0x00100000)					///< TKV相机 
#endif


//////////////////////////////////////////////////////////////////////////
#ifndef _NET_CAMERA_CMD_DEFINE_
#define _NET_CAMERA_CMD_DEFINE_
#define NORECOGNIZE        0
#define BAYONET            1
#define EPOLICE            2

#define ERR_OK                0											///<  OK
#define ERR_ADDR            1											///<  无效的IP地址 
#define ERR_SOCKET            2										///<  未能创建SOCKET 
#define ERR_CONNECT            3									///<  未能连接目标 
#define ERR_RECV            4											///<  接收错误 
#define ERR_RECV_TIMEOUT    5									///<  接收错误超时 
#define ERR_SEND            6											///<  发送错误 
#define ERR_SEND_TIMEOUT    7									///<  发送错误超时 
#define ERR_MEMORY          8										///<  没有足够的内存空间 
#define ERR_TIMEOUT            9									///<  连接超时 
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CAM_HB_ACK 0xAA55AA55
#define LIB_HB_ACK 0x55AA55AA

//camera cmd define
//read only
//#define  ID                 0x00											///< 设备码(0--无设备,!0--有设备) 
#define  WIDTH              0x01										///< 传感器水平像素数(宽度) 
#define  HEIGHT             0x02										///< 传感器垂直像素数(高度) 
#define  BITS               0x03											///< 图像数据的A/D位数 
#define  COLOR              0x04										///< 图像数据格式(0--灰度，1--Bayer_RG，2--Bayer_GR，3--Bayer_BG，5--RGB，6--YUV422，7--JPEG ,8--MPEG4 ,9--H.264 ) 
#define  SP_CHECK           0x06										///< 写：0x13572468 0x1C2E3A9B 
#define  RTC_STATE              0x06									///<  读：实时时钟功能是否正常,0－不正常，1－正常 


#define  C_SOFT_VERSION     0x07								///< 0x07~0x0A 16个字节表示版本信息 
#define  C_PRODUCT_NO       0x0B								///< 0x0B~0x0E 16个字节表示产品信息 
#define  CCD_TEMPERATRUE     0x0F							///<  前端温度 - 读
#define  HEART_BEAT         0x0F									///<  心跳命令 - 写
#define  SOFT_VERSION       0x12									///< 固件程序版本 
#define  DEVICE_NO          0x13									///< 摄像机编号 
#define  PRODUCT_NO         0x14									///< 参数编号 0-DC1335J 1-DC2035J 2-DC1334N 3-DC2034N 

#define  C_DEVICE_NO        0x15									///< 0x15~0x18 16个字节表示产品编号信息 
#define  C_SP_PRODUCT_INFO  0x19							///< 0x19~0x1C 16个字节表示产品特殊信息 
#define  USB_CAPABILITY     0x1D									///< U盘容量 

#define     USB_SPECIAL        0x1E									///< STORAGE 控制命令 
  #define IF_USB_FORMAT         0x1								///< U盘格式化 
  #define IF_USB_CHECK            0x2								///< U盘自检 
  #define IF_USB_AUTO_FORMAT    0x3							///< U盘自动格式化 
  #define IF_USB_FLAG_FORMATED    0x4						///< U盘禁用验 
  #define IF_USB_DISABLE        0x5								///< U盘禁用验 

#define     ST_SPECIAL            0x1F								///< 内部状态 
// #define ST_USB_CHECK_OK            0x1						///< U盘自检 OK 
// #define ST_USB_CHECK_ERR        0x2						///< U盘自检 ERR 
// #define ST_USB_CHECK_NONE        0x3						///< U盘自检 NOT FOUND 

#define  GAIN_UNIT             0x20									///< 当前增益值 
#define  OFFSET_UNIT        0x21									///< 当前偏置值 
#define  FREQ_UNIT          0x22										///< 当前帧频值 
#define  TIME_UNIT          0x23										///< 当前曝光时间值 
#define  RGB_GAIN_UINT         0x24								///< 彩色增益调整单位 

#define  MCU_UPDATE_NOW        0x2E
#define  CUR_Y_AVG            0x2F									///< 平均亮度 

#define  MAX_GAIN           0x30									///< 增益最大值 
#define  MIN_GAIN           0x31									///< 增益最小值 
#define  MAX_OFFSET         0x32									///< 偏置最大值 
#define  MIN_OFFSET         0x33									///< 偏置最小值 
#define  MAX_FREQ           0x34									///< 帧频最大值 
#define  MIN_FREQ           0x35										///< 帧频最小值 
#define  MAX_TIME           0x36									///< 曝光时间最大值 
#define  MIN_TIME           0x37										///< 曝光时间最小值 
#define  MAX_BALANCEMODE    0x38							///< 白平衡方式选择最大值 
#define  MAX_AGCLIMIT       0x39								///< AGC门限值最大值 
#define  MAX_AGCSELECT      0x3a								///< AGC取样区域选择最大值 


#define  MIX_TOTAL_GAIN        0x3b							///< 彩色增益最小值 
#define  MAX_TOTAL_GAIN     0x3c								///< 彩色增益最大值 

/**< spv中0x60~0x63 16个字节表示版本信息*/
#define  C_AFE_INFO            0x60									///< 0x60~0x63 16个字节表示版本信息
#define  AFE_TCLK_A         0x64									///< A通道  TCLK
#define  AFE_TCLK_B         0x65									///<  B通道  TCLK
#define  AFE_TCLK_C         0x66									///<  C通道  TCLK
#define  AFE_TCLK_D         0x67									///< D通道  TCLK
#define  C_AFE_FW_INFO        0x68								///<  spv中0x68~0x6b 16个字节表示版前端固件信息

#define  ONVIF_VERSION        0x70								///<  0x70~0x73 16个字节表示onvif版本信息*/

#define  PROTECT_AREA       0x8f									///<  0--0x13f区域为命令区，可以任意设置 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//not read only
#define SWAP32(y) (( ((y)>>24) & 0x000000ff) | (((y) >> 8)  & 0x0000ff00) | (((y) << 8)  & 0x00ff0000) | (((y) << 24) & 0xff000000) )
#define  GAIN               0x90											///< 当前增益值 
#define  OFFSET             0x91										///< 当前偏置值 
#define  FREQ               0x92											///< 当前帧频值 
#define  TIME               0x93											///< 当前曝光时间值 
#define  SYNC               0x94										///< 同步方式(0--外触发，1--单次触发) 
#define        SYNC_ONE_SHOT    0x01							///< 抓拍模式 
#define        SYNC_TRIG        0x02									///< 监控模式 
#define        SYNC_VIDEO        0x03								///< 视频模式 
//...
#define  AGCMODE            0xb0									///< 增益控制方式(1--自动(AGC)，0--手动) 
#define  AGCLIMIT            0xb1									///< AGC门限值 (0-255) 
#define  AGCSELECT          0xb2									///< AGC取样区域选择 低16位有效，分别对应4x4的16个区域，顺序为从左到右，从上到下 
#define  AGCTIME            0xb3										///< AGC与电子快门连动 
#define  AGC_GAIN_MAX       0xb4								///< AGC调整范围 0 ~ Max dB 
#define  AGC_TIME_MIN       0xb5								///< AGC与电子快门连动时，电子快门调整最小值(us) 
#define  AGC_TIME_MAX       0xb6								///< AGC与电子快门连动时，电子快门调整最大值(us) 
#define  AREA_X                0xb7									///< 开窗区域左上角x坐标 
#define  AREA_Y             0xb8										///< 开窗区域左上角y坐标 
#define  AREA_W                 0xb9									///< 开窗区域宽度 
#define  AREA_H                 0xba									///< 开窗区域高度 

#define     AE_MODE            0xbb									///< 自动使用的测光模式(0-basic) 
#define     BL_PARAM            0xbc								///< 评价测光的强度(-100 ~ 100) 
#define  ROTATION            0xbd									///< JPEG图片旋转模式(0,90,180,270) 
//...
#define  GAIN_R             0xc9										///< 当前R增益值 
#define  GAIN_G             0xca										///< 当前G增益值 
#define  GAIN_B             0xcb										///< 当前B增益值 
#define  FTP_ACCOUNT_SET    0xcc								///< FTP帐号设置 
#define  WATER_MARK_SET        0xcd							///< 水印设置 
#define  IO_LOGIC_SET        0xce									///< 线圈逻辑设置 
#define  CHARACTER_INFO_SET    0xcf							///< jpeg文字叠加设置 

#define  NTP_SETTING        0xd1									///< NTP服务器设置 
#define  BALANCEMODE        0xd2								///< 白平衡校正方式选择 0--手动白平衡 1--自动白平衡 
#define  PRE_SET_GAIN_R     0xd3									///< 预设R增益 
#define  PRE_SET_GAIN_G     0xd4									///< 预设G增益 
#define  PRE_SET_GAIN_B     0xd5									///< 预设B增益 

#define  RADAR_LOGIC_SET    0xd6								///< 串口逻辑设置 
#define  NETCOM_BAUD_SET    0xd7							///< 透明串口波特率设置 
#define  CHAL_OF_TRIG_IMG    0xd8								///< 抓拍图片传送通道 (0 - server, 1 - normal, 2 - both) 
//...
#define AUTO_JPEG_Q_EN        0xda								///< JPEG码流控制使能
#define  GAMMA_DATA              0xdb							///< 加载GAMMA曲线参数  4K SHORT
#define LED_HIGHTIME_EN        0xdc							///< 频闪灯高电平时间可调使能,D0=0禁止，D0＝1使能
#define LED_HIGHTIME        0xdd									///< 频闪灯高电平时间,单位us，0--6000us
#define  ONVIF_AUTH         0xde									///< 1:登录onvif需要认证   其他值：登录onvif不需要认证 
//...

#define  AUTOGRAY           0xe0									///< 自动灰度增强(1--使能，0--禁止) 
#define  GRID               0xe1											///< 像素抽点值 (不抽点，1/2抽点) 
// #define    GRID_1B1  0												///< 不抽点 
// #define    GRID_1B2  1												///< 1/2抽点 
// #define    GRID_1B4  2												///< 1/4抽点 

#define  BIN                0xe2											///< 像素抽点合并 (d0-水平合并 d1-垂直合并) 
#define  GAMMA              0xe3										///< GAMMA使能 (0- 默认 1-auto gamma) 
#define  T1_MODE            0xe4										///< 
#define  IMAGE_FORMAT       0xe4								///< 视频模式图像输出格式 0-BAYER_8b 1-BAYER_12b 2-YUV422 3-JPEG 4-MPEG4 
// #define    IF_BAYER_8B   0											///< 
// #define    IF_BAYER_12B  1										///< 
// #define    IF_YUV422     2											///< 
// #define    IF_JPEG       3												///< 单JPEG 
// #define    IF_MPEG4      4											///< 单H264 
// #define    IF_MAX        4

#define  TRIG_IMAGE_FORMAT     0xe5							///< 触发图像格式 
#define  IMAGE_ZOOM_W       0xe6								///< 图像输出尺寸:宽 
#define  IMAGE_ZOOM_H       0xe7								///< 图像输出尺寸:高 
//...
#define  CHARACTER_X_TRIG   0xe9								///< 抓拍图片显字坐标:X -1不显示 最新固件已经不用了
#define  CHARACTER_Y_TRIG   0xea								///< 抓拍图片显字坐标:Y -1不显示 最新固件已经不用了
//..
#define  AUTOGRAY_PARAM     0xec								///< 自动灰度增强强度（0~32） 
#define  MPEG_INIT          0xed									///< 初始化码流 
#define  CHARACTER_X        0xee									///< 显字坐标:X -1不显示 
#define  CHARACTER_Y        0xef									///< 显字坐标:Y -1不显示 

//#define  IO_DIR                  0xe8									///< 
//#define  IO_INT                  0xe9									///< 抓拍图片显字坐标:X -1不显示 
#define  IO_VAL                  0xea									///< 抓拍图片显字坐标:Y -1不显示 
#define  IO_ACK                  0xeb									///< TORAGE send to server every .. ms 
#define  IO_LED                  0xec									///< 自动灰度增强强度（0~32） 
#define  REBOOT                  0xed									///< 初始化码流 

#define  SOFT_TRIGGER       0xf0									///< 软件触发命令 1-主通道闪光 0-副通道闪光 
#define  TEST_IMAGE         0xf1									///< 测试图形选择 1-测试图像 0-正常图像 
#define  TIME_STAMP         0xf2									///< 对时命令，参数为标准相差秒 
#define  NOISE_FLT          0xf4										///< 噪声滤波参数
#define  EDGE_ENAHNCER      0xf5								///< 边缘增强级数(0--4)

#define  DRC                0xf6											///< 动态增强参数 

#define  IMAGE_ZOOM_W_STREAM    0xf7					///< 流媒体图像输出尺寸 
#define  IMAGE_ZOOM_H_STREAM    0xf8					///< 流媒体图像输出尺寸 
#define  POSITION_FLASH              0xf9						///< flash pos (d0~d15:start percent, d16~d31:wide percent) 
#define  POSITION_LED              0xfa							///< led pos (d0~d15:start percent, d16~d31:wide percent) 
#define  LOGO_POSITION            0xfb							///< d0~d15: x, d16~d31: y 
#define  SATURATION                0xfc							///< saturation set (0 ~ 255 , 127 def, 180 or 220 suggested) 
#define  GIC                    0xff										///< gic params 

#define  SERVER_IP          0x100									///< 服务器IP(监控模式有效) 
#define  SERVER_PORT        0x101									///< 服务器PORT(监控模式有效) 
#define  TRI_JPEG_Q         0x102									///< 抓拍时JPEG质量(监控模式有效) 
#define  TRI_EXP_TIME       0x103									///< 监控模式有效(抓拍时采用的曝光时间) 
#define  TRI_GAIN           0x104									///< 监控模式有效(抓拍时采用的增益) 
#define  TRI_GAIN_R            0x105								///< 监控模式有效(抓拍时采用的R增益) 
#define  TRI_GAIN_G            0x106								///< 监控模式有效(抓拍时采用的G增益) 
#define  TRI_GAIN_B            0x107								///< 监控模式有效(抓拍时采用的B增益) 
#define  TRI_AGC_TIME_MAX               0x108				///<  自动调节时，抓拍电子快门调整最大值(us)*/
#define  CHARACTER_INFO_SET_TRIG     0x109				///<  抓拍文字叠加*/
#define  SETVIRTUALCOIL              0x10a						///<  虚拟线圈设置*/
#define  SETRECOGNIZEZONE            0x10b					///<  识别区域设置*/

#define  EXT_SYNC_DELAY        0x110							///< 外同步延时 
#define  EXT_TRIG_DELAY     0x111								///< 外触发延时 
#define  FLASH_MODE            0x112								///< 闪光灯输出模式（软触发时有效）（0-普通模式 1-轮闪模式） 
#define  IOTRIG_MODE        0x113								///< 触发抓拍模式选择 
#define    IOTRIG_MODE_SPEED    0								///< 测速模式(以微秒为单位) 
#define    IOTRIG_MODE_SIGNAL    1							///< 红绿灯模式(以秒为单位) 
#define  LED_OUT_MODE     0x114								///< 频闪倍频与否(d0-频闪倍频使能位,d1-频闪输出极性位) 
#define VEDIO_OUTPUT_EN    0x115								///< 控制模拟视频输出（0-禁止 1-使能） 

#define  CMOS_FLASH_MODE        0x116						///< CMOS闪光灯模式 (0 - LED灯模式, 1 - 气体灯模式) 0 led mode(ers always), 1 flash mode(ers when day, grr when night) 
#define  CHL_OF_TRIG_IMG        0x117							///< trigger image trans chl (0 - snap server, 1 - ftp server)
    
#define  CMOS_FLASH_MOD_FRM    0x118					///< 昼夜适应判断张数 
#define  CMOS_FLASH_MOD_LV    0x119						///< 昼夜适应判断基准 
#define SnapPicSetMode     0x11a								///< 抓拍图像尺寸设置模式 0--全尺寸 1--可设置尺寸      
#define RADAR_TRIG_AGREEMENT 0x11E						///< trig agreement select
#define RADAR_MODE_DEF        0									///< id chl inv_h inv_l data0 data1 data2 data3 sum
#define RADAR_MODE_RADAR    1								///< id data0 data1 data2 data3

#define  TRI_PARAM_EN           0x11F							///< 监控模式抓拍参数使能(D0 - "曝光时间",1表示参数使用对立参数,0表示使用视频参数 D1 - "增益" D2 - "RGB彩色增益") 
//#define TRI_PARAM_TIME_EN      (1<<0)					///<  抓拍图像独立曝光时间使能 
//#define TRI_PARAM_GAIN_EN      (1<<1)					///<  抓拍图像独立增益使能 
//#define TRI_PARAM_RGB_GAIN_EN  (1<<2)				///<  抓拍图像独立RGB增益使能 
//#define TRI_PARAM_GRAY_EXT_EN  (1<<3)				///<  抓拍图像启用灰度增强 
//#define TRI_PARAM_GRAY_DRC_EN  (1<<4)				///<  抓拍图像启用动态增强（连拍帧数大于1时，必须禁用） 

#define  IP_ADDR            0x120									///< 摄像机IP地址 
#define  GATEWAY            0x121									///< 摄像机网关 
#define     NETMASK            0x122								///< 网络地址掩码 
#define  CTRL_PORT          0x123									///< 控制端口 
#define  TRAN_PORT          0x124									///< 传输端口 

#define  TF_COUNT           0x125									///< 连续抓拍帧数 (1-3) 
#define  JPEG_SET           0x126										///< JPEG压缩品质 (0-100) 0--最差 100--最优 
#define  EXP_OUT_EN            0x127								///< 曝光同步输出( d0--闪光灯 d1--频闪 d2--频闪自动 d3--闪光自动 ) 
#define  TRIG_SET           0x128										///< 触发帧间隔12 (ms) 
#define  XC_SET             0x129										///< 消除触发输入颤抖(消颤),1~511,单位us 老J系列可能有 
#define  AI_LEVEL            0x12A
#define  RADAR_SET_0        0x12B									///< 设置获得雷达数据超时参数(单位ms) 
#define  RADAR_SET_1        0x12C									///< 预留 
#define  RADAR_SET_2        0x12D								///< 测速雷达串口同步头字节定义，32B参数分成4个字节，可以使用4种同步字节摄像机在收到抓拍触发后，接收RS232数据，如果接收的RS232数据字节与4个同步字节机在收到抓拍触发后，接收RS232数据，如果接收的RS232数据字节与4个同步字节的任意一个匹配时，记录其后的n个，并嵌入帧信息头回传 
#define  RADAR_SET_3        0x12E									///< 设置串口波特率 
#define  RADAR_SET_4        0x12F									///< 设置同步字后信息长度 1~4字节 

#define  YEAR               0x130										///< 时间:年 
#define  MONTH              0x131									///< 时间:月 
#define  DAY                0x132										///< 时间:日 
#define  HOUR               0x133										///< 时间:时 
#define  MINUTE             0x134									///< 时间:分 
#define  SECOND                0x135									///< 时间:秒 写入秒后更新系统时间 

#define  TRIGGER_DELAY      0x136								///<  抓拍延时(us) 
#define  EXP_DELAY          0x137									///<  曝光延时(us) 
#define  EXP_OUT_JX         0x138									///<  曝光输出极性，d0--闪光灯 d1--频闪，值的表示：0－正极性，1－负极性 
#define  ROTATE             0x139										///<  0－不旋转，1－旋转90度，2－旋转180度，3－旋转270度 
#define  H264_BRATE         0x13a									///<  H264 bit-rate, [0,100] 
#define  SELECT_IMAGE       0x13F								///<  选图命令 内部使用 
#define  USER_INFO          0x140									///< 选图命令 内部使用 


#define  SOFT_RESET            0xFF0002							///< 软复位命令 
#define  SYNC_PARAM            0xFF0003						///< 同步参数命令 
#define  SAVE_PARAM            0xFF0004						///< 保存参数到摄像机 
#define  RESET_PARAM        0xFF0005							///< 复位摄像机参数 
#define  CMD_GAMMA            0xFF0006						///< 加载GAMMA曲线参数 + 4K SHORT 
#define  CMD_HDJZ           0xFF0007							///< 加载坏点校正参数 + 16K SHORT 
#define  REFRESH_NET        0xFF0008							///< 使能网络配置 
#define  RED_LIGHT_STATUS    0xFF0009						///< 红灯信号 

#define RET_ERR    0
#define RET_OK    1

#endif

namespace cy_sys
{
    namespace cy_netcamera
    {
        static int NetInit=0;
        class NC_EXT SearchCam
        {        
            int CamNum;
            char v_IPList[100][20];
            CRITICAL_SECTION m_cs;
        public:
            SearchCam(){InitializeCriticalSection(&m_cs);}
            ~SearchCam(){DeleteCriticalSection(&m_cs);}
            void Lock(){EnterCriticalSection(&m_cs);}
            void UnLock(){LeaveCriticalSection(&m_cs);}
            /**
             * @brief 搜索相机
             */
            void SearchCamera(unsigned int nTimeout = 3);

            int getCamNum(void)
            {
                Lock();
                UnLock();
                return CamNum;
            }
            const char* getCamIPBySeq(int n)
            {
                if (n >= CamNum)
                {
                    return NULL;
                }
                else
                {
                    return v_IPList[n];
                }
            }
        #ifdef _WIN32
            static unsigned CALLBACK SearchCam_Thread(void* Param);
        #else
            static void* SearchCam_Thread(void* Param);
        #endif
        };
        
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //lyr 12/07/24 透明串口
        class NC_EXT NetCom  
        {
#define CAM_RECV_PORT 8889
#define CAM_SEND_PORT 8890
#define RETRY_MAX 2
#define NETCOM_TIMEOUT 500 

            typedef struct _NETCOM_HEADER_ 
            {
                unsigned short hdr_ver; // 通信头版本 0x0100 Rev.1.0
                unsigned short type;  // 通信命令代码  0-初始化 1-数据 2-ACK
                unsigned short port;  // 指示发送该数据包的设备的接收端口
                unsigned short chl;  // 数据通道号
                unsigned short retry; // 重传计数，0表示远端第一次发送该数据包
                unsigned short rev_0; // 保留
                unsigned short sq_no; // 数据包号，应答时使用
                unsigned short content_len; // 承载有效数据长度
            } NETCOM_HEADER;
        public:
            NetCom();
            virtual ~NetCom();

        private:
            unsigned short recv_port; // 本机接收端口
            unsigned short send_port; // 本机发送端口
            unsigned short send_inx;
            unsigned short recv_inx;
            SOCKET recv_socket;
            SOCKET send_socket;
            unsigned long cam_ip; // 目标摄像机IP
        public:
            int NetCom_Open(char *target_ip,unsigned short snd_port,unsigned short rcv_port); // 设置发送端口与接收端口，并初始化
            int NetCom_Send(char *msg,int len,int chl);
            int NetCom_Recv(char *msg,int len,int *chl,int timeout); // 超时单位毫秒
            static int recvfrom_timeout(SOCKET sc,char *buf,int len,sockaddr_in *paddr,int tout);
        };
    }
}


class NC_EXT NetCamera 
{
    friend class cy_sys::cy_netcamera::NetCom;
#ifdef _WIN32
    void* jcprops;
#endif
public:
    
    //read block data
    int ReadBlockData(int nCmd, int* nDataLength, unsigned char *pData,unsigned char nType=NORECOGNIZE);

    //update block data
    int UpdateBlockData(int nCmd,int nDataLength,unsigned char* pData);

    //  reset current state
    int Reset();

    //Set dest Camera : IP / ctrl port / transport
    int SetIPAddress(const char* IP,unsigned short portc=8886,unsigned short ports=8888);

    //Identify the camera type
    int IdentifyCameraType(void);
    
    //GetCameraType
    int GetCameraType();
    //read camera param 
    int ReadParam(int nCmd,int nState = 0);    

    //get fps ctrl unit
    double GetFrameUnit();
    
    //get gain ctrl unit
    double GetGainUnit(int* nB);

    //get time ctrl unit
    double GetTimeUnit();

    //update camera param
    int UpdateParam(unsigned int nCmd,unsigned int param1 = 0,unsigned int param2 = 0,unsigned char* buff=NULL);

    //get a image frame(frameheader+image)
    int GetImage(unsigned char* pBuf);

    //clear the image queue
    int ClearFrameQueue(void);

    //static bayer-->RGB
    static int ImageBayerToRGB(unsigned char* pRGB, unsigned char* pBayer, int image_width, int image_height);
    //static bayer-->RGB(>8Bit)
    static int ImageBayerToRGB_HiBit(unsigned short* pRGB, unsigned short* pBayer, int image_width, int image_height, int bits = 12);
    //static bayer-->RGB(High quality)
    static int ImageBayerToRGB_HQI(unsigned char* pRGB, unsigned char* pBayer, int image_width, int image_height);
    //static bayer-->RGB(High quality, > 8bits)
    static int ImageBayerToRGB_HQI_HiBit(unsigned short* pRGB, unsigned short* pBayer, int image_width, int image_height, int bits = 12);

#if defined(_WIN32)&&(!defined(_WIN64))
    //static jpeg -->rgb
    int JpegDecode(unsigned char* pJpeg, int jpeg_size, unsigned char* pRGB, int* rgb_size, int* image_width, int* image_height);//, int* image_bits, int* Reserve);
    /**    
     @ brief 把内存buffer中的RGB数据存储为jpg文件
     @ param filepath  -- 文件的完全路径
     @ param pRGB      -- 指向RGB数据
     @ param width          -- 图像宽度
     @ paramheight      -- 图像高度
     @ param jpegQ          -- 图像的压缩系数(默认为85)
     @ return bool -- 如果成功保存文件 返回true 否则返回false
    */
    bool SaveRgbToJpgFile(char* filepath,BYTE* pRGB,int width,int height,int jpegQ = 85);
    /**
    * @brief    usm锐化处理
    * @param    srcPR        图像数据源地址，RGB格式
    * @param    destPR        图像数据目标地址，RGB格式
    * @param    w            图像宽度，单位为像素
    * @param    h            图像高度，单位为像素
    * @param    radius        USM处理参数，半径，范围0.1~20，建议取值5
    * @param    amount        USM处理参数，数量，范围0.1~120，建议取值0.5
    * @param    threshold    USM处理参数，阈值，范围1~255，建议取值2
    * @return    void        
    */
#endif
    static void usm_process (unsigned char *srcPR, unsigned char *destPR, int w, int h, double radius, double amount, int threshold);
    /**
    * @brief    对比度增强处理
    * @param    buf            图像数据源地址，RGB格式
    * @param    w            图像宽度，单位为像素
    * @param    h            图像高度，单位为像素
    * @param    threshold    处理阈值，范围0~0.1，建议取值0.01
    * @return    void        
    */
    static void contrast_enhance_process (unsigned char *buf, int w, int h, double threshold);

    NetCamera();
    virtual ~NetCamera();
    char* getIp(void){return m_IP;}
private:
    char                m_IP[20];                //ip addr
    int                m_nTransPort;                //trans port
    int                m_nCtrlPort;                //ctrl port
    int                m_nCameraStyle;            //camera type
    void*            m_pCamera;                //point to the "real" camera class
public:
    /**
    * @brief 获得相机传输状态
    * @return -1--网络连接失败 0--手动断开连接 1--连接状态 -2 --相机对象为NULL
    */
    int getCameraTransState(void);
    /**
    * @brief 获得相机控制通道状态
    * @return -2 --相机对象为NULL 0--手动断开连接 1--连接状态
    */
    int getCameraCtrlState(void);
    /**
    * @brief 连接或者断开连接
    * @param op1:0断开控制通道 1开启控制通道 -1不做操作
    * @param op2:0断开传输通道 1开启传输通道 -1不做操作
    * @param m_bopenCtlTh: 1-开启控制心跳 0-不开启
    *return:TRUE连接或者断开成功 FALSE连接失败
    */
    int ConnectCamera(int op1=1, int op2=1, int m_bopenCtlTh=1)
    {
        int ret=1;
        if (op1 != -1)
        {
            ret=StartCtrl(op1 == 1,m_bopenCtlTh);
        }
        if (op2 != -1)
        {
            ret=StartTrans(op2 == 1);
        }
        return 1-ret;
    }

public:
	/************************************************************************/
	/*			 封装函数			           */
	/************************************************************************/
	/**
    * @brief 网络设置
	* @param pIP_ADDR ip地址
	* @param pGATEWAY 摄像机网关
	* @param pNETMASK 网络地址掩码
	* @param pSERVER_IP 服务器IP（监听模式有效）
	* @return 成功返回0，否则返回小于0
    */
	int UpdateSetNet(char* pIP_ADDR, char* pGATEWAY, char* pNETMASK ,char* pSERVER_IP = NULL );
	/**
    * @brief 读取IP地址
	* @return 返回IP地址
    */
	int ReadIPAddr();
	/**
    * @brief 读取子网掩码
	* @return 返回子网掩码
    */
	int ReadGateWay();
	/**
    * @brief 读取网关
	* @return 返回网关
    */
	int ReadNetMask();;
	/**
    * @brief 读取读取服务器IP（监听模式有效）
	* @return 返回服务器IP
    */
	int ReadServerIP();
	/**
    * @brief 网络校时
	* @return 校时成功返回0 ，否则返回小于0
    */
	int UpdateNetTime();
	/**
    * @brief 同步参数到摄像机
	* @return 同步成功返回0 ，否则返回小于0
    */
	int UpdateSyncParam();
	/**
    * @brief 保存参数到摄像机
	* @return 保存成功返回0 ，否则返回小于0
    */
	int UpdateSaveParam();
	/**
    * @brief 软重启
	* @return 软重启成功返回0 ，否则返回小于0
    */
	int UpdateSoftReset();
	/**
    * @brief 复位相机
	* @return 复位成功返回0 ，否则返回小于0
    */
	int UpdateResetParam();
	/**
    * @brief 设置增益控制方式
	* @param AGCMode 增益控制方式（1--自动(AGC)，0--手动）
	* @return 设置增益方式成功返回0，否则返回小于0
    */
	 int UpdateAGCMode(int AGCMode);
	/**
    * @brief 读取增益控制方式
	* @param nState 缺省值为0,读取本地配置，1表示直接从相机端读取
	* @return 读取增益方式值0为手动，1自动
    */
	 int ReadAGCMode(int nState = 0);
	/**
    * @brief 设置AGC门限值 (0-255) 
	* @param AGCLimit 门限值
	* @return 设置AGC门限值成功返回0，否则返回小于0（详见错误列表宏定义）
    */
	 int UpdateAGCLimit(int AGCLimit);
	/**
	* @brief 读取AGC门限值 (0-255) 
	* @param nState 缺省值为0,读取本地配置，1表示直接从相机端读取
	* @return 返回门限值
    */
	 int ReadAGCLimit(int nState = 0);
	/**
    * @brief AGC与电子快门连动时，电子快门调整值设置
	* @param AGCTimeMin 最小值(us) 
	* @param AGCTimeMax 最大值(us) 
	* @return 设置成功返回为0，否则失败
    */
	 int UpdateAGCTimeRange(int AGCTimeMin,int AGCTimeMax);
	/**
    * @brief AGC与电子快门连动时，读取电子快门调整值最小值
	* @param  nState 缺省值为0,读取本地配置，1表示直接从相机端读取
	* @return 返回最小值
    */
	 int ReadAGCTimeMin(int nState = 0);
	/**
	* @brief AGC与电子快门连动时，读取电子快门调整值最大值
	* @param  nState 缺省值为0,读取本地配置，1表示直接从相机端读取
	* @return 返回最大值
    */
	 int ReadAGCTimeMax(int nState = 0);
	/**
    * @brief 设置监控模式抓拍独立参数使能
	* @param m_TRIParamTimeEN 时间使能
	* @param m_TRIParamGainEN 增益使能
	* @param m_TRIParamRGBEN 色彩使能
	* @return 设置成功0，否则返回小于0
    */
	 int UpdateTRIParamEN(bool m_TRIParamTimeEN, bool m_TRIParamGainEN, bool m_TRIParamRGBEN);
	/**
    * @brief 读取监控模式抓拍参数使能
	* @return 返回使能状态0&1
    */
	 int ReadTRIParamEN();
	/**
    * @brief 抓拍时曝光时间设置
	* @param m_TRIEXPTime 时间参数
	* @return 设置成功返回0，否则返回小于0
    */
	 int updateTRIEXPTime(int m_TRIEXPTime);
	/**
    * @brief 获取抓拍时采用的曝光时间
	* @return 返回曝光时间
    */
	 int ReadTRIEXPTime();
	/**
    * @brief 设置监控模式有效(抓拍时采用的增益)（*spv无效）
	* @param m_TRIGain 抓拍增益
	* @return 设置成功返回0，否则返回小于0
    */
	 int updateTRIGain(int m_TRIGain);
	 /**
    * @brief 获取抓拍时采用的增益
	* @return 返回抓拍增益值
    */
	 int ReadTRIGain();
	/**
    * @brief 设置抓拍RGB增益
	* @param m_TRIRGain 抓拍采用的R增益
	* @param m_TRIGGain 抓拍采用的G增益
	* @param m_TRIBGain 抓拍采用的B增益
	* @return 设置成功返回0，否则失败
    */
	 int UpdateTRIParamRGBGain(int m_TRIRGain, int m_TRIGGain, int m_TRIBGain);
	/**
    * @brief 获取抓拍是采用的R增益
	* @return 返回R增益值
    */
	 int ReadTRIParamRGain();
	/**
    * @brief 获取抓拍是采用的G增益
	* @return 返回G增益值
    */
	 int ReadTRIParamGGain();
	 /**
    * @brief 获取抓拍是采用的B增益
	* @return 返回B增益值
    */
	 int ReadTRIParamBGain();
	/**
    * @brief 设置白平衡当前RGB增益值
	* @param m_TRIRGain 当前R增益
	* @param m_TRIGGain 当前G增益
	* @param m_TRIBGain 当前B增益
	* @return 设置当前增益成功返回0，否则失败
    */
	 int UpdateGainRGB(int m_GainR, int m_GainG, int m_GainB);
	 /**
    * @brief 获取白平衡当前R增益
	* @return 返回当前R增益
    */
	 int ReadGainR();
	/**
    * @brief 获取白平衡当前G增益
	* @return 返回当前G增益
    */
	 int ReadGainG();
	 /**
    * @brief 获取白平衡当前B增益
	* @return 返回当前B增益
    */
	 int ReadGainB();
	/**
    * @brief 设置白平衡校正方式
	* @param m_BalaceMode 0--手动白平衡 1--自动白平衡 
	* @return 设置成功返回0，否则返回小于0
    */
	 int UpdateBalanceMode(int m_BalaceMode);
	 /**
    * @brief 获取白平衡校正方式
	* @param nState 缺省值为0,读取本地配置，1表示直接从相机端读取
	* @return 返回白平衡校正方式
    */
	 int ReadBalanceMode(int nState = 0);
	/**
    * @brief 设置jpeg图像尺寸
	* @param m_ImageZoomW 图片宽
	* @param m_ImageZoomH 图片高
	* @return  设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateImageZoomSize(int m_ImageZoomW, int m_ImageZoomH);
	 /**
    * @brief 获取jpeg图像宽
	* @return 返回宽
    */
	 int ReadImageZoomW();
	 /**
    * @brief 获取jpeg图像高
	* @return 返回高
    */
	 int ReadImageZoomH();
	 /**
    * @brief 设置流媒体图像输出尺寸
	* @param m_ImageZoomWStream 流媒体图像宽
	* @param m_ImageZoomHStream 流媒体图像高
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateImageZoomSizeStream(int m_ImageZoomWStream, int m_ImageZoomHStream);
	 /**
    * @brief 获取流媒体图像输出宽
	* @return 返回流媒体图像的宽
    */
	 int ReadiImageZoomWStream();
	 /**
    * @brief 获取流媒体图像输出高
	* @return 返回流媒体图像的高
    */
	 int ReadiImageZoomHStream();
	 /**
    * @brief upadate文字叠加
	* @param info_struct 文字叠加结构体指针
	* @return Update成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateBlockCharacterInfo(character_info* info_struct);
	 /**
    * @brief Read文字叠加
	* @param info_struct 文字叠加结构体指针,保存信息
	* @return Read成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int ReadBlockCharacterInfo(character_info* info_struct);
	 /**
    * @brief 软触发-模拟抓拍
	* @param m_st_chl 闪光灯通道
	* @param m_st_fcnt 抓拍张数
	* @param m_st_inv 抓拍间隔
	* @param m_st_info 自定义信息
	* @return 软触发成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateSoftTrigger(int m_st_chl, int m_st_fcnt, int  m_st_inv,long m_st_info);
	 /**
    * @brief 设置闪光灯频闪灯使能控制
	* @param EXPOutEN 双字节参数
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateEXPOutEN(unsigned int   EXPOutEN);
	  /**
    * @brief 读取闪光灯频闪灯使能状态
	* @return 返回闪光灯频闪灯使能状态
    */
	 int ReadEXPOutEN();
	 /**
    * @brief 设置频闪灯倍频及输出反向
	* @param m_bool1 频闪灯倍频
	* @param m_bool2 频闪灯输出反向
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateLedOutMode(bool m_bool1, bool m_bool2);
	  /**
    * @brief 读取频闪灯倍频及输出反向
	* @return 返回信息
    */
	 int ReadLedOutMode();
	 /**
    * @brief 设置视频模式图像输出格式（视频流选择）
	* @param m_ImageFormat JPEG、H264、双码流
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateImageFormat(int m_ImageFormat);
	  /**
    * @brief 读取视频模式图像输出格式（视频流选择）
	* @return 返回视频流格式参数
    */
	 int ReadImageFormat();
	 /**
    * @brief 设置相机工作模式
	* @param m_SYNC 工作模式：1-抓拍模式 3-监控模式 5-视频模式
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateSYNC(int m_SYNC);
	 /**
    * @brief 读取相机工作模式
	* @return 返回相机工作模式参数
    */
	 int ReadSYNC();
	  /**
    * @brief 设置JPEG图片压缩品质
	* @param m_JpegQuality 图片品质
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateJPEGQuality(int m_JpegQuality);
	 /**
    * @brief 读取JPEG图片压缩品质
	* @return 返回JPEG图片压缩品质参数
    */
	 int ReadJPEGQuality();
	 /**
    * @brief FTP设置
	* @param buffer 存储数据的缓冲区
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UPdateBlockFTPAccountSet(char buffer[]);
	  /**
    * @brief NTP设置
	* @param buffer 存储数据的缓冲区
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UPdateBlockNTPSetting(char buffer[]);
	 /**
    * @brief 串口抓拍设置
	* @param info_struct 串口抓拍结构体指针
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateBlockRadarLogicSet(ser_groupinfo* info_struct);
	 /**
    * @brief io抓拍设置 
	* @param info_stuct io抓拍结构体指针
	* @return 设置成功返回0，否则返回小于0（详细请见错误列表）
    */
	 int UpdateBlockIOLogicSet(giogroupinfo* info_stuct);
private:
	unsigned int m_ip;
	unsigned int m_mask;
	unsigned int m_gw;
private:
    bool IsValidIPAddress(const char *buf);
    static int Init(void);
    static int UnInit(void);
    int StartTrans(bool bStart = true);
    int StartCtrl(bool bStart = true, int m_bopenCtlTh=1);
};

#endif
