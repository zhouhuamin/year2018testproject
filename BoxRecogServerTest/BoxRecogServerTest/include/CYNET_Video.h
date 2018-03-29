#ifndef _cysys_cynet_video_
#define _cysys_cynet_video_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//lyr 12/08/06
#include <string>
#include <map>
#include "NetCamera_Video.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
#include <Windows.h>
#pragma comment(lib,"NetCamera_Video.lib")
#endif

namespace cy_sys
{
	namespace cynet_video
	{
		class __declspec(dllexport) app
		{
			std::map<std::string, NetCamera_Video*>* M_Video; //存放相机对应的视频列表
		public:
			app()
			{
				M_Video=new std::map<std::string, NetCamera_Video*>;
			}
			~app()
			{
				if (!M_Video->empty())
				{
					for (std::map<std::string, NetCamera_Video*>::iterator it=M_Video->begin(); it != M_Video->end(); ++it)
					{
						NetCamera_Video* pVideo=it->second;
						delete pVideo;
					}

				}
				delete M_Video;
			}

			NetCamera_Video* getVideoByIp(const char* m_ip)
			{
				std::map<std::string, NetCamera_Video*>::iterator it=M_Video->find(m_ip);
				if (it != M_Video->end())
				{
					return it->second;
				}
				return NULL;
			}

			BOOL AddVideoByIp(const char* m_ip)
			{
				std::map<std::string, NetCamera_Video*>::iterator it=M_Video->find(m_ip);
				if (it == M_Video->end())
				{
					(*M_Video)[m_ip]=new NetCamera_Video;
					return TRUE;
				}
				return FALSE;
			}

			BOOL DelVideoByIp(const char* m_ip)
			{
				std::map<std::string, NetCamera_Video*>::iterator it=M_Video->find(m_ip);
				if (it == M_Video->end())
				{
					return FALSE;
				}
				delete it->second;
				M_Video->erase(it);
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

/* SDK API */
extern "C" 
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	* @brief 添加Video
	* @param ip:NULL-192.168.1.10
	* @return 是否添加成功
	*/
	__declspec(dllexport) BOOL CYNET_AddVideoByIp(const char* ip=NULL);

	/*
	* @brief 删除Video
	* @param ip:NULL-192.168.1.10
	* @return 是否删除成功
	*/
	__declspec(dllexport) BOOL CYNET_DelVideolByIp(const char* ip=NULL);
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//Video
	/*
	* @brief 判断Video是否存在
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_IsVideoExist(const char* ip=NULL);

	/*
	* @brief 调用Video类函数AppendDataToVideo 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_AppendDataToVideo(char *buf, int nSize, int frmType,const char* ip=NULL);

	/*
	* @brief 调用Video类函数CloseAvi 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_CloseAvi(const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetMaxRecordTime 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetMaxRecordTime(int nMaxTime,const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetMaxRecordSize 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetMaxRecordSize(int nMaxSize,const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetVideoFormat 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetVideoFormat(int videoFormat,const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetAudioFormat 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetAudioFormat(int audioFormat,const char* ip=NULL);


	/*
	* @brief 调用Video类函数GetFrameNum 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_GetFrameNum(const char* ip=NULL);

	/*
	* @brief 调用Video类函数GetVideoFormat 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_GetVideoFormat(const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetFileDir 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetFileDir(char* szFilePath,const char* ip=NULL);

	/*
	* @brief 调用Video类函数AutoSetFilePath 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) bool CYNET_Video_AutoSetFilePath(const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetWidth 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetWidth(int nWidth,const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetHeight 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetHeight(int nHeight,const char* ip=NULL);

	/*
	* @brief 调用Video类函数SetFps 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetFps(int nFps,const char* ip=NULL);

	/*
	* @brief 调用Video类函数setFname 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) void CYNET_Video_setFname(char* p,int type, int timeFormatType=0, int len=-1,const char* ip=NULL);

	/**
	 * @brief 获得avi视频文件相关信息
	 * @param w 视频图像宽度
	 * @param h 视频图像高度
	 * @param FrameNum 视频图像帧数
	 * @param m_fps 帧频
	 * @return 返回视频格式 -1错误 
	*/
	__declspec(dllexport) int CYNET_Video_getAVIInfo(const char* FPath, int& w, int& h, unsigned& FrameNum, unsigned& m_fps,const char* ip=NULL);

	/**
	 * @brief 获得视频帧数据
	 * @buff 存放视频帧数据的指针
	 * @return  返回帧数据长度 -1表示数据不正常
	*/
    __declspec(dllexport) unsigned CYNET_Video_getFrameData(unsigned char* buff,const char* ip=NULL);

	/*
	* @brief 调用Video类函数setFilename 参考文档2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) void CYNET_Video_setFilename(char* pPath,const char* ip=NULL);

	/**
	* @brief 调用Video类函数AddFrame 参考文档2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_AddFrame(char *buf, unsigned long dwSize, int frmType,const char* ip=NULL);

	/**
	* @brief 调用Video类函数OpenAvi 参考文档2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_OpenAvi(char* szFileName, unsigned long dwWidth, unsigned long dwHeight,const char* ip=NULL);

	/**
	* @brief 调用Video类函数IsFileFull 参考文档2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) bool CYNET_Video_IsFileFull(const char* ip=NULL);

	/**
	* @brief 调用Video类函数ResetAllState 参考文档2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) void CYNET_Video_ResetAllState(const char* ip=NULL);
}
#endif

