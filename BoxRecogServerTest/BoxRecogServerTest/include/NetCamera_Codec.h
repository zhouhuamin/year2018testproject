//2012-11-28
//V2.2.6
#ifndef _NET_CAMERA_CODEC_H_
#define _NET_CAMERA_CODEC_H_

#ifndef _WIN32
#ifndef _CRI_SEC_
#define _CRI_SEC_
#include <pthread.h>
#define CRITICAL_SECTION pthread_mutex_t
#define InitializeCriticalSection(x) pthread_mutex_init(x,NULL)
#define DeleteCriticalSection(x) pthread_mutex_destroy(x)
#define EnterCriticalSection(x) pthread_mutex_lock(x)
#define LeaveCriticalSection(x) pthread_mutex_unlock(x)
#endif
#ifndef _CY_Type_
#define _CY_Type_
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#endif
#define NC_CODEC_EXT
#else
#ifdef _CY_CAMERA_CODEC_CLS
	#define NC_CODEC_EXT _declspec(dllexport)
#else
	#define NC_CODEC_EXT _declspec(dllimport)
#endif
#endif

class NC_CODEC_EXT NetCamera_Image
{
public:
//	static int JpegDecode(unsigned char *lpSrcBuffer, unsigned long dwSrcSize,  unsigned char *lpDstBuffer, unsigned long *dwDstSize,unsigned long *lpdwWidth,  unsigned long *lpdwHeight, unsigned long *lpBits, unsigned long *dwReserve1);
	static int RGBToYUV422w(unsigned short* pRGB, unsigned short* pYUV422, int nWidth, int nHeight);
	static int RGBToYUV422(unsigned char* pRGB=NULL, unsigned char* pYUV422=NULL, int nWidth=0, int nHeight=0);
	static int YUV422ToRGB(unsigned char* pRGB=NULL, unsigned char* pYUV422=NULL, int nWidth=0, int nHeight=0);
	static int YUV422ToRGBw(unsigned short* pRGB, unsigned short* pYUV422, int nWidth, int nHeight, int nBits);
	static unsigned short GetYUVB(int nY,int nU,int nV);
	static unsigned short GetYUVG(int nY,int nU,int nV);
	static unsigned short GetYUVR(int nY,int nU,int nV);

	/*!
	* @brief  bayer格式图像转换为RGB彩色图像
	* @param  pRGB     转换后存放RGB图像的内存指针
	* @param  pBayer   bayer格式图像内存指针
	* @param  nWidht   图像的宽(像素)
	* @param  nHeihgt  图像的高(像素)
	* @param  nBits    图像的位数
	* @param  nColor   图像要求显示的格式
	* @return 见错误列表
	*/
	static int BayerToRGBw(unsigned short* pRGB=NULL, unsigned short* pBayer=NULL, int nWidth=0, int nHeight=0, int nBits=8, int nColor=0);
	/*!
	* @brief  bayer格式图像转换为RGB彩色图像
	* @param  pRGB     转换后存放RGB图像的内存指针
	* @param  pBayer   bayer格式图像内存指针
	* @param  nWidht   图像的宽(像素)
	* @param  nHeihgt  图像的高(像素)
	* @param  nColor   图像要求显示的格式
	* @return 见错误列表
	*/
	static int BayerToRGB(unsigned char* pRGB=NULL, unsigned char* pBayer=NULL, int nWidth=0, int nHeight=0, int nColor=0);
};

class NC_CODEC_EXT NetCamera_Codec  
{
	void* avpkt;/*AVPacket*/
	int m_outfmt; //0-输出bgr 1-bgra 2-rgb
public:
	NetCamera_Codec();
	virtual ~NetCamera_Codec();
public:
	int m_iBytesCnt;
	/**
	 * @brief 设置解码出的数据格式
	 * @param fmt 0-输出bgr 1-bgra 2-rgb
	*/
	void setOutFmt(int fmt){m_outfmt=fmt;}

	/**
	* @brief  判断是否是H264的SPS信息
	* @param  pData 视频数据流缓冲区
	* @return 如果是H264的sps信息，则返回true，否则返回false
	*/
	static bool IsSpsInfo(unsigned char* pData);

	/**
	 * @brief 判断是否是H264的pps信息
	 * @param  pData 视频数据流缓冲区
	 * @return 如果是H264的pps信息，则返回1，否则返回-1
	*/
	static int IsPpsInfo(unsigned char* pData);

	int Decode(unsigned char* pRGB=NULL, unsigned char* pScr=NULL, int nSize=0);
	/*!
	* @brief  初始化解码器
	* @param  nImageWidth  图像宽(像素)
	* @param  nImageHeight 图像高(像素)
	* @param  codecID 解码器类型ID
	* @return 见错误列表
	*/
	int InitCodec(int nImageWidth=0, int nImageHeight=0, unsigned codecID=28/*AV_CODEC_ID_H264*/);
	/*!
	* @brief  初始化解码器
	* @param  nImageWidth  图像宽(像素)
	* @param  nImageHeight 图像高(像素)
	* @param  nDesWidth    目标  图像宽(像素)
	* @param  nDesHeigth   目标  图像高(像素)
	* @param  codecID 解码器类型ID
	* @return 见错误列表
	*/
	int InitCodecA(int nImageWidth=0, int nImageHeight=0,int nDesWidth = 0,int nDesHeigth =0,unsigned codecID = 28/*AV_CODEC_ID_H264*/);

	

	/*!
	* @brief  取得图像帧的宽(像素)
	* @param  void
	* @return 返回图像帧的宽(像素)
	*/
	int GetWidth();

	/*!
	* @brief  取得图像帧的高(像素)
	* @param  void
	* @return 返回图像帧的高(像素)
	*/
	int GetHeight();

	
	/*!	
	* @brief	清空资源,在退出该类(或要重新初始化之前)时该函数释放资源
	* @param	void
	* @return	返回0 无意义
	*/
	int UnInit();

	int DecodeJpeg(unsigned char* pRGB, unsigned char* pSrc, int w,int h, int sz)
	{
		if(!m_bInit)
		{
			if(InitCodec(w,h,8)<0)
			{
				return -1;
			}
		}
		return DecodeData(pRGB,pSrc,sz);
	}
//////////////////////////////////////////////////////////////////////////
// 私有函数
private:
	

	/*
	* @brief  解码H264图像数据
	* @param  pRGB    转换后的存放RGB图像的内存指针
	* @param  pScr    存放视频数据源的指针
	* @param  nSize   原始RGB数据的长度
	* @return 见错误列表
	*/
	int DecodeH264(unsigned char* pRGB=NULL, unsigned char* pScr=NULL, int nSize=0);

	/*
	* @brief  对H264的SPS进行解码
	* @param  pSrc 指向视频流数据缓冲区指针
	* @param  nSize 原始RGB数据的长度
	* @return 见错误列表
	*/
	int DecodeSPS(unsigned char* pSrc=NULL, int nSize=0);

	

	/*!
	* @brief  从数据流中找出一帧的长
	* @param  pData 数据流缓冲区指针
	* @param  nLength 传入的buf长度
	* @return 见错误列表
	*/
public:
	int DecodeData(unsigned char* pRGB, unsigned char* pScr, int nSize);
private:
	/*AVCodec*/void*		m_pCodec;
	/*AVCodecContext*/void*	m_pCodecContx;
	/*AVFrame*/void*		m_pFrameRGB;
	/*SwsContext*/void*		m_pImg_convert_ctx;	
	/*AVFrame*/void*		m_picture;
	unsigned char*	m_buffer;
	int				m_nImageWidth;
	int				m_nImageHeight;
	unsigned		m_nCodecID;
	bool			m_bInit;
};

#endif
