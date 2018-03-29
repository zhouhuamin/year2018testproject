#if !defined(_EXIF_C_)
#define _EXIF_C_
typedef void * HANDLE;
#include "exif_data.h"
extern "C"
{
    /**
	* @brief ����exif��ȡ���
	* @param ��
	* @return �Ƿ���ӳɹ� ��0�ɹ� С��0ʧ��
	*/
    __declspec(dllexport) HANDLE CYNET_CreateExifHandle();
    /**
	* @brief �ͷž��
	* @param ��
	* @return �Ƿ���ӳɹ� 0�ɹ� С��0ʧ��
	*/
    __declspec(dllexport) int CYNET_ReleaseExifHandle(HANDLE hExif);
	/**
	* @brief �������н���exif��Ϣ
	* @param ImageData:Դ����
	* @param Imagesize:���ݳ���
	* @return �Ƿ���ӳɹ� 0�ɹ� С��0ʧ��
	*/
	__declspec(dllexport) int CYNET_GetExifFromData(HANDLE hExif,char *ImageData,unsigned int Imagesize);
		/**
	* @brief ���ļ��н���exif��Ϣ
	* @param filename:�ļ�·��(�����ļ���)
	* @return �Ƿ���ӳɹ� 0�ɹ� С��0ʧ��
	*/
	__declspec(dllexport) int CYNET_GetExifFromFile(HANDLE hExif,char * filename);
		/**
	* @brief ����exif�е�֡ͷ��Ϣ
	* @param NULL
	* @return �Ƿ���ӳɹ�
	*/
	__declspec(dllexport) FRAME__HEADER * CYNET_GetExifFrameHead(HANDLE hExif);
		/**
	* @brief ����exif�еĳ�����Ϣ
	* @param NULL
	* @return �Ƿ���ӳɹ�
	*/
	__declspec(dllexport) Exif_Data * CYNET_GetExifPlatMsg(HANDLE hExif);
};
#endif