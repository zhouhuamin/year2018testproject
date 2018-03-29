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
	* @brief  bayer��ʽͼ��ת��ΪRGB��ɫͼ��
	* @param  pRGB     ת������RGBͼ����ڴ�ָ��
	* @param  pBayer   bayer��ʽͼ���ڴ�ָ��
	* @param  nWidht   ͼ��Ŀ�(����)
	* @param  nHeihgt  ͼ��ĸ�(����)
	* @param  nBits    ͼ���λ��
	* @param  nColor   ͼ��Ҫ����ʾ�ĸ�ʽ
	* @return �������б�
	*/
    __declspec(dllexport) int CYNET_BayerToRGBw(unsigned short* pRGB, unsigned short* pBayer, int nWidth=0, int nHeight=0, int nBits=8, int nColor=0);
	/*!
	* @brief  bayer��ʽͼ��ת��ΪRGB��ɫͼ��
	* @param  pRGB     ת������RGBͼ����ڴ�ָ��
	* @param  pBayer   bayer��ʽͼ���ڴ�ָ��
	* @param  nWidht   ͼ��Ŀ�(����)
	* @param  nHeihgt  ͼ��ĸ�(����)
	* @param  nColor   ͼ��Ҫ����ʾ�ĸ�ʽ
	* @return �������б�
	*/
	__declspec(dllexport) int CYNET_BayerToRGB(unsigned char* pRGB, unsigned char* pBayer, int nWidth=0, int nHeight=0, int nColor=0);


    	/*!
	* @brief  ��ʼ��������
	* @param  nImageWidth  ͼ���(����)
	* @param  nImageHeight ͼ���(����)
	* @param  codecID ����������ID
	* @return �������б�
	*/
	__declspec(dllexport) int CYNET_InitCodec(int nImageWidth=0, int nImageHeight=0, unsigned codecID=28/*AV_CODEC_ID_H264*/,HANDLE * hCodec = 0);
	/*!
	* @brief  ��ʼ��������
	* @param  nImageWidth  ͼ���(����)
	* @param  nImageHeight ͼ���(����)
	* @param  nDesWidth    Ŀ��  ͼ���(����)
	* @param  nDesHeigth   Ŀ��  ͼ���(����)
	* @param  codecID ����������ID
	* @return �������б�
	*/
	__declspec(dllexport) int CYNET_InitCodecA(int nImageWidth=0, int nImageHeight=0,int nDesWidth = 0,int nDesHeigth =0,unsigned codecID = 28/*AV_CODEC_ID_H264*/,HANDLE * hCodec=0);
    __declspec(dllexport) void CYNET_setOutFmt(int fmt,HANDLE hCodec=0);

	/**
	* @brief  �ж��Ƿ���H264��SPS��Ϣ
	* @param  pData ��Ƶ������������
	* @return �����H264��sps��Ϣ���򷵻�true�����򷵻�false
	*/
	__declspec(dllexport) bool CYNET_IsSpsInfo(unsigned char* pData,HANDLE hCodec=0);

	/**
	 * @brief �ж��Ƿ���H264��pps��Ϣ
	 * @param  pData ��Ƶ������������
	 * @return �����H264��pps��Ϣ���򷵻�1�����򷵻�-1
	*/
	__declspec(dllexport) int CYNET_IsPpsInfo(unsigned char* pData,HANDLE hCodec=0);

	__declspec(dllexport) int CYNET_Decode(unsigned char* pRGB, unsigned char* pScr, int nSize=0,HANDLE hCodec=0);
	/*!
	* @brief  ȡ��ͼ��֡�Ŀ�(����)
	* @param  void
	* @return ����ͼ��֡�Ŀ�(����)
	*/
	__declspec(dllexport) int CYNET_GetWidth(HANDLE hCodec=0);

	/*!
	* @brief  ȡ��ͼ��֡�ĸ�(����)
	* @param  void
	* @return ����ͼ��֡�ĸ�(����)
	*/
	__declspec(dllexport) int CYNET_GetHeight(HANDLE hCodec=0);
	
	/*!	
	* @brief	�����Դ,���˳�����(��Ҫ���³�ʼ��֮ǰ)ʱ�ú����ͷ���Դ
	* @param	void
	* @return	����0 ������
	*/
	__declspec(dllexport) int CYNET_UnInit(HANDLE hCodec=0);
	__declspec(dllexport) int CYNET_DecodeJpeg(unsigned char* pRGB, unsigned char* pSrc, int w,int h, int sz,HANDLE hCodec=0);
    __declspec(dllexport) int CYNET_DecodeData(unsigned char* pRGB, unsigned char* pScr, int nSize,HANDLE hCodec=0);
};