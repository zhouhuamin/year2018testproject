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
	* @brief	������Ϣ��ȡ��
	*/
class EXIF_PROCESS_API Cexif_process {
public:
	Cexif_process(void);
	~Cexif_process();
	/**
	* @brief	����Ҫ��ȡ������Ϣ��Դ(�����ļ�)
	* @return	int		
	*/
	int GetExifFromFile(char *filename);
	/**
	* @brief	����Ҫ��ȡ������Ϣ��Դ(������)
	* @param	ImageData	ͼ�����ݻ�����
	* @param	Imagesize	ͼ�����ݴ�С
	* @return	int		
	*/
	int GetExifFromImageData(char *ImageData,unsigned int Imagesize);
	/**
	* @brief	���¼�������
	* @param	��
	* @return	�¼�����
	*/
	int GetEventType();
	/**
	* @brief	��ȡΥ������
	* @param	��
	* @return	Υ������
	*/
	int GetViolateCode();
	/**
	* @brief	��ȡ��ǰ����
	* @param	��
	* @return	��ǰ����
	*/
	int GetLimiteSpeed();
	/**
	* @brief	��ȡ��ַ��Ϣ
	* @param	buf		�洢��ַ��Ϣ�Ļ�����
	* @return	������Ϣ
	*/
	int GetAddressInfo(char *buf);
	/**
	* @brief	��ȡ���ƺ���
	* @param	buf		�洢���ƺ��뻺����
	* @return	������Ϣ
	*/
	int GetPlate(char *buf);
	/**
	* @brief	��ȡ������ɫ
	* @param	buf		�洢������ɫ��Ϣ������
	* @return	������Ϣ
	*/
	int GetPlateColor(char *buf);
	/**
	* @brief	��ȡ��������
	* @param	��
	* @return	��������
	*/
	int GetPlateType();
	/**
	* @brief	��ȡ������ɫ
	* @param	buf		�洢������ɫ������
	* @return	������Ϣ
	*/
	int GetCarColor(char *buf);
	/**
	* @brief	��ȡ������Ϣ
	* @param	NULL	
	* @return	������Ϣ�ṹ��ָ��
	*/
    Exif_Data    * GetExifData();
    /**
	* @brief	��ȡ������Ϣ
    * @param	pPlae ����������Ϣ�Ľṹ��ռ�	
    * @param	size ����������Ϣ�Ľṹ��ռ��С	
	* @return	�ɹ�����0  ʧ�ܷ��� С��0
	*/
    int       GetExifDataA(Exif_Data * pPlate = 0,unsigned int size = 0);
		/**
	* @brief	��ȡץ����Ϣ
	* @param	NULL
	* @return	ץ����Ϣ�ṹ��ָ��
	*/
    FRAME__HEADER * GetFrameHeader();
    /**
	* @brief	��ȡץ����Ϣ
    * @param	pPlae ����ץ����Ϣ�Ľṹ��ռ�	
    * @param	size ����ץ����Ϣ�Ľṹ��ռ��С
	* @return	�ɹ�����0  ʧ�ܷ��� С��0
	*/
    int GetFrameHeaderA(FRAME__HEADER * pFrame = 0,unsigned int size = 0);
		/**
	* @brief	��ȡjpegͷ��Ϣ
	* @param	buf		�洢������ɫ������
	* @return	Jpegͷ��Ϣָ��
	*/
	EXIFINFO    * GetImgMsg();
    /**
	* @brief	��ȡjpegͷ��Ϣ
    * @param	pPlae ����jpegͷ��Ϣ�Ľṹ��ռ�	
    * @param	size ����jpegͷ��Ϣ�Ľṹ��ռ��С
	* @return	�ɹ�����0  ʧ�ܷ��� С��0
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

