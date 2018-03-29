/* 
 * File:   ProcessImage.cpp
 * Author: root
 * 
 * Created on 2015年1月28日, 上午10:45
 */
#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <set>
#include <vector>
#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "ProcessImage.h"
#include <syslog.h>
#include "JieziBoxStruct.h"
#include "ace/Log_Priority.h"
#include "ace/Log_Msg.h"
#include "BoxNumberCheckAlgo.h"
#include "SysMessage.h"
#include "MSGHandleCenter.h"
#include "SimpleConfig.h"
#include "MyLog.h"
#include "ace/SOCK_Connector.h"
#include "ace/Signal.h"
#include "RecoClientSDK.h"
#include "jzLocker.h"


using namespace boost::uuids;
using namespace std;
using namespace boost;
int GlobalStopFlag = 0;
int g_nHoppingCount = 0;
vector<pair<BYTE, structUploadData> > g_statusVect;
std::vector<std::string> g_statusTableVect;
std::vector<std::string> g_statusTablePassVect;
std::vector<BYTE> g_loseSeqNoVect;
vector<string> g_DOStatusVect;
vector<char> g_resDOStatusVect;




#define MAX_HOST_DATA 4096
extern std::stack<structImageData> g_ImageDataStack;
extern vector<structBoxNumberRecogResult> g_boxNumberSet;
extern pthread_mutex_t g_ImageDataMutex;
extern pthread_cond_t g_ImageDataCond;
extern pthread_mutex_t g_BoxNumberMutex;

extern map<std::string, T_RECOGNIZE_RESULT_POSITON> g_positonMap;
extern locker g_positionMapLocker;

// =========================================
extern list<T_CONTA_ALL_INFO_V2> g_contaList;
extern locker g_contaLocker;
extern sem g_contaStat;
// =========================================


// 2017/10/11
extern list<T_RECOGNIZE_RESULT_POSITON> g_positionList;
extern locker g_positionListLocker;
extern sem g_positionListStat;

using namespace std;

int GetATimeStamp(char* szBuf, int nMaxLength)
{

    if (!szBuf)
    {
        return -1;
    }

    strcpy(szBuf, "");


    struct tm* tmNow;
    struct timeval tv;
    ::gettimeofday(&tv, NULL);

    tmNow = localtime(&tv.tv_sec);
    int nLen = sprintf(szBuf, "%04d%02d%02d%02d%02d%02d%03d",
            tmNow->tm_year + 1900,
            tmNow->tm_mon + 1,
            tmNow->tm_mday,
            tmNow->tm_hour,
            tmNow->tm_min,
            tmNow->tm_sec,
            tv.tv_usec / 1000
            );
    if (nLen > nMaxLength)
    {
        nLen = nMaxLength;
        szBuf[nLen - 1] = '\0';
    }

    return 0;
}

void UploadFtpFile(std::string remote_path, std::string local_file, std::string remote_file)
{
    char sh_cmd[1024] = {0};
    std::string strRemoteFile = "";
    int nPos = remote_file.find_last_of("/");
    if (nPos >= 0)
    {
        strRemoteFile = remote_file.substr(nPos + 1, remote_file.length());
    }

    sprintf(sh_cmd, "bash /root/jiezisoft/jzftp/ftpup.sh %s %s %s > /dev/null &", remote_path.c_str(), local_file.c_str(), strRemoteFile.c_str());
    system(sh_cmd);
}

void SendRecoResultData(unsigned char* read_data, void* user_data, const std::string &strSequenceID)
{
    structBoxNumberRecogResultCorrected *pUserData = (structBoxNumberRecogResultCorrected *) user_data;
    if (pUserData == NULL)
        return;

    char szDateDir[20] = {0};
    char szDatePath[128] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(szDateDir, "%04d-%02d-%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);
    sprintf(szDatePath, "%s/%s", CSimpleConfig::REMOTE_FILE_PATH.c_str(), szDateDir);

    char szRemoteFilePrefix[255] = {0};
    sprintf(szRemoteFilePrefix, "%s/%s%s_%s", szDatePath, CSimpleConfig::GATHER_AREA_ID, CSimpleConfig::GATHER_CHANNE_NO, strSequenceID.c_str());

    std::string strLocalFile_F = CSimpleConfig::LOCAL_FILE_PATH + pUserData->CONTA_PIC_F;
    std::string strLocalFile_B = CSimpleConfig::LOCAL_FILE_PATH + pUserData->CONTA_PIC_B;
    std::string strLocalFile_LF = CSimpleConfig::LOCAL_FILE_PATH + pUserData->CONTA_PIC_LF;
    std::string strLocalFile_LB = CSimpleConfig::LOCAL_FILE_PATH + pUserData->CONTA_PIC_LB;
    std::string strLocalFile_RF = CSimpleConfig::LOCAL_FILE_PATH + pUserData->CONTA_PIC_RF;
    std::string strLocalFile_RB = CSimpleConfig::LOCAL_FILE_PATH + pUserData->CONTA_PIC_RB;

    std::string strRemoteFile_F = "";
    std::string strRemoteFile_B = "";
    std::string strRemoteFile_LF = "";
    std::string strRemoteFile_LB = "";
    std::string strRemoteFile_RF = "";
    std::string strRemoteFile_RB = "";
    std::string strRemoteFilePrefix = szRemoteFilePrefix;

    if (pUserData->CONTA_PIC_F != "")
        strRemoteFile_F = strRemoteFilePrefix + "_11" + ".jpg";
    if (pUserData->CONTA_PIC_B != "")
        strRemoteFile_B = strRemoteFilePrefix + "_16" + ".jpg";
    if (pUserData->CONTA_PIC_RF != "")
        strRemoteFile_RF = strRemoteFilePrefix + "_12" + ".jpg";
    if (pUserData->CONTA_PIC_LF != "")
        strRemoteFile_LF = strRemoteFilePrefix + "_13" + ".jpg";
    if (pUserData->CONTA_PIC_RB != "")
        strRemoteFile_RB = strRemoteFilePrefix + "_14" + ".jpg";
    if (pUserData->CONTA_PIC_LB != "")
        strRemoteFile_LB = strRemoteFilePrefix + "_15" + ".jpg";



    char szXMLGatherInfo[10 * 1024] = {0};
    string strContNum = "";
    if (pUserData->nBoxType == 1 || pUserData->nBoxType == 3 || pUserData->nBoxType == 4)
        strContNum = "1";
    else if (pUserData->nBoxType == 2)
        strContNum = "2";
    if (pUserData->strFrontBoxNumber != "" && pUserData->strBackBoxNumber != "")
        strContNum = "2";
    string strContReco = pUserData->strRecogResult;

    if (strContReco.empty() || strContNum.empty() || strContReco == "0")
        if (pUserData->strFrontBoxNumber != "" && pUserData->strBackBoxNumber != "" && pUserData->strFrontBoxNumber == pUserData->strBackBoxNumber)
        {
            strContNum = "1";
        }

    if (strContNum == "1")
    {
        sprintf(szXMLGatherInfo,
                "<GATHER_INFO CHNL_NO=\"%s%s\" SEQ_NO=\"%s\"><CONTA>"
                "<CONTA_NUM>%s</CONTA_NUM>"
                "<CONTA_RECO>%s</CONTA_RECO>"
                "<CONTA_ID_F>%s</CONTA_ID_F>"
                "<CONTA_ID_B>%s</CONTA_ID_B>"
                "<CONTA_MODEL_F>%s</CONTA_MODEL_F>"
                "<CONTA_MODEL_B>%s</CONTA_MODEL_B>"
                "</CONTA>"
                "<CONTA_PIC>"
                "<CONTA_PIC_F>%s</CONTA_PIC_F>"
                "<CONTA_PIC_B>%s</CONTA_PIC_B>"
                "<CONTA_PIC_RF>%s</CONTA_PIC_RF>"
                "<CONTA_PIC_LB>%s</CONTA_PIC_LB>"
                "<CONTA_PIC_LF>%s</CONTA_PIC_LF>"
                "<CONTA_PIC_RB>%s</CONTA_PIC_RB>"
                "</CONTA_PIC></GATHER_INFO>"
                , CSimpleConfig::GATHER_AREA_ID
                , CSimpleConfig::GATHER_CHANNE_NO
                , strSequenceID.c_str()
                , strContNum.c_str()
                , strContReco.c_str()
                , pUserData->strFrontBoxNumber.c_str()
                , pUserData->strBackBoxNumber.c_str()
                , pUserData->strFrontBoxModel.c_str()
                , pUserData->strBackBoxModel.c_str()
                , strRemoteFile_F.c_str()
                , strRemoteFile_B.c_str()
                , strRemoteFile_RF.c_str()
                , strRemoteFile_LB.c_str()
                , strRemoteFile_LF.c_str()
                , strRemoteFile_RB.c_str()
                );
    }
    else
    {
        sprintf(szXMLGatherInfo,
                "<GATHER_INFO CHNL_NO=\"%s%s\" SEQ_NO=\"%s\"><CONTA>"
                "<CONTA_NUM>%s</CONTA_NUM>"
                "<CONTA_RECO>%s</CONTA_RECO>"
                "<CONTA_ID_F>%s</CONTA_ID_F>"
                "<CONTA_ID_B>%s</CONTA_ID_B>"
                "<CONTA_MODEL_F>%s</CONTA_MODEL_F>"
                "<CONTA_MODEL_B>%s</CONTA_MODEL_B>"
                "</CONTA>"
                "<CONTA_PIC>"
                "<CONTA_PIC_F>%s</CONTA_PIC_F>"
                "<CONTA_PIC_B>%s</CONTA_PIC_B>"
                "<CONTA_PIC_RF>%s</CONTA_PIC_RF>"
                "<CONTA_PIC_LB>%s</CONTA_PIC_LB>"
                "<CONTA_PIC_LF>%s</CONTA_PIC_LF>"
                "<CONTA_PIC_RB>%s</CONTA_PIC_RB>"
                "</CONTA_PIC></GATHER_INFO>"
                , CSimpleConfig::GATHER_AREA_ID
                , CSimpleConfig::GATHER_CHANNE_NO
                , strSequenceID.c_str()
                , strContNum.c_str()
                , strContReco.c_str()
                , pUserData->strFrontBoxNumber.c_str()
                , pUserData->strBackBoxNumber.c_str()
                , pUserData->strFrontBoxModel.c_str()
                , pUserData->strBackBoxModel.c_str()
                , strRemoteFile_F.c_str()
                , strRemoteFile_B.c_str()
                , strRemoteFile_RF.c_str()
                , strRemoteFile_LB.c_str()
                , strRemoteFile_LF.c_str()
                , strRemoteFile_RB.c_str()
                );
    }
    syslog(LOG_DEBUG, "%s\n", szXMLGatherInfo);


    const char packetHead[4] = {(char) 0xE2, (char) 0x5C, (char) 0x4B, (char) 0x89};
    const char packetEnd[2] = {(char) 0xFF, (char) 0xFF};

    int nXmlLen = strlen(szXMLGatherInfo); // strSendXml.size();
    int nTotalLen = 4 + 4 + 1 + 20 + 4 + nXmlLen + 2;

    BYTE *pPacket = new BYTE[nTotalLen + 1];
    memset(pPacket, 0, nTotalLen + 1);

    BYTE *p;
    p = pPacket;

    memcpy(p, packetHead, 4);
    p += 4;

    memcpy(p, (BYTE*) & nTotalLen, sizeof (int));
    p += 4;

    //消息类型
    char szMsg[2] = {0x04, 0x00};
    memcpy(p, szMsg, 1);
    p += 1;

    std::string strAreaNo = CSimpleConfig::GATHER_AREA_ID;
    memcpy(p, strAreaNo.c_str(), 10);
    p += 10;

    std::string strChannelNo = CSimpleConfig::GATHER_CHANNE_NO;
    memcpy(p, strChannelNo.c_str(), 10);
    p += 10;

    memcpy(p, &nXmlLen, 4);
    p += 4;

    memcpy(p, szXMLGatherInfo, nXmlLen);
    p += nXmlLen;

    memcpy(p, packetEnd, 2);

    if (CSimpleConfig::m_strBackServerIP == "" || CSimpleConfig::m_nBackServerPort <= 0)
        return;

    //按照不同的平台，对数据重新打包
    ProcessImage tmpImage;
    int send_socket = tmpImage.CreateSocket();
    if (tmpImage.ConnectSocket(send_socket, CSimpleConfig::m_strBackServerIP.c_str(), CSimpleConfig::m_nBackServerPort) <= 0)
    {
        tmpImage.CloseSocket(send_socket);
        syslog(LOG_DEBUG, "********connect to platform %s:%d fail...\n", CSimpleConfig::m_strBackServerIP.c_str(), CSimpleConfig::m_nBackServerPort);
        return;
    }

    int nRet = tmpImage.SocketWrite(send_socket, (char*) pPacket, nTotalLen, 1000);
    if (nRet == nTotalLen)
    {
        syslog(LOG_DEBUG, "send gather data to platform %s:%d succ %d...\n", CSimpleConfig::m_strBackServerIP.c_str(), CSimpleConfig::m_nBackServerPort, nRet);
    }

    tmpImage.CloseSocket(send_socket);



    if (pUserData->CONTA_PIC_F != "")
        UploadFtpFile(szDateDir, strLocalFile_F, strRemoteFile_F);
    if (pUserData->CONTA_PIC_B != "")
        UploadFtpFile(szDateDir, strLocalFile_B, strRemoteFile_B);
    if (pUserData->CONTA_PIC_RF != "")
        UploadFtpFile(szDateDir, strLocalFile_RF, strRemoteFile_RF);
    if (pUserData->CONTA_PIC_LF != "")
        UploadFtpFile(szDateDir, strLocalFile_LF, strRemoteFile_LF);
    if (pUserData->CONTA_PIC_RB != "")
        UploadFtpFile(szDateDir, strLocalFile_RB, strRemoteFile_RB);
    if (pUserData->CONTA_PIC_LB != "")
        UploadFtpFile(szDateDir, strLocalFile_LB, strRemoteFile_LB);


    // 2017/10/11
    // 2017/9/25
    std::string CHNL_NO = CSimpleConfig::GATHER_CHANNE_NO;
    T_CONTA_ALL_INFO_V2 position;
    std::string SEQ_NO = strSequenceID;
    map<std::string, T_RECOGNIZE_RESULT_POSITON>::iterator mapIter;
    position.CHNL_NO = CHNL_NO;
    position.SEQ_NO = SEQ_NO;
    if (!pUserData->CONTA_PIC_F.empty())
    {
        std::string tmpImagePath = pUserData->CONTA_PIC_F;
        //position.PIC_TYPE 		= "11";
        position.LOCAL_PIC_F = tmpImagePath;
    }
    if (!pUserData->CONTA_PIC_B.empty())
    {
        std::string tmpImagePath = pUserData->CONTA_PIC_B;
        //position.PIC_TYPE 		= "16";
        position.LOCAL_PIC_B = tmpImagePath;
    }

    if (!pUserData->CONTA_PIC_RF.empty())
    {
        std::string tmpImagePath = pUserData->CONTA_PIC_RF;
        //position.PIC_TYPE 		= "12";
        position.LOCAL_PIC_RF = tmpImagePath;
    }

    if (!pUserData->CONTA_PIC_LF.empty())
    {
        std::string tmpImagePath = pUserData->CONTA_PIC_LF;
        //position.PIC_TYPE 		= "13";
        position.LOCAL_PIC_LF = tmpImagePath;
        //position.IMAGE_IP 		= CSimpleConfig::CameraLeft;		
    }

    if (!pUserData->CONTA_PIC_RB.empty())
    {
        std::string tmpImagePath = pUserData->CONTA_PIC_RB;
        //position.PIC_TYPE 		= "14";
        position.LOCAL_PIC_RB = tmpImagePath;
        //position.IMAGE_IP 		= CSimpleConfig::CameraRight;		
    }

    if (!pUserData->CONTA_PIC_LB.empty())
    {
        std::string tmpImagePath = pUserData->CONTA_PIC_LB;
        //position.PIC_TYPE 		= "15";
        position.LOCAL_PIC_LB = tmpImagePath;
        //position.IMAGE_IP 		= CSimpleConfig::CameraLeft;		
    }

    position.CONTA_ID_F = pUserData->strFrontBoxNumber;
    position.CONTA_MODEL_F = pUserData->strFrontBoxModel;

    position.CONTA_ID_B = pUserData->strBackBoxNumber;
    position.CONTA_MODEL_B = pUserData->strBackBoxModel;


    position.FTP_PIC_F = strRemoteFile_F;
    position.FTP_PIC_B = strRemoteFile_B;
    position.FTP_PIC_LF = strRemoteFile_LF;
    position.FTP_PIC_RF = strRemoteFile_RF;
    position.FTP_PIC_LB = strRemoteFile_LB;
    position.FTP_PIC_RB = strRemoteFile_RB;

    g_contaLocker.lock();
    g_contaList.push_back(position);
    g_contaLocker.unlock();
    g_contaStat.post();
}

void ReceiveReaderData(unsigned char* read_data, void* user_data, const std::string &strSequenceID)
{
    structBoxNumberRecogResultCorrected *pUserData = (structBoxNumberRecogResultCorrected *) user_data;
    if (pUserData == NULL)
        return;
    //	NET_PACKET_MSG* pMsg_ = NULL;
    //	CMSG_Handle_Center::get_message(pMsg_);
    //	if (!pMsg_) 
    //	{
    //		return;
    //	}
    //	memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
    //	pMsg_->msg_head.msg_type = SYS_MSG_PUBLISH_EVENT;
    //	pMsg_->msg_head.packet_len = sizeof (T_SysEventData);
    //	T_SysEventData* pReadData = (T_SysEventData*) pMsg_->msg_body;



    //strcpy(pReadData->event_id, CSimpleConfig::m_eventVect[9].event_id); // SYS_EVENT_CONTA_RECOG_COMPLETE);
    //pReadData->event_id[31]	= '\0';
    //	for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
    //	{
    //		if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "10") == 0)
    //		{
    //			strcpy(pReadData->event_id, CSimpleConfig::m_eventVect[i].event_id);
    //			pReadData->event_id[31]	= '\0';
    //			break;
    //		}
    //	}
    //
    //
    //	strcpy(pReadData->device_tag, CSimpleConfig::DEVICE_TAG);
    //	// strcpy(pReadData->device_tag, "CONTA");
    //	pReadData->is_data_full	= 0; // 1;
    char szXMLGatherInfo[10 * 1024] = {0};
    string strContNum = "";
    if (pUserData->nBoxType == 1 || pUserData->nBoxType == 3 || pUserData->nBoxType == 4)
        strContNum = "1";
    else if (pUserData->nBoxType == 2)
        strContNum = "2";
    if (pUserData->strFrontBoxNumber != "" && pUserData->strBackBoxNumber != "")
        strContNum = "2";
    string strContReco = pUserData->strRecogResult;

    //	if (strContReco.empty() || strContNum.empty() || strContReco == "0")
    //		pReadData->is_data_full	=  1;

    if (pUserData->strFrontBoxNumber != "" && pUserData->strBackBoxNumber != "" && pUserData->strFrontBoxNumber == pUserData->strBackBoxNumber)
    {
        strContNum = "1";
        pUserData->strBackBoxNumber = "";
    }

    if (strContNum == "1")
    {
        sprintf(szXMLGatherInfo,
                "<CONTA>"
                "<CONTA_NUM>%s</CONTA_NUM>"
                "<CONTA_RECO>%s</CONTA_RECO>"
                "<CONTA_ID_F>%s</CONTA_ID_F>"
                "<CONTA_ID_B>%s</CONTA_ID_B>"
                "<CONTA_MODEL_F>%s</CONTA_MODEL_F>"
                "<CONTA_MODEL_B>%s</CONTA_MODEL_B>"
                "</CONTA>"
                "<CONTA_PIC>"
                "<CONTA_PIC_F>%s</CONTA_PIC_F>"
                "<CONTA_PIC_B>%s</CONTA_PIC_B>"
                "<CONTA_PIC_RF>%s</CONTA_PIC_RF>"
                "<CONTA_PIC_LB>%s</CONTA_PIC_LB>"
                "<CONTA_PIC_LF>%s</CONTA_PIC_LF>"
                "<CONTA_PIC_RB>%s</CONTA_PIC_RB>"
                "</CONTA_PIC>"
                , strContNum.c_str()
                , strContReco.c_str()
                , pUserData->strFrontBoxNumber.c_str()
                , pUserData->strBackBoxNumber.c_str()
                , pUserData->strFrontBoxModel.c_str()
                , pUserData->strBackBoxModel.c_str()
                , pUserData->CONTA_PIC_F.c_str()
                , pUserData->CONTA_PIC_B.c_str()
                , pUserData->CONTA_PIC_RF.c_str()
                , pUserData->CONTA_PIC_LB.c_str()
                , pUserData->CONTA_PIC_LF.c_str()
                , pUserData->CONTA_PIC_RB.c_str()
                );
    }
    else
    {
        sprintf(szXMLGatherInfo,
                "<CONTA>"
                "<CONTA_NUM>%s</CONTA_NUM>"
                "<CONTA_RECO>%s</CONTA_RECO>"
                "<CONTA_ID_F>%s</CONTA_ID_F>"
                "<CONTA_ID_B>%s</CONTA_ID_B>"
                "<CONTA_MODEL_F>%s</CONTA_MODEL_F>"
                "<CONTA_MODEL_B>%s</CONTA_MODEL_B>"
                "</CONTA>"
                "<CONTA_PIC>"
                "<CONTA_PIC_F>%s</CONTA_PIC_F>"
                "<CONTA_PIC_B>%s</CONTA_PIC_B>"
                "<CONTA_PIC_LF>%s</CONTA_PIC_LF>"
                "<CONTA_PIC_RF>%s</CONTA_PIC_RF>"
                "<CONTA_PIC_LB>%s</CONTA_PIC_LB>"
                "<CONTA_PIC_RB>%s</CONTA_PIC_RB>"
                "</CONTA_PIC>"
                , strContNum.c_str()
                , strContReco.c_str()
                , pUserData->strFrontBoxNumber.c_str()
                , pUserData->strBackBoxNumber.c_str()
                , pUserData->strFrontBoxModel.c_str()
                , pUserData->strBackBoxModel.c_str()
                , pUserData->CONTA_PIC_F.c_str()
                , pUserData->CONTA_PIC_B.c_str()
                , pUserData->CONTA_PIC_LF.c_str()
                , pUserData->CONTA_PIC_RF.c_str()
                , pUserData->CONTA_PIC_LB.c_str()
                , pUserData->CONTA_PIC_RB.c_str()
                );
    }
    syslog(LOG_DEBUG, "%s\n", szXMLGatherInfo);
    //	strcpy(pReadData->xml_data, szXMLGatherInfo);
    //	pReadData->xml_data_len = strlen(szXMLGatherInfo) + 1;
    //	pMsg_->msg_head.packet_len = sizeof (T_SysEventData) + pReadData->xml_data_len;
    //	syslog(LOG_DEBUG, "pack len:%d\n", pMsg_->msg_head.packet_len);
    //	MSG_HANDLE_CENTER::instance()->put(pMsg_);


    //// 2017/9/25
    //        std::string CHNL_NO = CSimpleConfig::GATHER_CHANNE_NO;
    //T_CONTA_ALL_INFO position;
    //std::string SEQ_NO = strSequenceID;
    //map<std::string,T_RECOGNIZE_RESULT_POSITON>::iterator mapIter;
    //if (!pUserData->CONTA_PIC_F.empty())
    //{
    //	std::string tmpImagePath= pUserData->CONTA_PIC_F;
    //	g_positionMapLocker.lock();
    //	mapIter = g_positonMap.find(tmpImagePath);
    //	if (mapIter != g_positonMap.end())
    //	{
    //		position.CONTA_ID 		= mapIter->second.CONTA_ID;
    //		position.CONTA_MODEL 	= mapIter->second.CONTA_MODEL;
    //	}
    //	g_positionMapLocker.unlock();	
    //	
    //	position.CHNL_NO 		= CHNL_NO;
    //	position.SEQ_NO 		= SEQ_NO;
    //	position.PIC_TYPE 		= "11";
    //	position.IMAGE_PATH 	= tmpImagePath;
    //	position.IMAGE_IP 		= CSimpleConfig::CameraFront;	
    //    g_contaLocker.lock();
    //    g_contaList.push_back(position);
    //    g_contaLocker.unlock();
    //    g_contaStat.post();		
    //}
    //if (!pUserData->CONTA_PIC_B.empty())
    //{
    //	std::string tmpImagePath= pUserData->CONTA_PIC_B;
    //	g_positionMapLocker.lock();
    //	mapIter = g_positonMap.find(tmpImagePath);
    //	if (mapIter != g_positonMap.end())
    //	{
    //		position.CONTA_ID 		= mapIter->second.CONTA_ID;
    //		position.CONTA_MODEL 	= mapIter->second.CONTA_MODEL;
    //	}
    //	g_positionMapLocker.unlock();	
    //	
    //	position.CHNL_NO 		= CHNL_NO;
    //	position.SEQ_NO 		= SEQ_NO;
    //	position.PIC_TYPE 		= "16";
    //	position.IMAGE_PATH 	= tmpImagePath;
    //	position.IMAGE_IP 		= CSimpleConfig::CameraBack;	
    //    g_contaLocker.lock();
    //    g_contaList.push_back(position);
    //    g_contaLocker.unlock();
    //    g_contaStat.post();		
    //}
    //
    //if (!pUserData->CONTA_PIC_RF.empty())
    //{
    //	std::string tmpImagePath= pUserData->CONTA_PIC_RF;
    //	g_positionMapLocker.lock();
    //	mapIter = g_positonMap.find(tmpImagePath);
    //	if (mapIter != g_positonMap.end())
    //	{
    //		position.CONTA_ID 		= mapIter->second.CONTA_ID;
    //		position.CONTA_MODEL 	= mapIter->second.CONTA_MODEL;
    //	}
    //	g_positionMapLocker.unlock();	
    //	
    //	position.CHNL_NO 		= CHNL_NO;
    //	position.SEQ_NO 		= SEQ_NO;
    //	position.PIC_TYPE 		= "12";
    //	position.IMAGE_PATH 	= tmpImagePath;
    //	position.IMAGE_IP 		= CSimpleConfig::CameraRight;	
    //    g_contaLocker.lock();
    //    g_contaList.push_back(position);
    //    g_contaLocker.unlock();
    //    g_contaStat.post();		
    //}
    //
    //if (!pUserData->CONTA_PIC_LF.empty())
    //{
    //	std::string tmpImagePath= pUserData->CONTA_PIC_LF;
    //	g_positionMapLocker.lock();
    //	mapIter = g_positonMap.find(tmpImagePath);
    //	if (mapIter != g_positonMap.end())
    //	{
    //		position.CONTA_ID 		= mapIter->second.CONTA_ID;
    //		position.CONTA_MODEL 	= mapIter->second.CONTA_MODEL;
    //	}
    //	g_positionMapLocker.unlock();	
    //	
    //	position.CHNL_NO 		= CHNL_NO;
    //	position.SEQ_NO 		= SEQ_NO;
    //	position.PIC_TYPE 		= "13";
    //	position.IMAGE_PATH 	= tmpImagePath;
    //	position.IMAGE_IP 		= CSimpleConfig::CameraLeft;	
    //    g_contaLocker.lock();
    //    g_contaList.push_back(position);
    //    g_contaLocker.unlock();
    //    g_contaStat.post();		
    //}
    //
    //if (!pUserData->CONTA_PIC_RB.empty())
    //{
    //	std::string tmpImagePath= pUserData->CONTA_PIC_RB;
    //	g_positionMapLocker.lock();
    //	mapIter = g_positonMap.find(tmpImagePath);
    //	if (mapIter != g_positonMap.end())
    //	{
    //		position.CONTA_ID 		= mapIter->second.CONTA_ID;
    //		position.CONTA_MODEL 	= mapIter->second.CONTA_MODEL;
    //	}
    //	g_positionMapLocker.unlock();	
    //	
    //	position.CHNL_NO 		= CHNL_NO;
    //	position.SEQ_NO 		= SEQ_NO;
    //	position.PIC_TYPE 		= "14";
    //	position.IMAGE_PATH 	= tmpImagePath;
    //	position.IMAGE_IP 		= CSimpleConfig::CameraRight;	
    //    g_contaLocker.lock();
    //    g_contaList.push_back(position);
    //    g_contaLocker.unlock();
    //    g_contaStat.post();		
    //}
    //
    //if (!pUserData->CONTA_PIC_LB.empty())
    //{
    //	std::string tmpImagePath= pUserData->CONTA_PIC_LB;
    //	g_positionMapLocker.lock();
    //	mapIter = g_positonMap.find(tmpImagePath);
    //	if (mapIter != g_positonMap.end())
    //	{
    //		position.CONTA_ID 		= mapIter->second.CONTA_ID;
    //		position.CONTA_MODEL 	= mapIter->second.CONTA_MODEL;
    //	}
    //	g_positionMapLocker.unlock();	
    //	
    //	position.CHNL_NO 		= CHNL_NO;
    //	position.SEQ_NO 		= SEQ_NO;
    //	position.PIC_TYPE 		= "15";
    //	position.IMAGE_PATH 	= tmpImagePath;
    //	position.IMAGE_IP 		= CSimpleConfig::CameraLeft;	
    //	
    //    g_contaLocker.lock();
    //    g_contaList.push_back(position);
    //    g_contaLocker.unlock();
    //    g_contaStat.post();	
    //	
    //}        

    //        g_positionMapLocker.lock();
    //        g_positonMap.clear();
    //        g_positionMapLocker.unlock();        

    return;
}

void PublishEventData(unsigned char* read_data, void* user_data)
{
    //	JZ_PUBLISH_EVENT_STRUCT *pUserData = (JZ_PUBLISH_EVENT_STRUCT *)user_data;
    //	if (pUserData == NULL)
    //		return;
    //	NET_PACKET_MSG* pMsg_ = NULL;
    //	CMSG_Handle_Center::get_message(pMsg_);
    //	if (!pMsg_) 
    //	{
    //		return;
    //	}
    //	memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
    //	pMsg_->msg_head.msg_type 		= SYS_MSG_PUBLISH_EVENT;
    //	pMsg_->msg_head.packet_len 		= sizeof (T_SysEventData);
    //	T_SysEventData* pReadData 		= (T_SysEventData*) pMsg_->msg_body;
    //	strcpy(pReadData->event_id,  pUserData->event_id);
    //	strcpy(pReadData->device_tag, pUserData->device_tag);
    //	pReadData->is_data_full			= 0; // 1;
    //	// char szXMLGatherInfo[10 * 1024] = {0};
    //	//sprintf(szXMLGatherInfo,);  
    //	// printf("%s\n", szXMLGatherInfo);
    //	// strcpy(pReadData->xml_data, szXMLGatherInfo);
    //	pReadData->xml_data_len = 0; // strlen(szXMLGatherInfo) + 1;
    //	pMsg_->msg_head.packet_len = sizeof (T_SysEventData); //  + pReadData->xml_data_len;
    //	syslog(LOG_DEBUG, "pack len:%d\n", pMsg_->msg_head.packet_len);
    //	MSG_HANDLE_CENTER::instance()->put(pMsg_);
    return;
}

ProcessImage::ProcessImage()
{
    syslog(LOG_DEBUG, "Enter ProcessImage Constructor!\n");
}

ProcessImage::ProcessImage(const ProcessImage& orig)
{
}

ProcessImage::~ProcessImage()
{
}

void ProcessImage::Init()
{
    //SetReadDataCallback((_READ_DATA_CALLBACK_) ReceiveReaderData, this);
    return;
}

void ProcessImage::Run()
{
    pthread_create(&controllerThreadID, NULL, ProcessControllerThread, NULL);
    pthread_create(&statusTableThreadID, NULL, ProcessStatusTableThread, NULL);
    pthread_create(&boxNumberThreadID, NULL, ProcessBoxNumberThread, NULL);
    return;
}

void *ProcessImage::ProcessControllerThread(void* pParam)
{
    ProcessImage *pThis = (ProcessImage*) pParam;
    pThis->ProcessControllerProc();
    return 0;
}

void *ProcessImage::ProcessStatusTableThread(void *pParam)
{
    ProcessImage *pThis = (ProcessImage*) pParam;
    pThis->ProcessStatusTableProc();
    return 0;
}

void *ProcessImage::ProcessBoxNumberThread(void* pParam)
{
    ProcessImage *pThis = (ProcessImage*) pParam;
    pThis->ProcessBoxNumberProc();
}

void ProcessImage::ProcessControllerProc()
{
    //printf("Enter ProcessControllerProc\n");
    //	客户端处理线程,把接收的数据发送给目标机器,并把目标机器返回的数据返回到客户端
    int nLen = 0;
    int nLoopTotal = 0;
    int nLoopMax = 20 * 300; //	300 秒判断选循环
#define	nMaxLen 256		// 0x1000
    char pBuffer[nMaxLen + 1];
    char pNewBuffer[nMaxLen + 1];
    memset(pBuffer, 0, nMaxLen);
    memset(pNewBuffer, 0, nMaxLen);
    int nSocketErrorFlag = 0;
    int nNewLen = 0;
    RTUProtocol rtu;
    while (!GlobalStopFlag)
    {
        int nNewSocket = CreateSocket();
        if (nNewSocket == -1)
        {
            //	不能建立套接字，直接返回
            syslog(LOG_DEBUG, "Can't create socket\n");
            sleep(10);
            nNewSocket = CreateSocket();
            if (nNewSocket == -1)
            {
                continue;
            }
        }
        //SetSocketNotBlock(nNewSocket);
        //if (ConnectSocket(nNewSocket, GlobalRemoteHost, GlobalRemotePort) <= 0)
        if (ConnectSocket(nNewSocket, CSimpleConfig::m_strGatherControlIP.c_str(), CSimpleConfig::m_nGatherControlPort) <= 0)
        {
            //	不能建立连接，直接返回
            //CloseSocket(nSocket);
            syslog(LOG_DEBUG, "Can't connect host\n");
            CloseSocket(nNewSocket);
            sleep(3);
            continue;
            //EndClient(pParam);
        }

        set_keep_live(nNewSocket, 3, 30);
        //ConnectSocket(nNewSocket, "192.168.1.188", 502);
        //struct timeval timeout ;
        //fd_set sets;
        //FD_ZERO(&sets);
        //FD_SET(nNewSocket, &sets);
        //timeout.tv_sec = 15; //连接超时15秒
        //timeout.tv_usec =0;
        //int result = select(0, 0, &sets, 0, &timeout);
        //if (result <=0)
        //{
        //	CloseSocket(nNewSocket);
        //	printf("Enter select\n");
        //	sleep(10);
        //	continue;
        //}
        vector<BYTE> dataVect;
        rtu.WriteRegisterRequest(dataVect);
        memcpy(pBuffer, &dataVect[0], dataVect.size());
        nLen = dataVect.size();
        nNewLen = SocketWrite(nNewSocket, pBuffer, nLen, 30);
        syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen);
        //syslog(LOG_DEBUG, "%s Send Data:", GetCurTime().c_str());

        char szTmpData[5 + 1] = {0};
        string strLogData = "Send Data:";
        for (int i = 0; i < nNewLen; ++i)
        {
            memset(szTmpData, 0x00, 5);
            sprintf(szTmpData, "%02X ", dataVect[i]);
            strLogData += szTmpData;
        }
        syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
        //SetSocketNotBlock(nNewSocket);
        while (!GlobalStopFlag && !nSocketErrorFlag)
        {
            nLoopTotal++;
            nLen = SocketRead(nNewSocket, pNewBuffer, nMaxLen);
            //	读取客户端数据
            syslog(LOG_DEBUG, "recv  nLen=%d\n", nLen);
            if (nLen > 0)
            {
                //syslog(LOG_DEBUG, "%s Recv Data:", GetCurTime().c_str());
                strLogData = "Recv Data:";
                for (size_t i = 0; i < nLen; ++i)
                {
                    memset(szTmpData, 0x00, 5);
                    sprintf(szTmpData, "%02X ", (BYTE) pNewBuffer[i]);
                    strLogData += szTmpData;
                }
                syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
                pNewBuffer[nLen] = 0;
                nLoopTotal = 0;
                int nProcessed = 0;
                int nLenLeft = nLen;
                while (nLenLeft > 0)
                {
                    // 进行判断,触发处理流程
                    if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x53)
                    {
                        int nStep = 10;
                        vector<BYTE> upDataVect((BYTE*) pNewBuffer, (BYTE*) pNewBuffer + nStep);
                        nLenLeft -= nStep;
                        if (nLenLeft > 0)
                            memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);

                        std::pair<BYTE, structUploadData> pairData;
                        //read(8);
                        bool bRet = rtu.ReadUploadData(upDataVect, pairData, CSimpleConfig::DI_BIT_REVERSE);
                        //printf("ReadUploadData:ret-%d\n", bRet);
                        // save local
                        pthread_mutex_lock(&g_ImageDataMutex);
                        bool bFlag = false;
                        for (size_t i = 0; i < g_statusVect.size(); ++i)
                        {
                            if (g_statusVect[i].first == pairData.first)
                            {
                                bFlag = true;
                                break;
                            }
                        }
                        if (!bFlag)
                            g_statusVect.push_back(pairData);
                        pthread_mutex_unlock(&g_ImageDataMutex);
                    }
                    else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x65)
                    {
                        //read(1 + 2);
                        int nStep = 5;
                        memcpy(pBuffer, pNewBuffer, nStep);
                        nLenLeft -= nStep;
                        if (nLenLeft > 0)
                            memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
                        int nNewLen2 = SocketWrite(nNewSocket, pBuffer, nStep, 30);
                        syslog(LOG_DEBUG, "write nlen=%d\n", nNewLen2);
                        //syslog(LOG_DEBUG, "%s Send Data:", GetCurTime().c_str());
                        strLogData = "Send Data:";
                        for (int i = 0; i < nNewLen2; ++i)
                        {
                            memset(szTmpData, 0x00, 5);
                            sprintf(szTmpData, "%02X ", (BYTE) pBuffer[i]);
                            strLogData += szTmpData;
                        }
                        syslog(LOG_DEBUG, "%s\n", strLogData.c_str());
                    }
                    else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x02)
                    {
                        //read(1 + 2);
                        int nStep = 6;
                        nLenLeft -= nStep;
                        if (nLenLeft > 0)
                            memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
                    }
                    else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x01)
                    {
                        //read(1 + 2);
                        int nStep = 6;
                        nLenLeft -= nStep;
                        if (nLenLeft > 0)
                            memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
                    }
                    else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x0F)
                    {
                        //read(1 + 2);
                        int nStep = 8;
                        nLenLeft -= nStep;
                        if (nLenLeft > 0)
                            memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
                    }
                    else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x10)
                    {
                        int nStep = 8;
                        nLenLeft -= nStep;
                        if (nLenLeft > 0)
                            memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);
                        //read(4 + 2);
                        bool bRet = false;
                        bRet = rtu.WriteRegisterResponse(dataVect);
                        if (bRet)
                        {
                            pthread_mutex_lock(&g_ImageDataMutex);
                            g_statusVect.clear();
                            pthread_mutex_unlock(&g_ImageDataMutex);
                        }
                    }
                    else if (pNewBuffer[0] == 0x01 && pNewBuffer[1] == 0x03)
                    {
                        //Length = read(1);
                        //read(Length + 2);
                        int nStep = 0;
                        BYTE Length = (BYTE) pNewBuffer[2];
                        nStep = Length + 1 + 4;
                        nLenLeft -= nStep;
                        vector<BYTE> readDataVect((BYTE*) pNewBuffer, (BYTE*) pNewBuffer + nStep);
                        if (nLenLeft > 0)
                            memmove(pNewBuffer, pNewBuffer + nStep, nLenLeft);

                        rtu.ReadRegisterResponse(readDataVect, Length);
                    }
                    else
                    {
                        syslog(LOG_DEBUG, "%s head[0] head[1]: 0x%02X, 0x%02X\n", GetCurTime().c_str(), (BYTE) pNewBuffer[0], (BYTE) pNewBuffer[1]);
                        nLenLeft = 0;
                    }
                    //pthread_cond_signal (&g_ImageDataCond);
                    // read history
                    //if (g_loseSeqNoVect.size() > 0)
                    //{
                    //	vector<BYTE> readDataVect;
                    //	rtu.ReadRegisterRequest(readDataVect, g_loseSeqNoVect[0]);
                    //	memcpy(nNewSocket, &readDataVect[0], readDataVect.size());
                    //	nLen	= readDataVect.size();
                    //	nNewLen=SocketWrite(nNewSocket,pNewBuffer,nLen,30);
                    //	if(nNewLen<0)	//	断开
                    //	{
                    //		CloseSocket(nNewSocket);
                    //		g_loseSeqNoVect.clear();
                    //		break;
                    //	}
                    //	g_loseSeqNoVect.erase(g_loseSeqNoVect.begin());
                    //	
                    //}
                }
            }
            if (nLen < 0)
            {
                //	读断开
                CloseSocket(nNewSocket);

                // 2015-10-14
                pthread_mutex_lock(&g_ImageDataMutex);
                g_statusVect.clear();
                g_statusTableVect.clear();
                g_DOStatusVect.clear();
                pthread_mutex_unlock(&g_ImageDataMutex);

                break;
            }
            //nNewLen=SocketRead(nNewSocket,pNewBuffer,nMaxLen);	
            ////	读取返回数据
            //if(nNewLen>0)
            //{
            //	pNewBuffer[nNewLen]=0;
            //	//			WriteBaseLog(pNewBuffer);
            //	nLoopTotal=0;
            //	nLen=SocketWrite(nNewSocket,pNewBuffer,nNewLen,30);
            //	if(nLen<0)	//	断开
            //		break;
            //}
            //if(nNewLen<0)
            //{
            //	//	读断开
            //	break;
            //}
            if ((nSocketErrorFlag == 0)&&(nLoopTotal > 0))
            {
                SysSleep(50);
                if (nLoopTotal >= nLoopMax)
                {
                    nLoopTotal = 0;
                }
            }
        }
    }
    SysSleep(50);
    pthread_exit(NULL);
    return;
}

void ProcessImage::ProcessStatusTableProc()
{
    bool bStart = false;
    bool bEnd = false;
    long lLastTime = getCurrentTime();
    long lNowTime = lLastTime;
    int nStatusSize = 0;
    pthread_mutex_lock(&g_ImageDataMutex);
    nStatusSize = g_statusTableVect.size();
    pthread_mutex_unlock(&g_ImageDataMutex);

    string strDiEnable = CSimpleConfig::DI_ENABLE;
    //printf("nStatusSize:%d\n", nStatusSize);
    vector<BYTE> seqNoVect;
    while (!GlobalStopFlag)
    {
        lNowTime = getCurrentTime();
        if (g_statusVect.size() > 0)
        {
            std::pair<BYTE, structUploadData> &pairData = g_statusVect.front();
            BYTE ch = pairData.second.Di_Last;
            string str = Byte2String(ch);
            // string inStr = str.substr(4);
            string inStr = "";
            for (int j = 0; j < 8; ++j)
            {
                if (strDiEnable[j] == '1')
                    inStr += str[j];
            }

            pthread_mutex_lock(&g_ImageDataMutex);
            g_statusTableVect.push_back(inStr);
            pthread_mutex_unlock(&g_ImageDataMutex);

            //printf("inStr :%s\n", inStr.c_str());
            ch = pairData.second.Di_Now;
            str = Byte2String(ch);

            //inStr = str.substr(4);
            inStr = "";
            for (int j = 0; j < 8; ++j)
            {
                if (strDiEnable[j] == '1')
                    inStr += str[j];
            }

            pthread_mutex_lock(&g_ImageDataMutex);
            g_statusTableVect.push_back(inStr);
            pthread_mutex_unlock(&g_ImageDataMutex);

            //printf("inStr :%s\n", inStr.c_str());
            ch = pairData.second.Do_Now;
            str = Byte2String(ch);

            seqNoVect.push_back(pairData.first);
            pthread_mutex_lock(&g_ImageDataMutex);
            g_DOStatusVect.push_back(str);
            g_statusVect.erase(g_statusVect.begin());
            pthread_mutex_unlock(&g_ImageDataMutex);
        }

        pthread_mutex_lock(&g_ImageDataMutex);
        nStatusSize = g_statusTableVect.size();
        pthread_mutex_unlock(&g_ImageDataMutex);

        //printf("=====================nStatusSize:%d===================\n", nStatusSize);
        //for (int i = 0; i < g_statusTableVect.size(); ++i)
        //{
        //	printf("%s \n", g_statusTableVect[i].c_str());
        //}
        //printf("=======================================================\n");
        if (nStatusSize > 1)
        {
            for (int i = 2; i < nStatusSize + 1; ++i)
            {
                if (!bStart && g_statusTableVect[i - 2] == "1110" && g_statusTableVect[i - 1] == "1100")
                {
                    int nImageSize = g_ImageDataStack.size();
                    syslog(LOG_DEBUG, "clear Image Count=%d\n", nImageSize);

                    pthread_mutex_lock(&g_ImageDataMutex);
                    while (!g_ImageDataStack.empty())
                    {
                        structImageData tmpImageData;
                        tmpImageData = g_ImageDataStack.top();
                        g_ImageDataStack.pop();
                        syslog(LOG_DEBUG, "clear stack pop!\n");
                        if (tmpImageData.nPicLen > 0 && tmpImageData.pPicBuffer != NULL)
                        {
                            delete []tmpImageData.pPicBuffer;
                            tmpImageData.pPicBuffer = NULL;
                        }
                    }
                    pthread_mutex_unlock(&g_ImageDataMutex);


                    // 集装箱到达事件
                    syslog(LOG_DEBUG, "Box arrived event:%s\n", GetCurTime().c_str());
                    JZ_PUBLISH_EVENT_STRUCT *pUserData = new JZ_PUBLISH_EVENT_STRUCT;
                    for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
                    {
                        if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "1") == 0)
                        {
                            strcpy(pUserData->sequence_no, "0");
                            pUserData->sequence_no[1] = '\0';
                            strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
                            pUserData->device_tag[31] = '\0';
                            strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
                            pUserData->event_id[31] = '\0';
                            break;
                        }
                    }
                    PublishEventData(NULL, (void*) pUserData);
                    delete pUserData;
                    pUserData = NULL;

                    // 2015-9-22
                    std::string strStatus1 = "1110";
                    std::string strStatus2 = "1100";
                    pthread_mutex_lock(&g_ImageDataMutex);
                    g_statusTableVect.clear();
                    g_statusTableVect.push_back(strStatus1);
                    g_statusTableVect.push_back(strStatus2);
                    pthread_mutex_unlock(&g_ImageDataMutex);

                    // start
                    syslog(LOG_DEBUG, "===============================================================time sequence start!\n");
                    bStart = true;
                    bEnd = false; // 2015-9-24
                    g_nHoppingCount = 0;
                    lLastTime = lNowTime;
                    BYTE ch = ' ';
                    if (seqNoVect.size() > 0)
                        ch = seqNoVect[seqNoVect.size() - 1];
                    seqNoVect.clear();
                    if (ch != ' ')
                        seqNoVect.push_back(ch);
                    //continue; // 2019-9-22
                    break;
                }
                //if (bStart && !bEnd &&  g_statusTableVect[i - 2].substr(1) == "001" && g_statusTableVect[i - 1].substr(1) == "011")
                if (bStart && !bEnd && g_statusTableVect[i - 1] == "0011")
                {
                    //BYTE chIncrement	= 0x01;
                    //BYTE chStartSeqNo	= 0;
                    //BYTE chEndSeqNo		= 0;
                    //if (seqNoVect.size() > 0)
                    //{
                    //	chStartSeqNo = seqNoVect[0];
                    //	chEndSeqNo   = seqNoVect[seqNoVect.size() - 1];
                    //}
                    //vector<BYTE> newSeqNoVect;
                    //for (BYTE j = chStartSeqNo; j != chEndSeqNo + 1; ++j)
                    //{
                    //	newSeqNoVect.push_back(j);
                    //}
                    //vector<BYTE> loseSeqNoVect;
                    //for (BYTE j = 0, k = 0; j < newSeqNoVect.size() && k < seqNoVect.size();)
                    //{
                    //	if (newSeqNoVect[j] == seqNoVect[k])
                    //	{
                    //		++j;
                    //		++k;
                    //		continue;
                    //	}
                    //	else
                    //	{
                    //		loseSeqNoVect.push_back(newSeqNoVect[j]);
                    //		++j;
                    //	}
                    //}
                    //bool bLose = false;
                    //if (loseSeqNoVect.size() > 0)
                    //{
                    //	// lose seqno
                    //	bLose	= true;
                    //}
                    //else
                    //{
                    //	bLose = false;
                    //}
                    //if (bLose)
                    //{
                    //	for (size_t i = 0; i < loseSeqNoVect.size(); ++i)
                    //		g_loseSeqNoVect.push_back(loseSeqNoVect[i]);
                    //	SysSleep(500);
                    //	// 后面不再检测了
                    //}
                    // trigger
                    // end

                    lLastTime = lNowTime;
                    bStart = false;
                    bEnd = true;
                    syslog(LOG_DEBUG, "===============================================================time sequence end!\n");
                    int nHoppingCount = 0;
                    for (size_t j = 0; j < g_DOStatusVect.size(); j += 1)
                    {
                        //vector<char> diVect1;
                        //vector<char> diVect2;
                        string str1 = g_DOStatusVect[j].substr(4);
                        if (j + 1 < g_DOStatusVect.size())
                        {
                            string str2 = g_DOStatusVect[j + 1].substr(4);
                            for (size_t k = 0, m = 0; k < str1.size() && m < str2.size(); ++k, ++m)
                            {
                                //diVect1[k] = str1[k];
                                if (str1[k] == '0' && str2[m] == '1')
                                    ++nHoppingCount;
                            }
                            //for (size_t k = 0; k < str2.size(); ++k)
                            //	diVect2[k] = str2[k];
                            //set_difference(diVect2.begin(), diVect2.end(), diVect1.begin(), diVect1.end(), back_inserter(g_resDOStatusVect));
                        }
                        else
                        {
                            break;
                        }
                    }
                    pthread_mutex_lock(&g_ImageDataMutex);
                    copy(g_statusTableVect.begin(), g_statusTableVect.end(), back_inserter(g_statusTablePassVect));
                    g_nHoppingCount = nHoppingCount;
                    g_statusVect.clear();
                    g_statusTableVect.clear();
                    g_DOStatusVect.clear();
                    pthread_cond_signal(&g_ImageDataCond);
                    pthread_mutex_unlock(&g_ImageDataMutex);
                    break;
                }
                else if (!bStart && bEnd && g_statusTableVect[i - 2] == "0111" && g_statusTableVect[i - 1] == "1111")
                {
                    // 车辆过卡口
                    syslog(LOG_DEBUG, "===============================================================car leave\n");
                    bStart = false;
                    bEnd = false;
                    pthread_mutex_lock(&g_ImageDataMutex);
                    g_statusVect.clear();
                    g_statusTableVect.clear();
                    g_DOStatusVect.clear();
                    pthread_mutex_unlock(&g_ImageDataMutex);
                    break;
                    //g_resDOStatusVect.clear();
                }
                else if (g_statusTableVect[i - 2] == "1110" && g_statusTableVect[i - 1] == "1111")
                {
                    //					// dao che
                    //					printf("===============================================================car astern\n");
                    //					lLastTime	= lNowTime;
                    //					bStart	= false;
                    //					bEnd	= false;
                    //					pthread_mutex_lock (&g_ImageDataMutex);  
                    //					g_statusVect.clear();
                    //					g_statusTableVect.clear();
                    //					g_DOStatusVect.clear();
                    //					pthread_mutex_unlock (&g_ImageDataMutex);
                    //					break;
                    //					//g_resDOStatusVect.clear();
                }
                else
                {
                    continue;
                }
            }
        }
        if (bStart && lNowTime - lLastTime > 600) // 60-120 2015-9-24
        {
            syslog(LOG_DEBUG, "===============================================================Timing Sequence is timeout!\n");
            bStart = false;
            bEnd = true;
            int nHoppingCount = 0;
            lLastTime = lNowTime;
            for (size_t j = 0; j < g_DOStatusVect.size(); j += 1)
            {
                //vector<char> diVect1;
                //vector<char> diVect2;
                string str1 = g_DOStatusVect[j];
                string str11 = str1.substr(4);
                if (j + 1 < g_DOStatusVect.size())
                {
                    string str2 = g_DOStatusVect[j + 1].substr(4);
                    string str22 = str2.substr(4);
                    for (size_t k = 0, m = 0; k < str11.size() && m < str22.size(); ++k, ++m)
                    {
                        //diVect1[k] = str1[k];
                        if (str11[k] == '0' && str22[m] == '1')
                            ++nHoppingCount;
                    }
                    //for (size_t k = 0; k < str2.size(); ++k)
                    //	diVect2[k] = str2[k];
                    //set_difference(diVect2.begin(), diVect2.end(), diVect1.begin(), diVect1.end(), back_inserter(g_resDOStatusVect));
                }
                else
                {
                    break;
                }
            }
            pthread_mutex_lock(&g_ImageDataMutex);
            copy(g_statusTableVect.begin(), g_statusTableVect.end(), back_inserter(g_statusTablePassVect));
            g_nHoppingCount = nHoppingCount;
            g_statusVect.clear();
            g_statusTableVect.clear();
            g_DOStatusVect.clear();
            pthread_cond_signal(&g_ImageDataCond);
            pthread_mutex_unlock(&g_ImageDataMutex);
        }
        //sleep(1);
        SysSleep(100);
    }
    return;
}

void ProcessImage::ProcessBoxNumberProc()
{
    //	// test code
    //	while (!GlobalStopFlag)
    //	{
    //		printf("============================\n");
    //		getchar();
    //
    //		//structBoxNumberRecogResultCorrected result;
    //		//result.strFrontBoxNumber	= "GESU5520648";
    //		//result.strBackBoxNumber		= "";
    //		//result.strFrontBoxModel		= "45G1";
    //		//result.strBackBoxModel		= "";
    //		//result.strRecogResult		= "1";
    //		//result.nBoxType				= 1;
    //		//result.nPicNumber			= 4;
    //		//result.CONTA_PIC_F			= "/root/testpic2015/2014-12-18_10-58-32_875_4773.jpg";
    //		//result.CONTA_PIC_B			= "/root/testpic2015/2014-12-18_10-58-32_875_4774.jpg";
    //		//result.CONTA_PIC_RF			= "/root/testpic2015/2014-12-18_10-58-32_875_4775.jpg";
    //		//result.CONTA_PIC_LB			= "/root/testpic2015/2014-12-18_10-58-32_875_4776.jpg";
    //
    //		//ReceiveReaderData(NULL, &result);
    //
    //
    //		char szFileName[256]={0};
    //		char szSeq[8]={0};
    //
    //		for(int i=0;i<1;i++)
    //		{
    //			sprintf(szFileName,"/root/testpic2015/1.jpg");
    //			sprintf(szSeq,"%s.jpg","1");
    //
    //			printf("ContaReco :%s, %s\n", szSeq, szFileName);
    //			ContaReco(szSeq,szFileName);
    //			sleep(2);;
    //		}
    //
    //
    //
    //		getchar();
    //		printf("============================\n");
    //	}
    //	return;

    // test code end
    while (!GlobalStopFlag)
    {
        pthread_mutex_lock(&g_ImageDataMutex);
        //if (g_ImageDataStack.size() == 0)        
        pthread_cond_wait(&g_ImageDataCond, &g_ImageDataMutex);
        pthread_mutex_unlock(&g_ImageDataMutex);

        sleep(2);
        syslog(LOG_DEBUG, "g_nHoppingCount:%d\n", g_nHoppingCount);
        syslog(LOG_DEBUG, "Start box number process!\n");
        std::vector<structImageData> imageDataVect;
        size_t nResDO = g_nHoppingCount; // g_resDOStatusVect.size();
        // 2015-9-19
        if (nResDO <= 0)
        {
            continue;
        }


        size_t nImageCaptureTimeout = nResDO + 1;
        while (nImageCaptureTimeout)
        {
            if (g_ImageDataStack.size() >= nResDO)
                break;
            else
            {
                --nImageCaptureTimeout;
                SysSleep(2000);
            }
        }
        //		if (nImageCaptureTimeout == 0)
        //		{
        //			// 图像抓拍超时
        //			syslog(LOG_DEBUG, "Image Capture Timeout!\n");
        //			JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
        //			for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
        //			{
        //				if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "11") == 0)
        //				{
        //					strcpy(pUserData->sequence_no, "11");
        //					pUserData->sequence_no[2]	= '\0';
        //					strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
        //					pUserData->device_tag[31]	= '\0';
        //					strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
        //					pUserData->event_id[31]	= '\0';
        //					break;
        //				}
        //			}
        //			PublishEventData(NULL, (void*)pUserData);
        //			delete pUserData;
        //			pUserData	= NULL;
        //                        continue;
        //		}
        //		else
        {
            syslog(LOG_DEBUG, "Image Capture success!\n");
        }
        int i = 0;
        int nProcessCount = nResDO;
        if (nResDO == 4)
        {
            nProcessCount = 4;
        }
        else if (nResDO >= 6)
        {
            nProcessCount = 6;
        }
        else
        {
            nProcessCount = nResDO;
        }

        pthread_mutex_lock(&g_ImageDataMutex);
        while (!g_ImageDataStack.empty())
        {
            if (i >= nProcessCount) // 6---nProcessCount
            {
                structImageData tmpImageData;
                tmpImageData = g_ImageDataStack.top();
                g_ImageDataStack.pop();
                if (tmpImageData.nPicLen > 0 && tmpImageData.pPicBuffer != NULL)
                {
                    delete []tmpImageData.pPicBuffer;
                    tmpImageData.pPicBuffer = NULL;
                }
                continue;
            }
            structImageData tmpImageData;
            tmpImageData = g_ImageDataStack.top();
            imageDataVect.push_back(tmpImageData);
            g_ImageDataStack.pop();
            //printf("stack pop\n");
            //ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) stack pop!\n")));
            syslog(LOG_DEBUG, "stack pop!\n");
            ++i;
        }
        pthread_mutex_unlock(&g_ImageDataMutex);

        int nImageSize = imageDataVect.size();
        syslog(LOG_DEBUG, "Image Count=%d\n", nImageSize);

        // 1. 本次时序结束
        // 2. 设置箱数
        // 3. 保存图片
        // 4. 开始识别箱号
        // 5. 校验箱号
        // 6. 识别结束
        // save localfile
        // 进行逻辑处理
        vector<structImagePath> imagePathVect;
        for (size_t i = 0; i < imageDataVect.size(); ++i)
        {
            FILE *pStream = NULL;
            random_generator rgen;
            uuid ranUUID = rgen();
            //cout << ranUUID << endl; 
            string strFileName = "/dev/shm/";
            string strUUID = lexical_cast<string>(ranUUID);
            strFileName += strUUID;
            strFileName += ".jpg";
            if ((pStream = fopen(strFileName.c_str(), "wb")) == NULL)
            {
                continue;
            }
            WriteAll(pStream, (void *) imageDataVect[i].pPicBuffer, imageDataVect[i].nPicLen);
            fclose(pStream);
            if (imageDataVect[i].nPicLen > 0 && imageDataVect[i].pPicBuffer != NULL)
            {
                delete []imageDataVect[i].pPicBuffer;
                imageDataVect[i].pPicBuffer = NULL;
            }
            // 右-左-后|右-左-前|右-左-前

            //			if (CSimpleConfig::m_nTestMode	== 1)
            //			{
            //				// test1 double
            //				if (i == 0)
            //				{
            //					strUUID		= "2015-01-10_16-39-23_710_10924";
            //					strFileName	= "/root/testpic2015/2015-01-10_16-39-23_710_10924.jpg";
            //				}
            //				else if (i == 1)
            //				{
            //					strUUID		= "2015-01-10_16-39-22_755_10922";
            //					strFileName	= "/root/testpic2015/2015-01-10_16-39-22_755_10922.jpg";
            //				}
            //				else if (i == 2)
            //				{
            //					strUUID		= "2015-01-10_16-25-47_690_10903";
            //					strFileName	= "/root/testpic2015/2015-01-10_16-25-47_690_10903.jpg";
            //				}
            //				else if (i == 3)
            //				{
            //					strUUID		= "2015-01-10_16-39-19_703_10921";
            //					strFileName	= "/root/testpic2015/2015-01-10_16-39-19_703_10921.jpg";
            //				}
            //				else if (i == 4)
            //				{
            //					strUUID		= "2015-01-10_16-39-19_021_10919";
            //					strFileName	= "/root/testpic2015/2015-01-10_16-39-19_021_10919.jpg";
            //				}
            //				else if (i == 5)
            //				{
            //					strUUID		= "2015-01-10_16-25-47_424_10902";
            //					strFileName	= "/root/testpic2015/2015-01-10_16-25-47_424_10902.jpg";
            //				}
            //
            //
            //				//// test long
            //				//if (i == 0)
            //				//{
            //				//	strUUID		= "2014-12-04_10-52-01_461_1125B";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-52-01_461_1125B.jpg";
            //				//}
            //				//else if (i == 1)
            //				//{
            //				//	strUUID		= "2014-12-04_10-52-02_945_1127";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-52-02_945_1127.jpg";
            //				//}
            //				//else if (i == 2)
            //				//{
            //				//	strUUID		= "2014-12-04_10-52-03_407_1129";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-52-03_407_1129.jpg";
            //				//}
            //				//else if (i == 3)
            //				//{
            //				//	strUUID		= "2014-12-04_10-52-03_227_1128";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-52-03_227_1128.jpg";
            //				//}
            //				//else if (i == 4)
            //				//{
            //				//	strUUID		= "2014-12-04_10-52-01_461_1125";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-52-01_461_1125.jpg";
            //				//}
            //				//else if (i == 5)
            //				//{
            //				//	strUUID		= "2014-12-04_10-52-01_895_264";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-52-01_895_264.jpg";
            //				//}
            //
            //				//// test short
            //				//if (i == 0)
            //				//{
            //				//	strUUID		= "2014-12-04_10-55-09_889_1132";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-55-09_889_1132.jpg";
            //				//}
            //				//else if (i == 1)
            //				//{
            //				//	strUUID		= "2014-12-04_10-55-10_064_1133";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-55-10_064_1133.jpg";
            //				//}
            //				//else if (i == 2)
            //				//{
            //				//	strUUID		= "2014-12-04_10-55-10_259_1134";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-55-10_259_1134.jpg";
            //				//}
            //				//else if (i == 3)
            //				//{
            //				//	strUUID		= "2014-12-04_10-55-10_294_265";
            //				//	strFileName	= "/root/testpic2015/2014-12-04_10-55-10_294_265.jpg";
            //				//}
            //
            //				////// test double
            //			}

            ContaReco(const_cast<char*> (strUUID.c_str()), const_cast<char*> (strFileName.c_str()));
            structImagePath tmpImagePath;
            tmpImagePath.strFileName = strFileName;
            tmpImagePath.strGUID = strUUID;
            //if (i == 0)
            //    tmpImagePath.direct = BACKRIGHT;
            //else if (i == 1)
            //    tmpImagePath.direct = BACKLEFT;
            //else if (i == 2)
            //    tmpImagePath.direct = BACK;
            //else if (i == 3)
            //    tmpImagePath.direct = FRONTRIGHT;
            //else if (i == 4)
            //    tmpImagePath.direct = FRONTLEFT;
            //else if (i == 5)
            //    tmpImagePath.direct = FRONT;
            imagePathVect.push_back(tmpImagePath);
        }

        //SysSleep(2000);
        int nRecogTimeout = 6 + 1;
        while (nRecogTimeout)
        {
            if (imagePathVect.size() == g_boxNumberSet.size())
            {
                // if (imagePathVect.size() - 1 <= g_boxNumberSet.size())
                break;
            }
            else
            {
                --nRecogTimeout;
                SysSleep(1000);
            }
        }

        if (nRecogTimeout == 0)
        {
            //                    int nsize1 = 0;
            //                    nsize1 = imagePathVect.size();
            //                    int nsize2 = 0;
            //                    nsize2 = g_boxNumberSet.size();
            //                    
            //                    int nleftx = nsize1 - nsize2;
            //                    for (int i = 0; i < nleftx; ++i)
            //                    {
            //                        structBoxNumberRecogResult result;
            //                        pthread_mutex_lock(&g_BoxNumberMutex);
            //                        g_boxNumberSet.push_back(result);
            //                        pthread_mutex_unlock(&g_BoxNumberMutex);
            //                    }


            vector<structImagePath> tryImagePathVect;

            for (int i = 0; i < imagePathVect.size(); ++i)
            {
                int nExist = 0;
                for (int j = 0; j < g_boxNumberSet.size(); ++j)
                {
                    if (imagePathVect[i].strGUID == g_boxNumberSet[j].strSeqNo)
                    {
                        nExist = 1;
                    }
                }
                if (nExist == 0)
                {
                    tryImagePathVect.push_back(imagePathVect[i]);
                }
            }



            // timeout pass /2017/4/24
            syslog(LOG_DEBUG, "==================================Reco Try Again===============================:%s\n", GetCurTime().c_str());
            //			char sh_cmd[512] = {0};
            //			sprintf(sh_cmd, "bash /root/jiezisoft/killrecoserver.sh");
            //			system(sh_cmd);
            //			SysSleep(3000);

            //			pthread_mutex_lock(&g_BoxNumberMutex);
            //			g_boxNumberSet.clear();
            //			pthread_mutex_unlock(&g_BoxNumberMutex);

            for (int i = 0; i < tryImagePathVect.size(); ++i)
            {
                ContaReco(const_cast<char*> (tryImagePathVect[i].strGUID.c_str()), const_cast<char*> (tryImagePathVect[i].strFileName.c_str()));
            }

            nRecogTimeout = 3 + 1;
            while (nRecogTimeout)
            {
                if (imagePathVect.size() == g_boxNumberSet.size())
                    break;
                else
                {
                    --nRecogTimeout;
                    SysSleep(1000);
                }
            }

            if (nRecogTimeout == 0)
            {
                // 识别超时
                syslog(LOG_DEBUG, "Recognize box number Timeout:%s\n", GetCurTime().c_str());
                //				JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
                //				for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
                //				{
                //					if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "12") == 0)
                //					{
                //						strcpy(pUserData->sequence_no, "12");
                //						pUserData->sequence_no[2]	= '\0';
                //						strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
                //						pUserData->device_tag[31]	= '\0';
                //						strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
                //						pUserData->event_id[31]	= '\0';
                //						break;
                //					}
                //				}
                //				PublishEventData(NULL, (void*)pUserData);
                //				delete pUserData;
                //				pUserData	= NULL;
                //                                continue;
            }
            else
            {
                syslog(LOG_DEBUG, "22Recognize box number success:%s\n", GetCurTime().c_str());
            }
        }
        else
        {
            syslog(LOG_DEBUG, "Recognize box number success:%s\n", GetCurTime().c_str());
            //// 集装箱到达事件
            //syslog(LOG_DEBUG, "Box arrived event:%s\n", GetCurTime().c_str());
            //JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
            //for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
            //{
            //	if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "10") == 0)
            //	{
            //		strcpy(pUserData->sequence_no, "0");
            //		pUserData->sequence_no[1]	= '\0';
            //		strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
            //		pUserData->device_tag[31]	= '\0';
            //		strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
            //		pUserData->event_id[31]	= '\0';
            //		break;
            //	}
            //}
            //PublishEventData(NULL, (void*)pUserData);
            //delete pUserData;
            //pUserData	= NULL;


        }




        //for (size_t i = 0; i < imageDataVect.size() && i < imagePathVect.size(); ++i)
        //{
        //                  if (i < 2)
        //                  {
        //		for (size_t j = 0; j < i; ++j)
        //		{
        //			if (imageDataVect[j].direct == CBACK)
        //			{
        //				imagePathVect[j].direct = BACK;
        //			}
        //			else if (imageDataVect[j].direct == CLEFT)
        //			{
        //                                          if (j == 0)
        //				imagePathVect[j].direct = FRONTLEFT;
        //                                          else
        //                                              imagePathVect[j].direct = BACKLEFT;
        //			}
        //			else if (imageDataVect[j].direct == CRIGHT)
        //			{
        //                                          if (j == 0)
        //				imagePathVect[j].direct = FRONTRIGHT;
        //                                          else
        //                                              imagePathVect[j].direct = BACKRIGHT;
        //			}
        //		}                        
        //                  }
        //                  
        //                  
        //                  
        //	if (i == 2)
        //	{
        //		for (size_t j = 0; j <= i; ++j)
        //		{
        //			if (imageDataVect[j].direct == CBACK)
        //			{
        //				imagePathVect[j].direct = BACK;
        //			}
        //			else if (imageDataVect[j].direct == CLEFT)
        //			{
        //				imagePathVect[j].direct = BACKLEFT;
        //			}
        //			else if (imageDataVect[j].direct == CRIGHT)
        //			{
        //				imagePathVect[j].direct = BACKRIGHT;
        //			}
        //		}
        //	}
        //	if (i == 5)
        //	{
        //		for (size_t j = 3; j < i + 1; ++j)
        //		{
        //			if (imageDataVect[j].direct == CFRONT)
        //			{
        //				imagePathVect[j].direct = FRONT;
        //			}
        //			else if (imageDataVect[j].direct == CLEFT)
        //			{
        //				imagePathVect[j].direct = FRONTLEFT;
        //			}
        //			else if (imageDataVect[j].direct == CRIGHT)
        //			{
        //				imagePathVect[j].direct = FRONTRIGHT;
        //			}
        //		}
        //	}
        //}
        int nFrontCount = 0;
        int nBackCount = 0;
        int nFrontLeftCount = 0;
        int nFrontRightCount = 0;
        int nBackLeftCount = 0;
        int nBackRightCount = 0;
        int nImageDataVectSize = imageDataVect.size();
        int nImagePathVectSize = imagePathVect.size();
        int nMin = nImageDataVectSize;
        if (nImagePathVectSize < nMin)
            nMin = nImagePathVectSize;
        for (size_t i = 0; i < nMin; ++i)
        {
            if (nBackRightCount == 0 && imageDataVect[i].direct == CRIGHT)
            {
                imagePathVect[i].direct = BACKRIGHT;
                ++nBackRightCount;
                continue;
            }
            if (nBackLeftCount == 0 && imageDataVect[i].direct == CLEFT)
            {
                imagePathVect[i].direct = BACKLEFT;
                ++nBackLeftCount;
                continue;
            }
            if (nBackCount == 0 && imageDataVect[i].direct == CBACK)
            {
                imagePathVect[i].direct = BACK;
                ++nBackCount;
                continue;
            }
            if (nBackRightCount == 1 && imageDataVect[i].direct == CRIGHT)
            {
                imagePathVect[i].direct = FRONTRIGHT;
                ++nFrontRightCount;
                continue;
            }
            if (nBackLeftCount == 1 && imageDataVect[i].direct == CLEFT)
            {
                imagePathVect[i].direct = FRONTLEFT;
                ++nFrontLeftCount;
                continue;
            }
            if (nFrontCount == 0 && imageDataVect[i].direct == CFRONT)
            {
                imagePathVect[i].direct = FRONT;
                ++nFrontCount;
                continue;
            }
        }



        vector<structBoxNumberRecogResult> boxNumberSet;
        for (size_t i = 0; i < g_boxNumberSet.size(); ++i)
        {
            structBoxNumberRecogResult boxResult;
            boxResult.strBoxNumber = g_boxNumberSet[i].strBoxNumber;
            boxResult.strBoxModel = g_boxNumberSet[i].strBoxModel;
            boxResult.strBoxColor = g_boxNumberSet[i].strBoxColor;
            boxResult.strArrangement = g_boxNumberSet[i].strArrangement;

            for (int j = 0; j < imagePathVect.size(); ++j)
            {
                if (g_boxNumberSet[i].strSeqNo == imagePathVect[j].strGUID)
                {
                    boxResult.direct = imagePathVect[j].direct;
                    break;
                }
            }
            boxResult.nAccuracy = g_boxNumberSet[i].nAccuracy;

            //boxResult.direct            = g_boxNumberSet[i].direct;
            // strBoxNumber = IBoxNumberRecog(imagePathVect[i].strFileName.c_str(), string);
            boxNumberSet.push_back(boxResult);
        }

        // clear g_boxNumberSet
        pthread_mutex_lock(&g_BoxNumberMutex);
        g_boxNumberSet.clear();
        pthread_mutex_unlock(&g_BoxNumberMutex);

        // 箱号识别
        int nRet = 0;
        char szBoxNumber[20 + 1] = {0};
        //nRet = IBoxNumberRecog(0, NULL, szBoxNumber);
        vector<std::string> statusVect;

        pthread_mutex_lock(&g_ImageDataMutex);
        copy(g_statusTablePassVect.begin(), g_statusTablePassVect.end(), back_inserter(statusVect));
        g_statusTablePassVect.clear();
        pthread_mutex_unlock(&g_ImageDataMutex);
        if (boxNumberSet.size() > 0)
        {
            syslog(LOG_DEBUG, "\n%s Status Table:", GetCurTime().c_str());
            for (size_t i = 0; i < statusVect.size(); ++i)
            {
                syslog(LOG_DEBUG, "%s ", statusVect[i].c_str());
            }
            syslog(LOG_DEBUG, "\n");
            syslog(LOG_DEBUG, "\n%s Recog result:", GetCurTime().c_str());
            for (size_t i = 0; i < boxNumberSet.size(); ++i)
            {
                syslog(LOG_DEBUG, "%s,%s,%s,%s,%d\n", boxNumberSet[i].strBoxNumber.c_str(), boxNumberSet[i].strBoxModel.c_str(), boxNumberSet[i].strBoxColor.c_str(), \
					boxNumberSet[i].strArrangement.c_str(), (int) boxNumberSet[i].direct);
            }
            syslog(LOG_DEBUG, "\n");
        }
        //structBoxNumberRecogResultCorrected resultCorrected;
        //boost::shared_ptr<implementation> sp1(new implementation());
        //boost::shared_ptr<structBoxNumberRecogResultCorrected> spResult(new structBoxNumberRecogResultCorrected());
        structBoxNumberRecogResultCorrected *pResult = NULL;
        pResult = new structBoxNumberRecogResultCorrected;
        // nRet = JudgeBoxType(statusVect, boxNumberSet, *pResult);

        nRet = ContaCheck(statusVect, boxNumberSet, *pResult);

        for (int j = 0; j < imagePathVect.size(); ++j)
        {
            if (imagePathVect[j].direct == FRONT)
                pResult->CONTA_PIC_F = imagePathVect[j].strFileName;
            if (imagePathVect[j].direct == BACK)
                pResult->CONTA_PIC_B = imagePathVect[j].strFileName;
            if (imagePathVect[j].direct == FRONTLEFT)
                pResult->CONTA_PIC_LF = imagePathVect[j].strFileName;
            if (imagePathVect[j].direct == FRONTRIGHT)
                pResult->CONTA_PIC_RF = imagePathVect[j].strFileName;
            if (imagePathVect[j].direct == BACKLEFT)
                pResult->CONTA_PIC_LB = imagePathVect[j].strFileName;
            if (imagePathVect[j].direct == BACKRIGHT)
                pResult->CONTA_PIC_RB = imagePathVect[j].strFileName;
        }

        char m_szSequenceID[32] = {0};
        {
            GetATimeStamp(m_szSequenceID, 32);

            srand(time(0));
            int nRand = rand() % 1000;
            sprintf(m_szSequenceID, "%s%03d", m_szSequenceID, nRand);

        }
        std::string strSequenceID = m_szSequenceID;



        //(unsigned char* read_data, void* user_data)
        ReceiveReaderData(NULL, (void*) pResult, strSequenceID);

        SendRecoResultData(NULL, (void*) pResult, strSequenceID);

        if (pResult != NULL)
        {
            delete pResult;
            pResult = NULL;
        }


        //// 
        //syslog(LOG_DEBUG, "Box arrived event:%s\n", GetCurTime().c_str());
        //JZ_PUBLISH_EVENT_STRUCT *pUserData	= new JZ_PUBLISH_EVENT_STRUCT;
        //for (int i = 0; i < CSimpleConfig::m_eventVect.size(); ++i)
        //{
        //	if (strcmp(CSimpleConfig::m_eventVect[i].sequence_no, "10") == 0)
        //	{
        //		strcpy(pUserData->sequence_no, "0");
        //		pUserData->sequence_no[1]	= '\0';
        //		strcpy(pUserData->device_tag, CSimpleConfig::m_eventVect[i].device_tag);
        //		pUserData->device_tag[31]	= '\0';
        //		strcpy(pUserData->event_id, CSimpleConfig::m_eventVect[i].event_id);
        //		pUserData->event_id[31]	= '\0';
        //		break;
        //	}
        //}
        //PublishEventData(NULL, (void*)pUserData);
        //delete pUserData;
        //pUserData	= NULL;

        //// 箱号校验
        //if (boxNumberSet.size() == 0)
        //{
        //}
        //else if (boxNumberSet.size() == 1)
        //{
        //}
        //else if (boxNumberSet.size() == 2)
        //{
        //}        
        //else if (boxNumberSet.size() > 2)
        //{
        //}
        //char chVerifyCode   = ' ';
        //string strBoxNumber = "";
        //BoxNumberCheckAlgo check;
        //nRet = check.GetBoxNumCheckbit(strBoxNumber, chVerifyCode);
        /*
        长箱的判断标准是四对对射全部被挡住，即状态为0000
        双箱的判断标准是：
        1、010x-> 000x
        2、000x -> 010x
        3、x010 -> x000
        4、x000 -> x010
        5、xx01 -> xx00
        其中x可为0或1任意
        本处0为挡住、1为未挡住
         */
        // 发布事件
        ////
        //FILE	*fd = NULL;
        //if ((fd = fopen("/xxxx","wb")) == NULL)
        //{
        //	printf("write error!\n");
        //}
        //else 
        //{
        //	WriteAll(fd, NULL, 0);
        //	fclose(fd);
        //	printf("successfully saved\n");
        //}  
    }
    return;
}

int ProcessImage::CreateSocket()
{
    int nSocket;
    nSocket = (int) socket(PF_INET, SOCK_STREAM, 0);
    return nSocket;
}

int ProcessImage::ConnectSocket(int nSocket, const char * szHost, int nPort)
{
    hostent *pHost = NULL;
#if defined(_WIN32)||defined(_WIN64)
    pHost = gethostbyname(szHost);
    if (pHost == 0)
    {
        return 0;
    }
#else
    hostent localHost;
    char pHostData[MAX_HOST_DATA];
    int h_errorno = 0;
    //#ifdef	Linux
    int h_rc = gethostbyname_r(szHost, &localHost, pHostData, MAX_HOST_DATA, &pHost, &h_errorno);
    if ((pHost == 0) || (h_rc != 0))
    {
        return 0;
    }
    //#else
    //	//	we assume defined SunOS
    //	pHost=gethostbyname_r(szHost,&localHost,pHostData,MAX_HOST_DATA,&h_errorno);
    //	if((pHost==0))
    //	{
    //		return 0;
    //	}
    //#endif
#endif
    struct in_addr in;
    memcpy(&in.s_addr, pHost->h_addr_list[0], sizeof (in.s_addr));
    sockaddr_in name;
    memset(&name, 0, sizeof (sockaddr_in));
    name.sin_family = AF_INET;
    name.sin_port = htons((unsigned short) nPort);
    name.sin_addr.s_addr = in.s_addr;
#if defined(_WIN32)||defined(_WIN64)
    int rc = connect((SOCKET) nSocket, (sockaddr *) & name, sizeof (sockaddr_in));
#else
    int rc = connect(nSocket, (sockaddr *) & name, sizeof (sockaddr_in));
#endif
    if (rc >= 0)
        return 1;
    return 0;
}

int ProcessImage::CheckSocketValid(int nSocket)
{
    //	check socket valid
#if !defined(_WIN32)&&!defined(_WIN64)
    if (nSocket == -1)
        return 0;
    else
        return 1;
#else
    if (((SOCKET) nSocket) == INVALID_SOCKET)
        return 0;
    else
        return 1;
#endif
}

int ProcessImage::CloseSocket(int nSocket)
{
    int rc = 0;
    if (!CheckSocketValid(nSocket))
    {
        return rc;
    }
#if	defined(_WIN32)||defined(_WIN64)
    shutdown((SOCKET) nSocket, SD_BOTH);
    closesocket((SOCKET) nSocket);
#else
    shutdown(nSocket, SHUT_RDWR);
    close(nSocket);
#endif
    rc = 1;
    return rc;
}

void ProcessImage::SetSocketNotBlock(int nSocket)
{
    //	改变文件句柄为非阻塞模式
#if	defined(_WIN32)||defined(_WIN64)
    ULONG optval2 = 1;
    ioctlsocket((SOCKET) nSocket, FIONBIO, &optval2);
#else
    long fileattr;
    fileattr = fcntl(nSocket, F_GETFL);
    fcntl(nSocket, F_SETFL, fileattr | O_NDELAY);
#endif
}

void ProcessImage::SysSleep(long nTime) //	延时nTime毫秒，毫秒是千分之一秒
{
#if defined(_WIN32 )||defined(_WIN64)
    //	windows 代码
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
        if (GetMessage(&msg, NULL, 0, 0) != -1)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    Sleep(nTime);
#else
    //	unix/linux代码
    timespec localTimeSpec;
    timespec localLeftSpec;
    localTimeSpec.tv_sec = nTime / 1000;
    localTimeSpec.tv_nsec = (nTime % 1000)*1000000;
    nanosleep(&localTimeSpec, &localLeftSpec);
#endif
}

int ProcessImage::SocketWrite(int nSocket, char * pBuffer, int nLen, int nTimeout)
{
    int nOffset = 0;
    int nWrite;
    int nLeft = nLen;
    int nLoop = 0;
    int nTotal = 0;
    int nNewTimeout = nTimeout * 10;
    while ((nLoop <= nNewTimeout)&&(nLeft > 0))
    {
        nWrite = send(nSocket, pBuffer + nOffset, nLeft, 0);
        if (nWrite == 0)
        {
            return -1;
        }
#if defined(_WIN32)||defined(_WIN64)
        if (nWrite == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                return -1;
            }
        }
#else
        if (nWrite == -1)
        {
            if (errno != EAGAIN)
            {
                return -1;
            }
        }
#endif
        if (nWrite < 0)
        {
            return nWrite;
        }
        nOffset += nWrite;
        nLeft -= nWrite;
        nTotal += nWrite;
        if (nLeft > 0)
        {
            //	延时100ms
            SysSleep(100);
        }
        nLoop++;
    }
    return nTotal;
}

int ProcessImage::SocketRead(int nSocket, void * pBuffer, int nLen)
{
    if (nSocket == -1)
        return -1;
    int len = 0;
#if	defined(_WIN32)||defined(_WIN64)
    len = recv((SOCKET) nSocket, (char *) pBuffer, nLen, 0);
#else
    len = recv(nSocket, (char *) pBuffer, nLen, 0);
#endif
    if (len == 0)
    {
        return -1;
    }
    if (len == -1)
    {
#if	defined(_WIN32)||defined(_WIn64)
        int localError = WSAGetLastError();
        if (localError == WSAEWOULDBLOCK)
            return 0;
        return -1;
#else
        if (errno == 0)
            return -1;
        if (errno == EAGAIN)
            return 0;
#endif		
        return len;
    }
    if (len > 0)
        return len;
    else
        return -1;
}

size_t ProcessImage::ReadAll(FILE *fd, void *buff, size_t len)
{
    size_t n = 0;
    size_t sum = 0;
    do
    {
        n = fread((char*) buff + sum, 1, len - sum, fd);
        sum += n;
    } while (sum < len && n != 0);
    if (n == 0 && ferror(fd))
        return 0;
    if (n == 0)
        return 1;
    return 1;
}

size_t ProcessImage::WriteAll(FILE *fd, void *buff, size_t len)
{
    size_t n = 0;
    size_t sum = 0;
    do
    {
        n = fwrite((char*) buff + sum, 1, len - sum, fd);
        sum += n;
    } while (sum < len && n != 0);
    if (n == 0 && ferror(fd))
        return 0;
    if (n == 0)
        return 1;
    return 1;
}

int ProcessImage::JudgeBoxType(std::vector<std::string>& statusVect, const std::vector<structBoxNumberRecogResult>& boxNumberSet, structBoxNumberRecogResultCorrected &resultCorrected)
{
    if (statusVect.size() == 0)
    {
        return 0;
    }
    if (boxNumberSet.size() == 0)
    {
        return 0;
    }
    syslog(LOG_DEBUG, "Enter JudgeBoxType\n");
    // 根据对射触发判断长短箱流程-----single double judge procedure
    int nBoxType = 0; // 1:长箱   2:双箱  3:单箱 4:短箱
    for (size_t i = 0; i < statusVect.size() - 1; ++i)
    {
        // 010x-> 000x
        if (statusVect[i].substr(1) == "010" && statusVect[i + 1].substr(1) == "000")
        {
            nBoxType = 2;
            break;
        }
        if (statusVect[i].substr(1) == "000" && statusVect[i + 1].substr(1) == "010")
        {
            nBoxType = 2;
            break;
        }
        // 长箱的判断标准是四对对射全部被挡住，即状态为0000
        if (statusVect[i] == "1001" && statusVect[i + 1] == "1011")
        {
            nBoxType = 4;
        }
    }
    syslog(LOG_DEBUG, "1===================================nBoxType:%d\n", nBoxType);
    // 从时序开始到结束 没有出现一次0000，那肯定是短箱
    if (nBoxType == 0)
    {
        int nOneFlag = 0;
        for (size_t i = 0; i < statusVect.size(); ++i)
        {
            if (statusVect[i] == "0000")
            {
                nOneFlag = 1;
                break;
            }
        }
        if (nOneFlag == 0)
        {
            nBoxType = 4;
        }

        syslog(LOG_DEBUG, "2===================================nBoxType:%d\n", nBoxType);
    }

    if (nBoxType == 2 || nBoxType == 4)
    {
        // 确定是长箱或双箱或短箱
        syslog(LOG_DEBUG, "===================================222nBoxType:%d\n", nBoxType);
    }
    else
    {
        bool bModel = false;
        for (int i = 0; i < boxNumberSet.size(); ++i)
        {
            //if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxNumber.size() > 5 && boxNumberSet[i].strBoxNumber[4] == '2')
            if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxModel[0] == '2')
            {
                bModel = true;
                nBoxType = 2;
                break;
            }
            //if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxNumber.size() > 5 && boxNumberSet[i].strBoxNumber[4] == '4')
            if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxModel[0] == '4')
            {
                bModel = true;
                nBoxType = 1;
                break;
            }
        }
        syslog(LOG_DEBUG, "===================================333nBoxType:%d\n", nBoxType);
        if (bModel)
        {
            // 确定是双箱或长箱
        }
        else
        {
            string strFront = "";
            string strFrontRight = "";
            string strBack = "";
            string strBackLeft = "";
            string strFrontLeft = "";
            string strBackRight = "";
            string strFrontRightArrange = "";
            string strBackLeftArrange = "";
            string strFrontLeftColor = "";
            string strBackLeftColor = "";
            for (int i = 0; i < boxNumberSet.size(); ++i)
            {
                if (boxNumberSet[i].direct == FRONT)
                    strFront = boxNumberSet[i].strBoxNumber;
                if (boxNumberSet[i].direct == FRONTRIGHT)
                {
                    strFrontRight = boxNumberSet[i].strBoxNumber;
                    strFrontRightArrange = boxNumberSet[i].strArrangement;
                }
                if (boxNumberSet[i].direct == BACK)
                    strBack = boxNumberSet[i].strBoxNumber;
                if (boxNumberSet[i].direct == BACKLEFT)
                {
                    strBackLeft = boxNumberSet[i].strBoxNumber;
                    strBackLeftArrange = boxNumberSet[i].strArrangement;
                    strBackLeftColor = boxNumberSet[i].strBoxColor;
                }
                if (boxNumberSet[i].direct == FRONTLEFT)
                {
                    strFrontLeft = boxNumberSet[i].strBoxNumber;
                    strFrontLeftColor = boxNumberSet[i].strBoxColor;
                }
                if (boxNumberSet[i].direct == BACKRIGHT)
                    strBackRight = boxNumberSet[i].strBoxNumber;
            }
            // 前箱号前右箱号后4位相同么 && 后箱号与后左箱号后4位相同么
            if (strFront.size() == 11 && strBack.size() == 11 && strFrontRight.size() == 11 && strBackLeft.size() == 11 && strFront.substr(7, 4) == strBack.substr(7, 4) && strFrontRight.substr(7, 4) == strBackLeft.substr(7, 4))
            {
                // 确定是长箱
                nBoxType = 1;
            }
            else
            {
                //                int nRet = 0;
                //                char chVerifyCode   = ' ';
                //                string strBoxNumber = strFrontLeft;
                //                BoxNumberCheckAlgo check;
                //                nRet = check.GetBoxNumCheckbit(strBoxNumber, chVerifyCode);
                //                if (nRet == 0)
                //                {
                //                    chVerifyCode    = ' ';
                //                    strBoxNumber    = strBackRight;
                //                    nRet = check.GetBoxNumCheckbit(strBoxNumber, chVerifyCode);                    
                //                }
                // 比较前左、后右有一个识别出来了么
                if (strFrontLeft != "" || strBackRight != "")
                    //if (nRet == 1)
                {
                    // 确定是双箱
                    nBoxType = 2;
                }
                else
                {
                    // 比较前右、后左箱号 排列方式是否不同
                    if (strFrontRightArrange != strBackLeftArrange)
                    {
                        // 确定是双箱
                        nBoxType = 2;
                    }
                    else
                    {
                        // 比较前左，后左箱体颜色，不同且差距大么
                        if (strFrontLeftColor != strBackLeftColor)
                        {
                            // 确定是双箱
                            nBoxType = 2;
                        }
                        else
                        {
                            // 自认为是单箱
                            nBoxType = 3;
                        }
                    }
                }
            }
        }
    }
    //structBoxNumberRecogResultCorrected resultCorrected;
    // 能确定 是 单箱 、双箱 么
    if (nBoxType == 2 || nBoxType == 3)
    {
        // 单箱
        if (nBoxType == 3)
        {
            // 挑选出4副图片识别结果
            int nPicNumber = 4;
            // 进入箱号校验修正流程
            BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);
        }
        else if (nBoxType == 2)// 双箱
        {
            // 每个箱子至少三个识别结果
            int nPicNumber = 6;
            // 进入箱号校验修正流程
            BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);
        }
    }
    else if (nBoxType == 4)
    {
        // 挑选出4副图片识别结果
        int nPicNumber = 4;
        // 进入箱号校验修正流程
        BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);
    }
    else
    {
        // 按长箱处理
        nBoxType = 1;
        // 挑选出4副图片识别结果
        int nPicNumber = 4;
        // 进入箱号校验修正流程
        BoxNumberCorrection(boxNumberSet, nPicNumber, nBoxType, resultCorrected);
    }
    return 0;
}

int ProcessImage::BoxNumberCorrectionOld(const std::vector<structBoxNumberRecogResult>& boxNumberSet, int nPicNumber, int nBoxType, structBoxNumberRecogResultCorrected &resultCorrected)
{
    syslog(LOG_DEBUG, "\n");
    syslog(LOG_DEBUG, "%s Box Correction before - PicNumber:%d, BoxType:%d\n", GetCurTime().c_str(), nPicNumber, nBoxType);
    for (size_t i = 0; i < boxNumberSet.size(); ++i)
    {
        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d\n", boxNumberSet[i].strBoxNumber.c_str(), boxNumberSet[i].strBoxModel.c_str(), boxNumberSet[i].strBoxColor.c_str(), \
			boxNumberSet[i].strArrangement.c_str(), (int) boxNumberSet[i].direct);
    }

    // 4副图片 前、前右，后、后左识别结果
    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
    //structBoxNumberRecogResultCorrected resultCorrected;
    if (boxNumberSet.size() <= 0)
    {
        resultCorrected.strFrontBoxNumber = "";
        resultCorrected.strFrontBoxModel = "";
        resultCorrected.strBackBoxNumber = "";
        resultCorrected.strBackBoxModel = "";
        resultCorrected.nPicNumber = 0;
        resultCorrected.nBoxType = 0;
        resultCorrected.strRecogResult = "0";
        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
        return 0;
    }

    int nRet1 = 0;
    int nRet2 = 0;
    int nRet3 = 0;
    int nRet4 = 0;
    int nRet5 = 0;
    int nRet6 = 0;
    char chVerifyCode1 = ' ';
    char chVerifyCode2 = ' ';
    char chVerifyCode3 = ' ';
    char chVerifyCode4 = ' ';
    char chVerifyCode5 = ' ';
    char chVerifyCode6 = ' ';
    string strBoxNumber = "";
    BoxNumberCheckAlgo check;
    string strFront = "";
    string strFrontRight = "";
    string strBack = "";
    string strBackLeft = "";
    string strFrontLeft = "";
    string strBackRight = "";
    string strFrontBoxModel = "";
    string strBackBoxModel = "";
    for (int i = 0; i < boxNumberSet.size(); ++i)
    {
        if (boxNumberSet[i].direct == FRONT)
        {
            strFront = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strFrontBoxModel = boxNumberSet[i].strBoxModel;
        }
        if (boxNumberSet[i].direct == FRONTRIGHT)
        {
            strFrontRight = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strFrontBoxModel = boxNumberSet[i].strBoxModel;
        }
        if (boxNumberSet[i].direct == BACK)
        {
            strBack = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strBackBoxModel = boxNumberSet[i].strBoxModel;
        }
        if (boxNumberSet[i].direct == BACKLEFT)
        {
            strBackLeft = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strBackBoxModel = boxNumberSet[i].strBoxModel;
        }
        if (boxNumberSet[i].direct == FRONTLEFT)
        {
            strFrontLeft = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strFrontBoxModel = boxNumberSet[i].strBoxModel;
        }
        if (boxNumberSet[i].direct == BACKRIGHT)
        {
            strBackRight = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strBackBoxModel = boxNumberSet[i].strBoxModel;
        }
    }

    // 2015-9-27                -start
    if (boxNumberSet.size() < 4)
    {
        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
        nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
        nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
        nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);

        nRet5 = check.GetBoxNumCheckbit(strFrontLeft, chVerifyCode5);
        nRet6 = check.GetBoxNumCheckbit(strBackRight, chVerifyCode6);

        if (nRet1 == 1)
        {
            resultCorrected.strFrontBoxNumber = strFront;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet2 == 1)
        {
            resultCorrected.strFrontBoxNumber = strFrontRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet3 == 1)
        {
            resultCorrected.strFrontBoxNumber = strBackLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet4 == 1)
        {
            resultCorrected.strFrontBoxNumber = strBack;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet5 == 1)
        {
            resultCorrected.strFrontBoxNumber = strFrontLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet6 == 1)
        {
            resultCorrected.strFrontBoxNumber = strBackRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        resultCorrected.strFrontBoxNumber = "";
        resultCorrected.strFrontBoxModel = "";
        resultCorrected.strBackBoxNumber = "";
        resultCorrected.strBackBoxModel = "";
        resultCorrected.nPicNumber = 0;
        resultCorrected.nBoxType = 0;
        resultCorrected.strRecogResult = "0";
        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
        return 0;
    }
    //======================    -end





    if (nPicNumber == 4 && boxNumberSet.size() >= 4)
    {
        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
        nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
        nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
        nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);

        nRet5 = check.GetBoxNumCheckbit(strFrontLeft, chVerifyCode5);
        nRet6 = check.GetBoxNumCheckbit(strBackRight, chVerifyCode6);

        if (nRet1 == 1)
        {
            resultCorrected.strFrontBoxNumber = strFront;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet2 == 1)
        {
            resultCorrected.strFrontBoxNumber = strFrontRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet3 == 1)
        {
            resultCorrected.strFrontBoxNumber = strBackLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet4 == 1)
        {
            resultCorrected.strFrontBoxNumber = strBack;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet5 == 1)
        {
            resultCorrected.strFrontBoxNumber = strFrontLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet6 == 1)
        {
            resultCorrected.strFrontBoxNumber = strBackRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }


    }

    if (nPicNumber == 6 && boxNumberSet.size() >= 6)
    {
        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
        nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
        nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
        nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);
        nRet5 = check.GetBoxNumCheckbit(strFrontLeft, chVerifyCode5);
        nRet6 = check.GetBoxNumCheckbit(strBackRight, chVerifyCode6);
        if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
        {
            if (nRet1 == 1)
            {
                resultCorrected.strFrontBoxNumber = strFront;
                resultCorrected.strFrontBoxModel = strFrontBoxModel;
            }
            else if (nRet2 == 1)
            {
                resultCorrected.strFrontBoxNumber = strFrontRight;
                resultCorrected.strFrontBoxModel = strFrontBoxModel;
            }
            else if (nRet5 == 1)
            {
                resultCorrected.strFrontBoxNumber = strFrontLeft;
                resultCorrected.strFrontBoxModel = strFrontBoxModel;
            }


            if (nRet3 == 1)
            {
                resultCorrected.strBackBoxNumber = strBackLeft;
                resultCorrected.strBackBoxModel = strBackBoxModel;
            }
            else if (nRet4 == 1)
            {
                resultCorrected.strBackBoxNumber = strBack;
                resultCorrected.strBackBoxModel = strBackBoxModel;
            }
            else if (nRet6 == 1)
            {
                resultCorrected.strBackBoxNumber = strBackRight;
                resultCorrected.strBackBoxModel = strBackBoxModel;
            }

            resultCorrected.nPicNumber = 6;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
    }
    //    // 有校验正确的么
    //    if (nRet1 == 1 && nRet2 == 1 && nRet3 == 1 && nRet4 == 1)
    //    {
    //        // never go here
    //        ; // 获取正确的识别结果、箱型、长短箱标志输出
    //    }
    //    else
    //    {
    //                       
    //    }
    if (nPicNumber == 4)
    {
        // 确定箱主代码
        vector<structContaOwnerData> contaOwnerVect;
        GetContaOwnerData(contaOwnerVect);
        bool bFind = false;
        for (int i = 0; i < contaOwnerVect.size(); ++i)
        {
            if (strFront.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
            if (strFrontRight.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
            if (strBack.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
            if (strBackLeft.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
        }
        // call interface
        // bFind = xxxxx;
        // 不存在箱主代码么
        if (!bFind)
        {
            // 修改易错字母、并重新校验- front
            string strFrontTmp = strFront;
            vector<int> posVect;
            for (int i = 0; i < strFrontTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect.push_back(nFrontPos);
                    string strSub = strFrontTmp.substr(nFrontPos + 1);
                    strFrontTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontTmp = strFront;
            for (int i = 0; i < posVect.size() && i < strFrontRight.size(); ++i)
            {
                if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontRight.size())
                {
                    if (strFrontRight[posVect[i]] != '?')
                        strFrontTmp[posVect[i]] = strFrontRight[posVect[i]];
                }
            }
            // modify frontright
            string strFrontRightTmp = strFrontRight;
            vector<int> posVect2;
            for (int i = 0; i < strFrontRightTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontRightTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect2.push_back(nFrontPos);
                    string strSub = strFrontRightTmp.substr(nFrontPos + 1);
                    strFrontRightTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontRightTmp = strFrontRight;
            for (int i = 0; i < posVect2.size() && i < strFront.size(); ++i)
            {
                if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFront.size())
                {
                    if (strFront[posVect2[i]] != '?')
                        strFrontRightTmp[posVect2[i]] = strFront[posVect2[i]];
                }
            }
            // modify backLeft
            string strBackLeftTmp = strBackLeft;
            vector<int> posVect3;
            for (int i = 0; i < strBackLeftTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackLeftTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect3.push_back(nFrontPos);
                    string strSub = strBackLeftTmp.substr(nFrontPos + 1);
                    strBackLeftTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackLeftTmp = strBackLeft;
            for (int i = 0; i < posVect3.size() && i < strBack.size(); ++i)
            {
                if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBack.size())
                {
                    if (strBack[posVect3[i]] != '?')
                        strBackLeftTmp[posVect3[i]] = strBack[posVect3[i]];
                }
            }
            // modify back
            string strBackTmp = strBack;
            vector<int> posVect4;
            for (int i = 0; i < strBackTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect4.push_back(nFrontPos);
                    string strSub = strBackTmp.substr(nFrontPos + 1);
                    strBackTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackTmp = strBackLeft;
            for (int i = 0; i < posVect4.size() && i < strBackLeft.size(); ++i)
            {
                if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackLeft.size())
                {
                    if (strBackLeft[posVect4[i]] != '?')
                        strBackTmp[posVect4[i]] = strBackLeft[posVect4[i]];
                }
            }
            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            if (nPicNumber == 4 && boxNumberSet.size() >= 4)
            {
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
                if (nRet1 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet2 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet3 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet4 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            //            if (nPicNumber == 6 && boxNumberSet.size() >= 6)
            //            {
            //                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            //                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            //                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
            //                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);     
            //            } 
            // 有校验正确的么
            if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
            {
                // never go here
                // 获取正确的识别结果、箱型、长短箱标志输出
            }
            else
            {
                // 从箱号中挑选出一个正确的校验位,再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                // 1. supposing strFront right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                // 2. supposing strFrontRight right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontTmp[strFrontRightTmp.size() - 1] = strFrontRightTmp[strFrontTmp.size() - 1];
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                // 3. supposing strBackTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
                // 4. supposing strBackLeftTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackTmp[strBackLeftTmp.size() - 1] = strBackLeftTmp[strBackTmp.size() - 1];
                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
                // 有校验正确的么
                if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
                {
                    // 获取正确的识别结果、箱型、长短箱标志输出
                    if (nRet1 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strFrontTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    if (nRet2 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    if (nRet3 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strBackLeftTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    if (nRet4 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strBackTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                }
                else
                {
                    // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                    if (nPicNumber == 4)
                    {
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str1tmp = "";
                        int nMixSize = 0;
                        if (str1.size() > 0)
                            nMixSize = str1.size();
                        if (str2.size() > 0)
                            nMixSize = str2.size();
                        if (str3.size() > 0)
                            nMixSize = str3.size();
                        if (str4.size() > 0)
                            nMixSize = str4.size();
                        //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str1.size() > 0 && i < str1.size())
                            {
                                char ch1 = str1[i];
                                mapIter = statisticsMap.find(ch1);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch1, nCount));
                                }
                            }
                            if (str2.size() > 0 && i < str2.size())
                            {
                                char ch2 = str2[i];
                                mapIter = statisticsMap.find(ch2);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch2, nCount));
                                }
                            }
                            if (str3.size() > 0 && i < str3.size())
                            {
                                char ch3 = str3[i];
                                mapIter = statisticsMap.find(ch3);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch3, nCount));
                                }
                            }
                            if (str4.size() > 0 && i < str4.size())
                            {
                                char ch4 = str4[i];
                                mapIter = statisticsMap.find(ch4);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch4, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                //if ((*it).second > nMax)
                                //{
                                //	nMax	= (*it).second;
                                //	tmpCh	= (*it).first;
                                //}
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        // 有校验正确的么
                        if (nRet1 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = "";
                            resultCorrected.strBackBoxModel = "";
                            resultCorrected.nPicNumber = 4;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "1";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = "";
                            resultCorrected.strBackBoxModel = "";
                            resultCorrected.nPicNumber = 4;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 0;
                        }
                    }
                    else if (nPicNumber == 6)
                    {
                        // never go to here
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str5 = strFrontLeft;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str6 = strBackRight;
                        string str1tmp = "";
                        string str2tmp = "";
                        for (size_t i = 0; i < str1.size() && i < str2.size() && i < str5.size(); ++i)
                        {
                            map<char, int> statisticsMap;
                            char ch1 = str1[i];
                            map<char, int>::iterator mapIter = statisticsMap.find(ch1);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch1, nCount));
                            }
                            char ch2 = str2[i];
                            mapIter = statisticsMap.find(ch2);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch2, nCount));
                            }
                            char ch3 = str5[i];
                            mapIter = statisticsMap.find(ch3);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch3, nCount));
                            }
                            int nMax = 0;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    tmpCh = (*it).first;
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        for (size_t i = 0; i < str3.size() && i < str4.size() && i < str6.size(); ++i)
                        {
                            map<char, int> statisticsMap;
                            char ch1 = str3[i];
                            map<char, int>::iterator mapIter = statisticsMap.find(ch1);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch1, nCount));
                            }
                            char ch2 = str4[i];
                            mapIter = statisticsMap.find(ch2);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch2, nCount));
                            }
                            char ch3 = str6[i];
                            mapIter = statisticsMap.find(ch3);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch3, nCount));
                            }
                            int nMax = 0;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    tmpCh = (*it).first;
                                }
                            }
                            str2tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                        // 有校验正确的么
                        if (nRet1 == 1 && nRet2 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                        }
                        else if (nRet1 == 1 && nRet2 == 0)
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                        }
                        else if (nRet1 == 0 && nRet2 == 1)
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                        }
                        else
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                        }
                    }
                }
            }
        }
        else
        {/*
		 // 从箱号中挑选出一个正确的校验位,再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 通过算法算出校验位
		 // 产生不确定的箱号、箱型、长短箱标志
		 }
		 }*/
            // 从箱号中挑选出一个正确的校验位,再次进行校验
            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            // 1. supposing strFront right
            string strFrontRightTmp = strFrontRight;
            string strFrontTmp = strFront;
            if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
            nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            // 2. supposing strFrontRight right
            strFrontRightTmp = strFrontRight;
            strFrontTmp = strFront;
            if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                strFrontTmp[strFrontRightTmp.size() - 1] = strFrontRightTmp[strFrontTmp.size() - 1];
            nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            // 3. supposing strBackTmp right
            string strBackLeftTmp = strBackLeft;
            string strBackTmp = strBack;
            if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
            nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
            // 4. supposing strBackLeftTmp right
            strBackLeftTmp = strBackLeft;
            strBackTmp = strBack;
            if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                strBackTmp[strBackLeftTmp.size() - 1] = strBackLeftTmp[strBackTmp.size() - 1];
            nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
            // 有校验正确的么
            if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
            {
                // 获取正确的识别结果、箱型、长短箱标志输出
                if (nRet1 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet2 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet3 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet4 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            else
            {
                // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                if (nPicNumber == 4)
                {
                    string str1 = strFront;
                    string str2 = strFrontRight;
                    string str3 = strBack;
                    string str4 = strBackLeft;
                    string str1tmp = "";
                    int nMixSize = 0;
                    if (str1.size() > 0)
                        nMixSize = str1.size();
                    if (str2.size() > 0)
                        nMixSize = str2.size();
                    if (str3.size() > 0)
                        nMixSize = str3.size();
                    if (str4.size() > 0)
                        nMixSize = str4.size();
                    //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                    for (size_t i = 0; i < nMixSize; ++i)
                    {
                        map<char, int> statisticsMap;
                        map<char, int>::iterator mapIter;
                        if (str1.size() > 0 && i < str1.size())
                        {
                            char ch1 = str1[i];
                            mapIter = statisticsMap.find(ch1);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch1, nCount));
                            }
                        }
                        if (str2.size() > 0 && i < str2.size())
                        {
                            char ch2 = str2[i];
                            mapIter = statisticsMap.find(ch2);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch2, nCount));
                            }
                        }
                        if (str3.size() > 0 && i < str3.size())
                        {
                            char ch3 = str3[i];
                            mapIter = statisticsMap.find(ch3);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch3, nCount));
                            }
                        }
                        if (str4.size() > 0 && i < str4.size())
                        {
                            char ch4 = str4[i];
                            mapIter = statisticsMap.find(ch4);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch4, nCount));
                            }
                        }
                        int nMax = -1;
                        char tmpCh = ' ';
                        for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                        {
                            if ((*it).second > nMax)
                            {
                                nMax = (*it).second;
                                if (it->first != '?')
                                    tmpCh = (*it).first;
                                else
                                    tmpCh = '?';
                            }
                        }
                        str1tmp += tmpCh;
                    }
                    nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                    // 有校验正确的么
                    if (nRet1 == 1)
                    {
                        // 获取正确的识别结果、箱型、长短箱标志输出
                        resultCorrected.strFrontBoxNumber = str1tmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    else
                    {
                        // 通过算法算出校验位
                        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                        if (str1tmp.size() == 11 && str1tmp[10] == '?')
                            str1tmp[10] = chVerifyCode1;
                        int nTmpIndex = 0;
                        for (int i = 4; i < str1tmp.size(); ++i)
                        {
                            if (str1tmp[i] == '?')
                            {
                                nTmpIndex = i;
                                break;
                            }
                        }
                        if (nTmpIndex != 0)
                        {
                            for (int i = 0; i < 10; ++i)
                            {
                                str1tmp[nTmpIndex] = '0' + i;
                                nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                                if (nRet1 == 1)
                                {
                                    break;
                                }
                            }
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        if (str1tmp.size() == 11 && str1tmp[10] != chVerifyCode1)
                        {
                            str1tmp[10] = chVerifyCode1;
                        }
                        // 产生不确定的箱号、箱型、长短箱标志
                        resultCorrected.strFrontBoxNumber = str1tmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "0";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 0;
                    }
                }
                else if (nPicNumber == 6)
                {
                    // never go to here
                    //string str1	= strFront;
                    //string str2	= strFrontRight;
                    //string str5	= strFrontLeft;
                    //string str3	= strBack;
                    //string str4	= strBackLeft;						
                    //string str6 = strBackRight;
                    //string str1tmp	= "";
                    //string str2tmp	= "";
                    //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str5.size(); ++i)
                    //{
                    //	map<char,int> statisticsMap;
                    //	char ch1	= str1[i];
                    //	map<char,int>::iterator mapIter = statisticsMap.find(ch1);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch1, nCount));
                    //	}
                    //	char ch2	= str2[i];
                    //	mapIter = statisticsMap.find(ch2);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch2, nCount));
                    //	}	
                    //	char ch3	= str5[i];
                    //	mapIter = statisticsMap.find(ch3);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch3, nCount));
                    //	}	
                    //	int nMax = 0;		
                    //	char tmpCh = ' ';
                    //	for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                    //	{			
                    //		if ((*it).second > nMax)
                    //		{
                    //			nMax	= (*it).second;
                    //			tmpCh	= (*it).first;
                    //		}
                    //	}
                    //	str1tmp += tmpCh;		
                    //}                    
                    //for (size_t i = 0; i < str3.size() && i < str4.size() && i < str6.size(); ++i)
                    //{
                    //	map<char,int> statisticsMap;
                    //	char ch1	= str3[i];
                    //	map<char,int>::iterator mapIter = statisticsMap.find(ch1);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch1, nCount));
                    //	}
                    //	char ch2	= str4[i];
                    //	mapIter = statisticsMap.find(ch2);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch2, nCount));
                    //	}	
                    //	char ch3	= str6[i];
                    //	mapIter = statisticsMap.find(ch3);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch3, nCount));
                    //	}	
                    //	int nMax = 0;		
                    //	char tmpCh = ' ';
                    //	for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                    //	{			
                    //		if ((*it).second > nMax)
                    //		{
                    //			nMax	= (*it).second;
                    //			tmpCh	= (*it).first;
                    //		}
                    //	}
                    //	str2tmp += tmpCh;		
                    //}  						
                    //nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                    //nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                    //// 有校验正确的么
                    //if (nRet1 == 1 && nRet2 == 1)
                    //{
                    //	// 获取正确的识别结果、箱型、长短箱标志输出
                    //}
                    //else if (nRet1 == 1 && nRet2 == 0)
                    //{
                    //	// 通过算法算出校验位
                    //	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                    //	// 产生不确定的箱号、箱型、长短箱标志						
                    //}
                    //else if (nRet1 == 0 && nRet2 == 1)
                    //{
                    //	// 通过算法算出校验位
                    //	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                    //	// 产生不确定的箱号、箱型、长短箱标志						
                    //}
                    //else
                    //{
                    //	// 通过算法算出校验位
                    //	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                    //	// 产生不确定的箱号、箱型、长短箱标志
                    //}                         
                }
            }
        }
    }
    else if (nPicNumber == 6)
    {
        // 确定箱主代码
        vector<structContaOwnerData> contaOwnerVect;
        GetContaOwnerData(contaOwnerVect);
        bool bFrontFind = false;
        bool bBackFind = false;
        for (int i = 0; i < contaOwnerVect.size(); ++i)
        {
            if (strFront.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFrontFind = true;
                break;
            }
            if (strFrontRight.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFrontFind = true;
                break;
            }
            if (strFrontLeft.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFrontFind = true;
                break;
            }
        }

        for (int i = 0; i < contaOwnerVect.size(); ++i)
        {
            if (strBack.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bBackFind = true;
                break;
            }
            if (strBackLeft.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bBackFind = true;
                break;
            }
            if (strBackRight.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bBackFind = true;
                break;
            }
        }

        // call interface
        // bFind = xxxxx;
        // 不存在箱主代码么,两个只要有一个不存在，就认为都不存在
        if (!bFrontFind || !bBackFind)
        {
            // 修改易错字母、并重新校验- front
            string strFrontTmp = strFront;
            vector<int> posVect;
            for (int i = 0; i < strFrontTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect.push_back(nFrontPos);
                    string strSub = strFrontTmp.substr(nFrontPos + 1);
                    strFrontTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontTmp = strFront;
            for (int i = 0; i < posVect.size() && i < strFrontRight.size(); ++i)
            {
                if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontRight.size())
                {
                    if (strFrontRight[posVect[i]] != '?')
                        strFrontTmp[posVect[i]] = strFrontRight[posVect[i]];
                }
            }
            strFrontTmp = strFront;
            for (int i = 0; i < posVect.size() && i < strFrontLeft.size(); ++i)
            {
                if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontLeft.size())
                {
                    if (strFrontLeft[posVect[i]] != '?')
                        strFrontTmp[posVect[i]] = strFrontLeft[posVect[i]];
                }
            }

            // modify frontright
            string strFrontRightTmp = strFrontRight;
            vector<int> posVect2;
            for (int i = 0; i < strFrontRightTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontRightTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect2.push_back(nFrontPos);
                    string strSub = strFrontRightTmp.substr(nFrontPos + 1);
                    strFrontRightTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontRightTmp = strFrontRight;
            for (int i = 0; i < posVect2.size() && i < strFront.size(); ++i)
            {
                if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFront.size())
                {
                    if (strFront[posVect2[i]] != '?')
                        strFrontRightTmp[posVect2[i]] = strFront[posVect2[i]];
                }
            }
            strFrontRightTmp = strFrontRight;
            for (int i = 0; i < posVect2.size() && i < strFrontLeft.size(); ++i)
            {
                if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFrontLeft.size())
                {
                    if (strFrontLeft[posVect2[i]] != '?')
                        strFrontRightTmp[posVect2[i]] = strFrontLeft[posVect2[i]];
                }
            }

            // modify frontleft
            string strFrontLeftTmp = strFrontLeft;
            vector<int> posVect5;
            for (int i = 0; i < strFrontLeftTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontLeftTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect5.push_back(nFrontPos);
                    string strSub = strFrontLeftTmp.substr(nFrontPos + 1);
                    strFrontLeftTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontLeftTmp = strFrontLeft;
            for (int i = 0; i < posVect5.size() && i < strFront.size(); ++i)
            {
                if (posVect5[i] < strFrontLeftTmp.size() && posVect5[i] < strFront.size())
                {
                    if (strFront[posVect5[i]] != '?')
                        strFrontLeftTmp[posVect5[i]] = strFront[posVect5[i]];
                }
            }
            strFrontLeftTmp = strFrontLeft;
            for (int i = 0; i < posVect5.size() && i < strFrontRight.size(); ++i)
            {
                if (posVect5[i] < strFrontLeftTmp.size() && posVect5[i] < strFrontRight.size())
                {
                    if (strFrontRight[posVect5[i]] != '?')
                        strFrontLeftTmp[posVect5[i]] = strFrontRight[posVect5[i]];
                }
            }

            // modify backLeft
            string strBackLeftTmp = strBackLeft;
            vector<int> posVect3;
            for (int i = 0; i < strBackLeftTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackLeftTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect3.push_back(nFrontPos);
                    string strSub = strBackLeftTmp.substr(nFrontPos + 1);
                    strBackLeftTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackLeftTmp = strBackLeft;
            for (int i = 0; i < posVect3.size() && i < strBack.size(); ++i)
            {
                if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBack.size())
                {
                    if (strBack[posVect3[i]] != '?')
                        strBackLeftTmp[posVect3[i]] = strBack[posVect3[i]];
                }
            }
            strBackLeftTmp = strBackLeft;
            for (int i = 0; i < posVect3.size() && i < strBackRight.size(); ++i)
            {
                if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBackRight.size())
                {
                    if (strBackRight[posVect3[i]] != '?')
                        strBackLeftTmp[posVect3[i]] = strBackRight[posVect3[i]];
                }
            }

            // modify back
            string strBackTmp = strBack;
            vector<int> posVect4;
            for (int i = 0; i < strBackTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect4.push_back(nFrontPos);
                    string strSub = strBackTmp.substr(nFrontPos + 1);
                    strBackTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackTmp = strBack;
            for (int i = 0; i < posVect4.size() && i < strBackLeft.size(); ++i)
            {
                if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackLeft.size())
                {
                    if (strBackLeft[posVect4[i]] != '?')
                        strBackTmp[posVect4[i]] = strBackLeft[posVect4[i]];
                }
            }
            strBackTmp = strBack;
            for (int i = 0; i < posVect4.size() && i < strBackRight.size(); ++i)
            {
                if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackRight.size())
                {
                    if (strBackRight[posVect4[i]] != '?')
                        strBackTmp[posVect4[i]] = strBackRight[posVect4[i]];
                }
            }

            // modify backRight
            string strBackRightTmp = strBackRight;
            vector<int> posVect6;
            for (int i = 0; i < strBackRightTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackRightTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect6.push_back(nFrontPos);
                    string strSub = strBackRightTmp.substr(nFrontPos + 1);
                    strBackRightTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackRightTmp = strBackRight;
            for (int i = 0; i < posVect6.size() && i < strBack.size(); ++i)
            {
                if (posVect6[i] < strBackRightTmp.size() && posVect6[i] < strBack.size())
                {
                    if (strBack[posVect6[i]] != '?')
                        strBackRightTmp[posVect6[i]] = strBack[posVect6[i]];
                }
            }
            strBackRightTmp = strBackRight;
            for (int i = 0; i < posVect6.size() && i < strBackLeft.size(); ++i)
            {
                if (posVect6[i] < strBackRightTmp.size() && posVect6[i] < strBackLeft.size())
                {
                    if (strBackLeft[posVect6[i]] != '?')
                        strBackRightTmp[posVect6[i]] = strBackLeft[posVect6[i]];
                }
            }

            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            if (nPicNumber == 6) //  && boxNumberSet.size() >= 6)	//2015-2-11
            {
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
                nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            }
            // 有校验正确的么
            if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
            {
                // 获取正确的识别结果、箱型、长短箱标志输出
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    if (nRet1 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontTmp;
                    else if (nRet2 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    else if (nRet5 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    if (nRet3 == 1)
                        resultCorrected.strBackBoxNumber = strBackLeftTmp;
                    else if (nRet4 == 1)
                        resultCorrected.strBackBoxNumber = strBackTmp;
                    else if (nRet6 == 1)
                        resultCorrected.strBackBoxNumber = strBackRightTmp;
                    resultCorrected.strBackBoxModel = strBackBoxModel;
                    resultCorrected.nPicNumber = 6;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            else
            {
                // 从箱号中挑选出一个正确的校验位,再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                // 1. supposing strFront right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                    strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

                // 2. supposing strFrontRight right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                if (nRet5 == 0)
                {
                    if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                        strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                    nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                }

                // 3. supposing strBackTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                    strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);

                // 4. supposing strBackLeftTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                if (nRet6 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                    nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
                }

                // 4. supposing strBackRightTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (nRet3 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                }
                if (nRet4 == 0)
                {
                    if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                        strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                }

                // 有校验正确的么
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    // 获取正确的识别结果、箱型、长短箱标志输出
                    if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                    {
                        if (nRet1 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontTmp;
                        else if (nRet2 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                        else if (nRet5 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        if (nRet3 == 1)
                            resultCorrected.strBackBoxNumber = strBackLeftTmp;
                        else if (nRet4 == 1)
                            resultCorrected.strBackBoxNumber = strBackTmp;
                        else if (nRet6 == 1)
                            resultCorrected.strBackBoxNumber = strBackRightTmp;
                        resultCorrected.strBackBoxModel = strBackBoxModel;
                        resultCorrected.nPicNumber = 6;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                }
                else
                {
                    // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                    if (nPicNumber == 6)
                    {
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str5 = strFrontLeft;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str6 = strBackRight;
                        string str1tmp = "";
                        string str2tmp = "";
                        int nMixSize = 0;
                        if (str1.size() > 0)
                            nMixSize = str1.size();
                        if (str2.size() > 0)
                            nMixSize = str2.size();
                        if (str5.size() > 0)
                            nMixSize = str5.size();
                        //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str1.size() > 0 && i < str1.size())
                            {
                                char ch1 = str1[i];
                                mapIter = statisticsMap.find(ch1);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch1, nCount));
                                }
                            }
                            if (str2.size() > 0 && i < str2.size())
                            {
                                char ch2 = str2[i];
                                mapIter = statisticsMap.find(ch2);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch2, nCount));
                                }
                            }
                            if (str5.size() > 0 && i < str5.size())
                            {
                                char ch5 = str5[i];
                                mapIter = statisticsMap.find(ch5);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch5, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        nMixSize = 0;
                        if (str3.size() > 0)
                            nMixSize = str3.size();
                        if (str4.size() > 0)
                            nMixSize = str4.size();
                        if (str6.size() > 0)
                            nMixSize = str6.size();
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str3.size() > 0 && i < str3.size())
                            {
                                char ch3 = str3[i];
                                mapIter = statisticsMap.find(ch3);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch3, nCount));
                                }
                            }
                            if (str4.size() > 0 && i < str4.size())
                            {
                                char ch4 = str4[i];
                                mapIter = statisticsMap.find(ch4);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch4, nCount));
                                }
                            }
                            if (str6.size() > 0 && i < str6.size())
                            {
                                char ch6 = str6[i];
                                mapIter = statisticsMap.find(ch6);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch6, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str2tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                        // 有校验正确的么
                        if (nRet1 == 1 && nRet2 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            if (nRet2 == 1)
                                resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "1";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 1 && nRet2 == 0)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 0 && nRet2 == 1)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                    }
                }
            }
        }
        else
        {
            //// 从箱号中挑选出一个正确的校验位,再次进行校验
            //if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
            //{
            //// 获取正确的识别结果、箱型、长短箱标志输出                
            //}
            //else
            //{
            //// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
            //if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
            //{
            //// 获取正确的识别结果、箱型、长短箱标志输出                
            //}
            //else
            //{
            //// 通过算法算出校验位
            //// 产生不确定的箱号、箱型、长短箱标志
            //}
            //}
            // 从箱号中挑选出一个正确的校验位,再次进行校验
            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            string strFrontRightTmp = strFrontRight;
            string strFrontTmp = strFront;
            string strFrontLeftTmp = strFrontLeft;
            string strBackLeftTmp = strBackLeft;
            string strBackTmp = strBack;
            string strBackRightTmp = strBackRight;
            nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

            nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
            nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
            nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            //// 1. supposing strFront right
            //strFrontRightTmp	= strFrontRight;
            //strFrontTmp 		= strFront;
            //strFrontLeftTmp		= strFrontLeft;
            //if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
            //	strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
            //nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            //if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
            //	strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
            //nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

            //// 2. supposing strFrontRight right
            //strFrontRightTmp	= strFrontRight;
            //strFrontTmp 		= strFront;
            //strFrontLeftTmp		= strFrontLeft;
            //if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
            //	strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
            //nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            //if (nRet5 == 0)
            //{
            //	if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
            //		strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
            //	nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
            //}
            //// 3. supposing strBackTmp right
            //strBackLeftTmp		= strBackLeft;
            //strBackTmp 			= strBack;
            //strBackRightTmp		= strBackRight;
            //if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
            //	strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
            //nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
            //if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
            //	strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
            //nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            //// 4. supposing strBackLeftTmp right
            //strBackLeftTmp		= strBackLeft;
            //strBackTmp 			= strBack;
            //strBackRightTmp		= strBackRight;
            //if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
            //	strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];								
            //nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
            //if (nRet6 == 0)
            //{
            //	if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
            //		strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
            //	nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            //}
            //// 4. supposing strBackRightTmp right
            //strBackLeftTmp		= strBackLeft;
            //strBackTmp 			= strBack;
            //strBackRightTmp		= strBackRight;
            //if (nRet3 == 0)
            //{
            //	if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
            //		strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];								
            //	nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
            //}
            //if (nRet4 == 0)
            //{
            //	if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
            //		strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
            //	nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
            //}
            // 有校验正确的么
            if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
            {
                // 获取正确的识别结果、箱型、长短箱标志输出
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    if (nRet1 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontTmp;
                    else if (nRet2 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    else if (nRet5 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    if (nRet3 == 1)
                        resultCorrected.strBackBoxNumber = strBackLeftTmp;
                    else if (nRet4 == 1)
                        resultCorrected.strBackBoxNumber = strBackTmp;
                    else if (nRet6 == 1)
                        resultCorrected.strBackBoxNumber = strBackRightTmp;
                    resultCorrected.strBackBoxModel = strBackBoxModel;
                    resultCorrected.nPicNumber = 6;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            else
            {

                // 从箱号中挑选出一个正确的校验位,再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                // 1. supposing strFront right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                    strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                // 2. supposing strFrontRight right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                if (nRet5 == 0)
                {
                    if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                        strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                    nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                }
                // 3. supposing strBackTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                    strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
                // 4. supposing strBackLeftTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                if (nRet6 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                    nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
                }
                // 4. supposing strBackRightTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (nRet3 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                }
                if (nRet4 == 0)
                {
                    if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                        strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                }
                // 有校验正确的么
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    // 获取正确的识别结果、箱型、长短箱标志输出
                    if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                    {
                        if (nRet1 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontTmp;
                        else if (nRet2 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                        else if (nRet5 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        if (nRet3 == 1)
                            resultCorrected.strBackBoxNumber = strBackLeftTmp;
                        else if (nRet4 == 1)
                            resultCorrected.strBackBoxNumber = strBackTmp;
                        else if (nRet6 == 1)
                            resultCorrected.strBackBoxNumber = strBackRightTmp;
                        resultCorrected.strBackBoxModel = strBackBoxModel;
                        resultCorrected.nPicNumber = 6;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                }
                else
                {
                    // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                    if (nPicNumber == 6)
                    {
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str5 = strFrontLeft;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str6 = strBackRight;
                        string str1tmp = "";
                        string str2tmp = "";
                        int nMixSize = 0;
                        if (str1.size() > 0)
                            nMixSize = str1.size();
                        if (str2.size() > 0)
                            nMixSize = str2.size();
                        if (str5.size() > 0)
                            nMixSize = str5.size();
                        //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str1.size() > 0 && i < str1.size())
                            {
                                char ch1 = str1[i];
                                mapIter = statisticsMap.find(ch1);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch1, nCount));
                                }
                            }
                            if (str2.size() > 0 && i < str2.size())
                            {
                                char ch2 = str2[i];
                                mapIter = statisticsMap.find(ch2);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch2, nCount));
                                }
                            }
                            if (str5.size() > 0 && i < str5.size())
                            {
                                char ch5 = str5[i];
                                mapIter = statisticsMap.find(ch5);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch5, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        nMixSize = 0;
                        if (str3.size() > 0)
                            nMixSize = str3.size();
                        if (str4.size() > 0)
                            nMixSize = str4.size();
                        if (str6.size() > 0)
                            nMixSize = str6.size();
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str3.size() > 0 && i < str3.size())
                            {
                                char ch3 = str3[i];
                                mapIter = statisticsMap.find(ch3);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch3, nCount));
                                }
                            }
                            if (str4.size() > 0 && i < str4.size())
                            {
                                char ch4 = str4[i];
                                mapIter = statisticsMap.find(ch4);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch4, nCount));
                                }
                            }
                            if (str6.size() > 0 && i < str6.size())
                            {
                                char ch6 = str6[i];
                                mapIter = statisticsMap.find(ch6);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch6, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str2tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                        // 有校验正确的么
                        if (nRet1 == 1 && nRet2 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            if (nRet2 == 1)
                                resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "1";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 1 && nRet2 == 0)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 0 && nRet2 == 1)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int ProcessImage::BoxNumberCorrection(const std::vector<structBoxNumberRecogResult>& boxNumberSet, int nPicNumber, int nBoxType, structBoxNumberRecogResultCorrected &resultCorrected)
{
    syslog(LOG_DEBUG, "\n");
    syslog(LOG_DEBUG, "%s Box Correction before - PicNumber:%d, BoxType:%d\n", GetCurTime().c_str(), nPicNumber, nBoxType);
    for (size_t i = 0; i < boxNumberSet.size(); ++i)
    {
        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d\n", boxNumberSet[i].strBoxNumber.c_str(), boxNumberSet[i].strBoxModel.c_str(), boxNumberSet[i].strBoxColor.c_str(), \
			boxNumberSet[i].strArrangement.c_str(), (int) boxNumberSet[i].direct);
    }

    // 4副图片 前、前右，后、后左识别结果
    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
    //structBoxNumberRecogResultCorrected resultCorrected;
    if (boxNumberSet.size() <= 0)
    {
        resultCorrected.strFrontBoxNumber = "";
        resultCorrected.strFrontBoxModel = "";
        resultCorrected.strBackBoxNumber = "";
        resultCorrected.strBackBoxModel = "";
        resultCorrected.nPicNumber = 0;
        resultCorrected.nBoxType = 0;
        resultCorrected.strRecogResult = "0";
        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
        return 0;
    }

    int nRet1 = 0;
    int nRet2 = 0;
    int nRet3 = 0;
    int nRet4 = 0;
    int nRet5 = 0;
    int nRet6 = 0;
    char chVerifyCode1 = ' ';
    char chVerifyCode2 = ' ';
    char chVerifyCode3 = ' ';
    char chVerifyCode4 = ' ';
    char chVerifyCode5 = ' ';
    char chVerifyCode6 = ' ';
    string strBoxNumber = "";
    BoxNumberCheckAlgo check;

    string strFront = "";
    string strFrontRight = "";
    string strBack = "";
    string strBackLeft = "";
    string strFrontLeft = "";
    string strBackRight = "";
    string strFrontBoxModel = "";
    string strBackBoxModel = "";

    // add accuracy
    int nFA = 0;
    int nFLA = 0;
    int nFRA = 0;
    int nBA = 0;
    int nBLA = 0;
    int nBRA = 0;
    int nMaxA = 0;


    // accuracy end



    for (int i = 0; i < boxNumberSet.size(); ++i)
    {
        if (boxNumberSet[i].direct == FRONT && boxNumberSet[i].strBoxNumber != "")
        {
            strFront = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strFrontBoxModel = boxNumberSet[i].strBoxModel;
            nFA = boxNumberSet[i].nAccuracy;
        }
        if (boxNumberSet[i].direct == FRONTRIGHT && boxNumberSet[i].strBoxNumber != "")
        {
            strFrontRight = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strFrontBoxModel = boxNumberSet[i].strBoxModel;
            nFRA = boxNumberSet[i].nAccuracy;
        }
        if (boxNumberSet[i].direct == BACK && boxNumberSet[i].strBoxNumber != "")
        {
            strBack = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strBackBoxModel = boxNumberSet[i].strBoxModel;
            nBA = boxNumberSet[i].nAccuracy;
        }
        if (boxNumberSet[i].direct == BACKLEFT && boxNumberSet[i].strBoxNumber != "")
        {
            strBackLeft = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strBackBoxModel = boxNumberSet[i].strBoxModel;
            nBLA = boxNumberSet[i].nAccuracy;
        }
        if (boxNumberSet[i].direct == FRONTLEFT && boxNumberSet[i].strBoxNumber != "")
        {
            strFrontLeft = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strFrontBoxModel = boxNumberSet[i].strBoxModel;
            nFLA = boxNumberSet[i].nAccuracy;
        }
        if (boxNumberSet[i].direct == BACKRIGHT && boxNumberSet[i].strBoxNumber != "")
        {
            strBackRight = boxNumberSet[i].strBoxNumber;
            if (boxNumberSet[i].strBoxModel != "")
                strBackBoxModel = boxNumberSet[i].strBoxModel;
            nBRA = boxNumberSet[i].nAccuracy;
        }
    }
    syslog(LOG_DEBUG, "F,FL,FR,B,BL,BR:%d,%d,%d,%d,%d,%d\n", nFA, nFLA, nFRA, nBA, nBLA, nBRA);

    // 2015-9-27                -start
    if (boxNumberSet.size() < 4)
    {
        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
        nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
        nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
        nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);

        nRet5 = check.GetBoxNumCheckbit(strFrontLeft, chVerifyCode5);
        nRet6 = check.GetBoxNumCheckbit(strBackRight, chVerifyCode6);

        std::string strContaNo = "";
        {
            map<char, structVerifyCode> statisticsVerify;
            map<char, structVerifyCode>::iterator mapIter;

            //2015-11-30
            map<std::string, int> statisticsCount;
            map<std::string, int>::iterator mapIterCount;
            if (nRet1 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode1;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strFront;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strFront);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strFront, nTmpCount));
                }
            }
            if (nRet2 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode2;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strFrontRight;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strFrontRight);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strFrontRight, nTmpCount));
                }
            }
            if (nRet3 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode3;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strBackLeft;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strBackLeft);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strBackLeft, nTmpCount));
                }
            }
            if (nRet4 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode4;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strBack;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strBack);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strBack, nTmpCount));
                }
            }
            if (nRet5 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode5;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strFrontLeft;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strFrontLeft);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strFrontLeft, nTmpCount));
                }
            }

            if (nRet6 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode6;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strBackRight;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strBackRight);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strBackRight, nTmpCount));
                }
            }

            int nMax = 0;

            for (map<char, structVerifyCode>::iterator it = statisticsVerify.begin(); it != statisticsVerify.end(); ++it)
            {
                if ((*it).second.nCount > nMax)
                {
                    nMax = (*it).second.nCount;
                    strContaNo = (*it).second.strContaNo;
                }
            }

            //2015-11-30
            int nMaxPrio = 0;
            std::string strContaNoPrio = "";
            for (map<std::string, int>::iterator it = statisticsCount.begin(); it != statisticsCount.end(); ++it)
            {
                if ((*it).second > nMaxPrio)
                {
                    nMaxPrio = (*it).second;
                    strContaNoPrio = (*it).first;
                }
            }

            if (nMaxPrio <= 1)
            {
                ;
            }
            else
            {
                strContaNo = strContaNoPrio;
            }
        }




        if (nRet1 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strFront;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet2 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strFrontRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet3 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strBackLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet4 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strBack;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet5 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strFrontLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet6 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strBackRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        resultCorrected.strFrontBoxNumber = "";
        resultCorrected.strFrontBoxModel = "";
        resultCorrected.strBackBoxNumber = "";
        resultCorrected.strBackBoxModel = "";
        resultCorrected.nPicNumber = 0;
        resultCorrected.nBoxType = 0;
        resultCorrected.strRecogResult = "0";
        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
        return 0;
    }
    //======================    -end





    if (nPicNumber == 4 && boxNumberSet.size() >= 4)
    {
        int nRetArray[7] = {0};
        int nRetArray2[7] = {0};
        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
        if (nRet1 == 1)
            nRetArray[1] = 1;
        nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
        if (nRet2 == 1)
            nRetArray[2] = 1;
        nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
        if (nRet3 == 1)
            nRetArray[3] = 1;
        nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);
        if (nRet4 == 1)
            nRetArray[4] = 1;
        nRet5 = check.GetBoxNumCheckbit(strFrontLeft, chVerifyCode5);
        if (nRet5 == 1)
            nRetArray[5] = 1;
        nRet6 = check.GetBoxNumCheckbit(strBackRight, chVerifyCode6);
        if (nRet6 == 1)
            nRetArray[6] = 1;

        int nTmpSum = 0;
        for (int i = 0; i < 7; ++i)
        {
            if (nRetArray[i] == 1)
                nTmpSum += 1;
        }


        std::string strContaNo = "";



        if (nTmpSum == 1)
        {
            if (nRet4 == 1)
                strContaNo = strBack;
            else if (nRet1 == 1)
                strContaNo = strFront;
            else
            {
                if (nRet2 == 1)
                {
                    strContaNo = strFrontRight;
                }
                else if (nRet3 == 1)
                {
                    strContaNo = strBackLeft;
                }
                else if (nRet5 == 1)
                {
                    strContaNo = strFrontLeft;
                }
                else if (nRet6 == 1)
                {
                    strContaNo = strBackRight;
                }
            }
        }
        else if (nTmpSum >= 2)
        {
            //                    if (nRetArray[1] == 1)
            //                        nRetArray2[1] = IsInContaOwnerCode(strFront);
            //                    
            //                    if (nRetArray[2] == 1)
            //                        nRetArray2[2] = IsInContaOwnerCode(strFrontRight);
            //                    
            //                    if (nRetArray[3] == 1)
            //                        nRetArray2[3] = IsInContaOwnerCode(strBackLeft);                    
            //                    
            //                    if (nRetArray[4] == 1)
            //                        nRetArray2[4] = IsInContaOwnerCode(strBack);                     
            //                    
            //                    if (nRetArray[5] == 1)
            //                        nRetArray2[5] = IsInContaOwnerCode(strFrontLeft);                      
            //                    
            //                    if (nRetArray[6] == 1)
            //                        nRetArray2[6] = IsInContaOwnerCode(strBackRight);    
            //                    
            //                    int  nTmpSum2 = 0;
            //                    for (int i = 0; i < 7; ++i)
            //                    {
            //                        if (nRetArray2[i] == 1)
            //                            nTmpSum2 += 1;
            //                    }  
            //                    
            //                    if (nTmpSum2 == 0)
            //                    {
            //                        if (nRet4 == 1)
            //                                strContaNo = strBack;
            //                        else if (nRet1 == 1)
            //                                strContaNo = strFront;  
            //                        else
            //                        {
            //                            if (nRet1 == 1)
            //                            {
            //                                    strContaNo = strFront;
            //                            }
            //                            else if (nRet2 == 1)
            //                            {
            //                                    strContaNo = strFrontRight;
            //                            }                            
            //                            else if (nRet3 == 1)
            //                            {
            //                                    strContaNo = strBackLeft;
            //                            } 
            //                            else if (nRet4 == 1)
            //                            {
            //                                    strContaNo = strBack;
            //                            }
            //                            else if (nRet5 == 1)
            //                            {
            //                                    strContaNo = strFrontLeft;
            //                            }
            //                            else if (nRet6 == 1)
            //                            {
            //                                    strContaNo = strBackRight;
            //                            }                            
            //                        }
            //                    }
            //                    else if (nTmpSum2 == 1)
            //                    {
            //                        for (int i = 0; i < 7; ++i)
            //                        {
            //                            if (i == 1 && nRetArray2[i] == 1)
            //                            {
            //                                strContaNo = strFront;
            //                                break;
            //                            }
            //                            if (i == 2 && nRetArray2[i] == 1)
            //                            {
            //                                strContaNo = strFrontRight;
            //                                break;
            //                            }                            
            //                            if (i == 3 && nRetArray2[i] == 1)
            //                            {
            //                                strContaNo = strBackLeft;
            //                                break;
            //                            } 
            //                            if (i == 4 && nRetArray2[i] == 1)
            //                            {
            //                                strContaNo = strBack;
            //                                break;
            //                            }
            //                            if (i == 5 && nRetArray2[i] == 1)
            //                            {
            //                                strContaNo = strFrontLeft;
            //                                break;
            //                            }
            //                            if (i == 6 && nRetArray2[i] == 1)
            //                            {
            //                                strContaNo = strBackRight;
            //                                break;
            //                            }
            //                        }                          
            //                    }
            //                    else if (nTmpSum2 == 2)
            //                    {
            //                        if (nRetArray2[4] == 1)
            //                            strContaNo = strBack;
            //                        else if (nRetArray2[1] == 1)
            //                            strContaNo = strFront;  
            //                        else
            //                        {
            //                            if (nRetArray2[1] == 1)
            //                            {
            //                                strContaNo = strFront;
            //                            }
            //                            else if (nRetArray2[2] == 1)
            //                            {
            //                                strContaNo = strFrontRight;
            //                            }                            
            //                            else if (nRetArray2[3] == 1)
            //                            {
            //                                strContaNo = strBackLeft;
            //                            } 
            //                            else if (nRetArray2[4] == 1)
            //                            {
            //                                strContaNo = strBack;
            //                            }
            //                            else if (nRetArray2[5] == 1)
            //                            {
            //                                strContaNo = strFrontLeft;
            //                            }
            //                            else if (nRetArray2[6] == 1)
            //                            {
            //                                strContaNo = strBackRight;
            //                            }                            
            //                        }
            //                    }  

            syslog(LOG_DEBUG, "F,FL,FR,B,BL,BR:%d,%d,%d,%d,%d,%d,%d\n", nFA, nFLA, nFRA, nBA, nBLA, nBRA, __LINE__);
            if (nRet4 == 1 && nBA > nMaxA)
            {
                strContaNo = strBack;
                nMaxA = nBA;
            }

            if (nRet1 == 1 && nFA > nMaxA)
            {
                strContaNo = strFront;
                nMaxA = nFA;
            }
            if (nRet2 == 1 && nFRA > nMaxA)
            {
                strContaNo = strFrontRight;
                nMaxA = nFRA;
            }
            if (nRet3 == 1 && nBLA > nMaxA)
            {
                strContaNo = strBackLeft;
                nMaxA = nBLA;
            }
            if (nRet5 == 1 && nFLA > nMaxA)
            {
                strContaNo = strFrontLeft;
                nMaxA = nFLA;
            }
            if (nRet6 == 1 && nBRA > nMaxA)
            {
                strContaNo = strBackRight;
                nMaxA = nBRA;
            }

            // 2017/4/27
            map<std::string, int> statisticsCount100;
            map<std::string, int>::iterator mapIterCount100;

            //============================1=====================================
            if (nRet1 == 1 && nFA == 100)
            {
                // 2017/4/27
                mapIterCount100 = statisticsCount100.find(strFront);
                if (mapIterCount100 != statisticsCount100.end())
                {
                    ++(mapIterCount100->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount100.insert(make_pair(strFront, nTmpCount));
                }
            }
            //=============================1======================================
            //============================2=====================================
            if (nRet2 == 1 && nFRA == 100)
            {
                // 2017/4/27
                mapIterCount100 = statisticsCount100.find(strFrontRight);
                if (mapIterCount100 != statisticsCount100.end())
                {
                    ++(mapIterCount100->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount100.insert(make_pair(strFrontRight, nTmpCount));
                }
            }
            //=============================2======================================
            //============================3=====================================
            if (nRet3 == 1 && nBLA == 100)
            {
                // 2017/4/27
                mapIterCount100 = statisticsCount100.find(strBackLeft);
                if (mapIterCount100 != statisticsCount100.end())
                {
                    ++(mapIterCount100->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount100.insert(make_pair(strBackLeft, nTmpCount));
                }
            }
            //=============================3======================================
            //============================4=====================================
            if (nRet4 == 1 && nBA == 100)
            {
                // 2017/4/27
                mapIterCount100 = statisticsCount100.find(strBack);
                if (mapIterCount100 != statisticsCount100.end())
                {
                    ++(mapIterCount100->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount100.insert(make_pair(strBack, nTmpCount));
                }
            }
            //=============================4======================================
            //============================5=====================================
            if (nRet5 == 1 && nFLA == 100)
            {
                // 2017/4/27
                mapIterCount100 = statisticsCount100.find(strFrontLeft);
                if (mapIterCount100 != statisticsCount100.end())
                {
                    ++(mapIterCount100->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount100.insert(make_pair(strFrontLeft, nTmpCount));
                }
            }
            //=============================5======================================
            //============================6=====================================
            if (nRet6 == 1 && nBRA == 100)
            {
                // 2017/4/27
                mapIterCount100 = statisticsCount100.find(strBackRight);
                if (mapIterCount100 != statisticsCount100.end())
                {
                    ++(mapIterCount100->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount100.insert(make_pair(strBackRight, nTmpCount));
                }
            }
            //=============================6======================================
            int nMax100 = 2;

            for (map<std::string, int>::iterator it = statisticsCount100.begin(); it != statisticsCount100.end(); ++it)
            {
                if ((*it).second >= nMax100)
                {
                    nMax100 = (*it).second;
                    strContaNo = (*it).first;
                }
            }
        }
        else
        {
            map<char, structVerifyCode> statisticsVerify;
            map<char, structVerifyCode>::iterator mapIter;

            //2015-11-30
            map<std::string, int> statisticsCount;
            map<std::string, int>::iterator mapIterCount;
            if (nRet1 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode1;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strFront;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strFront);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strFront, nTmpCount));
                }
            }
            if (nRet2 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode2;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strFrontRight;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strFrontRight);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strFrontRight, nTmpCount));
                }
            }
            if (nRet3 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode3;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strBackLeft;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strBackLeft);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strBackLeft, nTmpCount));
                }
            }
            if (nRet4 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode4;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strBack;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strBack);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strBack, nTmpCount));
                }
            }
            if (nRet5 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode5;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strFrontLeft;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strFrontLeft);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strFrontLeft, nTmpCount));
                }
            }

            if (nRet6 == 1)
            {
                structVerifyCode tmpVCode;
                char chTmp = chVerifyCode6;
                mapIter = statisticsVerify.find(chTmp);
                if (mapIter != statisticsVerify.end())
                {
                    ++(mapIter->second.nCount);
                }
                else
                {
                    tmpVCode.chCode = chTmp;
                    tmpVCode.strContaNo = strBackRight;
                    tmpVCode.nCount = 1;
                    statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                }

                // 2015-11-30
                mapIterCount = statisticsCount.find(strBackRight);
                if (mapIterCount != statisticsCount.end())
                {
                    ++(mapIterCount->second);
                }
                else
                {
                    int nTmpCount = 1;
                    statisticsCount.insert(make_pair(strBackRight, nTmpCount));
                }
            }

            int nMax = 0;

            for (map<char, structVerifyCode>::iterator it = statisticsVerify.begin(); it != statisticsVerify.end(); ++it)
            {
                if ((*it).second.nCount > nMax)
                {
                    nMax = (*it).second.nCount;
                    strContaNo = (*it).second.strContaNo;
                }
            }

            //2015-11-30
            int nMaxPrio = 0;
            std::string strContaNoPrio = "";
            for (map<std::string, int>::iterator it = statisticsCount.begin(); it != statisticsCount.end(); ++it)
            {
                if ((*it).second > nMaxPrio)
                {
                    nMaxPrio = (*it).second;
                    strContaNoPrio = (*it).first;
                }
            }

            if (nMaxPrio <= 1)
            {
                ;
            }
            else
            {
                strContaNo = strContaNoPrio;
            }
        }


        if (nRet1 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strFront;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet2 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strFrontRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet3 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strBackLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
        if (nRet4 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strBack;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet5 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strFrontLeft;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }

        if (nRet6 == 1)
        {
            resultCorrected.strFrontBoxNumber = strContaNo; // strBackRight;
            resultCorrected.strFrontBoxModel = strFrontBoxModel;
            resultCorrected.strBackBoxNumber = "";
            resultCorrected.strBackBoxModel = "";
            resultCorrected.nPicNumber = 4;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
    }

    if (nPicNumber == 6 && boxNumberSet.size() >= 6)
    {
        int nRetArray[7] = {0};
        int nRetArray2[7] = {0};
        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
        if (nRet1 == 1)
            nRetArray[1] = 1;

        nRet2 = check.GetBoxNumCheckbit(strFrontRight, chVerifyCode2);
        if (nRet2 == 1)
            nRetArray[2] = 1;

        nRet3 = check.GetBoxNumCheckbit(strBackLeft, chVerifyCode3);
        if (nRet3 == 1)
            nRetArray[3] = 1;

        nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);
        if (nRet4 == 1)
            nRetArray[4] = 1;

        nRet5 = check.GetBoxNumCheckbit(strFrontLeft, chVerifyCode5);
        if (nRet5 == 1)
            nRetArray[5] = 1;
        nRet6 = check.GetBoxNumCheckbit(strBackRight, chVerifyCode6);
        if (nRet6 == 1)
            nRetArray[6] = 1;

        if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
        {
            //			if (nRet1 == 1)
            //			{
            //				resultCorrected.strFrontBoxNumber   = strFront;
            //				resultCorrected.strFrontBoxModel    = strFrontBoxModel;
            //			}
            //			else if (nRet2 == 1)
            //			{
            //				resultCorrected.strFrontBoxNumber   = strFrontRight; 
            //				resultCorrected.strFrontBoxModel    = strFrontBoxModel;
            //			}
            //			else if (nRet5 == 1)
            //			{
            //				resultCorrected.strFrontBoxNumber   = strFrontLeft; 
            //				resultCorrected.strFrontBoxModel    = strFrontBoxModel;
            //			}
            //
            //			
            //			if (nRet3 == 1)
            //			{
            //				resultCorrected.strBackBoxNumber    = strBackLeft;
            //				resultCorrected.strBackBoxModel     = strBackBoxModel;
            //			}
            //			else if (nRet4 == 1)
            //			{
            //				resultCorrected.strBackBoxNumber    = strBack;
            //				resultCorrected.strBackBoxModel     = strBackBoxModel;
            //			}
            //			else if (nRet6 == 1)
            //			{
            //				resultCorrected.strBackBoxNumber    = strBackRight;
            //				resultCorrected.strBackBoxModel     = strBackBoxModel;
            //			}

            {
                map<char, structVerifyCode> statisticsVerify;
                map<char, structVerifyCode>::iterator mapIter;

                //2015-11-30
                map<std::string, int> statisticsCount;
                map<std::string, int>::iterator mapIterCount;
                if (nRet1 == 1)
                {
                    structVerifyCode tmpVCode;
                    char chTmp = chVerifyCode1;
                    mapIter = statisticsVerify.find(chTmp);
                    if (mapIter != statisticsVerify.end())
                    {
                        ++(mapIter->second.nCount);
                    }
                    else
                    {
                        tmpVCode.chCode = chTmp;
                        tmpVCode.strContaNo = strFront;
                        tmpVCode.nCount = 1;
                        statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                    }

                    // 2015-11-30
                    mapIterCount = statisticsCount.find(strFront);
                    if (mapIterCount != statisticsCount.end())
                    {
                        ++(mapIterCount->second);
                    }
                    else
                    {
                        int nTmpCount = 1;
                        statisticsCount.insert(make_pair(strFront, nTmpCount));
                    }
                }
                if (nRet2 == 1)
                {
                    structVerifyCode tmpVCode;
                    char chTmp = chVerifyCode2;
                    mapIter = statisticsVerify.find(chTmp);
                    if (mapIter != statisticsVerify.end())
                    {
                        ++(mapIter->second.nCount);
                    }
                    else
                    {
                        tmpVCode.chCode = chTmp;
                        tmpVCode.strContaNo = strFrontRight;
                        tmpVCode.nCount = 1;
                        statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                    }
                    // 2015-11-30
                    mapIterCount = statisticsCount.find(strFrontRight);
                    if (mapIterCount != statisticsCount.end())
                    {
                        ++(mapIterCount->second);
                    }
                    else
                    {
                        int nTmpCount = 1;
                        statisticsCount.insert(make_pair(strFrontRight, nTmpCount));
                    }
                }
                if (nRet5 == 1)
                {
                    structVerifyCode tmpVCode;
                    char chTmp = chVerifyCode5;
                    mapIter = statisticsVerify.find(chTmp);
                    if (mapIter != statisticsVerify.end())
                    {
                        ++(mapIter->second.nCount);
                    }
                    else
                    {
                        tmpVCode.chCode = chTmp;
                        tmpVCode.strContaNo = strFrontLeft;
                        tmpVCode.nCount = 1;
                        statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                    }
                    // 2015-11-30
                    mapIterCount = statisticsCount.find(strFrontLeft);
                    if (mapIterCount != statisticsCount.end())
                    {
                        ++(mapIterCount->second);
                    }
                    else
                    {
                        int nTmpCount = 1;
                        statisticsCount.insert(make_pair(strFrontLeft, nTmpCount));
                    }
                }
                int nMax = 0;
                std::string strContaNo = "";
                for (map<char, structVerifyCode>::iterator it = statisticsVerify.begin(); it != statisticsVerify.end(); ++it)
                {
                    if ((*it).second.nCount > nMax)
                    {
                        nMax = (*it).second.nCount;
                        strContaNo = (*it).second.strContaNo;
                    }
                }

                //2015-11-30
                int nMaxPrio = 0;
                std::string strContaNoPrio = "";
                for (map<std::string, int>::iterator it = statisticsCount.begin(); it != statisticsCount.end(); ++it)
                {
                    if ((*it).second > nMaxPrio)
                    {
                        nMaxPrio = (*it).second;
                        strContaNoPrio = (*it).first;
                    }
                }

                if (nMaxPrio <= 1)
                {
                    syslog(LOG_DEBUG, "F,FL,FR,B,BL,BR:%d,%d,%d,%d,%d,%d,%d\n", nFA, nFLA, nFRA, nBA, nBLA, nBRA, __LINE__);
                    nMaxA = 0;
                    if (nRet1 == 1 && nFA > nMaxA)
                    {
                        strContaNo = strFront;
                        nMaxA = nFA;
                    }
                    if (nRet2 == 1 && nFRA > nMaxA)
                    {
                        strContaNo = strFrontRight;
                        nMaxA = nFRA;
                    }
                    if (nRet5 == 1 && nFLA > nMaxA)
                    {
                        strContaNo = strFrontLeft;
                        nMaxA = nFLA;
                    }

                    // 2017/4/27
                    map<std::string, int> statisticsCount100;
                    map<std::string, int>::iterator mapIterCount100;

                    //============================1=====================================
                    if (nRet1 == 1 && nFA == 100)
                    {
                        // 2017/4/27
                        mapIterCount100 = statisticsCount100.find(strFront);
                        if (mapIterCount100 != statisticsCount100.end())
                        {
                            ++(mapIterCount100->second);
                        }
                        else
                        {
                            int nTmpCount = 1;
                            statisticsCount100.insert(make_pair(strFront, nTmpCount));
                        }
                    }
                    //=============================1======================================
                    //============================2=====================================
                    if (nRet2 == 1 && nFRA == 100)
                    {
                        // 2017/4/27
                        mapIterCount100 = statisticsCount100.find(strFrontRight);
                        if (mapIterCount100 != statisticsCount100.end())
                        {
                            ++(mapIterCount100->second);
                        }
                        else
                        {
                            int nTmpCount = 1;
                            statisticsCount100.insert(make_pair(strFrontRight, nTmpCount));
                        }
                    }
                    //=============================2======================================
                    //============================5=====================================
                    if (nRet5 == 1 && nFLA == 100)
                    {
                        // 2017/4/27
                        mapIterCount100 = statisticsCount100.find(strFrontLeft);
                        if (mapIterCount100 != statisticsCount100.end())
                        {
                            ++(mapIterCount100->second);
                        }
                        else
                        {
                            int nTmpCount = 1;
                            statisticsCount100.insert(make_pair(strFrontLeft, nTmpCount));
                        }
                    }
                    //=============================5======================================
                    int nMax100 = 2;
                    for (map<std::string, int>::iterator it = statisticsCount100.begin(); it != statisticsCount100.end(); ++it)
                    {
                        if ((*it).second >= nMax100)
                        {
                            nMax100 = (*it).second;
                            strContaNo = (*it).first;
                        }
                    }
                }
                else
                {
                    strContaNo = strContaNoPrio;
                }

                if (nRet1 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strContaNo; // strFront;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                }
                else if (nRet2 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strContaNo; // strFrontRight; 
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                }
                else if (nRet5 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strContaNo; // strFrontLeft; 
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                }
            }


            {
                map<char, structVerifyCode> statisticsVerify;
                map<char, structVerifyCode>::iterator mapIter;

                //2015-11-30
                map<std::string, int> statisticsCount;
                map<std::string, int>::iterator mapIterCount;
                if (nRet3 == 1)
                {
                    structVerifyCode tmpVCode;
                    char chTmp = chVerifyCode3;
                    mapIter = statisticsVerify.find(chTmp);
                    if (mapIter != statisticsVerify.end())
                    {
                        ++(mapIter->second.nCount);
                    }
                    else
                    {
                        tmpVCode.chCode = chTmp;
                        tmpVCode.strContaNo = strBackLeft;
                        tmpVCode.nCount = 1;
                        statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                    }

                    // 2015-11-30
                    mapIterCount = statisticsCount.find(strBackLeft);
                    if (mapIterCount != statisticsCount.end())
                    {
                        ++(mapIterCount->second);
                    }
                    else
                    {
                        int nTmpCount = 1;
                        statisticsCount.insert(make_pair(strBackLeft, nTmpCount));
                    }
                }
                if (nRet4 == 1)
                {
                    structVerifyCode tmpVCode;
                    char chTmp = chVerifyCode4;
                    mapIter = statisticsVerify.find(chTmp);
                    if (mapIter != statisticsVerify.end())
                    {
                        ++(mapIter->second.nCount);
                    }
                    else
                    {
                        tmpVCode.chCode = chTmp;
                        tmpVCode.strContaNo = strBack;
                        tmpVCode.nCount = 1;
                        statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                    }

                    // 2015-11-30
                    mapIterCount = statisticsCount.find(strBack);
                    if (mapIterCount != statisticsCount.end())
                    {
                        ++(mapIterCount->second);
                    }
                    else
                    {
                        int nTmpCount = 1;
                        statisticsCount.insert(make_pair(strBack, nTmpCount));
                    }
                }
                if (nRet6 == 1)
                {
                    structVerifyCode tmpVCode;
                    char chTmp = chVerifyCode6;
                    mapIter = statisticsVerify.find(chTmp);
                    if (mapIter != statisticsVerify.end())
                    {
                        ++(mapIter->second.nCount);
                    }
                    else
                    {
                        tmpVCode.chCode = chTmp;
                        tmpVCode.strContaNo = strBackRight;
                        tmpVCode.nCount = 1;
                        statisticsVerify.insert(make_pair(chTmp, tmpVCode));
                    }

                    // 2015-11-30
                    mapIterCount = statisticsCount.find(strBackRight);
                    if (mapIterCount != statisticsCount.end())
                    {
                        ++(mapIterCount->second);
                    }
                    else
                    {
                        int nTmpCount = 1;
                        statisticsCount.insert(make_pair(strBackRight, nTmpCount));
                    }
                }
                int nMax = 0;
                std::string strContaNo = "";
                for (map<char, structVerifyCode>::iterator it = statisticsVerify.begin(); it != statisticsVerify.end(); ++it)
                {
                    if ((*it).second.nCount > nMax)
                    {
                        nMax = (*it).second.nCount;
                        strContaNo = (*it).second.strContaNo;
                    }
                }

                //2015-11-30
                int nMaxPrio = 0;
                std::string strContaNoPrio = "";
                for (map<std::string, int>::iterator it = statisticsCount.begin(); it != statisticsCount.end(); ++it)
                {
                    if ((*it).second > nMaxPrio)
                    {
                        nMaxPrio = (*it).second;
                        strContaNoPrio = (*it).first;
                    }
                }

                if (nMaxPrio <= 1)
                {
                    syslog(LOG_DEBUG, "F,FL,FR,B,BL,BR:%d,%d,%d,%d,%d,%d,%d\n", nFA, nFLA, nFRA, nBA, nBLA, nBRA, __LINE__);
                    nMaxA = 0;
                    if (nRet4 == 1)
                    {
                        strContaNo = strBack;
                        nMaxA = nBA;
                    }
                    if (nRet3 == 1 && nBLA > nMaxA)
                    {
                        strContaNo = strBackLeft;
                        nMaxA = nBLA;
                    }
                    if (nRet6 == 1 && nBRA > nMaxA)
                    {
                        strContaNo = strBackRight;
                        nMaxA = nBRA;
                    }

                    // 2017/4/27
                    map<std::string, int> statisticsCount100;
                    map<std::string, int>::iterator mapIterCount100;

                    //============================3=====================================
                    if (nRet3 == 1 && nBLA == 100)
                    {
                        // 2017/4/27
                        mapIterCount100 = statisticsCount100.find(strBackLeft);
                        if (mapIterCount100 != statisticsCount100.end())
                        {
                            ++(mapIterCount100->second);
                        }
                        else
                        {
                            int nTmpCount = 1;
                            statisticsCount100.insert(make_pair(strBackLeft, nTmpCount));
                        }
                    }
                    //=============================3======================================
                    //============================4=====================================
                    if (nRet4 == 1 && nBA == 100)
                    {
                        // 2017/4/27
                        mapIterCount100 = statisticsCount100.find(strBack);
                        if (mapIterCount100 != statisticsCount100.end())
                        {
                            ++(mapIterCount100->second);
                        }
                        else
                        {
                            int nTmpCount = 1;
                            statisticsCount100.insert(make_pair(strBack, nTmpCount));
                        }
                    }
                    //=============================4======================================
                    //============================6=====================================
                    if (nRet6 == 1 && nBRA == 100)
                    {
                        // 2017/4/27
                        mapIterCount100 = statisticsCount100.find(strBackRight);
                        if (mapIterCount100 != statisticsCount100.end())
                        {
                            ++(mapIterCount100->second);
                        }
                        else
                        {
                            int nTmpCount = 1;
                            statisticsCount100.insert(make_pair(strBackRight, nTmpCount));
                        }
                    }
                    //=============================6======================================
                    int nMax100 = 2;

                    for (map<std::string, int>::iterator it = statisticsCount100.begin(); it != statisticsCount100.end(); ++it)
                    {
                        if ((*it).second >= nMax100)
                        {
                            nMax100 = (*it).second;
                            strContaNo = (*it).first;
                        }
                    }

                }
                else
                {
                    strContaNo = strContaNoPrio;
                }

                if (nRet3 == 1)
                {
                    resultCorrected.strBackBoxNumber = strContaNo; // strBackLeft;
                    resultCorrected.strBackBoxModel = strBackBoxModel;
                }
                else if (nRet4 == 1)
                {
                    resultCorrected.strBackBoxNumber = strContaNo; // strBack;
                    resultCorrected.strBackBoxModel = strBackBoxModel;
                }
                else if (nRet6 == 1)
                {
                    resultCorrected.strBackBoxNumber = strContaNo; // strBackRight;
                    resultCorrected.strBackBoxModel = strBackBoxModel;
                }
            }


            resultCorrected.nPicNumber = 6;
            resultCorrected.nBoxType = nBoxType;
            resultCorrected.strRecogResult = "1";
            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
            return 1;
        }
    }
    //    // 有校验正确的么
    //    if (nRet1 == 1 && nRet2 == 1 && nRet3 == 1 && nRet4 == 1)
    //    {
    //        // never go here
    //        ; // 获取正确的识别结果、箱型、长短箱标志输出
    //    }
    //    else
    //    {
    //                       
    //    }
    if (nPicNumber == 4)
    {
        // 确定箱主代码
        vector<structContaOwnerData> contaOwnerVect;
        GetContaOwnerData(contaOwnerVect);
        bool bFind = false;
        for (int i = 0; i < contaOwnerVect.size(); ++i)
        {
            if (strFront.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
            if (strFrontRight.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
            if (strBack.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
            if (strBackLeft.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFind = true;
                break;
            }
        }
        // call interface
        // bFind = xxxxx;
        // 不存在箱主代码么
        if (!bFind)
        {
            // 修改易错字母、并重新校验- front
            string strFrontTmp = strFront;
            vector<int> posVect;
            for (int i = 0; i < strFrontTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect.push_back(nFrontPos);
                    string strSub = strFrontTmp.substr(nFrontPos + 1);
                    strFrontTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontTmp = strFront;
            for (int i = 0; i < posVect.size() && i < strFrontRight.size(); ++i)
            {
                if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontRight.size())
                {
                    if (strFrontRight[posVect[i]] != '?')
                        strFrontTmp[posVect[i]] = strFrontRight[posVect[i]];
                }
            }
            // modify frontright
            string strFrontRightTmp = strFrontRight;
            vector<int> posVect2;
            for (int i = 0; i < strFrontRightTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontRightTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect2.push_back(nFrontPos);
                    string strSub = strFrontRightTmp.substr(nFrontPos + 1);
                    strFrontRightTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontRightTmp = strFrontRight;
            for (int i = 0; i < posVect2.size() && i < strFront.size(); ++i)
            {
                if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFront.size())
                {
                    if (strFront[posVect2[i]] != '?')
                        strFrontRightTmp[posVect2[i]] = strFront[posVect2[i]];
                }
            }
            // modify backLeft
            string strBackLeftTmp = strBackLeft;
            vector<int> posVect3;
            for (int i = 0; i < strBackLeftTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackLeftTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect3.push_back(nFrontPos);
                    string strSub = strBackLeftTmp.substr(nFrontPos + 1);
                    strBackLeftTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackLeftTmp = strBackLeft;
            for (int i = 0; i < posVect3.size() && i < strBack.size(); ++i)
            {
                if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBack.size())
                {
                    if (strBack[posVect3[i]] != '?')
                        strBackLeftTmp[posVect3[i]] = strBack[posVect3[i]];
                }
            }
            // modify back
            string strBackTmp = strBack;
            vector<int> posVect4;
            for (int i = 0; i < strBackTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect4.push_back(nFrontPos);
                    string strSub = strBackTmp.substr(nFrontPos + 1);
                    strBackTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackTmp = strBackLeft;
            for (int i = 0; i < posVect4.size() && i < strBackLeft.size(); ++i)
            {
                if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackLeft.size())
                {
                    if (strBackLeft[posVect4[i]] != '?')
                        strBackTmp[posVect4[i]] = strBackLeft[posVect4[i]];
                }
            }
            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            if (nPicNumber == 4 && boxNumberSet.size() >= 4)
            {
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
                if (nRet1 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet2 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet3 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet4 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            //            if (nPicNumber == 6 && boxNumberSet.size() >= 6)
            //            {
            //                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            //                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            //                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
            //                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);     
            //            } 
            // 有校验正确的么
            if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
            {
                // never go here
                // 获取正确的识别结果、箱型、长短箱标志输出
            }
            else
            {
                // 从箱号中挑选出一个正确的校验位,再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                // 1. supposing strFront right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                // 2. supposing strFrontRight right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontTmp[strFrontRightTmp.size() - 1] = strFrontRightTmp[strFrontTmp.size() - 1];
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                // 3. supposing strBackTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
                // 4. supposing strBackLeftTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackTmp[strBackLeftTmp.size() - 1] = strBackLeftTmp[strBackTmp.size() - 1];
                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
                // 有校验正确的么
                if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
                {
                    // 获取正确的识别结果、箱型、长短箱标志输出
                    if (nRet1 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strFrontTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    if (nRet2 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    if (nRet3 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strBackLeftTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    if (nRet4 == 1)
                    {
                        resultCorrected.strFrontBoxNumber = strBackTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                }
                else
                {
                    // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                    if (nPicNumber == 4)
                    {
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str1tmp = "";
                        int nMixSize = 0;
                        if (str1.size() > 0)
                            nMixSize = str1.size();
                        if (str2.size() > 0)
                            nMixSize = str2.size();
                        if (str3.size() > 0)
                            nMixSize = str3.size();
                        if (str4.size() > 0)
                            nMixSize = str4.size();
                        //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str1.size() > 0 && i < str1.size())
                            {
                                char ch1 = str1[i];
                                mapIter = statisticsMap.find(ch1);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch1, nCount));
                                }
                            }
                            if (str2.size() > 0 && i < str2.size())
                            {
                                char ch2 = str2[i];
                                mapIter = statisticsMap.find(ch2);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch2, nCount));
                                }
                            }
                            if (str3.size() > 0 && i < str3.size())
                            {
                                char ch3 = str3[i];
                                mapIter = statisticsMap.find(ch3);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch3, nCount));
                                }
                            }
                            if (str4.size() > 0 && i < str4.size())
                            {
                                char ch4 = str4[i];
                                mapIter = statisticsMap.find(ch4);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch4, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                //if ((*it).second > nMax)
                                //{
                                //	nMax	= (*it).second;
                                //	tmpCh	= (*it).first;
                                //}
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        // 有校验正确的么
                        if (nRet1 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = "";
                            resultCorrected.strBackBoxModel = "";
                            resultCorrected.nPicNumber = 4;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "1";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = "";
                            resultCorrected.strBackBoxModel = "";
                            resultCorrected.nPicNumber = 4;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 0;
                        }
                    }
                    else if (nPicNumber == 6)
                    {
                        // never go to here
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str5 = strFrontLeft;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str6 = strBackRight;
                        string str1tmp = "";
                        string str2tmp = "";
                        for (size_t i = 0; i < str1.size() && i < str2.size() && i < str5.size(); ++i)
                        {
                            map<char, int> statisticsMap;
                            char ch1 = str1[i];
                            map<char, int>::iterator mapIter = statisticsMap.find(ch1);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch1, nCount));
                            }
                            char ch2 = str2[i];
                            mapIter = statisticsMap.find(ch2);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch2, nCount));
                            }
                            char ch3 = str5[i];
                            mapIter = statisticsMap.find(ch3);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch3, nCount));
                            }
                            int nMax = 0;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    tmpCh = (*it).first;
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        for (size_t i = 0; i < str3.size() && i < str4.size() && i < str6.size(); ++i)
                        {
                            map<char, int> statisticsMap;
                            char ch1 = str3[i];
                            map<char, int>::iterator mapIter = statisticsMap.find(ch1);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch1, nCount));
                            }
                            char ch2 = str4[i];
                            mapIter = statisticsMap.find(ch2);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch2, nCount));
                            }
                            char ch3 = str6[i];
                            mapIter = statisticsMap.find(ch3);
                            if (mapIter != statisticsMap.end())
                            {
                                ++(mapIter->second);
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch3, nCount));
                            }
                            int nMax = 0;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    tmpCh = (*it).first;
                                }
                            }
                            str2tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                        // 有校验正确的么
                        if (nRet1 == 1 && nRet2 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                        }
                        else if (nRet1 == 1 && nRet2 == 0)
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                        }
                        else if (nRet1 == 0 && nRet2 == 1)
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                        }
                        else
                        {
                            // 通过算法算出校验位
                            nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                        }
                    }
                }
            }
        }
        else
        {/*
		 // 从箱号中挑选出一个正确的校验位,再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 通过算法算出校验位
		 // 产生不确定的箱号、箱型、长短箱标志
		 }
		 }*/
            // 从箱号中挑选出一个正确的校验位,再次进行校验
            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            // 1. supposing strFront right
            string strFrontRightTmp = strFrontRight;
            string strFrontTmp = strFront;
            if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
            nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            // 2. supposing strFrontRight right
            strFrontRightTmp = strFrontRight;
            strFrontTmp = strFront;
            if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                strFrontTmp[strFrontRightTmp.size() - 1] = strFrontRightTmp[strFrontTmp.size() - 1];
            nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            // 3. supposing strBackTmp right
            string strBackLeftTmp = strBackLeft;
            string strBackTmp = strBack;
            if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
            nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
            // 4. supposing strBackLeftTmp right
            strBackLeftTmp = strBackLeft;
            strBackTmp = strBack;
            if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                strBackTmp[strBackLeftTmp.size() - 1] = strBackLeftTmp[strBackTmp.size() - 1];
            nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
            // 有校验正确的么
            if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
            {
                // 获取正确的识别结果、箱型、长短箱标志输出
                if (nRet1 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet2 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet3 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
                if (nRet4 == 1)
                {
                    resultCorrected.strFrontBoxNumber = strBackTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    resultCorrected.strBackBoxNumber = "";
                    resultCorrected.strBackBoxModel = "";
                    resultCorrected.nPicNumber = 4;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            else
            {
                // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                if (nPicNumber == 4)
                {
                    string str1 = strFront;
                    string str2 = strFrontRight;
                    string str3 = strBack;
                    string str4 = strBackLeft;
                    string str1tmp = "";
                    int nMixSize = 0;
                    if (str1.size() > 0)
                        nMixSize = str1.size();
                    if (str2.size() > 0)
                        nMixSize = str2.size();
                    if (str3.size() > 0)
                        nMixSize = str3.size();
                    if (str4.size() > 0)
                        nMixSize = str4.size();
                    //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                    for (size_t i = 0; i < nMixSize; ++i)
                    {
                        map<char, int> statisticsMap;
                        map<char, int>::iterator mapIter;
                        if (str1.size() > 0 && i < str1.size())
                        {
                            char ch1 = str1[i];
                            mapIter = statisticsMap.find(ch1);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch1, nCount));
                            }
                        }
                        if (str2.size() > 0 && i < str2.size())
                        {
                            char ch2 = str2[i];
                            mapIter = statisticsMap.find(ch2);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch2, nCount));
                            }
                        }
                        if (str3.size() > 0 && i < str3.size())
                        {
                            char ch3 = str3[i];
                            mapIter = statisticsMap.find(ch3);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch3, nCount));
                            }
                        }
                        if (str4.size() > 0 && i < str4.size())
                        {
                            char ch4 = str4[i];
                            mapIter = statisticsMap.find(ch4);
                            if (mapIter != statisticsMap.end() && mapIter->first != '?')
                            {
                                ++(mapIter->second);
                            }
                            else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                            {
                                mapIter->second = 0;
                            }
                            else
                            {
                                int nCount = 1;
                                statisticsMap.insert(make_pair(ch4, nCount));
                            }
                        }
                        int nMax = -1;
                        char tmpCh = ' ';
                        for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                        {
                            if ((*it).second > nMax)
                            {
                                nMax = (*it).second;
                                if (it->first != '?')
                                    tmpCh = (*it).first;
                                else
                                    tmpCh = '?';
                            }
                        }
                        str1tmp += tmpCh;
                    }
                    nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                    // 有校验正确的么
                    if (nRet1 == 1)
                    {
                        // 获取正确的识别结果、箱型、长短箱标志输出
                        resultCorrected.strFrontBoxNumber = str1tmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                    else
                    {
                        // 通过算法算出校验位
                        nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                        if (str1tmp.size() == 11 && str1tmp[10] == '?')
                            str1tmp[10] = chVerifyCode1;
                        int nTmpIndex = 0;
                        for (int i = 4; i < str1tmp.size(); ++i)
                        {
                            if (str1tmp[i] == '?')
                            {
                                nTmpIndex = i;
                                break;
                            }
                        }
                        if (nTmpIndex != 0)
                        {
                            for (int i = 0; i < 10; ++i)
                            {
                                str1tmp[nTmpIndex] = '0' + i;
                                nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                                if (nRet1 == 1)
                                {
                                    break;
                                }
                            }
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        if (str1tmp.size() == 11 && str1tmp[10] != chVerifyCode1)
                        {
                            str1tmp[10] = chVerifyCode1;
                        }
                        // 产生不确定的箱号、箱型、长短箱标志
                        resultCorrected.strFrontBoxNumber = str1tmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        resultCorrected.strBackBoxNumber = "";
                        resultCorrected.strBackBoxModel = "";
                        resultCorrected.nPicNumber = 4;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "0";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 0;
                    }
                }
                else if (nPicNumber == 6)
                {
                    // never go to here
                    //string str1	= strFront;
                    //string str2	= strFrontRight;
                    //string str5	= strFrontLeft;
                    //string str3	= strBack;
                    //string str4	= strBackLeft;						
                    //string str6 = strBackRight;
                    //string str1tmp	= "";
                    //string str2tmp	= "";
                    //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str5.size(); ++i)
                    //{
                    //	map<char,int> statisticsMap;
                    //	char ch1	= str1[i];
                    //	map<char,int>::iterator mapIter = statisticsMap.find(ch1);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch1, nCount));
                    //	}
                    //	char ch2	= str2[i];
                    //	mapIter = statisticsMap.find(ch2);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch2, nCount));
                    //	}	
                    //	char ch3	= str5[i];
                    //	mapIter = statisticsMap.find(ch3);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch3, nCount));
                    //	}	
                    //	int nMax = 0;		
                    //	char tmpCh = ' ';
                    //	for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                    //	{			
                    //		if ((*it).second > nMax)
                    //		{
                    //			nMax	= (*it).second;
                    //			tmpCh	= (*it).first;
                    //		}
                    //	}
                    //	str1tmp += tmpCh;		
                    //}                    
                    //for (size_t i = 0; i < str3.size() && i < str4.size() && i < str6.size(); ++i)
                    //{
                    //	map<char,int> statisticsMap;
                    //	char ch1	= str3[i];
                    //	map<char,int>::iterator mapIter = statisticsMap.find(ch1);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch1, nCount));
                    //	}
                    //	char ch2	= str4[i];
                    //	mapIter = statisticsMap.find(ch2);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch2, nCount));
                    //	}	
                    //	char ch3	= str6[i];
                    //	mapIter = statisticsMap.find(ch3);
                    //	if (mapIter != statisticsMap.end())
                    //	{
                    //		++(mapIter->second);
                    //	}
                    //	else
                    //	{
                    //		int nCount = 1;
                    //		statisticsMap.insert(make_pair(ch3, nCount));
                    //	}	
                    //	int nMax = 0;		
                    //	char tmpCh = ' ';
                    //	for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                    //	{			
                    //		if ((*it).second > nMax)
                    //		{
                    //			nMax	= (*it).second;
                    //			tmpCh	= (*it).first;
                    //		}
                    //	}
                    //	str2tmp += tmpCh;		
                    //}  						
                    //nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                    //nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                    //// 有校验正确的么
                    //if (nRet1 == 1 && nRet2 == 1)
                    //{
                    //	// 获取正确的识别结果、箱型、长短箱标志输出
                    //}
                    //else if (nRet1 == 1 && nRet2 == 0)
                    //{
                    //	// 通过算法算出校验位
                    //	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                    //	// 产生不确定的箱号、箱型、长短箱标志						
                    //}
                    //else if (nRet1 == 0 && nRet2 == 1)
                    //{
                    //	// 通过算法算出校验位
                    //	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                    //	// 产生不确定的箱号、箱型、长短箱标志						
                    //}
                    //else
                    //{
                    //	// 通过算法算出校验位
                    //	nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                    //	// 产生不确定的箱号、箱型、长短箱标志
                    //}                         
                }
            }
        }
    }
    else if (nPicNumber == 6)
    {
        // 确定箱主代码
        vector<structContaOwnerData> contaOwnerVect;
        GetContaOwnerData(contaOwnerVect);
        bool bFrontFind = false;
        bool bBackFind = false;
        for (int i = 0; i < contaOwnerVect.size(); ++i)
        {
            if (strFront.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFrontFind = true;
                break;
            }
            if (strFrontRight.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFrontFind = true;
                break;
            }
            if (strFrontLeft.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bFrontFind = true;
                break;
            }
        }

        for (int i = 0; i < contaOwnerVect.size(); ++i)
        {
            if (strBack.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bBackFind = true;
                break;
            }
            if (strBackLeft.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bBackFind = true;
                break;
            }
            if (strBackRight.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
            {
                bBackFind = true;
                break;
            }
        }

        // call interface
        // bFind = xxxxx;
        // 不存在箱主代码么,两个只要有一个不存在，就认为都不存在
        if (!bFrontFind || !bBackFind)
        {
            // 修改易错字母、并重新校验- front
            string strFrontTmp = strFront;
            vector<int> posVect;
            for (int i = 0; i < strFrontTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect.push_back(nFrontPos);
                    string strSub = strFrontTmp.substr(nFrontPos + 1);
                    strFrontTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontTmp = strFront;
            for (int i = 0; i < posVect.size() && i < strFrontRight.size(); ++i)
            {
                if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontRight.size())
                {
                    if (strFrontRight[posVect[i]] != '?')
                        strFrontTmp[posVect[i]] = strFrontRight[posVect[i]];
                }
            }
            strFrontTmp = strFront;
            for (int i = 0; i < posVect.size() && i < strFrontLeft.size(); ++i)
            {
                if (posVect[i] < strFrontTmp.size() && posVect[i] < strFrontLeft.size())
                {
                    if (strFrontLeft[posVect[i]] != '?')
                        strFrontTmp[posVect[i]] = strFrontLeft[posVect[i]];
                }
            }

            // modify frontright
            string strFrontRightTmp = strFrontRight;
            vector<int> posVect2;
            for (int i = 0; i < strFrontRightTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontRightTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect2.push_back(nFrontPos);
                    string strSub = strFrontRightTmp.substr(nFrontPos + 1);
                    strFrontRightTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontRightTmp = strFrontRight;
            for (int i = 0; i < posVect2.size() && i < strFront.size(); ++i)
            {
                if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFront.size())
                {
                    if (strFront[posVect2[i]] != '?')
                        strFrontRightTmp[posVect2[i]] = strFront[posVect2[i]];
                }
            }
            strFrontRightTmp = strFrontRight;
            for (int i = 0; i < posVect2.size() && i < strFrontLeft.size(); ++i)
            {
                if (posVect2[i] < strFrontRightTmp.size() && posVect2[i] < strFrontLeft.size())
                {
                    if (strFrontLeft[posVect2[i]] != '?')
                        strFrontRightTmp[posVect2[i]] = strFrontLeft[posVect2[i]];
                }
            }

            // modify frontleft
            string strFrontLeftTmp = strFrontLeft;
            vector<int> posVect5;
            for (int i = 0; i < strFrontLeftTmp.size(); ++i)
            {
                string::size_type nFrontPos = strFrontLeftTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect5.push_back(nFrontPos);
                    string strSub = strFrontLeftTmp.substr(nFrontPos + 1);
                    strFrontLeftTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strFrontLeftTmp = strFrontLeft;
            for (int i = 0; i < posVect5.size() && i < strFront.size(); ++i)
            {
                if (posVect5[i] < strFrontLeftTmp.size() && posVect5[i] < strFront.size())
                {
                    if (strFront[posVect5[i]] != '?')
                        strFrontLeftTmp[posVect5[i]] = strFront[posVect5[i]];
                }
            }
            strFrontLeftTmp = strFrontLeft;
            for (int i = 0; i < posVect5.size() && i < strFrontRight.size(); ++i)
            {
                if (posVect5[i] < strFrontLeftTmp.size() && posVect5[i] < strFrontRight.size())
                {
                    if (strFrontRight[posVect5[i]] != '?')
                        strFrontLeftTmp[posVect5[i]] = strFrontRight[posVect5[i]];
                }
            }

            // modify backLeft
            string strBackLeftTmp = strBackLeft;
            vector<int> posVect3;
            for (int i = 0; i < strBackLeftTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackLeftTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect3.push_back(nFrontPos);
                    string strSub = strBackLeftTmp.substr(nFrontPos + 1);
                    strBackLeftTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackLeftTmp = strBackLeft;
            for (int i = 0; i < posVect3.size() && i < strBack.size(); ++i)
            {
                if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBack.size())
                {
                    if (strBack[posVect3[i]] != '?')
                        strBackLeftTmp[posVect3[i]] = strBack[posVect3[i]];
                }
            }
            strBackLeftTmp = strBackLeft;
            for (int i = 0; i < posVect3.size() && i < strBackRight.size(); ++i)
            {
                if (posVect3[i] < strBackLeftTmp.size() && posVect3[i] < strBackRight.size())
                {
                    if (strBackRight[posVect3[i]] != '?')
                        strBackLeftTmp[posVect3[i]] = strBackRight[posVect3[i]];
                }
            }

            // modify back
            string strBackTmp = strBack;
            vector<int> posVect4;
            for (int i = 0; i < strBackTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect4.push_back(nFrontPos);
                    string strSub = strBackTmp.substr(nFrontPos + 1);
                    strBackTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackTmp = strBack;
            for (int i = 0; i < posVect4.size() && i < strBackLeft.size(); ++i)
            {
                if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackLeft.size())
                {
                    if (strBackLeft[posVect4[i]] != '?')
                        strBackTmp[posVect4[i]] = strBackLeft[posVect4[i]];
                }
            }
            strBackTmp = strBack;
            for (int i = 0; i < posVect4.size() && i < strBackRight.size(); ++i)
            {
                if (posVect4[i] < strBackTmp.size() && posVect4[i] < strBackRight.size())
                {
                    if (strBackRight[posVect4[i]] != '?')
                        strBackTmp[posVect4[i]] = strBackRight[posVect4[i]];
                }
            }

            // modify backRight
            string strBackRightTmp = strBackRight;
            vector<int> posVect6;
            for (int i = 0; i < strBackRightTmp.size(); ++i)
            {
                string::size_type nFrontPos = strBackRightTmp.find('?');
                if (nFrontPos != string::npos)
                {
                    posVect6.push_back(nFrontPos);
                    string strSub = strBackRightTmp.substr(nFrontPos + 1);
                    strBackRightTmp = strSub;
                    i = 0;
                }
                else
                {
                    break;
                }
            }
            strBackRightTmp = strBackRight;
            for (int i = 0; i < posVect6.size() && i < strBack.size(); ++i)
            {
                if (posVect6[i] < strBackRightTmp.size() && posVect6[i] < strBack.size())
                {
                    if (strBack[posVect6[i]] != '?')
                        strBackRightTmp[posVect6[i]] = strBack[posVect6[i]];
                }
            }
            strBackRightTmp = strBackRight;
            for (int i = 0; i < posVect6.size() && i < strBackLeft.size(); ++i)
            {
                if (posVect6[i] < strBackRightTmp.size() && posVect6[i] < strBackLeft.size())
                {
                    if (strBackLeft[posVect6[i]] != '?')
                        strBackRightTmp[posVect6[i]] = strBackLeft[posVect6[i]];
                }
            }

            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            if (nPicNumber == 6) //  && boxNumberSet.size() >= 6)	//2015-2-11
            {
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
                nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
                nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            }
            // 有校验正确的么
            if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
            {
                // 获取正确的识别结果、箱型、长短箱标志输出
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    if (nRet1 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontTmp;
                    else if (nRet2 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    else if (nRet5 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    if (nRet3 == 1)
                        resultCorrected.strBackBoxNumber = strBackLeftTmp;
                    else if (nRet4 == 1)
                        resultCorrected.strBackBoxNumber = strBackTmp;
                    else if (nRet6 == 1)
                        resultCorrected.strBackBoxNumber = strBackRightTmp;
                    resultCorrected.strBackBoxModel = strBackBoxModel;
                    resultCorrected.nPicNumber = 6;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            else
            {
                // 从箱号中挑选出一个正确的校验位,再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                // 1. supposing strFront right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                    strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

                // 2. supposing strFrontRight right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                if (nRet5 == 0)
                {
                    if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                        strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                    nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                }

                // 3. supposing strBackTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                    strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);

                // 4. supposing strBackLeftTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                if (nRet6 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                    nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
                }

                // 4. supposing strBackRightTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (nRet3 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                }
                if (nRet4 == 0)
                {
                    if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                        strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                }

                // 有校验正确的么
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    // 获取正确的识别结果、箱型、长短箱标志输出
                    if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                    {
                        if (nRet1 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontTmp;
                        else if (nRet2 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                        else if (nRet5 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        if (nRet3 == 1)
                            resultCorrected.strBackBoxNumber = strBackLeftTmp;
                        else if (nRet4 == 1)
                            resultCorrected.strBackBoxNumber = strBackTmp;
                        else if (nRet6 == 1)
                            resultCorrected.strBackBoxNumber = strBackRightTmp;
                        resultCorrected.strBackBoxModel = strBackBoxModel;
                        resultCorrected.nPicNumber = 6;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                }
                else
                {
                    // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                    if (nPicNumber == 6)
                    {
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str5 = strFrontLeft;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str6 = strBackRight;
                        string str1tmp = "";
                        string str2tmp = "";
                        int nMixSize = 0;
                        if (str1.size() > 0)
                            nMixSize = str1.size();
                        if (str2.size() > 0)
                            nMixSize = str2.size();
                        if (str5.size() > 0)
                            nMixSize = str5.size();
                        //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str1.size() > 0 && i < str1.size())
                            {
                                char ch1 = str1[i];
                                mapIter = statisticsMap.find(ch1);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch1, nCount));
                                }
                            }
                            if (str2.size() > 0 && i < str2.size())
                            {
                                char ch2 = str2[i];
                                mapIter = statisticsMap.find(ch2);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch2, nCount));
                                }
                            }
                            if (str5.size() > 0 && i < str5.size())
                            {
                                char ch5 = str5[i];
                                mapIter = statisticsMap.find(ch5);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch5, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        nMixSize = 0;
                        if (str3.size() > 0)
                            nMixSize = str3.size();
                        if (str4.size() > 0)
                            nMixSize = str4.size();
                        if (str6.size() > 0)
                            nMixSize = str6.size();
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str3.size() > 0 && i < str3.size())
                            {
                                char ch3 = str3[i];
                                mapIter = statisticsMap.find(ch3);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch3, nCount));
                                }
                            }
                            if (str4.size() > 0 && i < str4.size())
                            {
                                char ch4 = str4[i];
                                mapIter = statisticsMap.find(ch4);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch4, nCount));
                                }
                            }
                            if (str6.size() > 0 && i < str6.size())
                            {
                                char ch6 = str6[i];
                                mapIter = statisticsMap.find(ch6);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch6, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str2tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                        // 有校验正确的么
                        if (nRet1 == 1 && nRet2 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            if (nRet2 == 1)
                                resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "1";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 1 && nRet2 == 0)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 0 && nRet2 == 1)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                    }
                }
            }
        }
        else
        {
            //// 从箱号中挑选出一个正确的校验位,再次进行校验
            //if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
            //{
            //// 获取正确的识别结果、箱型、长短箱标志输出                
            //}
            //else
            //{
            //// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
            //if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
            //{
            //// 获取正确的识别结果、箱型、长短箱标志输出                
            //}
            //else
            //{
            //// 通过算法算出校验位
            //// 产生不确定的箱号、箱型、长短箱标志
            //}
            //}
            // 从箱号中挑选出一个正确的校验位,再次进行校验
            // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
            string strFrontRightTmp = strFrontRight;
            string strFrontTmp = strFront;
            string strFrontLeftTmp = strFrontLeft;
            string strBackLeftTmp = strBackLeft;
            string strBackTmp = strBack;
            string strBackRightTmp = strBackRight;
            nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

            nRet3 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode3);
            nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
            nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            //// 1. supposing strFront right
            //strFrontRightTmp	= strFrontRight;
            //strFrontTmp 		= strFront;
            //strFrontLeftTmp		= strFrontLeft;
            //if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
            //	strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
            //nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
            //if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
            //	strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
            //nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);

            //// 2. supposing strFrontRight right
            //strFrontRightTmp	= strFrontRight;
            //strFrontTmp 		= strFront;
            //strFrontLeftTmp		= strFrontLeft;
            //if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
            //	strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
            //nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
            //if (nRet5 == 0)
            //{
            //	if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
            //		strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
            //	nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
            //}
            //// 3. supposing strBackTmp right
            //strBackLeftTmp		= strBackLeft;
            //strBackTmp 			= strBack;
            //strBackRightTmp		= strBackRight;
            //if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
            //	strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
            //nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
            //if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
            //	strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
            //nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            //// 4. supposing strBackLeftTmp right
            //strBackLeftTmp		= strBackLeft;
            //strBackTmp 			= strBack;
            //strBackRightTmp		= strBackRight;
            //if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
            //	strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];								
            //nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
            //if (nRet6 == 0)
            //{
            //	if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
            //		strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
            //	nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
            //}
            //// 4. supposing strBackRightTmp right
            //strBackLeftTmp		= strBackLeft;
            //strBackTmp 			= strBack;
            //strBackRightTmp		= strBackRight;
            //if (nRet3 == 0)
            //{
            //	if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
            //		strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];								
            //	nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3); 
            //}
            //if (nRet4 == 0)
            //{
            //	if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
            //		strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
            //	nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
            //}
            // 有校验正确的么
            if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
            {
                // 获取正确的识别结果、箱型、长短箱标志输出
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    if (nRet1 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontTmp;
                    else if (nRet2 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                    else if (nRet5 == 1)
                        resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                    resultCorrected.strFrontBoxModel = strFrontBoxModel;
                    if (nRet3 == 1)
                        resultCorrected.strBackBoxNumber = strBackLeftTmp;
                    else if (nRet4 == 1)
                        resultCorrected.strBackBoxNumber = strBackTmp;
                    else if (nRet6 == 1)
                        resultCorrected.strBackBoxNumber = strBackRightTmp;
                    resultCorrected.strBackBoxModel = strBackBoxModel;
                    resultCorrected.nPicNumber = 6;
                    resultCorrected.nBoxType = nBoxType;
                    resultCorrected.strRecogResult = "1";
                    syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                    syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                            resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                    return 1;
                }
            }
            else
            {

                // 从箱号中挑选出一个正确的校验位,再次进行校验
                // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                // 1. supposing strFront right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontRightTmp[strFrontRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet2 = check.GetBoxNumCheckbit(strFrontRightTmp, chVerifyCode2);
                if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                    strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
                nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                // 2. supposing strFrontRight right
                strFrontRightTmp = strFrontRight;
                strFrontTmp = strFront;
                strFrontLeftTmp = strFrontLeft;
                if (strFrontRightTmp != "" && strFrontTmp != "" && strFrontRightTmp.size() == strFrontTmp.size())
                    strFrontTmp[strFrontTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
                if (nRet5 == 0)
                {
                    if (strFrontLeftTmp != "" && strFrontTmp != "" && strFrontLeftTmp.size() == strFrontTmp.size())
                        strFrontLeftTmp[strFrontLeftTmp.size() - 1] = strFrontRightTmp[strFrontRightTmp.size() - 1];
                    nRet5 = check.GetBoxNumCheckbit(strFrontLeftTmp, chVerifyCode5);
                }
                // 3. supposing strBackTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                    strBackRightTmp[strBackRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
                nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
                // 4. supposing strBackLeftTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                    strBackTmp[strBackTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                if (nRet6 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackRightTmp[strBackRightTmp.size() - 1] = strBackLeftTmp[strBackLeftTmp.size() - 1];
                    nRet6 = check.GetBoxNumCheckbit(strBackRightTmp, chVerifyCode6);
                }
                // 4. supposing strBackRightTmp right
                strBackLeftTmp = strBackLeft;
                strBackTmp = strBack;
                strBackRightTmp = strBackRight;
                if (nRet3 == 0)
                {
                    if (strBackRightTmp != "" && strBackTmp != "" && strBackRightTmp.size() == strBackTmp.size())
                        strBackTmp[strBackTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);
                }
                if (nRet4 == 0)
                {
                    if (strBackLeftTmp != "" && strBackTmp != "" && strBackLeftTmp.size() == strBackTmp.size())
                        strBackLeftTmp[strBackLeftTmp.size() - 1] = strBackRightTmp[strBackRightTmp.size() - 1];
                    nRet4 = check.GetBoxNumCheckbit(strBackLeftTmp, chVerifyCode4);
                }
                // 有校验正确的么
                if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                {
                    // 获取正确的识别结果、箱型、长短箱标志输出
                    if ((nRet1 == 1 || nRet2 == 1 || nRet5 == 1) && (nRet3 == 1 || nRet4 == 1 || nRet6 == 1))
                    {
                        if (nRet1 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontTmp;
                        else if (nRet2 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontRightTmp;
                        else if (nRet5 == 1)
                            resultCorrected.strFrontBoxNumber = strFrontLeftTmp;
                        resultCorrected.strFrontBoxModel = strFrontBoxModel;
                        if (nRet3 == 1)
                            resultCorrected.strBackBoxNumber = strBackLeftTmp;
                        else if (nRet4 == 1)
                            resultCorrected.strBackBoxNumber = strBackTmp;
                        else if (nRet6 == 1)
                            resultCorrected.strBackBoxNumber = strBackRightTmp;
                        resultCorrected.strBackBoxModel = strBackBoxModel;
                        resultCorrected.nPicNumber = 6;
                        resultCorrected.nBoxType = nBoxType;
                        resultCorrected.strRecogResult = "1";
                        syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                        syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                        return 1;
                    }
                }
                else
                {
                    // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
                    // 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
                    if (nPicNumber == 6)
                    {
                        string str1 = strFront;
                        string str2 = strFrontRight;
                        string str5 = strFrontLeft;
                        string str3 = strBack;
                        string str4 = strBackLeft;
                        string str6 = strBackRight;
                        string str1tmp = "";
                        string str2tmp = "";
                        int nMixSize = 0;
                        if (str1.size() > 0)
                            nMixSize = str1.size();
                        if (str2.size() > 0)
                            nMixSize = str2.size();
                        if (str5.size() > 0)
                            nMixSize = str5.size();
                        //for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str1.size() > 0 && i < str1.size())
                            {
                                char ch1 = str1[i];
                                mapIter = statisticsMap.find(ch1);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch1, nCount));
                                }
                            }
                            if (str2.size() > 0 && i < str2.size())
                            {
                                char ch2 = str2[i];
                                mapIter = statisticsMap.find(ch2);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch2, nCount));
                                }
                            }
                            if (str5.size() > 0 && i < str5.size())
                            {
                                char ch5 = str5[i];
                                mapIter = statisticsMap.find(ch5);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch5, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str1tmp += tmpCh;
                        }
                        nMixSize = 0;
                        if (str3.size() > 0)
                            nMixSize = str3.size();
                        if (str4.size() > 0)
                            nMixSize = str4.size();
                        if (str6.size() > 0)
                            nMixSize = str6.size();
                        for (size_t i = 0; i < nMixSize; ++i)
                        {
                            map<char, int> statisticsMap;
                            map<char, int>::iterator mapIter;
                            if (str3.size() > 0 && i < str3.size())
                            {
                                char ch3 = str3[i];
                                mapIter = statisticsMap.find(ch3);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch3, nCount));
                                }
                            }
                            if (str4.size() > 0 && i < str4.size())
                            {
                                char ch4 = str4[i];
                                mapIter = statisticsMap.find(ch4);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch4, nCount));
                                }
                            }
                            if (str6.size() > 0 && i < str6.size())
                            {
                                char ch6 = str6[i];
                                mapIter = statisticsMap.find(ch6);
                                if (mapIter != statisticsMap.end() && mapIter->first != '?')
                                {
                                    ++(mapIter->second);
                                }
                                else if (mapIter != statisticsMap.end() && mapIter->first == '?')
                                {
                                    mapIter->second = 0;
                                }
                                else
                                {
                                    int nCount = 1;
                                    statisticsMap.insert(make_pair(ch6, nCount));
                                }
                            }
                            int nMax = -1;
                            char tmpCh = ' ';
                            for (map<char, int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
                            {
                                if ((*it).second > nMax)
                                {
                                    nMax = (*it).second;
                                    if (it->first != '?')
                                        tmpCh = (*it).first;
                                    else
                                        tmpCh = '?';
                                }
                            }
                            str2tmp += tmpCh;
                        }
                        nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
                        nRet2 = check.GetBoxNumCheckbit(str2tmp, chVerifyCode2);
                        // 有校验正确的么
                        if (nRet1 == 1 && nRet2 == 1)
                        {
                            // 获取正确的识别结果、箱型、长短箱标志输出
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            if (nRet2 == 1)
                                resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "1";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 1 && nRet2 == 0)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            if (nRet1 == 1)
                                resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else if (nRet1 == 0 && nRet2 == 1)
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志						
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                        else
                        {
                            // 通过算法算出校验位
                            //nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
                            // 产生不确定的箱号、箱型、长短箱标志
                            resultCorrected.strFrontBoxNumber = str1tmp;
                            resultCorrected.strFrontBoxModel = strFrontBoxModel;
                            resultCorrected.strBackBoxNumber = str2tmp;
                            resultCorrected.strBackBoxModel = strBackBoxModel;
                            resultCorrected.nPicNumber = 6;
                            resultCorrected.nBoxType = nBoxType;
                            resultCorrected.strRecogResult = "0";
                            syslog(LOG_DEBUG, "\n%s Box Correction after:", GetCurTime().c_str());
                            syslog(LOG_DEBUG, "%s,%s,%s,%s,%d,%d,%d\n", resultCorrected.strFrontBoxNumber.c_str(), resultCorrected.strFrontBoxModel.c_str(),
                                    resultCorrected.strBackBoxNumber.c_str(), resultCorrected.strBackBoxModel.c_str(), resultCorrected.nPicNumber, resultCorrected.nBoxType, __LINE__);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void ProcessImage::GetContaOwnerData(std::vector<structContaOwnerData> &contaOwnerVect)
{
    int i = 0;
    char szLine[256] = {0};
    FILE *pFile = fopen("/etc/ContaOwnerData.txt", "r");
    if (pFile == NULL)
    {
        return;
    }
    else
    {
        do
        {
            memset(szLine, 0x00, sizeof (szLine));
            if (fgets(szLine, 255, pFile) == NULL)
            {
                break;
            }
            else
            {
                if (szLine[0] == '\n')
                {
                    memset(szLine, 0x00, sizeof (szLine));
                    continue;
                }
                char *p = NULL;
                char *pStart = szLine;
                if (pStart != NULL)
                    p = strchr(pStart, ',');
                else
                    break;
                if (p != NULL)
                {
                    structContaOwnerData tmpData;
                    *p = '\0';
                    if (i == 0)
                    {
                        tmpData.nLineNo = atoi(p - 1);
                    }
                    else
                    {
                        tmpData.nLineNo = atoi(pStart);
                    }
                    pStart = p + 1;
                    p = strchr(pStart, ',');
                    if (p != NULL)
                    {
                        char *pTemp = NULL;
                        pTemp = strchr(pStart, ',');
                        if (pTemp != NULL)
                        {
                            *pTemp = '\0';
                            tmpData.strCompanyCode = pStart;
                        }
                        *p = '\0';
                        pStart = p + 1;
                        p = strchr(pStart, '\n');
                        if (p != NULL)
                        {
                            *p = '\0';
                            tmpData.strCompanyName = pStart;
                        }
                    }
                    contaOwnerVect.push_back(tmpData);
                    //printf("%d, %s, %s\n", tmpData.nLineNo, tmpData.strCompanyCode.c_str(), tmpData.strCompanyName.c_str());
                }
                i++;
            }
        } while (1);
    }
    fclose(pFile);
}
//int ProcessImage::SetReadDataCallback(_CAMERA_SNAPPER_DATA_CALLBACK pReadDataCallback, void* pUserData)
//{
//	//SetRecoResultCallback(pReadDataCallback);
//	return 0;
//}

std::string ProcessImage::Byte2String(BYTE ch)
{
    BYTE a = ch;
    char buf[128] = {0};
    char tmp[10] = {0};
    char sav[10] = {0};
    //for (int i = 0; i < sizeof(a) / sizeof(BYTE); i++)
    for (int i = 0; i < 1; i++)
    {
        int j = 0;
        // itoa((int)a[i], sav, 2);
        my_itoa((int) a, sav, 2);
        for (int w = 0; w < 8 - strlen(sav); w++)
            tmp[w] = '0';
        for (int w = 8 - strlen(sav); w < 8; w++)
            tmp[w] = sav[j++];
        sprintf(buf, "%8s", tmp);
        //sprintf(buf, "%s%8s", buf, tmp);
        string strRet = buf;
        return strRet;
    }
    return "";
}

int ProcessImage::my_itoa(int val, char* buf, int radix)
{
    //const int radix = 2;
    char* p;
    int a; //every digit
    int len;
    char* b; //start of the digit char
    char temp;
    p = buf;
    if (val < 0)
    {
        *p++ = '-';
        val = 0 - val;
    }
    b = p;
    do
    {
        a = val % radix;
        val /= radix;
        *p++ = a + '0';
    } while (val > 0);
    len = (int) (p - buf);
    *p-- = 0;
    //swap
    do
    {
        temp = *p;
        *p = *b;
        *b = temp;
        --p;
        ++b;
    } while (b < p);
    return len;
}

string ProcessImage::GetCurTime(void)
{
    time_t t = time(0);
    tm *ld = NULL;
    char tmp[64] = "";
    ld = localtime(&t);
    strftime(tmp, sizeof (tmp), "%Y-%m-%d %H:%M:%S", ld);
    return string(tmp);
}

long ProcessImage::getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    //return tv.tv_sec * 1000 + tv.tv_usec / 1000; 
    return tv.tv_sec;
}

int ProcessImage::set_keep_live(int nSocket, int keep_alive_times, int keep_alive_interval)
{

#ifdef WIN32                                //WIndows下
    TCP_KEEPALIVE inKeepAlive = {0}; //输入参数
    unsigned long ulInLen = sizeof (TCP_KEEPALIVE);

    TCP_KEEPALIVE outKeepAlive = {0}; //输出参数
    unsigned long ulOutLen = sizeof (TCP_KEEPALIVE);

    unsigned long ulBytesReturn = 0;

    //设置socket的keep alive为5秒，并且发送次数为3次
    inKeepAlive.on_off = keep_alive_times;
    inKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //两次KeepAlive探测间的时间间隔
    inKeepAlive.keep_alive_time = keep_alive_interval * 1000; //开始首次KeepAlive探测前的TCP空闭时间


    outKeepAlive.on_off = keep_alive_times;
    outKeepAlive.keep_alive_interval = keep_alive_interval * 1000; //两次KeepAlive探测间的时间间隔
    outKeepAlive.keep_alive_time = keep_alive_interval * 1000; //开始首次KeepAlive探测前的TCP空闭时间


    if (WSAIoctl((unsigned int) nSocket, SIO_KEEPALIVE_VALS,
            (LPVOID) & inKeepAlive, ulInLen,
            (LPVOID) & outKeepAlive, ulOutLen,
            &ulBytesReturn, NULL, NULL) == SOCKET_ERROR)
    {
        //ACE_DEBUG((LM_INFO,
        //	ACE_TEXT("(%P|%t) WSAIoctl failed. error code(%d)!\n"), WSAGetLastError()));
    }

#else                                        //linux下
    int keepAlive = 1; //设定KeepAlive
    int keepIdle = keep_alive_interval; //开始首次KeepAlive探测前的TCP空闭时间
    int keepInterval = keep_alive_interval; //两次KeepAlive探测间的时间间隔
    int keepCount = keep_alive_times; //判定断开前的KeepAlive探测次数
    if (setsockopt(nSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*) & keepAlive, sizeof (keepAlive)) == -1)
    {
        syslog(LOG_DEBUG, "setsockopt SO_KEEPALIVE error!\n");
    }
    if (setsockopt(nSocket, SOL_TCP, TCP_KEEPIDLE, (const char *) & keepIdle, sizeof (keepIdle)) == -1)
    {
        syslog(LOG_DEBUG, "setsockopt TCP_KEEPIDLE error!\n");
    }
    if (setsockopt(nSocket, SOL_TCP, TCP_KEEPINTVL, (const char *) & keepInterval, sizeof (keepInterval)) == -1)
    {
        syslog(LOG_DEBUG, "setsockopt TCP_KEEPINTVL error!\n");
    }
    if (setsockopt(nSocket, SOL_TCP, TCP_KEEPCNT, (const char *) & keepCount, sizeof (keepCount)) == -1)
    {
        syslog(LOG_DEBUG, "setsockopt TCP_KEEPCNT error!\n");
    }

#endif

    return 0;

}

int ProcessImage::IsInContaOwnerCode(const std::string &strBoxNumber)
{
    // 确定箱主代码
    vector<structContaOwnerData> contaOwnerVect;
    GetContaOwnerData(contaOwnerVect);
    bool bFind = false;
    for (int i = 0; i < contaOwnerVect.size(); ++i)
    {
        if (strBoxNumber.substr(0, 3) == contaOwnerVect[i].strCompanyCode)
        {
            bFind = true;
            break;
        }
    }

    if (!bFind)
        return 0;
    return 1;
}

