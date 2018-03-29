#pragma once
#include "windows.h"
#include "NetCamera_Codec.h"
extern "C"
{
    __declspec(dllexport) int CYNET_RGBToYUV422w(unsigned short* pRGB, unsigned short* pYUV422, int nWidth, int nHeight);
	__declspec(dllexport) int CYNET_RGBToYUV422(unsigned char* pRGB, unsigned char* pYUV422, int nWidth=0, int nHeight=0);
	__declspec(dllexport) int CYNET_YUV422ToRGB(unsigned char* pRGB, unsigned char* pYUV422, int nWidth=0, int nHeight=0);
	__declspec(dllexport) int CYNET_YUV422ToRGBw(unsigned short* pRGB, unsigned short* pYUV422, int nWidth, int nHeight, int nBits);
	__declspec(dllexport) unsigned short CYNET_GetYUVB(int nY,int nU,int nV);
	__declspec(dllexport) unsigned short CYNET_GetYUVG(int nY,int nU,int nV);
	__declspec(dllexport) unsigned short CYNET_GetYUVR(int nY,int nU,int nV);
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
    __declspec(dllexport) int CYNET_BayerToRGBw(unsigned short* pRGB, unsigned short* pBayer, int nWidth=0, int nHeight=0, int nBits=8, int nColor=0);
	/*!
	* @brief  bayer格式图像转换为RGB彩色图像
	* @param  pRGB     转换后存放RGB图像的内存指针
	* @param  pBayer   bayer格式图像内存指针
	* @param  nWidht   图像的宽(像素)
	* @param  nHeihgt  图像的高(像素)
	* @param  nColor   图像要求显示的格式
	* @return 见错误列表
	*/
	__declspec(dllexport) int CYNET_BayerToRGB(unsigned char* pRGB, unsigned char* pBayer, int nWidth=0, int nHeight=0, int nColor=0);


    	/*!
	* @brief  初始化解码器
	* @param  nImageWidth  图像宽(像素)
	* @param  nImageHeight 图像高(像素)
	* @param  codecID 解码器类型ID
	* @return 见错误列表
	*/
	__declspec(dllexport) int CYNET_InitCodec(int nImageWidth=0, int nImageHeight=0, unsigned codecID=28/*AV_CODEC_ID_H264*/,HANDLE * hCodec = 0);
	/*!
	* @brief  初始化解码器
	* @param  nImageWidth  图像宽(像素)
	* @param  nImageHeight 图像高(像素)
	* @param  nDesWidth    目标  图像宽(像素)
	* @param  nDesHeigth   目标  图像高(像素)
	* @param  codecID 解码器类型ID
	* @return 见错误列表
	*/
	__declspec(dllexport) int CYNET_InitCodecA(int nImageWidth=0, int nImageHeight=0,int nDesWidth = 0,int nDesHeigth =0,unsigned codecID = 28/*AV_CODEC_ID_H264*/,HANDLE * hCodec=0);
    __declspec(dllexport) void CYNET_setOutFmt(int fmt,HANDLE hCodec=0);

	/**
	* @brief  判断是否是H264的SPS信息
	* @param  pData 视频数据流缓冲区
	* @return 如果是H264的sps信息，则返回true，否则返回false
	*/
	__declspec(dllexport) bool CYNET_IsSpsInfo(unsigned char* pData,HANDLE hCodec=0);

	/**
	 * @brief 判断是否是H264的pps信息
	 * @param  pData 视频数据流缓冲区
	 * @return 如果是H264的pps信息，则返回1，否则返回-1
	*/
	__declspec(dllexport) int CYNET_IsPpsInfo(unsigned char* pData,HANDLE hCodec=0);

	__declspec(dllexport) int CYNET_Decode(unsigned char* pRGB, unsigned char* pScr, int nSize=0,HANDLE hCodec=0);
	/*!
	* @brief  取得图像帧的宽(像素)
	* @param  void
	* @return 返回图像帧的宽(像素)
	*/
	__declspec(dllexport) int CYNET_GetWidth(HANDLE hCodec=0);

	/*!
	* @brief  取得图像帧的高(像素)
	* @param  void
	* @return 返回图像帧的高(像素)
	*/
	__declspec(dllexport) int CYNET_GetHeight(HANDLE hCodec=0);
	
	/*!	
	* @brief	清空资源,在退出该类(或要重新初始化之前)时该函数释放资源
	* @param	void
	* @return	返回0 无意义
	*/
	__declspec(dllexport) int CYNET_UnInit(HANDLE hCodec=0);
	__declspec(dllexport) int CYNET_DecodeJpeg(unsigned char* pRGB, unsigned char* pSrc, int w,int h, int sz,HANDLE hCodec=0);
    __declspec(dllexport) int CYNET_DecodeData(unsigned char* pRGB, unsigned char* pScr, int nSize,HANDLE hCodec=0);
};