//2013-07-05
//V2.3.0

#ifndef _NETCAMERA_SNAPSERVER_H_
#define _NETCAMERA_SNAPSERVER_H_

#ifndef _WIN32	//linux
#include <pthread.h>
#include <assert.h>
#define SOCKET int
typedef pthread_t HANDLE ;
#define NC_SNAP_EXT
#ifndef _CRI_SEC_
#define _CRI_SEC_
#define CRITICAL_SECTION pthread_mutex_t
#define InitializeCriticalSection(x) pthread_mutex_init(x,NULL)
#define DeleteCriticalSection(x) pthread_mutex_destroy(x)
#define EnterCriticalSection(x) pthread_mutex_lock(x)
#define LeaveCriticalSection(x) pthread_mutex_unlock(x)
#endif
#else		//windows
#define _CRTDBG_MAP_ALLOC
#include<stdlib.h>
#include<windows.h>
#ifndef NETCAMERA_SNAPSERVER_EXPORTS
#define NC_SNAP_EXT	__declspec(dllimport)
#else
#define NC_SNAP_EXT	__declspec(dllexport)
#endif
#endif
#include <algorithm>
#include <queue>
#include <list>


////////////////////////////////////////////////////////////////////////////////
//STRUCT DEFINE
#ifndef _FRAME_HEADER_
#define _FRAME_HEADER_
struct FRAME_HEADER
{
	unsigned short w;							// 图像宽
	unsigned short h;							// 图像高	
	unsigned short bits;						// 图像位数 8~16bits
	unsigned short format;					    // 图像类型代码(0--灰度，1--Bayer_RG，2--Bayer_GR，3--Bayer_BG，5--RGB，6--YUV422，7--JPEG, 8--MPEG4 9--h264)	
	unsigned short frame_type;				    // 帧类型(0--普通，1--抓拍图像，2--心跳帧)
	unsigned short frame_rev;					// 保留	
	unsigned int  firmware_version;			    // 固件程序版本
	unsigned int  device_no;					// 设备编号	
	unsigned int  len;						    // 图像数据长度	
	unsigned int  speed;						// 双线圈测速值(us)
	unsigned int  rs232;						// 串口信息(1~4字节)
	unsigned short year;						// 图像采集时间
	unsigned short month;
	unsigned short day;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;	
	unsigned int ip;							// 采集当前帧的摄像机IP
	unsigned int frame_count;					// 总共的抓拍帧数目
	unsigned int trigger_count;				// 总共的触发数
	unsigned int trigger_index;				// 触发组索引
	unsigned int frame_no;					// 帧号	
	unsigned int gain;						// 当前抓拍参数
	unsigned int time;						// 曝光时间
	unsigned int gain_r;						// 红增益
	unsigned int gain_g;						// 绿增益
	unsigned int gain_b;						// 蓝增益
	unsigned int mode;						// 摄像机工作模式
	unsigned int JpegQ;						// JPEG压缩品质
	unsigned int td1;							// 抓拍延时(单位us)
	unsigned int td2;							// 曝光延时(单位us)
	unsigned int trig_chl;					// 触发通道
	unsigned int msecond;						// ms计时
	unsigned int yavg;						// 平均亮度
	unsigned int vid_head;					// 流媒体关键帧
	unsigned int st_usr_param;				// 软触发信息
	unsigned int red_time;					// 红灯计时
	//unsigned long rev[6];					// 保留参数
	unsigned int grey;						// 一个点的原始亮度
	unsigned int wz_type;						// 违章类型，d0-超速,d1-闯红灯
	unsigned short traffic_code;				// 违章代码
	unsigned char capture_no;					// 本次触发抓拍张数（total）
	unsigned char rev_1;						// 保留位
	unsigned int rev[3];						// 保留参数
	unsigned char user_info[64];				// 用户数据
};
#endif
typedef struct _CapClientStruct
{
    HANDLE m_hCapThread;
    char m_strIp[16];
    SOCKET m_sockCap;
}CapClientStruct;
class NC_SNAP_EXT NetCamera_SnapServer  
{
	SOCKET listen_s; //监听的套接字
public:
    NetCamera_SnapServer();
    virtual ~NetCamera_SnapServer();
public:
    class Cap_Lock
    {
        CRITICAL_SECTION        m_cs;
    public:
        Cap_Lock();
        ~Cap_Lock();
    public:
        int Lock();
        int UnLock();
    };
public:
	SOCKET getListenS(void){return listen_s;}
	void setListenS(SOCKET s){listen_s=s;}
	int &getState(void){return m_nState;}
	int StartRecv(unsigned short port=9999);
	int GetImage(unsigned char* buf);
	unsigned long GetImageSize();
	int AddImage(unsigned char* pImage,int buflen);
    unsigned short GetRecvPort();
private:
	int						m_nState;
	int						m_nMaxQueueLength;
	unsigned short			m_nPort;
    HANDLE m_hCapThread;
	unsigned char* pImgData;
	int queIndex;
	std::queue<unsigned char*>*	m_pImageQueue;
	CRITICAL_SECTION		m_cs;
public:
	static int MySend(SOCKET& s, char *b, int l,int to);
	static int MyRecv(SOCKET s, char *b, int l, int to);
private:
	static int MySendWithTimeout(SOCKET s, char *b, int l, int to, int *send_len);
	static int MyRecvWithTimeout(SOCKET s, char *b, int l, int to, int *recv_len);
};

#endif
