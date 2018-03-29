/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//lyr 12/08/20
#ifndef _cysys_cynet_stream_
#define _cysys_cynet_stream_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <map>
#include "NetCamera_Stream2.h"
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#ifndef _WIN32
#ifndef _cynet_linux_def_
#define _cynet_linux_def_
#define __declspec(dllexport)
//#define NULL 0 
#endif
#else
#pragma comment(lib,"NetCamera_Stream2.lib")
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace cy_sys
{
	namespace cynet_stream
	{
		using namespace cy_stream;
		class __declspec(dllexport) NetCameraStreamMainCtl
		{
			NetCamera_Stream2 *	Rtsp;
		public:
			NetCameraStreamMainCtl()
			{
				Rtsp=new NetCamera_Stream2;
			}
			~NetCameraStreamMainCtl()
			{
				delete Rtsp;
			}
			NetCamera_Stream2* getRtsp(void){return Rtsp;}
		};

		class __declspec(dllexport) app
		{
			std::map<std::string, NetCameraStreamMainCtl*>* M_StreamMainCtl; //�����ý����Ҫ�õ��ĸ����������
		public:
			app()
			{
				M_StreamMainCtl=new std::map<std::string, NetCameraStreamMainCtl*>;
			}

			~app()
			{
				if (!M_StreamMainCtl->empty())
				{
					for (std::map<std::string, NetCameraStreamMainCtl*>::iterator it=M_StreamMainCtl->begin(); it != M_StreamMainCtl->end(); ++it)
					{
						NetCameraStreamMainCtl* pSMainCtl=it->second;
						delete pSMainCtl;
					}		
				}
				delete M_StreamMainCtl;
			}

			NetCameraStreamMainCtl* getStreamMainCtlByIp(const char* m_ip)
			{
				std::map<std::string, NetCameraStreamMainCtl*>::iterator it=M_StreamMainCtl->find(m_ip);
				if(it == M_StreamMainCtl->end())
				{
					return NULL;
				}
				return it->second;
			}

			BOOL AddStreamMainCtlByIp(const char* m_ip)
			{
				std::map<std::string, NetCameraStreamMainCtl*>::iterator it=M_StreamMainCtl->find(m_ip);
				if(it != M_StreamMainCtl->end())
				{
					return FALSE;
				}
				(*M_StreamMainCtl)[m_ip]=new NetCameraStreamMainCtl;
				return TRUE;
			}

			BOOL DelStreamMainCtlByIp(const char* m_ip)
			{
				std::map<std::string, NetCameraStreamMainCtl*>::iterator it=M_StreamMainCtl->find(m_ip);
				if (it != M_StreamMainCtl->end())
				{
					delete it->second;
					M_StreamMainCtl->erase(it);
					return TRUE;
				}
				return FALSE;
			}

			static app* getApp(void)
			{
				static app _app;
				return &_app;
			}	
		};
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	/*
	* @brief ���StreamMainCtl
	* @param ip:NULL-192.168.1.10
	* @return �Ƿ���ӳɹ�
	*/
	__declspec(dllexport) BOOL CYNET_AddStreamMainCtlByIp(const char* ip=NULL);


	/*
	* @brief ɾ��StreamMainCtl
	* @param ip:NULL-192.168.1.10
	* @return �Ƿ�ɾ���ɹ�
	*/
	__declspec(dllexport) BOOL CYNET_DelStreamMainCtlByIp(const char* ip=NULL);

	/*
	* @brief �ж�treamMainCtl�Ƿ����
	* @param ip:NULL-192.168.1.10
	* @return �Ƿ����
	*/
	__declspec(dllexport) BOOL CYNET_IsStreamMainCtlExist(const char* ip=NULL);

	//NetCameraRTP
/*
* @brief ����RTP��SetCallBack���� �ο��ĵ�2.2.5
* @param ip:NULL-192.168.1.10
*/
void __declspec(dllexport) CYNET_RTP_SetCallBack(Callback_RecvData pFun,void* pUser1,void* pUser2,const char* ip=NULL);

/**
* @brief ����RTP�ຯ��GetMode �ο��ĵ�2.2.6
* @param ip:NULL-192.168.1.10
*/
__declspec(dllexport) unsigned CYNET_RTP_getH264Data(unsigned char* buf,const char* ip=NULL);

/*
* @brief ����RTSP�ຯ��Close �ο��ĵ�2.2.5
* @param ip:NULL-192.168.1.10
*/
__declspec(dllexport) int CYNET_RTSP_Close(const char* ip=NULL);


/*
* @brief ����RTSP�ຯ��Open �ο��ĵ�2.2.5
* @param ip:NULL-192.168.1.10
*/
__declspec(dllexport) int CYNET_RTSP_Open(unsigned short transport = 9000,const char* ip=NULL,int t=0,int m_autoconn=1);

/**
* @brief ��ȡh264������(PS��ES)
* @param ip
* @return ����������(H264_TYPE_PS ΪPS����H264_TYPE_ESΪES��)
*/
/*__declspec(dllexport) int getH264Type(const char* ip);*/

__declspec(dllexport) unsigned char* getSPS(unsigned int* len, const char* m_ip = NULL);

__declspec(dllexport) unsigned char* getPPS(unsigned int* len, const char* m_ip = NULL);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
};



