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
 *@brief ��Ƶ�洢��:���ڶ���Ƶ�����д洢
 */
class NC_VIDEO_EXT NetCamera_Video  
{
	void* m_LSIF;
	int m_IsFileFull;
// constructor and destructor
public:
	/*!
	* @brief ���캯������ʼ����ĸ���Ա
	* @param void
	* @return ���캯���޷���ֵ
	*/

	NetCamera_Video();

	/*!
	* @brief  ��������
	* @param  void
	* @return 
	*/
	virtual ~NetCamera_Video();

// Operation
public:
	/*!
	* @brief  ���һ֡���ݵ���Ƶ��
	* @param  buf һ֡���ݵĻ�����(IN)
	* @param  nSize  ֡���ݵĴ�С
	* @return �������б�
	*/
	int AppendDataToVideo(char *buf, int nSize, int frmType);

	/*!
	* @brief  �رյ�ǰ����¼�����Ƶ�ļ�����д����Ƶͷ��Ϣ
	* @param  void
	* @return �������б�
	*/
	int CloseAvi();

	/*!
	* @brief  ��������¼��ʱ��(������Ƶ)����λ��s(��)
	* @param  nMaxTime ���¼��ʱ��
	* @return ���ú�ǰ�����¼��ʱ��
	*/
	int SetMaxRecordTime(int nMaxTime);

	/*!
	* @brief  ��������¼���С(������Ƶ)����λ��mbyte
	* @param  nMaxSize ����mbyte�ֽ���
	* @return ���ú�ĵ�ǰ���¼���С
	*/
	int	SetMaxRecordSize(int nMaxSize);

	/*!
	* @brief  ������Ƶ��ʽ Ŀǰ֧�� VIDEO_MJPEG / VIDEO_MPEG4  /VIDEO_H264
	* @param  ������Ƶ��ʽ������
	* @return ������ֵ
	*/
	int SetVideoFormat(int videoFormat);

	/*!
	* @brief  ����д��֡�ĸ�ʽ ��Ƶ����Ƶ��������
	* @param  ��ǰ֡���ͣ�д��֡
	* @return ������ֵ
	*/
	int SetAudioFormat(int audioFormat);


	/*!
	* @brief  ��ȡ��ǰ�Ѵ��֡��
	* @param  void
	* @return ��ǰ�Ѵ��֡��
	*/
	int GetFrameNum();

	/*!
	* @brief  ��ȡ��ǰ��¼��Ƶ�ĸ�ʽ
	* @param  void
	* @return ��ǰ��¼��Ƶ�ĸ�ʽ
	*/
	int GetVideoFormat();

	/*!
	* @brief  �����ļ������·��
	* @param  szFilePath �ļ������·��
	* @return �������б�
	*/
	int SetFileDir(char* szFilePath);

	/*!
	* @brief  ����szUserInfo��Ϣ���ɱ���Ŀ¼
	* @param  void
	* @return �ɹ�����true,ʧ�ܷ���false
	*/
	bool AutoSetFilePath(void);

	/*!
	* @brief  ������Ƶ���
	* @param  nWidth
	* @return �ɹ��������ú����Ƶ��(Ϊһ����)����������������б�
	*/
	int SetWidth(int nWidth);

	/*!
	* @brief  ������Ƶ���
	* @param  nHeight
	* @return �ɹ��������ú����Ƶ��(Ϊһ����)����������������б�
	*/
	int SetHeight(int nHeight);

	/*!
	* @brief  ������Ƶ֡��
	* @param  ������Ƶ��֡�ʴ�С
	* @return ���óɹ��򷵻����ú����Ƶ֡��(Ϊһ����), ��������������б�
	*/
	int SetFps(int nFps);

	/*!
	* @brief  ���һ֡ͼ����Ƶ
	* @param  buf һ֡ͼ��Ļ�����(IN)
	* @param  dwSize ͼ�����ݵĳ���
	* @return �������б�
	*/
	int AddFrame(char *buf, unsigned int dwSize, int frmType);
    int SetVideoModifyFilenameCallBack(NetCamera_Video_SetFileName_callback  callBackName);
	int AddH264Frame(char *buf, unsigned int dwSize, int frmType);

	/*!
	* @brief  ��һ����Ƶ�ļ���Ϊд�ļ���׼��
	* @param  szFileName ����Ƶ�ļ���·�����ļ���
	* @param  dwWidth  ��ʼ����Ƶ�ļ�ͼ��Ŀ�
	* @param  dwHeight ��ʼ����Ƶ�ļ�ͼ��ĸ�
	* @return �������б�
	*/
	int OpenAvi(char* szFileName, unsigned int dwWidth, unsigned int dwHeight);

	//����ļ��Ƿ����� (�����½����µ���Ƶ�ļ�)(���ļ����ȴﵽԼ�����Ȼ�ʱ��ﵽԼ������ �򷵻�true ����false) 
	//�ļ����ȴﵽ2000Mֱ�ӷ���true
	bool IsFileFull();

	// ������������
	void ResetAllState();
private:
	/*!
	* @brief ����¼���ʱ
	* @param nTime ���ӵ�ʱ�䣬��λ��
	* @return ����ɹ����ص�ǰ��¼���ʱ(Ϊһ����)�����ʧ�ܣ��������б�
	*/
	int AddTime(int nTime=1);
	
	// ����д���ļ�
	void WriteQuartet(void* fp, unsigned int i);

	// ������������������
	void SetStreamType();
public:	
	char* getFilePath(void){return m_FilePathName;}
// Attributes
private:
	void* pVariable;
	//////////////////////////////////////////////////////////////////////////
	int m_VideoFormat;		   // ��Ƶ��ʽ 0:δ֪ 1:MJPEG 2:MPEG4 3:H264
	int m_AudioFormat;         // ��Ƶ��ʽ
    int m_close_flag;

	int   m_nMaxTime;				   // ����¼ʱ��
	int   m_nMaxSize;				   // ����¼��С
	int   m_nCurTime;                  // ��ǰ��¼ʱ��
	char  m_FilePath[256];			   // �ļ�·��
	char  m_FilePathName[256];         // �ļ�·�����ļ���

	int   m_nStreamType;               // ������������
	bool  m_bAudMix;                   // �Ƿ�������Ƶ�ļ�

	unsigned int m_dwWidth;                   // ͼ���
	unsigned int m_dwHeight;				   // ͼ���
	unsigned int m_dwFps;                     // ֡��
	void   *m_pfDest;                    // Ҫ�洢���ļ�ָ��
	unsigned int m_dwPerUsec;                 // ֡����
	unsigned int m_dwFrameNum;                // ֡��

	int  m_lRiffSize;
	int  m_lJpgSize;
	unsigned int m_dwRiffSize;
	unsigned int m_dwJpgSize;
	long  m_TotalSize;                 // ��Ƶ�ĵ�ǰ����
	unsigned int* m_idxSize;
	unsigned int* m_idxOffset;
	//lyr 12/05/18
	static bool CreateMultiLevelDirectory(const char *strFilePathName);
	//lyr 12/05/22
	char Fname[2][256]; //[0]�ļ�����ǰ׺ [1]�ļ����ƺ�׺
	int FnameTimeFormatType; //�ļ������м��ʱ���ʽ���� 0��-��ʱ�� ����-��ʱ��
    NetCamera_Video_SetFileName_callback m_setVideonameCallBack;
    struct timeval m_tmFirst ;
    struct timeval m_tmEnd ;
public:
	/**
	 * @brief ����avi�ļ�����ǰ׺���ߺ�׺
	 * @param p     ָ��Ҫ���õ��ַ���
	 * @param type 0��ʾǰ׺ 1��ʾ��׺
	 * @param timeFormatType	�ļ������м��ʱ���ʽ���� 0��-��ʱ�� ����-��ʱ��
	 * @param len Ҫ�����ļ����ַ������� -1��ʾ�����ַ���
	*/
	void setFname(char* p,int type, int timeFormatType=0, int len=-1);
	
    /**
     * @brief �����ļ������û��Զ��� 
	 * @param Fpath �ļ�·��
	*/
	void setFileName(char* Fpath)
	{
		OpenAvi(Fpath,m_dwWidth,m_dwHeight);
	}

	/**
	 * @brief �رյ�ǰ�ļ� �����ļ�������Ƶ
	*/
	void ColseCurAVI();

	//lyr 12/08/23
	/**
	 * @brief ���avi��Ƶ�ļ������Ϣ
	 * @param w ��Ƶͼ����
	 * @param h ��Ƶͼ��߶�
	 * @param FrameNum ��Ƶͼ��֡��
	 * @param m_fps ֡Ƶ
	 * @return ������Ƶ��ʽ -1���� 
	*/
	int getAVIInfo(const char* FPath, int& w, int& h, unsigned& FrameNum, unsigned& m_fps);

	/**
	 * @brief �����Ƶ֡����
	 * @buff �����Ƶ֡���ݵ�ָ��
	 * @return  ����֡���ݳ��� -1��ʾ���ݲ�����
	*/
    unsigned getFrameData(unsigned char* buff);
};
#endif

