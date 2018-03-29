///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//lyr 12/08/20
#ifndef _cysys_cynet_cam_
#define _cysys_cynet_cam_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <map>
#include "NetCamera.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _WIN32
#ifndef _cynet_linux_def_
#define _cynet_linux_def_
#define __declspec(dllexport)
typedef int BOOL;
#define TRUE 1
#define FALSE 0
//#define NULL 0 
#endif
#else
#ifdef _DEBUG
#pragma comment(lib,"NetCamera.lib")
#else
#pragma comment(lib,"NetCamera.lib")
#endif
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
namespace cy_sys
{
	namespace cynet_cam
	{
		class __declspec(dllexport) app
		{
			std::map<std::string, NetCamera*>* M_Cam; //存放相机对应的视频列表
			std::map<std::string, cy_netcamera::NetCom*>* M_NetCom; //存放NetCon对应的map
			cy_netcamera::SearchCam SC;
		public:
			app()
			{
				M_Cam=new std::map<std::string, NetCamera*>;
				M_NetCom=new std::map<std::string, cy_netcamera::NetCom*>;
			}
			~app()
			{
				if (!M_Cam->empty())
				{
					for (std::map<std::string, NetCamera*>::iterator it=M_Cam->begin(); it != M_Cam->end(); ++it)
					{
						NetCamera* pCam=it->second;
						delete pCam;
					}

				}
				delete M_Cam;

				if (!M_NetCom->empty())
				{
					for (std::map<std::string, cy_netcamera::NetCom*>::iterator it=M_NetCom->begin(); it != M_NetCom->end(); ++it)
					{
						cy_netcamera::NetCom* pCom=it->second;
						delete pCom;
					}
				}
				delete M_NetCom;
			}

			cy_netcamera::SearchCam* getSearchCam(void)
			{
				return &SC;
			}

			cy_netcamera::NetCom* getNetComByIp(const char* m_ip)
			{
				std::map<std::string, cy_netcamera::NetCom*>::iterator it=M_NetCom->find(m_ip);
				if (it != M_NetCom->end())
				{
					return it->second;
				}
				return NULL;
			}

			NetCamera* getCamByIp(const char* m_ip)
			{
				std::map<std::string, NetCamera*>::iterator it=M_Cam->find(m_ip);
				if (it != M_Cam->end())
				{
					return it->second;
				}
				return NULL;
			}

			BOOL AddCamByIp(const char* m_ip)
			{
				std::map<std::string, NetCamera*>::iterator it=M_Cam->find(m_ip);
				if (it == M_Cam->end())
				{
					(*M_Cam)[m_ip]=new NetCamera;
					return TRUE;
				}
				return FALSE;
			}

			BOOL AddNetComByIp(const char* m_ip)
			{
				std::map<std::string, cy_netcamera::NetCom*>::iterator it=M_NetCom->find(m_ip);
				if (it == M_NetCom->end())
				{
					(*M_NetCom)[m_ip]=new cy_netcamera::NetCom;
					return TRUE;
				}
				return FALSE;
			}

			BOOL DelCamByIp(const char* m_ip)
			{
				std::map<std::string, NetCamera*>::iterator it=M_Cam->find(m_ip);
				if (it == M_Cam->end())
				{
					return FALSE;
				}
				delete it->second;
				M_Cam->erase(it);
				return TRUE;
			}

			BOOL DelNetComByIp(const char* m_ip)
			{
				std::map<std::string,cy_netcamera::NetCom*>::iterator it=M_NetCom->find(m_ip);
				if (it == M_NetCom->end())
				{
					return FALSE;
				}
				delete it->second;
				M_NetCom->erase(it);
				return TRUE;
			}

			static app* getApp(void)
			{
				static app _app;
				return &_app;
			}
		};
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C"
{//透明串口
	/**
	* @brief 添加Cam
	* @param ip:NULL-192.168.1.10
	* @return 是否添加成功
	*/
	__declspec(dllexport) BOOL CYNET_AddCamByIp(const char* ip=NULL);

	/**
	* @brief 删除Cam
	* @param ip:NULL-192.168.1.10
	* @return 是否删除成功
	*/
	__declspec(dllexport) BOOL CYNET_DelCamByIp(const char* ip=NULL);

	/*
	* @brief 添加Com
	* @param ip:NULL-192.168.1.10
	* @return 是否添加成功
	*/
	__declspec(dllexport) BOOL CYNET_AddComByIp(const char* ip=NULL);

	/*
	* @brief 删除Com
	* @param ip:NULL-192.168.1.10
	* @return 是否删除成功
	*/
	__declspec(dllexport) BOOL CYNET_DelComByIp(const char* ip=NULL);

	/**
	* @brief 调用NetCom类函数NetCom_Open 参考文档2.2.5
	*/
	__declspec(dllexport) int CYNET_NetCom_Open(char *target_ip,unsigned short snd_port,unsigned short rcv_port);

	/**
	* @brief 调用NetCom类函数NetCom_Send 参考文档2.2.5
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCom_Send(char *msg,int len,int chl,const char* ip=NULL);

	/**
	* @brief 调用NetCom类函数NetCom_Recv 参考文档2.2.5
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCom_Recv(char *msg,int len,int *chl,int timeout,const char* ip=NULL);

	//NetCamera
	/*
	* @brief 调用NetCamera类函数ReadBlockData 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_ReadBlockData(int nCmd, int* nDataLength, unsigned char *pData,const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数UpdateBlockData 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_UpdateBlockData(int nCmd,int nDataLength,unsigned char* pData,const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数Reset 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_Reset(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数SetIPAddress 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_SetIPAddress(const char* IP=NULL,unsigned short portc=8886,unsigned short ports=8888);

	/*
	* @brief 调用NetCamera类函数IdentifyCameraType 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_IdentifyCameraType(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数GetCameraType 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_GetCameraType(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数ReadParam 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_ReadParam(int nCmd,int nState = 0,const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数GetFrameUnit 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) double CYNET_NetCamera_GetFrameUnit(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数GetGainUnit 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) double CYNET_NetCamera_GetGainUnit(int* nB,const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数GetTimeUnit 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) double CYNET_NetCamera_GetTimeUnit(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数UpdateParam 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_UpdateParam(unsigned int nCmd,unsigned int param1 = 0,unsigned int param2 = 0,unsigned char* buff=NULL,const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数GetImage 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_GetImage(unsigned char* pBuf,const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数ClearFrameQueue 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_ClearFrameQueue(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数ImageBayerToRGB 参考文档2.2.3
	*/
	__declspec(dllexport) int CYNET_NetCamera_ImageBayerToRGB(unsigned char* pRGB, unsigned char* pBayer, int image_width, int image_height);

	/*
	* @brief 调用NetCamera类函数ImageBayerToRGB_HiBit 参考文档2.2.3
	*/
	__declspec(dllexport) int CYNET_NetCamera_ImageBayerToRGB_HiBit(unsigned short* pRGB, unsigned short* pBayer, int image_width, int image_height, int bits = 12);

	/*
	* @brief 调用NetCamera类函数ImageBayerToRGB_HQI 参考文档2.2.3
	*/
	__declspec(dllexport) int CYNET_NetCamera_ImageBayerToRGB_HQI(unsigned char* pRGB, unsigned char* pBayer, int image_width, int image_height);

	/**
	* @brief 调用NetCamera类函数ImageBayerToRGB_HQI_HiBit 参考文档2.2.3
	*/
	__declspec(dllexport) int CYNET_NetCamera_ImageBayerToRGB_HQI_HiBit(unsigned short* pRGB, unsigned short* pBayer, int image_width, int image_height, int bits = 12);

#ifdef _WIN32
	/**
	* @brief 调用NetCamera类函数JpegDecode 参考文档2.2.3
	*/
	__declspec(dllexport) int CYNET_NetCamera_JpegDecode(unsigned char* pJpeg, int jpeg_size, unsigned char* pRGB, int* rgb_size, int* image_width, int* image_height);

	/**
	* @brief 调用NetCamera类函数SaveRgbToJpgFile 参考文档2.2.6
	*/
	__declspec(dllexport) bool CYNET_NetCamera_SaveRgbToJpgFile(char* filepath,BYTE* pRGB,int width,int height,int jpegQ = 85);
#endif

	/**
	* @brief 调用NetCamera类函数usm_process 参考文档2.2.6
	*/
	__declspec(dllexport) void CYNET_NetCamera_usm_process (unsigned char *srcPR, unsigned char *destPR, int w, int h,
		double radius, double amount, int threshold);

	/**
	* @brief 调用NetCamera类函数contrast_enhance_process 参考文档2.2.6
	*/
	__declspec(dllexport) void CYNET_NetCamera_contrast_enhance_process (unsigned char *buf, int w, int h, double threshold);

	/*
	* @brief 调用NetCamera类函数getCameraTransState 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_getCameraTransState(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数getCameraCtrlState 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_getCameraCtrlState(const char* ip=NULL);

	/*
	* @brief 调用NetCamera类函数ConnectCamera 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_NetCamera_ConnectCamera(int op1=1, int op2=1,const char* ip=NULL); 

	/**
	 * @brief 搜索相机
	*/
	__declspec(dllexport) void CYNET_NetCamera_SearchCam();

	/**
	 * @brief 获得搜索到的相机数量
	*/
	__declspec(dllexport) int CYNET_NetCamera_getCamNum();

	/**
	 * @brief 获得指定索引的相机ip
	*/
	__declspec(dllexport) const char* CYNET_NetCamera_getCamIPBySeq(int n);
};
#endif


