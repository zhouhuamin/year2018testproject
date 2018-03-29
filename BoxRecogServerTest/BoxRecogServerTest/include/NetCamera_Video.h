//2013-07-05
//V2.3.0
#ifndef _NET_CAMERA_VIDEO_H_
#define _NET_CAMERA_VIDEO_H_
#pragma once
#ifdef _WIN32
#include "WinSock2.h"
#else
#include "sys/time.h"
#endif
typedef int (*NetCamera_Video_SetFileName_callback)(char*,int);

#ifdef _WIN32
	#ifdef NETCAMERA_VIDEO_EXPORTS
	#define NC_VIDEO_EXT __declspec(dllexport)
	#else
	#define NC_VIDEO_EXT __declspec(dllimport)
	#endif
#else
	#define NC_VIDEO_EXT
#endif

namespace cy_sys
{
	namespace GNetCamera_Video
	{
		#define makeFOURCC( ch0, ch1, ch2, ch3 )\
			( (unsigned long)(unsigned char)(ch0) | ( (unsigned long)(unsigned char)(ch1) << 8 ) |\
			( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )



		#define VIDEO_UNKOWN		0x00//UNKOWN
		#define VIDEO_MJPEG         0x01//MJPEG
		#define VIDEO_MPEG4         0x02//MPEG4
		#define VIDEO_H264          0x03//H.264
		#define AUDIO_UNKOWN        0x00//UNKOWN
		#define AUDIO_WAVE          0x01//WAVE
		#define AUDIO_MP3           0x02//MP3
		#define FRAME_TYPE_VIDEO    0x00//FrameType_Video
		#define FRAME_TYPE_AUDIO    0x01//FrameType_AUDIO
		#define FRAME_TYPE_EXT      0x02//FrameType_ExtentData
		#define    NCVideo_ERROR_OK                      (000000)
		#define    NCVideo_ERROR_INVALID_PARAM           (-60000)
		#define	   NCVideo_ERROR_OPEN_FILE_FAILED        (-60001)
		#define    NCVideo_ERROR_UNKOWN_FORMAT           (-60002)
		#define    NCVideo_ERROR_ADDH264_FAILED           (-60003)
	}
}
/*
 *@brief 视频存储类:用于对视频流进行存储
 */
class NC_VIDEO_EXT NetCamera_Video  
{
	void* m_LSIF;
	int m_IsFileFull;
// constructor and destructor
public:
	/*!
	* @brief 构造函数，初始化类的各成员
	* @param void
	* @return 构造函数无返回值
	*/

	NetCamera_Video();

	/*!
	* @brief  析构函数
	* @param  void
	* @return 
	*/
	virtual ~NetCamera_Video();

// Operation
public:
	/*!
	* @brief  添加一帧数据到视频中
	* @param  buf 一帧数据的缓冲区(IN)
	* @param  nSize  帧数据的大小
	* @return 见错误列表
	*/
	int AppendDataToVideo(char *buf, int nSize, int frmType);

	/*!
	* @brief  关闭当前正在录像的视频文件，并写入视频头信息
	* @param  void
	* @return 见错误列表
	*/
	int CloseAvi();

	/*!
	* @brief  设置最大的录像时间(单个视频)，单位：s(秒)
	* @param  nMaxTime 最大录像时间
	* @return 设置后当前的最大录像时间
	*/
	int SetMaxRecordTime(int nMaxTime);

	/*!
	* @brief  设置最大的录像大小(单个视频)，单位：mbyte
	* @param  nMaxSize 最大的mbyte字节数
	* @return 设置后的当前最大录像大小
	*/
	int	SetMaxRecordSize(int nMaxSize);

	/*!
	* @brief  设置视频格式 目前支持 VIDEO_MJPEG / VIDEO_MPEG4  /VIDEO_H264
	* @param  代表视频格式的整数
	* @return 无意义值
	*/
	int SetVideoFormat(int videoFormat);

	/*!
	* @brief  设置写入帧的格式 视频、音频、或其它
	* @param  当前帧类型，写入帧
	* @return 无意义值
	*/
	int SetAudioFormat(int audioFormat);


	/*!
	* @brief  获取当前已存的帧数
	* @param  void
	* @return 当前已存的帧数
	*/
	int GetFrameNum();

	/*!
	* @brief  获取当前所录视频的格式
	* @param  void
	* @return 当前所录视频的格式
	*/
	int GetVideoFormat();

	/*!
	* @brief  设置文件保存的路径
	* @param  szFilePath 文件保存的路径
	* @return 见错误列表
	*/
	int SetFileDir(char* szFilePath);

	/*!
	* @brief  根据szUserInfo信息生成保存目录
	* @param  void
	* @return 成功返回true,失败返回false
	*/
	bool AutoSetFilePath(void);

	/*!
	* @brief  设置视频宽度
	* @param  nWidth
	* @return 成功返回设置后的视频宽(为一正数)，其他情况见错误列表
	*/
	int SetWidth(int nWidth);

	/*!
	* @brief  设置视频宽度
	* @param  nHeight
	* @return 成功返回设置后的视频宽(为一正数)，其他情况见错误列表
	*/
	int SetHeight(int nHeight);

	/*!
	* @brief  设置视频帧率
	* @param  设置视频的帧率大小
	* @return 设置成功则返回设置后的视频帧率(为一正数), 其他情况见错误列表
	*/
	int SetFps(int nFps);

	/*!
	* @brief  添加一帧图像到视频
	* @param  buf 一帧图像的缓冲区(IN)
	* @param  dwSize 图像数据的长度
	* @return 见错误列表
	*/
	int AddFrame(char *buf, unsigned int dwSize, int frmType);
    int SetVideoModifyFilenameCallBack(NetCamera_Video_SetFileName_callback  callBackName);
	int AddH264Frame(char *buf, unsigned int dwSize, int frmType);

	/*!
	* @brief  打开一个视频文件，为写文件做准备
	* @param  szFileName 打开视频文件的路径及文件名
	* @param  dwWidth  初始化视频文件图像的宽
	* @param  dwHeight 初始化视频文件图像的高
	* @return 见错误列表
	*/
	int OpenAvi(char* szFileName, unsigned int dwWidth, unsigned int dwHeight);

	//检测文件是否已满 (需重新建立新的视频文件)(当文件长度达到约定长度或时间达到约定长度 则返回true 否则false) 
	//文件长度达到2000M直接返回true
	bool IsFileFull();

	// 重置所有属性
	void ResetAllState();
private:
	/*!
	* @brief 增加录像计时
	* @param nTime 增加的时间，单位秒
	* @return 如果成功返回当前的录像计时(为一正数)，如果失败，见错误列表
	*/
	int AddTime(int nTime=1);
	
	// 倒序写入文件
	void WriteQuartet(void* fp, unsigned int i);

	// 设置数据流种类数量
	void SetStreamType();
public:	
	char* getFilePath(void){return m_FilePathName;}
// Attributes
private:
	void* pVariable;
	//////////////////////////////////////////////////////////////////////////
	int m_VideoFormat;		   // 视频格式 0:未知 1:MJPEG 2:MPEG4 3:H264
	int m_AudioFormat;         // 音频格式
    int m_close_flag;

	int   m_nMaxTime;				   // 最大记录时间
	int   m_nMaxSize;				   // 最大记录大小
	int   m_nCurTime;                  // 当前记录时间
	char  m_FilePath[256];			   // 文件路径
	char  m_FilePathName[256];         // 文件路径及文件名

	int   m_nStreamType;               // 数据流种类数
	bool  m_bAudMix;                   // 是否混合了音频文件

	unsigned int m_dwWidth;                   // 图像宽
	unsigned int m_dwHeight;				   // 图像高
	unsigned int m_dwFps;                     // 帧率
	void   *m_pfDest;                    // 要存储的文件指针
	unsigned int m_dwPerUsec;                 // 帧周期
	unsigned int m_dwFrameNum;                // 帧数

	int  m_lRiffSize;
	int  m_lJpgSize;
	unsigned int m_dwRiffSize;
	unsigned int m_dwJpgSize;
	long  m_TotalSize;                 // 视频的当前长度
	unsigned int* m_idxSize;
	unsigned int* m_idxOffset;
	//lyr 12/05/18
	static bool CreateMultiLevelDirectory(const char *strFilePathName);
	//lyr 12/05/22
	char Fname[2][256]; //[0]文件名称前缀 [1]文件名称后缀
	int FnameTimeFormatType; //文件名称中间的时间格式类型 0带-的时间 不带-的时间
    NetCamera_Video_SetFileName_callback m_setVideonameCallBack;
    struct timeval m_tmFirst ;
    struct timeval m_tmEnd ;
public:
	/**
	 * @brief 设置avi文件名称前缀或者后缀
	 * @param p     指向要设置的字符串
	 * @param type 0表示前缀 1表示后缀
	 * @param timeFormatType	文件名称中间的时间格式类型 0带-的时间 不带-的时间
	 * @param len 要设置文件名字符串长度 -1表示整个字符串
	*/
	void setFname(char* p,int type, int timeFormatType=0, int len=-1);
	
    /**
     * @brief 设置文件名由用户自定义 
	 * @param Fpath 文件路径
	*/
	void setFileName(char* Fpath)
	{
		OpenAvi(Fpath,m_dwWidth,m_dwHeight);
	}

	/**
	 * @brief 关闭当前文件 打开新文件保存视频
	*/
	void ColseCurAVI();

	//lyr 12/08/23
	/**
	 * @brief 获得avi视频文件相关信息
	 * @param w 视频图像宽度
	 * @param h 视频图像高度
	 * @param FrameNum 视频图像帧数
	 * @param m_fps 帧频
	 * @return 返回视频格式 -1错误 
	*/
	int getAVIInfo(const char* FPath, int& w, int& h, unsigned& FrameNum, unsigned& m_fps);

	/**
	 * @brief 获得视频帧数据
	 * @buff 存放视频帧数据的指针
	 * @return  返回帧数据长度 -1表示数据不正常
	*/
    unsigned getFrameData(unsigned char* buff);
};
#endif

