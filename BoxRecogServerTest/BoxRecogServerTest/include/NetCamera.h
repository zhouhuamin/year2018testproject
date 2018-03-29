/*! \file 
  \brief NetCameraͷ�ļ�
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
  \brief �ṹ�壺�����������ƿ�ṹ��
*/
struct FRAME_HEADER
{
	unsigned short w;								///< ͼ���
	unsigned short h;								///< ͼ���
	unsigned short bits;							///< ͼ��λ�� 8~16bits
	unsigned short format;                    	///< ͼ�����ʹ���(0--�Ҷȣ�1--Bayer_RG��2--Bayer_GR��3--Bayer_BG��5--RGB��6--YUV422��7--JPEG, 8--MPEG4 9--h264)    
	unsigned short frame_type;              ///< ֡����(0--��ͨ��1--ץ��ͼ��2--����֡)
	unsigned short frame_rev;                ///< ����    
	unsigned int  firmware_version;        ///< �̼�����汾
	unsigned int  device_no;                   ///< �豸���    
	unsigned int  len;                        		///< ͼ�����ݳ���    
	unsigned int  speed;							///< ˫��Ȧ����ֵ(us)
	unsigned int  rs232;							///< ������Ϣ(1~4�ֽ�)
	unsigned short year;							///< ͼ��ɼ�ʱ��(��)
	unsigned short month;						///< ͼ��ɼ�ʱ��(��)
	unsigned short day;							///< ͼ��ɼ�ʱ��(��)
	unsigned short hour;						///< ͼ��ɼ�ʱ��(ʱ)
	unsigned short minute;						///< ͼ��ɼ�ʱ��(��)
	unsigned short second;    					///< ͼ��ɼ�ʱ��(��)
	unsigned int ip;									///< �ɼ���ǰ֡�������IP
	unsigned int frame_count;                ///< �ܹ���ץ��֡��Ŀ
	unsigned int trigger_count;               ///< �ܹ��Ĵ�����
	unsigned int trigger_index;				///< ����������
	unsigned int frame_no;						///< ֡��    
	unsigned int gain;								///< ��ǰץ�Ĳ���
	unsigned int time;								///< �ع�ʱ��
	unsigned int gain_r;							///< ������
	unsigned int gain_g;							///< ������
	unsigned int gain_b;							///< ������
	unsigned int mode;                        	///< ���������ģʽ
	unsigned int JpegQ;                        	///< JPEGѹ��Ʒ��
	unsigned int td1;								///< ץ����ʱ(��λus)
	unsigned int td2;								///< �ع���ʱ(��λus)
	unsigned int trig_chl;                    	///< ����ͨ��
	unsigned int msecond;                     ///< ms��ʱ
	unsigned int yavg;                        	///< ƽ������
	unsigned int vid_head;                    	///< ��ý��ؼ�֡
	unsigned int st_usr_param;               ///< ������Ϣ
	unsigned int red_time;                    	///< ��Ƽ�ʱ
	//unsigned long rev[6];                    	///< ��������
	unsigned int grey;								///< һ�����ԭʼ����
	unsigned int wz_type;                       ///< Υ�����ͣ�d0-����,d1-�����
	unsigned short traffic_code;             ///< Υ�´���
	unsigned char capture_no;               ///< ���δ���ץ��������total��
	unsigned char rev_1;                        ///< ����λ
	unsigned int rev[3];							///< ��������
	unsigned char user_info[64];				///< �û�����
};
#endif
/*!
  \brief ö�٣�ö�ٱ���
*/
typedef enum {
	DEFAULT_CITY_SET = 600,                    	///< ����Ĭ��ʡ��
	DEFAULT_CHAR_SET,								///< ����Ĭ�ϳ����ַ�
	LANE_0_SET,                                			///< ���ó���0��ʶ������
	LANE_1_SET,											///< ���ó���1��ʶ������
	LANE_2_SET,                                			///< ���ó���2��ʶ������
	LANE_3_SET,                                			///< ���ó���3��ʶ������
	PLATE_RECO_ENABLE,                        	///< ����ʶ��ʹ��(0�����ã�1��ʹ��)
	CAR_TYPE_ENABLE,								///< ����ʶ��ʹ��(0�����ã�1��ʹ��)
	CAR_COLOR_ENABLE,                          	///< ������ɫʶ��ʹ��(0�����ã�1��ʹ��)
	RECO_REGION_SET,								///< ����ʶ������
	SAVE_RECO_CONFIG,                         	///< ����ʶ�����
	VIDEO_DETECT_ENABLE,                       ///< ��Ƶ���ʹ��
	DETECT_REGION_SET,                        	///< ������Ƶ�������
	SAVE_DETECT_CONFIG,                        	///< Ƶ����߼�����
	TRIG_CHARACTER_INFO,                       ///< ��ȡץ����������
	DSP_ID_REQ,                                			///< 
	DSP_KEY_FILE,										///< 
	TRAFFIC_ENABLE,                            		///< ���̵�ʹ��
	TRAFFIC_REGION_SET,                        	///< �����������
	SAVE_TRAFFIC_CONFIG,                        ///< ������̵�����
	IO_SER_CHECK_EANABLE,                     	///< ����Դ�Զ��л�ʹ��
	TRAFFIC_MODE,                              		///< ������ģʽ(0�����ڵ羯��1����׼�羯��2����׼����)
	TRIG_JPRG_QT,                              		///< ץ��ͼƬƷ������
	ESPU_STATE = 650,                          		///< state of ESPU0608
	AT88_STATE,                                			///< state of AT88SC0104C
	DS28_STATE,                                			///< state of DS28CN01
	DEVICE_NO_TMP = 700,                            ///<�豸���
	ROAD_NO_TMP,                       ///<���ڱ��
	CHL_NO_TMP ,                        ///<�������
	VIDEO_LANE_ENABLE,                  ///<video detect lane_num enable
	IO_SER_CHEKCK_HEART_EANBLE,                ///<�ⴥ������Ƶ�����������л� 704
	PLATE_JPEG_GET_ENABLE ,             ///<����Сͼ��ȡʹ�� 705
	CAR_LOGO_ENABLE                       ///<����ʶ��706
}DSP_CMD;
/* ����ʶ�����Ƶ�������ṹ */                                                                

/*!
  \brief �ṹ�壺������Ȧ�ṹ
*/                                                                    
typedef struct Single_Reco_Region {                                                
	unsigned int  show_enable;                  ///< ʹ��״̬                                                 
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
  \brief �ṹ�壺ʶ������ṹ
*/                                                                        
typedef struct Reco_Region                                                         
{                                                                                  
    unsigned int Reco_Region_Count;			///< ����                                            
    Single_Reco_Region  Region[4];				///< �����������                                       
}Reco_Region;                                                                      


/*!
  \brief �ṹ�壺��Ƶ���ṹ
*/                                                                                       
typedef struct video_info{                                                         
	unsigned int  show_enable;  					///< ����Ƶ��Ȧ�����Ƿ���ʾ                                        
	int    total;               								///< ����Ƶ��Ȧץ��������                                         
	int    inv_12;              							///< ����Ƶ��Ȧץ�Ķ���ͼƬʱ��1�ź͵�2�ŵ�ץ��ʱ��������λms��                     
	int    inv_23;              							///< ����Ƶ��Ȧץ�Ķ���ͼƬʱ��2�ź͵�3�ŵ�ץ��ʱ��������λms��                     
	unsigned int  x0;    								///< ����Ƶ��Ȧ�����ֵx0 
	unsigned int  y0;									///< ����Ƶ��Ȧ�����ֵy0 
	unsigned int  x1;         							///< ����Ƶ��Ȧ�����ֵx1 
	unsigned int  y1;           						///< ����Ƶ��Ȧ�����ֵy1                                     
	unsigned int  x2;									///< ����Ƶ��Ȧ�����ֵx2 
	unsigned int  y2;									///< ����Ƶ��Ȧ�����ֵy2 
	unsigned int  x3;									///< ����Ƶ��Ȧ�����ֵx3 
	unsigned int  y3;									///< ����Ƶ��Ȧ�����ֵy3 
}video_info;

/*!
  \brief �ṹ�壺��Ƶ���ṹ
*/  
typedef struct Single_Detect_Config {                                              
	unsigned int mode;          						///< Ϊ����Ȧģʽ��1Ϊ˫��Ȧģʽ                                    
	int          flash_ring;       						///< ������Ƿ�����                                           
	int          flash_chan;    							///< ��Ƶ����ʱ�����ͨ��                                         
	int          distance_ab;   						///< ���е�һ����Ƶ��Ȧ���ڶ����ľ���                                   
	video_info   info[2];								///< ��Ƶ��Ϣ                                    
	char         resv[20];								///< resv��Ϣ                                                       
}Single_Detect_Config;                                                             

/*!
  \brief �ṹ�壺��Ƶ�������ṹ
*/                                                                    
typedef struct Detect_Config{                                                      
    unsigned int         Detect_Config_Count;       ///< ����                                  
    Single_Detect_Config Region[4];                   ///< ��Χ����                               
}Detect_Config;                                                                    


/*!
  \brief �ṹ�壺���̵Ʋ����ṹ
*/                                                                      
typedef struct lane_info{                                                          
	unsigned int  show_enable;  						///< �ú��̵������Ƿ���ʾ                                           
	unsigned int  x0;										///< �ú��̵����������ֵx0                                       
	unsigned int  y0;										///< �ú��̵����������ֵy0                                    
	unsigned int  x1;										///< �ú��̵����������ֵx1                                      
	unsigned int  y1;           							///< �ú��̵����������ֵy1                                         
	unsigned int  x2;										///< �ú��̵����������ֵx2                                       
	unsigned int  y2;										///< �ú��̵����������ֵy2                                    
	unsigned int  x3;										///< �ú��̵����������ֵx3                                     
	unsigned int  y3;										///< �ú��̵����������ֵy3                                                
}lane_info;                                                                        
/*!
  \brief �ṹ�壺��ͨ�����ṹ
*/  
typedef struct traffic_info {                                                      
    unsigned int total;									///< ����                          
    lane_info redarea[4];									///< ���̵���Ϣ                           
}traffic_info;                                                                     

/*!
  \brief �ṹ�壺���ֵ���
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
  \brief �ṹ�壺����ץ��
*/ 
typedef struct ser_groupinfo
{
	int flash_chl;			///< �����ͨ��
	int flash_ring;		///< ����ģʽʹ��
	int timeout;			///< ��ʱ����
	int delay;				///< �ӳٴ���ʱ��(����)
	int mode;				///< ����ģʽ:0--5�ֽڼ�ģʽ1--��У����ֽ�ģʽ
	int total;				///< ץ��������
	int inv_12;				///< ��ץ�Ķ���ʱ��һ�ź͵ڶ��ŵ�ץ��ʱ����
	int inv_23;				///< ��ץ�Ķ���ʱ�ڶ��ź͵����ŵ�ץ��ʱ����
	char sync[4];			///< ͬ����ͷ
	int info_fffec_len;	///< �û�������Ч����(0~INFO_MAX_LEN)
	int red_state;			///< ���״̬
	timeval red_start;	///< ��ƿ�ʼʱ��
	int option;				///< ѡ��
}ser_groupinfo;

/* ioץ������ */  
typedef struct giogroupinfo
{
	int flash_chl;			///< �����ͨ��
	int flash_ring;	   	///< ���������ģʽ(0--����1--ʹ��)
	int timeout;			///< ��ʱ(����)
	int delay;				///< �ӳٴ���ʱ��(����)
	int io_a;				///< ���еĵڸ�IO��(������~8)
	int io_b;				///< ���еĵڸ�IO��(������~8)
	int io_c;					///< ���еĵڸ�IO��(1~8,Ϊ��ʾ��ǰ���������ֻ������IO)

	int total_a;			///< ���еĵڸ�IOץ�ļ���,Ϊʱ��ץ��
	int total_b;			///< ���еĵڸ�IOץ�ļ���,Ϊʱ��ץ��
	int total_c;			///< ���еĵڸ�IOץ�ļ���,Ϊʱ��ץ��
	int distance_ab;	///< ���еڸ�IO���ڸ�IO����Ȧ����(��λ:cm)
	int distance_bc;		///< ���еڸ�IO���ڸ�IO����Ȧ����(��λ:cm)

	int inv_a_12;			///< ���ڸ�IO��ץ�Ķ���ͼƬʱ���ź͵��ŵ�ץ��ʱ����(��λ:ms)
	int inv_a_23;			///< ���ڸ�IO��ץ�Ķ���ͼƬʱ���ź͵��ŵ�ץ��ʱ����(��λ:ms)
	int inv_b_12;			///< ���ڸ�IO��ץ�Ķ���ͼƬʱ���ź͵��ŵ�ץ��ʱ����(��λ:ms)
	int inv_b_23;			///< ���ڸ�IO��ץ�Ķ���ͼƬʱ���ź͵��ŵ�ץ��ʱ����(��λ:ms)
	int inv_c_12;			///< ���ڸ�IO��ץ�Ķ���ͼƬʱ���ź͵��ŵ�ץ��ʱ����(��λ:ms)
	int inv_c_23;			///< ���ڸ�IO��ץ�Ķ���ͼƬʱ���ź͵��ŵ�ץ��ʱ����(��λ:ms)

	int red_state;			///< ���״̬��ʶ(1--���,0--�Ǻ��)
	timeval red_start;	///< ��ƿ�ʼʱ��
	int option;				///< ѡ��
}giogroupinfo;
/* ioץ������ */  
typedef struct pc_giogroupinfo
{
	int rev[5];				///< ������,��,�ѱ���չ
	int valid;				///< ��Ч��(0��Ч,1��Ч)
	int gio_type;			///< gio����(0--˫��Ȧ����1--����Ȧ����)
	giogroupinfo gioinfo;
}pc_giogroupinfo;


//////////////////////////////////////////////////////////////////////////
//CAMERA TYPE DEFINE
#ifndef _NET_CAMERA_TYPE_DEFINE_
#define _NET_CAMERA_TYPE_DEFINE_
#define NC_TYPE_UNKOWN          (0x00000000)			///< δ֪������� 
#define NC_TYPE_J               (0x00010000)					///< J����� 
#define NC_TYPE_N               (0x00020000)					///< NA����� 
#define NC_TYPE_G               (0x00040000)					///< GA����� 
#define NC_TYPE_SPV             (0x00080000)					///< SPV��� 
#define NC_TYPE_SMV             (0x00090000)				///< SMV��� 
#define NC_TYPE_TKV             (0x00100000)					///< TKV��� 
#endif


//////////////////////////////////////////////////////////////////////////
#ifndef _NET_CAMERA_CMD_DEFINE_
#define _NET_CAMERA_CMD_DEFINE_
#define NORECOGNIZE        0
#define BAYONET            1
#define EPOLICE            2

#define ERR_OK                0											///<  OK
#define ERR_ADDR            1											///<  ��Ч��IP��ַ 
#define ERR_SOCKET            2										///<  δ�ܴ���SOCKET 
#define ERR_CONNECT            3									///<  δ������Ŀ�� 
#define ERR_RECV            4											///<  ���մ��� 
#define ERR_RECV_TIMEOUT    5									///<  ���մ���ʱ 
#define ERR_SEND            6											///<  ���ʹ��� 
#define ERR_SEND_TIMEOUT    7									///<  ���ʹ���ʱ 
#define ERR_MEMORY          8										///<  û���㹻���ڴ�ռ� 
#define ERR_TIMEOUT            9									///<  ���ӳ�ʱ 
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CAM_HB_ACK 0xAA55AA55
#define LIB_HB_ACK 0x55AA55AA

//camera cmd define
//read only
//#define  ID                 0x00											///< �豸��(0--���豸,!0--���豸) 
#define  WIDTH              0x01										///< ������ˮƽ������(���) 
#define  HEIGHT             0x02										///< ��������ֱ������(�߶�) 
#define  BITS               0x03											///< ͼ�����ݵ�A/Dλ�� 
#define  COLOR              0x04										///< ͼ�����ݸ�ʽ(0--�Ҷȣ�1--Bayer_RG��2--Bayer_GR��3--Bayer_BG��5--RGB��6--YUV422��7--JPEG ,8--MPEG4 ,9--H.264 ) 
#define  SP_CHECK           0x06										///< д��0x13572468 0x1C2E3A9B 
#define  RTC_STATE              0x06									///<  ����ʵʱʱ�ӹ����Ƿ�����,0����������1������ 


#define  C_SOFT_VERSION     0x07								///< 0x07~0x0A 16���ֽڱ�ʾ�汾��Ϣ 
#define  C_PRODUCT_NO       0x0B								///< 0x0B~0x0E 16���ֽڱ�ʾ��Ʒ��Ϣ 
#define  CCD_TEMPERATRUE     0x0F							///<  ǰ���¶� - ��
#define  HEART_BEAT         0x0F									///<  �������� - д
#define  SOFT_VERSION       0x12									///< �̼�����汾 
#define  DEVICE_NO          0x13									///< �������� 
#define  PRODUCT_NO         0x14									///< ������� 0-DC1335J 1-DC2035J 2-DC1334N 3-DC2034N 

#define  C_DEVICE_NO        0x15									///< 0x15~0x18 16���ֽڱ�ʾ��Ʒ�����Ϣ 
#define  C_SP_PRODUCT_INFO  0x19							///< 0x19~0x1C 16���ֽڱ�ʾ��Ʒ������Ϣ 
#define  USB_CAPABILITY     0x1D									///< U������ 

#define     USB_SPECIAL        0x1E									///< STORAGE �������� 
  #define IF_USB_FORMAT         0x1								///< U�̸�ʽ�� 
  #define IF_USB_CHECK            0x2								///< U���Լ� 
  #define IF_USB_AUTO_FORMAT    0x3							///< U���Զ���ʽ�� 
  #define IF_USB_FLAG_FORMATED    0x4						///< U�̽����� 
  #define IF_USB_DISABLE        0x5								///< U�̽����� 

#define     ST_SPECIAL            0x1F								///< �ڲ�״̬ 
// #define ST_USB_CHECK_OK            0x1						///< U���Լ� OK 
// #define ST_USB_CHECK_ERR        0x2						///< U���Լ� ERR 
// #define ST_USB_CHECK_NONE        0x3						///< U���Լ� NOT FOUND 

#define  GAIN_UNIT             0x20									///< ��ǰ����ֵ 
#define  OFFSET_UNIT        0x21									///< ��ǰƫ��ֵ 
#define  FREQ_UNIT          0x22										///< ��ǰ֡Ƶֵ 
#define  TIME_UNIT          0x23										///< ��ǰ�ع�ʱ��ֵ 
#define  RGB_GAIN_UINT         0x24								///< ��ɫ���������λ 

#define  MCU_UPDATE_NOW        0x2E
#define  CUR_Y_AVG            0x2F									///< ƽ������ 

#define  MAX_GAIN           0x30									///< �������ֵ 
#define  MIN_GAIN           0x31									///< ������Сֵ 
#define  MAX_OFFSET         0x32									///< ƫ�����ֵ 
#define  MIN_OFFSET         0x33									///< ƫ����Сֵ 
#define  MAX_FREQ           0x34									///< ֡Ƶ���ֵ 
#define  MIN_FREQ           0x35										///< ֡Ƶ��Сֵ 
#define  MAX_TIME           0x36									///< �ع�ʱ�����ֵ 
#define  MIN_TIME           0x37										///< �ع�ʱ����Сֵ 
#define  MAX_BALANCEMODE    0x38							///< ��ƽ�ⷽʽѡ�����ֵ 
#define  MAX_AGCLIMIT       0x39								///< AGC����ֵ���ֵ 
#define  MAX_AGCSELECT      0x3a								///< AGCȡ������ѡ�����ֵ 


#define  MIX_TOTAL_GAIN        0x3b							///< ��ɫ������Сֵ 
#define  MAX_TOTAL_GAIN     0x3c								///< ��ɫ�������ֵ 

/**< spv��0x60~0x63 16���ֽڱ�ʾ�汾��Ϣ*/
#define  C_AFE_INFO            0x60									///< 0x60~0x63 16���ֽڱ�ʾ�汾��Ϣ
#define  AFE_TCLK_A         0x64									///< Aͨ��  TCLK
#define  AFE_TCLK_B         0x65									///<  Bͨ��  TCLK
#define  AFE_TCLK_C         0x66									///<  Cͨ��  TCLK
#define  AFE_TCLK_D         0x67									///< Dͨ��  TCLK
#define  C_AFE_FW_INFO        0x68								///<  spv��0x68~0x6b 16���ֽڱ�ʾ��ǰ�˹̼���Ϣ

#define  ONVIF_VERSION        0x70								///<  0x70~0x73 16���ֽڱ�ʾonvif�汾��Ϣ*/

#define  PROTECT_AREA       0x8f									///<  0--0x13f����Ϊ�������������������� 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//not read only
#define SWAP32(y) (( ((y)>>24) & 0x000000ff) | (((y) >> 8)  & 0x0000ff00) | (((y) << 8)  & 0x00ff0000) | (((y) << 24) & 0xff000000) )
#define  GAIN               0x90											///< ��ǰ����ֵ 
#define  OFFSET             0x91										///< ��ǰƫ��ֵ 
#define  FREQ               0x92											///< ��ǰ֡Ƶֵ 
#define  TIME               0x93											///< ��ǰ�ع�ʱ��ֵ 
#define  SYNC               0x94										///< ͬ����ʽ(0--�ⴥ����1--���δ���) 
#define        SYNC_ONE_SHOT    0x01							///< ץ��ģʽ 
#define        SYNC_TRIG        0x02									///< ���ģʽ 
#define        SYNC_VIDEO        0x03								///< ��Ƶģʽ 
//...
#define  AGCMODE            0xb0									///< ������Ʒ�ʽ(1--�Զ�(AGC)��0--�ֶ�) 
#define  AGCLIMIT            0xb1									///< AGC����ֵ (0-255) 
#define  AGCSELECT          0xb2									///< AGCȡ������ѡ�� ��16λ��Ч���ֱ��Ӧ4x4��16������˳��Ϊ�����ң����ϵ��� 
#define  AGCTIME            0xb3										///< AGC����ӿ������� 
#define  AGC_GAIN_MAX       0xb4								///< AGC������Χ 0 ~ Max dB 
#define  AGC_TIME_MIN       0xb5								///< AGC����ӿ�������ʱ�����ӿ��ŵ�����Сֵ(us) 
#define  AGC_TIME_MAX       0xb6								///< AGC����ӿ�������ʱ�����ӿ��ŵ������ֵ(us) 
#define  AREA_X                0xb7									///< �����������Ͻ�x���� 
#define  AREA_Y             0xb8										///< �����������Ͻ�y���� 
#define  AREA_W                 0xb9									///< ���������� 
#define  AREA_H                 0xba									///< ��������߶� 

#define     AE_MODE            0xbb									///< �Զ�ʹ�õĲ��ģʽ(0-basic) 
#define     BL_PARAM            0xbc								///< ���۲���ǿ��(-100 ~ 100) 
#define  ROTATION            0xbd									///< JPEGͼƬ��תģʽ(0,90,180,270) 
//...
#define  GAIN_R             0xc9										///< ��ǰR����ֵ 
#define  GAIN_G             0xca										///< ��ǰG����ֵ 
#define  GAIN_B             0xcb										///< ��ǰB����ֵ 
#define  FTP_ACCOUNT_SET    0xcc								///< FTP�ʺ����� 
#define  WATER_MARK_SET        0xcd							///< ˮӡ���� 
#define  IO_LOGIC_SET        0xce									///< ��Ȧ�߼����� 
#define  CHARACTER_INFO_SET    0xcf							///< jpeg���ֵ������� 

#define  NTP_SETTING        0xd1									///< NTP���������� 
#define  BALANCEMODE        0xd2								///< ��ƽ��У����ʽѡ�� 0--�ֶ���ƽ�� 1--�Զ���ƽ�� 
#define  PRE_SET_GAIN_R     0xd3									///< Ԥ��R���� 
#define  PRE_SET_GAIN_G     0xd4									///< Ԥ��G���� 
#define  PRE_SET_GAIN_B     0xd5									///< Ԥ��B���� 

#define  RADAR_LOGIC_SET    0xd6								///< �����߼����� 
#define  NETCOM_BAUD_SET    0xd7							///< ͸�����ڲ��������� 
#define  CHAL_OF_TRIG_IMG    0xd8								///< ץ��ͼƬ����ͨ�� (0 - server, 1 - normal, 2 - both) 
//...
#define AUTO_JPEG_Q_EN        0xda								///< JPEG��������ʹ��
#define  GAMMA_DATA              0xdb							///< ����GAMMA���߲���  4K SHORT
#define LED_HIGHTIME_EN        0xdc							///< Ƶ���Ƹߵ�ƽʱ��ɵ�ʹ��,D0=0��ֹ��D0��1ʹ��
#define LED_HIGHTIME        0xdd									///< Ƶ���Ƹߵ�ƽʱ��,��λus��0--6000us
#define  ONVIF_AUTH         0xde									///< 1:��¼onvif��Ҫ��֤   ����ֵ����¼onvif����Ҫ��֤ 
//...

#define  AUTOGRAY           0xe0									///< �Զ��Ҷ���ǿ(1--ʹ�ܣ�0--��ֹ) 
#define  GRID               0xe1											///< ���س��ֵ (����㣬1/2���) 
// #define    GRID_1B1  0												///< ����� 
// #define    GRID_1B2  1												///< 1/2��� 
// #define    GRID_1B4  2												///< 1/4��� 

#define  BIN                0xe2											///< ���س��ϲ� (d0-ˮƽ�ϲ� d1-��ֱ�ϲ�) 
#define  GAMMA              0xe3										///< GAMMAʹ�� (0- Ĭ�� 1-auto gamma) 
#define  T1_MODE            0xe4										///< 
#define  IMAGE_FORMAT       0xe4								///< ��Ƶģʽͼ�������ʽ 0-BAYER_8b 1-BAYER_12b 2-YUV422 3-JPEG 4-MPEG4 
// #define    IF_BAYER_8B   0											///< 
// #define    IF_BAYER_12B  1										///< 
// #define    IF_YUV422     2											///< 
// #define    IF_JPEG       3												///< ��JPEG 
// #define    IF_MPEG4      4											///< ��H264 
// #define    IF_MAX        4

#define  TRIG_IMAGE_FORMAT     0xe5							///< ����ͼ���ʽ 
#define  IMAGE_ZOOM_W       0xe6								///< ͼ������ߴ�:�� 
#define  IMAGE_ZOOM_H       0xe7								///< ͼ������ߴ�:�� 
//...
#define  CHARACTER_X_TRIG   0xe9								///< ץ��ͼƬ��������:X -1����ʾ ���¹̼��Ѿ�������
#define  CHARACTER_Y_TRIG   0xea								///< ץ��ͼƬ��������:Y -1����ʾ ���¹̼��Ѿ�������
//..
#define  AUTOGRAY_PARAM     0xec								///< �Զ��Ҷ���ǿǿ�ȣ�0~32�� 
#define  MPEG_INIT          0xed									///< ��ʼ������ 
#define  CHARACTER_X        0xee									///< ��������:X -1����ʾ 
#define  CHARACTER_Y        0xef									///< ��������:Y -1����ʾ 

//#define  IO_DIR                  0xe8									///< 
//#define  IO_INT                  0xe9									///< ץ��ͼƬ��������:X -1����ʾ 
#define  IO_VAL                  0xea									///< ץ��ͼƬ��������:Y -1����ʾ 
#define  IO_ACK                  0xeb									///< TORAGE send to server every .. ms 
#define  IO_LED                  0xec									///< �Զ��Ҷ���ǿǿ�ȣ�0~32�� 
#define  REBOOT                  0xed									///< ��ʼ������ 

#define  SOFT_TRIGGER       0xf0									///< ����������� 1-��ͨ������ 0-��ͨ������ 
#define  TEST_IMAGE         0xf1									///< ����ͼ��ѡ�� 1-����ͼ�� 0-����ͼ�� 
#define  TIME_STAMP         0xf2									///< ��ʱ�������Ϊ��׼����� 
#define  NOISE_FLT          0xf4										///< �����˲�����
#define  EDGE_ENAHNCER      0xf5								///< ��Ե��ǿ����(0--4)

#define  DRC                0xf6											///< ��̬��ǿ���� 

#define  IMAGE_ZOOM_W_STREAM    0xf7					///< ��ý��ͼ������ߴ� 
#define  IMAGE_ZOOM_H_STREAM    0xf8					///< ��ý��ͼ������ߴ� 
#define  POSITION_FLASH              0xf9						///< flash pos (d0~d15:start percent, d16~d31:wide percent) 
#define  POSITION_LED              0xfa							///< led pos (d0~d15:start percent, d16~d31:wide percent) 
#define  LOGO_POSITION            0xfb							///< d0~d15: x, d16~d31: y 
#define  SATURATION                0xfc							///< saturation set (0 ~ 255 , 127 def, 180 or 220 suggested) 
#define  GIC                    0xff										///< gic params 

#define  SERVER_IP          0x100									///< ������IP(���ģʽ��Ч) 
#define  SERVER_PORT        0x101									///< ������PORT(���ģʽ��Ч) 
#define  TRI_JPEG_Q         0x102									///< ץ��ʱJPEG����(���ģʽ��Ч) 
#define  TRI_EXP_TIME       0x103									///< ���ģʽ��Ч(ץ��ʱ���õ��ع�ʱ��) 
#define  TRI_GAIN           0x104									///< ���ģʽ��Ч(ץ��ʱ���õ�����) 
#define  TRI_GAIN_R            0x105								///< ���ģʽ��Ч(ץ��ʱ���õ�R����) 
#define  TRI_GAIN_G            0x106								///< ���ģʽ��Ч(ץ��ʱ���õ�G����) 
#define  TRI_GAIN_B            0x107								///< ���ģʽ��Ч(ץ��ʱ���õ�B����) 
#define  TRI_AGC_TIME_MAX               0x108				///<  �Զ�����ʱ��ץ�ĵ��ӿ��ŵ������ֵ(us)*/
#define  CHARACTER_INFO_SET_TRIG     0x109				///<  ץ�����ֵ���*/
#define  SETVIRTUALCOIL              0x10a						///<  ������Ȧ����*/
#define  SETRECOGNIZEZONE            0x10b					///<  ʶ����������*/

#define  EXT_SYNC_DELAY        0x110							///< ��ͬ����ʱ 
#define  EXT_TRIG_DELAY     0x111								///< �ⴥ����ʱ 
#define  FLASH_MODE            0x112								///< ��������ģʽ������ʱ��Ч����0-��ͨģʽ 1-����ģʽ�� 
#define  IOTRIG_MODE        0x113								///< ����ץ��ģʽѡ�� 
#define    IOTRIG_MODE_SPEED    0								///< ����ģʽ(��΢��Ϊ��λ) 
#define    IOTRIG_MODE_SIGNAL    1							///< ���̵�ģʽ(����Ϊ��λ) 
#define  LED_OUT_MODE     0x114								///< Ƶ����Ƶ���(d0-Ƶ����Ƶʹ��λ,d1-Ƶ���������λ) 
#define VEDIO_OUTPUT_EN    0x115								///< ����ģ����Ƶ�����0-��ֹ 1-ʹ�ܣ� 

#define  CMOS_FLASH_MODE        0x116						///< CMOS�����ģʽ (0 - LED��ģʽ, 1 - �����ģʽ) 0 led mode(ers always), 1 flash mode(ers when day, grr when night) 
#define  CHL_OF_TRIG_IMG        0x117							///< trigger image trans chl (0 - snap server, 1 - ftp server)
    
#define  CMOS_FLASH_MOD_FRM    0x118					///< ��ҹ��Ӧ�ж����� 
#define  CMOS_FLASH_MOD_LV    0x119						///< ��ҹ��Ӧ�жϻ�׼ 
#define SnapPicSetMode     0x11a								///< ץ��ͼ��ߴ�����ģʽ 0--ȫ�ߴ� 1--�����óߴ�      
#define RADAR_TRIG_AGREEMENT 0x11E						///< trig agreement select
#define RADAR_MODE_DEF        0									///< id chl inv_h inv_l data0 data1 data2 data3 sum
#define RADAR_MODE_RADAR    1								///< id data0 data1 data2 data3

#define  TRI_PARAM_EN           0x11F							///< ���ģʽץ�Ĳ���ʹ��(D0 - "�ع�ʱ��",1��ʾ����ʹ�ö�������,0��ʾʹ����Ƶ���� D1 - "����" D2 - "RGB��ɫ����") 
//#define TRI_PARAM_TIME_EN      (1<<0)					///<  ץ��ͼ������ع�ʱ��ʹ�� 
//#define TRI_PARAM_GAIN_EN      (1<<1)					///<  ץ��ͼ���������ʹ�� 
//#define TRI_PARAM_RGB_GAIN_EN  (1<<2)				///<  ץ��ͼ�����RGB����ʹ�� 
//#define TRI_PARAM_GRAY_EXT_EN  (1<<3)				///<  ץ��ͼ�����ûҶ���ǿ 
//#define TRI_PARAM_GRAY_DRC_EN  (1<<4)				///<  ץ��ͼ�����ö�̬��ǿ������֡������1ʱ��������ã� 

#define  IP_ADDR            0x120									///< �����IP��ַ 
#define  GATEWAY            0x121									///< ��������� 
#define     NETMASK            0x122								///< �����ַ���� 
#define  CTRL_PORT          0x123									///< ���ƶ˿� 
#define  TRAN_PORT          0x124									///< ����˿� 

#define  TF_COUNT           0x125									///< ����ץ��֡�� (1-3) 
#define  JPEG_SET           0x126										///< JPEGѹ��Ʒ�� (0-100) 0--��� 100--���� 
#define  EXP_OUT_EN            0x127								///< �ع�ͬ�����( d0--����� d1--Ƶ�� d2--Ƶ���Զ� d3--�����Զ� ) 
#define  TRIG_SET           0x128										///< ����֡���12 (ms) 
#define  XC_SET             0x129										///< ���������������(����),1~511,��λus ��Jϵ�п����� 
#define  AI_LEVEL            0x12A
#define  RADAR_SET_0        0x12B									///< ���û���״����ݳ�ʱ����(��λms) 
#define  RADAR_SET_1        0x12C									///< Ԥ�� 
#define  RADAR_SET_2        0x12D								///< �����״ﴮ��ͬ��ͷ�ֽڶ��壬32B�����ֳ�4���ֽڣ�����ʹ��4��ͬ���ֽ���������յ�ץ�Ĵ����󣬽���RS232���ݣ�������յ�RS232�����ֽ���4��ͬ���ֽڻ����յ�ץ�Ĵ����󣬽���RS232���ݣ�������յ�RS232�����ֽ���4��ͬ���ֽڵ�����һ��ƥ��ʱ����¼����n������Ƕ��֡��Ϣͷ�ش� 
#define  RADAR_SET_3        0x12E									///< ���ô��ڲ����� 
#define  RADAR_SET_4        0x12F									///< ����ͬ���ֺ���Ϣ���� 1~4�ֽ� 

#define  YEAR               0x130										///< ʱ��:�� 
#define  MONTH              0x131									///< ʱ��:�� 
#define  DAY                0x132										///< ʱ��:�� 
#define  HOUR               0x133										///< ʱ��:ʱ 
#define  MINUTE             0x134									///< ʱ��:�� 
#define  SECOND                0x135									///< ʱ��:�� д��������ϵͳʱ�� 

#define  TRIGGER_DELAY      0x136								///<  ץ����ʱ(us) 
#define  EXP_DELAY          0x137									///<  �ع���ʱ(us) 
#define  EXP_OUT_JX         0x138									///<  �ع�������ԣ�d0--����� d1--Ƶ����ֵ�ı�ʾ��0�������ԣ�1�������� 
#define  ROTATE             0x139										///<  0������ת��1����ת90�ȣ�2����ת180�ȣ�3����ת270�� 
#define  H264_BRATE         0x13a									///<  H264 bit-rate, [0,100] 
#define  SELECT_IMAGE       0x13F								///<  ѡͼ���� �ڲ�ʹ�� 
#define  USER_INFO          0x140									///< ѡͼ���� �ڲ�ʹ�� 


#define  SOFT_RESET            0xFF0002							///< ��λ���� 
#define  SYNC_PARAM            0xFF0003						///< ͬ���������� 
#define  SAVE_PARAM            0xFF0004						///< �������������� 
#define  RESET_PARAM        0xFF0005							///< ��λ��������� 
#define  CMD_GAMMA            0xFF0006						///< ����GAMMA���߲��� + 4K SHORT 
#define  CMD_HDJZ           0xFF0007							///< ���ػ���У������ + 16K SHORT 
#define  REFRESH_NET        0xFF0008							///< ʹ���������� 
#define  RED_LIGHT_STATUS    0xFF0009						///< ����ź� 

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
             * @brief �������
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
        //lyr 12/07/24 ͸������
        class NC_EXT NetCom  
        {
#define CAM_RECV_PORT 8889
#define CAM_SEND_PORT 8890
#define RETRY_MAX 2
#define NETCOM_TIMEOUT 500 

            typedef struct _NETCOM_HEADER_ 
            {
                unsigned short hdr_ver; // ͨ��ͷ�汾 0x0100 Rev.1.0
                unsigned short type;  // ͨ���������  0-��ʼ�� 1-���� 2-ACK
                unsigned short port;  // ָʾ���͸����ݰ����豸�Ľ��ն˿�
                unsigned short chl;  // ����ͨ����
                unsigned short retry; // �ش�������0��ʾԶ�˵�һ�η��͸����ݰ�
                unsigned short rev_0; // ����
                unsigned short sq_no; // ���ݰ��ţ�Ӧ��ʱʹ��
                unsigned short content_len; // ������Ч���ݳ���
            } NETCOM_HEADER;
        public:
            NetCom();
            virtual ~NetCom();

        private:
            unsigned short recv_port; // �������ն˿�
            unsigned short send_port; // �������Ͷ˿�
            unsigned short send_inx;
            unsigned short recv_inx;
            SOCKET recv_socket;
            SOCKET send_socket;
            unsigned long cam_ip; // Ŀ�������IP
        public:
            int NetCom_Open(char *target_ip,unsigned short snd_port,unsigned short rcv_port); // ���÷��Ͷ˿�����ն˿ڣ�����ʼ��
            int NetCom_Send(char *msg,int len,int chl);
            int NetCom_Recv(char *msg,int len,int *chl,int timeout); // ��ʱ��λ����
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
     @ brief ���ڴ�buffer�е�RGB���ݴ洢Ϊjpg�ļ�
     @ param filepath  -- �ļ�����ȫ·��
     @ param pRGB      -- ָ��RGB����
     @ param width          -- ͼ����
     @ paramheight      -- ͼ��߶�
     @ param jpegQ          -- ͼ���ѹ��ϵ��(Ĭ��Ϊ85)
     @ return bool -- ����ɹ������ļ� ����true ���򷵻�false
    */
    bool SaveRgbToJpgFile(char* filepath,BYTE* pRGB,int width,int height,int jpegQ = 85);
    /**
    * @brief    usm�񻯴���
    * @param    srcPR        ͼ������Դ��ַ��RGB��ʽ
    * @param    destPR        ͼ������Ŀ���ַ��RGB��ʽ
    * @param    w            ͼ���ȣ���λΪ����
    * @param    h            ͼ��߶ȣ���λΪ����
    * @param    radius        USM����������뾶����Χ0.1~20������ȡֵ5
    * @param    amount        USM�����������������Χ0.1~120������ȡֵ0.5
    * @param    threshold    USM�����������ֵ����Χ1~255������ȡֵ2
    * @return    void        
    */
#endif
    static void usm_process (unsigned char *srcPR, unsigned char *destPR, int w, int h, double radius, double amount, int threshold);
    /**
    * @brief    �Աȶ���ǿ����
    * @param    buf            ͼ������Դ��ַ��RGB��ʽ
    * @param    w            ͼ���ȣ���λΪ����
    * @param    h            ͼ��߶ȣ���λΪ����
    * @param    threshold    ������ֵ����Χ0~0.1������ȡֵ0.01
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
    * @brief ����������״̬
    * @return -1--��������ʧ�� 0--�ֶ��Ͽ����� 1--����״̬ -2 --�������ΪNULL
    */
    int getCameraTransState(void);
    /**
    * @brief ����������ͨ��״̬
    * @return -2 --�������ΪNULL 0--�ֶ��Ͽ����� 1--����״̬
    */
    int getCameraCtrlState(void);
    /**
    * @brief ���ӻ��߶Ͽ�����
    * @param op1:0�Ͽ�����ͨ�� 1��������ͨ�� -1��������
    * @param op2:0�Ͽ�����ͨ�� 1��������ͨ�� -1��������
    * @param m_bopenCtlTh: 1-������������ 0-������
    *return:TRUE���ӻ��߶Ͽ��ɹ� FALSE����ʧ��
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
	/*			 ��װ����			           */
	/************************************************************************/
	/**
    * @brief ��������
	* @param pIP_ADDR ip��ַ
	* @param pGATEWAY ���������
	* @param pNETMASK �����ַ����
	* @param pSERVER_IP ������IP������ģʽ��Ч��
	* @return �ɹ�����0�����򷵻�С��0
    */
	int UpdateSetNet(char* pIP_ADDR, char* pGATEWAY, char* pNETMASK ,char* pSERVER_IP = NULL );
	/**
    * @brief ��ȡIP��ַ
	* @return ����IP��ַ
    */
	int ReadIPAddr();
	/**
    * @brief ��ȡ��������
	* @return ������������
    */
	int ReadGateWay();
	/**
    * @brief ��ȡ����
	* @return ��������
    */
	int ReadNetMask();;
	/**
    * @brief ��ȡ��ȡ������IP������ģʽ��Ч��
	* @return ���ط�����IP
    */
	int ReadServerIP();
	/**
    * @brief ����Уʱ
	* @return Уʱ�ɹ�����0 �����򷵻�С��0
    */
	int UpdateNetTime();
	/**
    * @brief ͬ�������������
	* @return ͬ���ɹ�����0 �����򷵻�С��0
    */
	int UpdateSyncParam();
	/**
    * @brief ��������������
	* @return ����ɹ�����0 �����򷵻�С��0
    */
	int UpdateSaveParam();
	/**
    * @brief ������
	* @return �������ɹ�����0 �����򷵻�С��0
    */
	int UpdateSoftReset();
	/**
    * @brief ��λ���
	* @return ��λ�ɹ�����0 �����򷵻�С��0
    */
	int UpdateResetParam();
	/**
    * @brief ����������Ʒ�ʽ
	* @param AGCMode ������Ʒ�ʽ��1--�Զ�(AGC)��0--�ֶ���
	* @return �������淽ʽ�ɹ�����0�����򷵻�С��0
    */
	 int UpdateAGCMode(int AGCMode);
	/**
    * @brief ��ȡ������Ʒ�ʽ
	* @param nState ȱʡֵΪ0,��ȡ�������ã�1��ʾֱ�Ӵ�����˶�ȡ
	* @return ��ȡ���淽ʽֵ0Ϊ�ֶ���1�Զ�
    */
	 int ReadAGCMode(int nState = 0);
	/**
    * @brief ����AGC����ֵ (0-255) 
	* @param AGCLimit ����ֵ
	* @return ����AGC����ֵ�ɹ�����0�����򷵻�С��0����������б�궨�壩
    */
	 int UpdateAGCLimit(int AGCLimit);
	/**
	* @brief ��ȡAGC����ֵ (0-255) 
	* @param nState ȱʡֵΪ0,��ȡ�������ã�1��ʾֱ�Ӵ�����˶�ȡ
	* @return ��������ֵ
    */
	 int ReadAGCLimit(int nState = 0);
	/**
    * @brief AGC����ӿ�������ʱ�����ӿ��ŵ���ֵ����
	* @param AGCTimeMin ��Сֵ(us) 
	* @param AGCTimeMax ���ֵ(us) 
	* @return ���óɹ�����Ϊ0������ʧ��
    */
	 int UpdateAGCTimeRange(int AGCTimeMin,int AGCTimeMax);
	/**
    * @brief AGC����ӿ�������ʱ����ȡ���ӿ��ŵ���ֵ��Сֵ
	* @param  nState ȱʡֵΪ0,��ȡ�������ã�1��ʾֱ�Ӵ�����˶�ȡ
	* @return ������Сֵ
    */
	 int ReadAGCTimeMin(int nState = 0);
	/**
	* @brief AGC����ӿ�������ʱ����ȡ���ӿ��ŵ���ֵ���ֵ
	* @param  nState ȱʡֵΪ0,��ȡ�������ã�1��ʾֱ�Ӵ�����˶�ȡ
	* @return �������ֵ
    */
	 int ReadAGCTimeMax(int nState = 0);
	/**
    * @brief ���ü��ģʽץ�Ķ�������ʹ��
	* @param m_TRIParamTimeEN ʱ��ʹ��
	* @param m_TRIParamGainEN ����ʹ��
	* @param m_TRIParamRGBEN ɫ��ʹ��
	* @return ���óɹ�0�����򷵻�С��0
    */
	 int UpdateTRIParamEN(bool m_TRIParamTimeEN, bool m_TRIParamGainEN, bool m_TRIParamRGBEN);
	/**
    * @brief ��ȡ���ģʽץ�Ĳ���ʹ��
	* @return ����ʹ��״̬0&1
    */
	 int ReadTRIParamEN();
	/**
    * @brief ץ��ʱ�ع�ʱ������
	* @param m_TRIEXPTime ʱ�����
	* @return ���óɹ�����0�����򷵻�С��0
    */
	 int updateTRIEXPTime(int m_TRIEXPTime);
	/**
    * @brief ��ȡץ��ʱ���õ��ع�ʱ��
	* @return �����ع�ʱ��
    */
	 int ReadTRIEXPTime();
	/**
    * @brief ���ü��ģʽ��Ч(ץ��ʱ���õ�����)��*spv��Ч��
	* @param m_TRIGain ץ������
	* @return ���óɹ�����0�����򷵻�С��0
    */
	 int updateTRIGain(int m_TRIGain);
	 /**
    * @brief ��ȡץ��ʱ���õ�����
	* @return ����ץ������ֵ
    */
	 int ReadTRIGain();
	/**
    * @brief ����ץ��RGB����
	* @param m_TRIRGain ץ�Ĳ��õ�R����
	* @param m_TRIGGain ץ�Ĳ��õ�G����
	* @param m_TRIBGain ץ�Ĳ��õ�B����
	* @return ���óɹ�����0������ʧ��
    */
	 int UpdateTRIParamRGBGain(int m_TRIRGain, int m_TRIGGain, int m_TRIBGain);
	/**
    * @brief ��ȡץ���ǲ��õ�R����
	* @return ����R����ֵ
    */
	 int ReadTRIParamRGain();
	/**
    * @brief ��ȡץ���ǲ��õ�G����
	* @return ����G����ֵ
    */
	 int ReadTRIParamGGain();
	 /**
    * @brief ��ȡץ���ǲ��õ�B����
	* @return ����B����ֵ
    */
	 int ReadTRIParamBGain();
	/**
    * @brief ���ð�ƽ�⵱ǰRGB����ֵ
	* @param m_TRIRGain ��ǰR����
	* @param m_TRIGGain ��ǰG����
	* @param m_TRIBGain ��ǰB����
	* @return ���õ�ǰ����ɹ�����0������ʧ��
    */
	 int UpdateGainRGB(int m_GainR, int m_GainG, int m_GainB);
	 /**
    * @brief ��ȡ��ƽ�⵱ǰR����
	* @return ���ص�ǰR����
    */
	 int ReadGainR();
	/**
    * @brief ��ȡ��ƽ�⵱ǰG����
	* @return ���ص�ǰG����
    */
	 int ReadGainG();
	 /**
    * @brief ��ȡ��ƽ�⵱ǰB����
	* @return ���ص�ǰB����
    */
	 int ReadGainB();
	/**
    * @brief ���ð�ƽ��У����ʽ
	* @param m_BalaceMode 0--�ֶ���ƽ�� 1--�Զ���ƽ�� 
	* @return ���óɹ�����0�����򷵻�С��0
    */
	 int UpdateBalanceMode(int m_BalaceMode);
	 /**
    * @brief ��ȡ��ƽ��У����ʽ
	* @param nState ȱʡֵΪ0,��ȡ�������ã�1��ʾֱ�Ӵ�����˶�ȡ
	* @return ���ذ�ƽ��У����ʽ
    */
	 int ReadBalanceMode(int nState = 0);
	/**
    * @brief ����jpegͼ��ߴ�
	* @param m_ImageZoomW ͼƬ��
	* @param m_ImageZoomH ͼƬ��
	* @return  ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateImageZoomSize(int m_ImageZoomW, int m_ImageZoomH);
	 /**
    * @brief ��ȡjpegͼ���
	* @return ���ؿ�
    */
	 int ReadImageZoomW();
	 /**
    * @brief ��ȡjpegͼ���
	* @return ���ظ�
    */
	 int ReadImageZoomH();
	 /**
    * @brief ������ý��ͼ������ߴ�
	* @param m_ImageZoomWStream ��ý��ͼ���
	* @param m_ImageZoomHStream ��ý��ͼ���
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateImageZoomSizeStream(int m_ImageZoomWStream, int m_ImageZoomHStream);
	 /**
    * @brief ��ȡ��ý��ͼ�������
	* @return ������ý��ͼ��Ŀ�
    */
	 int ReadiImageZoomWStream();
	 /**
    * @brief ��ȡ��ý��ͼ�������
	* @return ������ý��ͼ��ĸ�
    */
	 int ReadiImageZoomHStream();
	 /**
    * @brief upadate���ֵ���
	* @param info_struct ���ֵ��ӽṹ��ָ��
	* @return Update�ɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateBlockCharacterInfo(character_info* info_struct);
	 /**
    * @brief Read���ֵ���
	* @param info_struct ���ֵ��ӽṹ��ָ��,������Ϣ
	* @return Read�ɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int ReadBlockCharacterInfo(character_info* info_struct);
	 /**
    * @brief ����-ģ��ץ��
	* @param m_st_chl �����ͨ��
	* @param m_st_fcnt ץ������
	* @param m_st_inv ץ�ļ��
	* @param m_st_info �Զ�����Ϣ
	* @return �����ɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateSoftTrigger(int m_st_chl, int m_st_fcnt, int  m_st_inv,long m_st_info);
	 /**
    * @brief ���������Ƶ����ʹ�ܿ���
	* @param EXPOutEN ˫�ֽڲ���
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateEXPOutEN(unsigned int   EXPOutEN);
	  /**
    * @brief ��ȡ�����Ƶ����ʹ��״̬
	* @return ���������Ƶ����ʹ��״̬
    */
	 int ReadEXPOutEN();
	 /**
    * @brief ����Ƶ���Ʊ�Ƶ���������
	* @param m_bool1 Ƶ���Ʊ�Ƶ
	* @param m_bool2 Ƶ�����������
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateLedOutMode(bool m_bool1, bool m_bool2);
	  /**
    * @brief ��ȡƵ���Ʊ�Ƶ���������
	* @return ������Ϣ
    */
	 int ReadLedOutMode();
	 /**
    * @brief ������Ƶģʽͼ�������ʽ����Ƶ��ѡ��
	* @param m_ImageFormat JPEG��H264��˫����
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateImageFormat(int m_ImageFormat);
	  /**
    * @brief ��ȡ��Ƶģʽͼ�������ʽ����Ƶ��ѡ��
	* @return ������Ƶ����ʽ����
    */
	 int ReadImageFormat();
	 /**
    * @brief �����������ģʽ
	* @param m_SYNC ����ģʽ��1-ץ��ģʽ 3-���ģʽ 5-��Ƶģʽ
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateSYNC(int m_SYNC);
	 /**
    * @brief ��ȡ�������ģʽ
	* @return �����������ģʽ����
    */
	 int ReadSYNC();
	  /**
    * @brief ����JPEGͼƬѹ��Ʒ��
	* @param m_JpegQuality ͼƬƷ��
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateJPEGQuality(int m_JpegQuality);
	 /**
    * @brief ��ȡJPEGͼƬѹ��Ʒ��
	* @return ����JPEGͼƬѹ��Ʒ�ʲ���
    */
	 int ReadJPEGQuality();
	 /**
    * @brief FTP����
	* @param buffer �洢���ݵĻ�����
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UPdateBlockFTPAccountSet(char buffer[]);
	  /**
    * @brief NTP����
	* @param buffer �洢���ݵĻ�����
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UPdateBlockNTPSetting(char buffer[]);
	 /**
    * @brief ����ץ������
	* @param info_struct ����ץ�Ľṹ��ָ��
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
    */
	 int UpdateBlockRadarLogicSet(ser_groupinfo* info_struct);
	 /**
    * @brief ioץ������ 
	* @param info_stuct ioץ�Ľṹ��ָ��
	* @return ���óɹ�����0�����򷵻�С��0����ϸ��������б�
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
