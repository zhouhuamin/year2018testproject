// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EXIF_PROCESS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EXIF_PROCESS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef _WIN32
#ifdef EXIF_PROCESS_EXPORTS
#define EXIF_PROCESS_API __declspec(dllexport)
#else
#define EXIF_PROCESS_API __declspec(dllimport)
#endif
#endif

#include "exif_data.h"
#define ADDRESS_INFO_LEN 64
#define PLATE_INFO_LEN 16
#define PLATE_COLOR_LEN 4
#define CAR_COLOR_LEN 4
#ifndef _WIN32
  #define EXIF_PROCESS_API
#endif
#define _SETEXIF_
// This class is exported from the exif_process.dll
	/**
	* @brief	车牌信息提取类
	*/
class EXIF_PROCESS_API Cexif_process {
public:
	Cexif_process(void);
	~Cexif_process();
	/**
	* @brief	设置要提取车牌信息的源(来自文件)
	* @return	int		
	*/
	int GetExifFromFile(char *filename);
	/**
	* @brief	设置要提取车牌信息的源(来自流)
	* @param	ImageData	图像数据缓冲区
	* @param	Imagesize	图像数据大小
	* @return	int		
	*/
	int GetExifFromImageData(char *ImageData,unsigned int Imagesize);
	/**
	* @brief	获事件法类型
	* @param	无
	* @return	事件类型
	*/
	int GetEventType();
	/**
	* @brief	获取违法代号
	* @param	无
	* @return	违法代号
	*/
	int GetViolateCode();
	/**
	* @brief	获取当前限速
	* @param	无
	* @return	当前限速
	*/
	int GetLimiteSpeed();
	/**
	* @brief	获取地址信息
	* @param	buf		存储地址信息的缓冲区
	* @return	错误信息
	*/
	int GetAddressInfo(char *buf);
	/**
	* @brief	获取车牌号码
	* @param	buf		存储车牌号码缓冲区
	* @return	错误信息
	*/
	int GetPlate(char *buf);
	/**
	* @brief	获取车牌颜色
	* @param	buf		存储车牌颜色信息缓冲区
	* @return	错误信息
	*/
	int GetPlateColor(char *buf);
	/**
	* @brief	获取车牌类型
	* @param	无
	* @return	车牌类型
	*/
	int GetPlateType();
	/**
	* @brief	获取车身颜色
	* @param	buf		存储车身颜色缓冲区
	* @return	错误信息
	*/
	int GetCarColor(char *buf);
	/**
	* @brief	获取车牌信息
	* @param	NULL	
	* @return	车牌信息结构体指针
	*/
    Exif_Data    * GetExifData();
    /**
	* @brief	获取车牌信息
    * @param	pPlae 引进车牌信息的结构体空间	
    * @param	size 引进车牌信息的结构体空间大小	
	* @return	成功返回0  失败返回 小于0
	*/
    int       GetExifDataA(Exif_Data * pPlate = 0,unsigned int size = 0);
		/**
	* @brief	获取抓拍信息
	* @param	NULL
	* @return	抓拍信息结构体指针
	*/
    FRAME__HEADER * GetFrameHeader();
    /**
	* @brief	获取抓拍信息
    * @param	pPlae 引进抓拍信息的结构体空间	
    * @param	size 引进抓拍信息的结构体空间大小
	* @return	成功返回0  失败返回 小于0
	*/
    int GetFrameHeaderA(FRAME__HEADER * pFrame = 0,unsigned int size = 0);
		/**
	* @brief	获取jpeg头信息
	* @param	buf		存储车身颜色缓冲区
	* @return	Jpeg头信息指针
	*/
	EXIFINFO    * GetImgMsg();
    /**
	* @brief	获取jpeg头信息
    * @param	pPlae 引进jpeg头信息的结构体空间	
    * @param	size 引进jpeg头信息的结构体空间大小
	* @return	成功返回0  失败返回 小于0
	*/
    int GetImgMsgA(EXIFINFO * pImgmsg = 0,unsigned int size = 0);
#ifdef _SETEXIF_
    int SaveExifToImageFile(char *filename);
    int SaveExifToBuff(unsigned char **ImageData,unsigned int ImageSrcsize,unsigned int *ImageDessize);
    int SetFrameHead(FRAME__HEADER * pFrame = 0,unsigned int size = 0);
    int SetExif_Data(Exif_Data * pPlate = 0,unsigned int size = 0);
#endif
private:
	Exif_Data *pPlateMsg;
	EXIFINFO *pJpgMsg;
	FRAME__HEADER *pFhMsg;
    void * pParesHandle;
    void * pMemHandle;
/*	void * pExif;*/
};

