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
			std::map<std::string, NetCamera_Video*>* M_Video; //��������Ӧ����Ƶ�б�
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
	* @brief ���Video
	* @param ip:NULL-192.168.1.10
	* @return �Ƿ���ӳɹ�
	*/
	__declspec(dllexport) BOOL CYNET_AddVideoByIp(const char* ip=NULL);

	/*
	* @brief ɾ��Video
	* @param ip:NULL-192.168.1.10
	* @return �Ƿ�ɾ���ɹ�
	*/
	__declspec(dllexport) BOOL CYNET_DelVideolByIp(const char* ip=NULL);
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//Video
	/*
	* @brief �ж�Video�Ƿ����
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_IsVideoExist(const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��AppendDataToVideo �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_AppendDataToVideo(char *buf, int nSize, int frmType,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��CloseAvi �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_CloseAvi(const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetMaxRecordTime �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetMaxRecordTime(int nMaxTime,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetMaxRecordSize �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetMaxRecordSize(int nMaxSize,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetVideoFormat �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetVideoFormat(int videoFormat,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetAudioFormat �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetAudioFormat(int audioFormat,const char* ip=NULL);


	/*
	* @brief ����Video�ຯ��GetFrameNum �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_GetFrameNum(const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��GetVideoFormat �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_GetVideoFormat(const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetFileDir �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetFileDir(char* szFilePath,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��AutoSetFilePath �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) bool CYNET_Video_AutoSetFilePath(const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetWidth �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetWidth(int nWidth,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetHeight �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetHeight(int nHeight,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��SetFps �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_SetFps(int nFps,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��setFname �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) void CYNET_Video_setFname(char* p,int type, int timeFormatType=0, int len=-1,const char* ip=NULL);

	/**
	 * @brief ���avi��Ƶ�ļ������Ϣ
	 * @param w ��Ƶͼ����
	 * @param h ��Ƶͼ��߶�
	 * @param FrameNum ��Ƶͼ��֡��
	 * @param m_fps ֡Ƶ
	 * @return ������Ƶ��ʽ -1���� 
	*/
	__declspec(dllexport) int CYNET_Video_getAVIInfo(const char* FPath, int& w, int& h, unsigned& FrameNum, unsigned& m_fps,const char* ip=NULL);

	/**
	 * @brief �����Ƶ֡����
	 * @buff �����Ƶ֡���ݵ�ָ��
	 * @return  ����֡���ݳ��� -1��ʾ���ݲ�����
	*/
    __declspec(dllexport) unsigned CYNET_Video_getFrameData(unsigned char* buff,const char* ip=NULL);

	/*
	* @brief ����Video�ຯ��setFilename �ο��ĵ�2.2.3
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) void CYNET_Video_setFilename(char* pPath,const char* ip=NULL);

	/**
	* @brief ����Video�ຯ��AddFrame �ο��ĵ�2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_AddFrame(char *buf, unsigned long dwSize, int frmType,const char* ip=NULL);

	/**
	* @brief ����Video�ຯ��OpenAvi �ο��ĵ�2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) int CYNET_Video_OpenAvi(char* szFileName, unsigned long dwWidth, unsigned long dwHeight,const char* ip=NULL);

	/**
	* @brief ����Video�ຯ��IsFileFull �ο��ĵ�2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) bool CYNET_Video_IsFileFull(const char* ip=NULL);

	/**
	* @brief ����Video�ຯ��ResetAllState �ο��ĵ�2.2.6
	* @param ip:NULL-192.168.1.10
	*/
	__declspec(dllexport) void CYNET_Video_ResetAllState(const char* ip=NULL);
}
#endif

