/* 
 * File:   JieziBoxStruct.h
 * Author: root
 *
 * Created on 2015年1月28日, 上午10:12
 */

#ifndef JIEZIBOXSTRUCT_H
#define	JIEZIBOXSTRUCT_H
#include <string>
#include "ContaCheckSDK.h"

//typedef enum jz_enumBoxNumberDirection
//{
//    FRONT       = 0,
//    FRONTLEFT   = 1,
//    FRONTRIGHT  = 2,
//    BACK        = 3,
//    BACKLEFT    = 4,
//    BACKRIGHT   = 5
//}BoxNumberDirection;

typedef enum jz_enumCameraDirection
{
    CFRONT       = 0,
    CLEFT        = 1,
    CRIGHT       = 2,
    CBACK        = 3
}CameraDirection;


struct structImagePath
{
    BoxNumberDirection  direct;
    std::string         strFileName;
	std::string			strGUID;
    
};

struct structImageData
{
        CameraDirection         direct;
	std::string		strIp;
	int				nPicLen;
	unsigned char	*pPicBuffer;

	structImageData()
	{
                direct   = CFRONT;
		strIp	= "";
		nPicLen	= 0;
		pPicBuffer	= NULL;
	}
};

//struct structContaOwnerData
//{
//	int nLineNo;
//	std::string strCompanyCode;
//	std::string strCompanyName;
//};

struct T_RECOGNIZE_RESULT_POSITON
{
	std::string CONTA_ID;
	std::string CONTA_MODEL;
	std::string IMAGE_PATH;
};

struct T_CONTA_ALL_INFO
{
	std::string CHNL_NO;
	std::string SEQ_NO;
	
	std::string CONTA_ID;
	std::string CONTA_MODEL;
	std::string IMAGE_TIME;
	std::string IMAGE_IP;	
	
	std::string PIC_TYPE;
	std::string IMAGE_PATH;
};

struct T_CONTA_ALL_INFO_V2
{
	std::string CHNL_NO;
	std::string SEQ_NO;
	
	std::string CONTA_ID_F;
	std::string CONTA_MODEL_F;
	std::string CONTA_ID_B;
	std::string CONTA_MODEL_B;	
	std::string SAVE_TIME;
	std::string LOCAL_PIC_F;
        std::string LOCAL_PIC_B;
        std::string LOCAL_PIC_LF;
        std::string LOCAL_PIC_RF;
        std::string LOCAL_PIC_LB;
        std::string LOCAL_PIC_RB;
        
        std::string FTP_PIC_F;
        std::string FTP_PIC_B;
        std::string FTP_PIC_LF;
        std::string FTP_PIC_RF;
        std::string FTP_PIC_LB;
        std::string FTP_PIC_RB;
};

#endif	/* JIEZIBOXSTRUCT_H */

