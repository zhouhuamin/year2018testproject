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
	* @brief  bayer��ʽͼ��ת��ΪRGB��ɫͼ��
	* @param  pRGB     ת������RGBͼ����ڴ�ָ��
	* @param  pBayer   bayer��ʽͼ���ڴ�ָ��
	* @param  nWidht   ͼ��Ŀ�(����)
	* @param  nHeihgt  ͼ��ĸ�(����)
	* @param  nBits    ͼ���λ��
	* @param  nColor   ͼ��Ҫ����ʾ�ĸ�ʽ
	* @return �������б�
	*/
	static int BayerToRGBw(unsigned short* pRGB=NULL, unsigned short* pBayer=NULL, int nWidth=0, int nHeight=0, int nBits=8, int nColor=0);
	/*!
	* @brief  bayer��ʽͼ��ת��ΪRGB��ɫͼ��
	* @param  pRGB     ת������RGBͼ����ڴ�ָ��
	* @param  pBayer   bayer��ʽͼ���ڴ�ָ��
	* @param  nWidht   ͼ��Ŀ�(����)
	* @param  nHeihgt  ͼ��ĸ�(����)
	* @param  nColor   ͼ��Ҫ����ʾ�ĸ�ʽ
	* @return �������б�
	*/
	static int BayerToRGB(unsigned char* pRGB=NULL, unsigned char* pBayer=NULL, int nWidth=0, int nHeight=0, int nColor=0);
};

class NC_CODEC_EXT NetCamera_Codec  
{
	void* avpkt;/*AVPacket*/
	int m_outfmt; //0-���bgr 1-bgra 2-rgb
public:
	NetCamera_Codec();
	virtual ~NetCamera_Codec();
public:
	int m_iBytesCnt;
	/**
	 * @brief ���ý���������ݸ�ʽ
	 * @param fmt 0-���bgr 1-bgra 2-rgb
	*/
	void setOutFmt(int fmt){m_outfmt=fmt;}

	/**
	* @brief  �ж��Ƿ���H264��SPS��Ϣ
	* @param  pData ��Ƶ������������
	* @return �����H264��sps��Ϣ���򷵻�true�����򷵻�false
	*/
	static bool IsSpsInfo(unsigned char* pData);

	/**
	 * @brief �ж��Ƿ���H264��pps��Ϣ
	 * @param  pData ��Ƶ������������
	 * @return �����H264��pps��Ϣ���򷵻�1�����򷵻�-1
	*/
	static int IsPpsInfo(unsigned char* pData);

	int Decode(unsigned char* pRGB=NULL, unsigned char* pScr=NULL, int nSize=0);
	/*!
	* @brief  ��ʼ��������
	* @param  nImageWidth  ͼ���(����)
	* @param  nImageHeight ͼ���(����)
	* @param  codecID ����������ID
	* @return �������б�
	*/
	int InitCodec(int nImageWidth=0, int nImageHeight=0, unsigned codecID=28/*AV_CODEC_ID_H264*/);
	/*!
	* @brief  ��ʼ��������
	* @param  nImageWidth  ͼ���(����)
	* @param  nImageHeight ͼ���(����)
	* @param  nDesWidth    Ŀ��  ͼ���(����)
	* @param  nDesHeigth   Ŀ��  ͼ���(����)
	* @param  codecID ����������ID
	* @return �������б�
	*/
	int InitCodecA(int nImageWidth=0, int nImageHeight=0,int nDesWidth = 0,int nDesHeigth =0,unsigned codecID = 28/*AV_CODEC_ID_H264*/);

	

	/*!
	* @brief  ȡ��ͼ��֡�Ŀ�(����)
	* @param  void
	* @return ����ͼ��֡�Ŀ�(����)
	*/
	int GetWidth();

	/*!
	* @brief  ȡ��ͼ��֡�ĸ�(����)
	* @param  void
	* @return ����ͼ��֡�ĸ�(����)
	*/
	int GetHeight();

	
	/*!	
	* @brief	�����Դ,���˳�����(��Ҫ���³�ʼ��֮ǰ)ʱ�ú����ͷ���Դ
	* @param	void
	* @return	����0 ������
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
// ˽�к���
private:
	

	/*
	* @brief  ����H264ͼ������
	* @param  pRGB    ת����Ĵ��RGBͼ����ڴ�ָ��
	* @param  pScr    �����Ƶ����Դ��ָ��
	* @param  nSize   ԭʼRGB���ݵĳ���
	* @return �������б�
	*/
	int DecodeH264(unsigned char* pRGB=NULL, unsigned char* pScr=NULL, int nSize=0);

	/*
	* @brief  ��H264��SPS���н���
	* @param  pSrc ָ����Ƶ�����ݻ�����ָ��
	* @param  nSize ԭʼRGB���ݵĳ���
	* @return �������б�
	*/
	int DecodeSPS(unsigned char* pSrc=NULL, int nSize=0);

	

	/*!
	* @brief  �����������ҳ�һ֡�ĳ�
	* @param  pData ������������ָ��
	* @param  nLength �����buf����
	* @return �������б�
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
