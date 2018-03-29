#if !defined(_EXIF_C_)
#define _EXIF_C_
typedef void * HANDLE;
#include "exif_data.h"
extern "C"
{
    /**
	* @brief 创建exif读取句柄
	* @param 无
	* @return 是否添加成功 非0成功 小于0失败
	*/
    __declspec(dllexport) HANDLE CYNET_CreateExifHandle();
    /**
	* @brief 释放句柄
	* @param 无
	* @return 是否添加成功 0成功 小于0失败
	*/
    __declspec(dllexport) int CYNET_ReleaseExifHandle(HANDLE hExif);
	/**
	* @brief 从数据中解析exif信息
	* @param ImageData:源数据
	* @param Imagesize:数据长度
	* @return 是否添加成功 0成功 小于0失败
	*/
	__declspec(dllexport) int CYNET_GetExifFromData(HANDLE hExif,char *ImageData,unsigned int Imagesize);
		/**
	* @brief 从文件中解析exif信息
	* @param filename:文件路径(包含文件名)
	* @return 是否添加成功 0成功 小于0失败
	*/
	__declspec(dllexport) int CYNET_GetExifFromFile(HANDLE hExif,char * filename);
		/**
	* @brief 返回exif中的帧头信息
	* @param NULL
	* @return 是否添加成功
	*/
	__declspec(dllexport) FRAME__HEADER * CYNET_GetExifFrameHead(HANDLE hExif);
		/**
	* @brief 返回exif中的车牌信息
	* @param NULL
	* @return 是否添加成功
	*/
	__declspec(dllexport) Exif_Data * CYNET_GetExifPlatMsg(HANDLE hExif);
};
#endif