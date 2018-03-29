// main.cpp : Defines the entry point for the console application.
//
#include "ace/Signal.h"
#include "SimpleConfig.h"
#include "ace/streams.h"
#include "ace/Thread_Manager.h"
#include "ace/Select_Reactor.h"
#include "MSGHandleCenter.h"
#include "MyLog.h"
#include "SysUtil.h"
#include "include/NetCamera_SnapServer.h"
#include "include/NetCamera.h"
#include "CameraCapture.h"
#include "JieziBoxStruct.h"
#include "ProcessImage.h"
#include "RecoClientSDK.h"
#include "jzLocker.h"
#include "IDDetect.h"


#include <syslog.h>
#include <dlfcn.h>
#include <stack>
#include <vector>
#include <new>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <zmq.h>


using namespace boost::uuids;
using namespace std;
using namespace boost;

std::stack<structImageData> g_ImageDataStack;
vector<structBoxNumberRecogResult> g_boxNumberSet;

// =========================================
list<structImageData> g_imageVect;
locker g_queuelocker;
sem g_queuestat;
// ==========================================

map<std::string, T_RECOGNIZE_RESULT_POSITON> g_positonMap;
locker g_positionMapLocker;

// =========================================
list<T_CONTA_ALL_INFO_V2> g_contaList;
locker g_contaLocker;
sem g_contaStat;
// =========================================

// 2017/10/11
list<T_RECOGNIZE_RESULT_POSITON> g_positionList;
locker g_positionListLocker;
sem g_positionListStat;


pthread_mutex_t g_ImageDataMutex;
pthread_cond_t g_ImageDataCond;

pthread_mutex_t g_BoxNumberMutex;

void recv_snap_pic(char* szServerIP, unsigned char* pPicBuffer, int nPicLen)
{
    syslog(LOG_DEBUG, "recv camera %s,pic len is %d\n", szServerIP, nPicLen);
    string strFrontIp = CSimpleConfig::CameraFront;
    string strBackIp = CSimpleConfig::CameraBack;
    string strLeft = CSimpleConfig::CameraLeft;
    string strRight = CSimpleConfig::CameraRight;
    if (pPicBuffer == NULL || nPicLen <= 0)
        return;
    structImageData imagedata;
    imagedata.strIp = szServerIP;
    imagedata.nPicLen = nPicLen;
    if (imagedata.strIp == strFrontIp)
        imagedata.direct = CFRONT;
    else if (imagedata.strIp == strBackIp)
        imagedata.direct = CBACK;
    else if (imagedata.strIp == strLeft)
        imagedata.direct = CLEFT;
    else if (imagedata.strIp == strRight)
        imagedata.direct = CRIGHT;

    try
    {
        imagedata.pPicBuffer = new unsigned char[nPicLen + 1];
    }
    catch (const bad_alloc& e)
    {
        syslog(LOG_DEBUG, "image data new failed\n");
        return;
    }

    memcpy(imagedata.pPicBuffer, pPicBuffer, nPicLen);
    pthread_mutex_lock(&g_ImageDataMutex);
    g_ImageDataStack.push(imagedata);
    pthread_mutex_unlock(&g_ImageDataMutex);
    return;
}

void RECVPASS_VEHICLE_INFO(char* szServerIP, unsigned char* pPicBuffer, int nPicLen)
{
    syslog(LOG_DEBUG, "recv camera %s,pic len is %d\n", szServerIP, nPicLen);
    string strFrontIp = CSimpleConfig::CameraFront;
    string strBackIp = CSimpleConfig::CameraBack;
    string strLeft = CSimpleConfig::CameraLeft;
    string strRight = CSimpleConfig::CameraRight;
    if (pPicBuffer == NULL || nPicLen <= 0)
        return;
    structImageData imagedata;
    imagedata.strIp = szServerIP;
    imagedata.nPicLen = nPicLen;
    if (imagedata.strIp == strFrontIp)
        imagedata.direct = CFRONT;
    else if (imagedata.strIp == strBackIp)
        imagedata.direct = CBACK;
    else if (imagedata.strIp == strLeft)
        imagedata.direct = CLEFT;
    else if (imagedata.strIp == strRight)
        imagedata.direct = CRIGHT;

    try
    {
        imagedata.pPicBuffer = new unsigned char[nPicLen + 1];
    }
    catch (const bad_alloc& e)
    {
        syslog(LOG_DEBUG, "image data new failed\n");
        return;
    }

//    memcpy(imagedata.pPicBuffer, pPicBuffer, nPicLen);
//    pthread_mutex_lock(&g_ImageDataMutex);
//    g_ImageDataStack.push(imagedata);
//    pthread_mutex_unlock(&g_ImageDataMutex);


    //=====================================================================================   

    if (CSimpleConfig::m_nTestMode == 1)
    {

        structImageData imagedata2;
        imagedata2.strIp = szServerIP;
        imagedata2.nPicLen = nPicLen;
        if (imagedata2.strIp == strFrontIp)
            imagedata2.direct = CFRONT;
        else if (imagedata2.strIp == strBackIp)
            imagedata2.direct = CBACK;
        else if (imagedata2.strIp == strLeft)
            imagedata2.direct = CLEFT;
        else if (imagedata2.strIp == strRight)
            imagedata2.direct = CRIGHT;

        try
        {
            imagedata2.pPicBuffer = new unsigned char[nPicLen + 1];
        }
        catch (const bad_alloc& e)
        {
            syslog(LOG_DEBUG, "image data new failed\n");
            return;
        }
        memcpy(imagedata2.pPicBuffer, pPicBuffer, nPicLen);
        g_queuelocker.lock();
        g_imageVect.push_back(imagedata2);
        g_queuelocker.unlock();
        g_queuestat.post();
    }


    return;
}
//
//void RECVPASS_VEHICLE_INFO2(VP_PASS_VEHICLE_INFO& pVehicleInfo)
//{
//    printf("recv snap pic info...\n");
//    printf("plate number is :%s\n",pVehicleInfo.szHPHM);
//    
////    if(pVehicleInfo.pPicBuffer)
////    {
////        char szFileName[256]={0};
////        sprintf(szFileName,"/dev/shm/%s.jpg","test111");
////        FILE* fPic=fopen(szFileName,"wb");
////        fwrite(pVehicleInfo.pPicBuffer,1,pVehicleInfo.nPicLen,fPic);
////        fclose(fPic);
////    }
//    
//    memcpy(pVehicleInfo.szDeviceID, "192.168.1.118", strlen("192.168.1.118"));
//    
//    syslog(LOG_DEBUG, "recv camera %s,pic len is %d\n", pVehicleInfo.szDeviceID, pVehicleInfo.nPicLen);
//    
//    int nPicLen = pVehicleInfo.nPicLen;
//    unsigned char *pPicBuffer = pVehicleInfo.pPicBuffer;
//    char *szServerIP = pVehicleInfo.szDeviceID;
//    
//    
//    string strFrontIp = CSimpleConfig::CameraFront;
//    string strBackIp = CSimpleConfig::CameraBack;
//    string strLeft = CSimpleConfig::CameraLeft;
//    string strRight = CSimpleConfig::CameraRight;
//    if (pVehicleInfo.pPicBuffer == NULL || pVehicleInfo.nPicLen <= 0)
//        return;
//    structImageData imagedata;
//    imagedata.strIp = pVehicleInfo.szDeviceID;
//    imagedata.nPicLen = pVehicleInfo.nPicLen;
//    if (imagedata.strIp == strFrontIp)
//        imagedata.direct = CFRONT;
//    else if (imagedata.strIp == strBackIp)
//        imagedata.direct = CBACK;
//    else if (imagedata.strIp == strLeft)
//        imagedata.direct = CLEFT;
//    else if (imagedata.strIp == strRight)
//        imagedata.direct = CRIGHT;
//
//    try
//    {
//        imagedata.pPicBuffer = new unsigned char[nPicLen + 1];
//    }
//    catch (const bad_alloc& e)
//    {
//        syslog(LOG_DEBUG, "image data new failed\n");
//        return;
//    }
//
//    memcpy(imagedata.pPicBuffer, pPicBuffer, nPicLen);
//    pthread_mutex_lock(&g_ImageDataMutex);
//    g_ImageDataStack.push(imagedata);
//    pthread_mutex_unlock(&g_ImageDataMutex);
//
//
//    //=====================================================================================   
//
//    if (CSimpleConfig::m_nTestMode == 1)
//    {
//
//        structImageData imagedata2;
//        imagedata2.strIp = szServerIP;
//        imagedata2.nPicLen = nPicLen;
//        if (imagedata2.strIp == strFrontIp)
//            imagedata2.direct = CFRONT;
//        else if (imagedata2.strIp == strBackIp)
//            imagedata2.direct = CBACK;
//        else if (imagedata2.strIp == strLeft)
//            imagedata2.direct = CLEFT;
//        else if (imagedata2.strIp == strRight)
//            imagedata2.direct = CRIGHT;
//
//        try
//        {
//            imagedata2.pPicBuffer = new unsigned char[nPicLen + 1];
//        }
//        catch (const bad_alloc& e)
//        {
//            syslog(LOG_DEBUG, "image data new failed\n");
//            return;
//        }
//        
//        memcpy(imagedata2.pPicBuffer, pPicBuffer, nPicLen);
//        g_queuelocker.lock();
//        g_imageVect.push_back(imagedata2);
//        g_queuelocker.unlock();
//        g_queuestat.post();
//    }
//
//    return;    
//}



void RecvRecoResult(char* szRecoSequence, JZ_RecoContaID* pRecoREsult)
{
    //if(pRecoREsult == NULL || pRecoREsult->nResult==-1)
    //{
    //	printf("reco fail......\n");
    //	return;
    //}

    if (szRecoSequence != NULL && pRecoREsult->ContaID[0] != '\0' && pRecoREsult->Type[0] != '\0')
        syslog(LOG_DEBUG, "reco seq %s---result conta id %s,%s,%.3f\n", szRecoSequence, pRecoREsult->ContaID, pRecoREsult->Type, pRecoREsult->fAccuracy);
    else if (szRecoSequence != NULL && pRecoREsult->ContaID[0] != '\0' && pRecoREsult->Type[0] == '\0')
        syslog(LOG_DEBUG, "reco seq %s---result conta id %s,%.3f\n", szRecoSequence, pRecoREsult->ContaID, pRecoREsult->fAccuracy);
    else if (szRecoSequence != NULL && pRecoREsult->ContaID[0] == '\0' && pRecoREsult->Type[0] == '\0')
        syslog(LOG_DEBUG, "reco seq %s,%.3f---result conta id\n", szRecoSequence, pRecoREsult->fAccuracy);
    else if (szRecoSequence == NULL)
        return;

    structBoxNumberRecogResult result;
    result.strSeqNo = szRecoSequence;
    result.strBoxNumber = pRecoREsult->ContaID;
    result.strBoxModel = pRecoREsult->Type;
    // RED, BLUE, WHITE, GRAY, GREEN, OTHER
    if (pRecoREsult->color == RED)
        result.strBoxColor = "RED";
    if (pRecoREsult->color == BLUE)
        result.strBoxColor = "BLUE";
    if (pRecoREsult->color == WHITE)
        result.strBoxColor = "WHITE";
    if (pRecoREsult->color == GRAY)
        result.strBoxColor = "GRAY";
    if (pRecoREsult->color == GREEN)
        result.strBoxColor = "GREEN";
    if (pRecoREsult->color == OTHER)
        result.strBoxColor = "OTHER";
    if (pRecoREsult->ali.Atype == H_Align)
        result.strArrangement = "H";
    if (pRecoREsult->ali.Atype == T_Align)
        result.strArrangement = "T";

    int nAcc = (int) (pRecoREsult->fAccuracy);
    if (nAcc > 0)
        result.nAccuracy = nAcc;
    else
        result.nAccuracy = 0;

    pthread_mutex_lock(&g_BoxNumberMutex);
    g_boxNumberSet.push_back(result);
    pthread_mutex_unlock(&g_BoxNumberMutex);


    // 2017/9/25
    T_RECOGNIZE_RESULT_POSITON position;
    std::string tmpImagePath = "";
    tmpImagePath += "/dev/shm/";
    tmpImagePath += result.strSeqNo;
    tmpImagePath += ".jpg";
    position.CONTA_ID = result.strBoxNumber;
    position.CONTA_MODEL = result.strBoxModel;
    position.IMAGE_PATH = tmpImagePath;

//    g_positionMapLocker.lock();
//    g_positonMap.insert(make_pair(tmpImagePath, position));
//    g_positionMapLocker.unlock();
    
    
    g_positionListLocker.lock();
    g_positionList.push_back(position);
    g_positionListLocker.unlock();
    g_positionListStat.post();		

}

void *pthread_worker_rt(void *arg)
{
    char szPublisherIp[64] = {0};
    signal(SIGPIPE, SIG_IGN);

    void * pCtx = NULL;
    void * pSock = NULL;

    snprintf(szPublisherIp, sizeof (szPublisherIp), "tcp://%s:%d", CSimpleConfig::m_publisher_server_ip.c_str(), CSimpleConfig::m_publisher_server_port);
    szPublisherIp[63] = '\0';
    const char * pAddr = szPublisherIp; // "tcp://192.168.1.101:7766";

    if ((pCtx = zmq_ctx_new()) == NULL)
    {
        return 0;
    }

    if ((pSock = zmq_socket(pCtx, ZMQ_DEALER)) == NULL)
    {
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    int iSndTimeout = 5000; // millsecond

    if (zmq_setsockopt(pSock, ZMQ_RCVTIMEO, &iSndTimeout, sizeof (iSndTimeout)) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }

    if (zmq_connect(pSock, pAddr) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }

    while (1)
    {
        g_queuestat.wait();
        g_queuelocker.lock();
        if (g_imageVect.empty())
        {
            g_queuelocker.unlock();
            continue;
        }
        structImageData imagedata = g_imageVect.front();
        g_imageVect.pop_front();
        g_queuelocker.unlock();

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
        fwrite(imagedata.pPicBuffer, 1, imagedata.nPicLen, pStream);
        fclose(pStream);
        if (imagedata.nPicLen > 0 && imagedata.pPicBuffer != NULL)
        {
            delete []imagedata.pPicBuffer;
            imagedata.pPicBuffer = NULL;
        }

//        ContainerID code_id;
//        memset(&code_id, 0, sizeof (ContainerID));
//
//
//        if (access(strFileName.c_str(), F_OK) == -1)
//        {
//            continue;
//        }
        

//        int ret = 0;
        //ret = PathReadCode(const_cast<char*> (strFileName.c_str()), &code_id);

//        structBoxNumberRecogResult result;
//        result.strBoxNumber = (char*) code_id.ID;
//        result.strBoxModel = (char*) code_id.Type;
//        // RED, BLUE, WHITE, GRAY, GREEN, OTHER
//        if (code_id.color == (enum Color)RED)
//            result.strBoxColor = "RED";
//        if (code_id.color == (enum Color)BLUE)
//            result.strBoxColor = "BLUE";
//        if (code_id.color == (enum Color)WHITE)
//            result.strBoxColor = "WHITE";
//        if (code_id.color == (enum Color)GRAY)
//            result.strBoxColor = "GRAY";
//        if (code_id.color == (enum Color)GREEN)
//            result.strBoxColor = "GREEN";
//        if (code_id.color == (enum Color)OTHER)
//            result.strBoxColor = "OTHER";
//
//        if (code_id.ali.Atype == (enum AlignType)H_Align)
//            result.strArrangement = "H";
//        if (code_id.ali.Atype == (enum AlignType)T_Align)
//            result.strArrangement = "T";
//
//        int nAcc = (int) (code_id.accuracy);
//        if (nAcc > 0)
//            result.nAccuracy = nAcc;
//        else
//            result.nAccuracy = 0;

        char cpYearMonDay[64] = {0};
        struct tm* tmNow;
        time_t tmTime = time(NULL);
        tmNow = localtime(&tmTime);
        sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);

        


        char szMsg[512] = {0};
//        snprintf(szMsg, sizeof (szMsg), "1|%s|%s|%s|%s|%s|%s|%d|%s|%s", CSimpleConfig::GATHER_CHANNE_NO, \
//imagedata.strIp.c_str(), result.strBoxNumber.c_str(), result.strBoxModel.c_str(), result.strBoxColor.c_str(), result.strArrangement.c_str(), \
//result.nAccuracy, cpYearMonDay, strFileName.c_str());
        
        std::string strChno = CSimpleConfig::GATHER_AREA_ID;
        strChno += CSimpleConfig::GATHER_CHANNE_NO;
        
        snprintf(szMsg, sizeof (szMsg), "1|%s|%s|%s|%s|%s|%s|%d|%s|%s", strChno.c_str(), \
imagedata.strIp.c_str(), "", "", "", "", \
0, cpYearMonDay, strFileName.c_str());        
        
        
        szMsg[511] = '\0';

        if (zmq_send(pSock, szMsg, sizeof (szMsg), 0) < 0)
        {
            syslog(LOG_ERR, "send message faild\n");
            continue;
        }
        
        ContaReco(const_cast<char*> (strUUID.c_str()), const_cast<char*> (strFileName.c_str()));

        //		static int i = 0;
        //		char szMsg[1024] = {0};
        //		pthread_mutex_lock(&g_StatusMutex);
        //		BuildStatusString(&g_ObjectStatus, szMsg, i++);
        //		pthread_mutex_unlock(&g_StatusMutex);
        //
        //		//_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
        //		//printf("Enter to send...\n");
        //		syslog(LOG_DEBUG, "Enter to send...\n");
        //		if(zmq_send(pSock, szMsg, sizeof(szMsg), 0) < 0)
        //		{
        //			//fprintf(stderr, "send message faild\n");
        //			syslog(LOG_ERR, "send message faild\n");
        //			continue;
        //		}
        //		syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
        //		// getchar();
        //		usleep (5000 * 1000);
    }

    return 0;
}

void *pthread_worker_rt2(void *arg)
{
    char szPublisherIp[64] = {0};
    signal(SIGPIPE, SIG_IGN);

    void * pCtx = NULL;
    void * pSock = NULL;

    snprintf(szPublisherIp, sizeof (szPublisherIp), "tcp://%s:%d", CSimpleConfig::m_publisher_server_ip.c_str(), CSimpleConfig::m_publisher_server_port);
    szPublisherIp[63] = '\0';
    const char * pAddr = szPublisherIp; // "tcp://192.168.1.101:7766";

    if ((pCtx = zmq_ctx_new()) == NULL)
    {
        return 0;
    }

    if ((pSock = zmq_socket(pCtx, ZMQ_DEALER)) == NULL)
    {
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    int iSndTimeout = 5000; // millsecond

    if (zmq_setsockopt(pSock, ZMQ_RCVTIMEO, &iSndTimeout, sizeof (iSndTimeout)) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }

    if (zmq_connect(pSock, pAddr) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }

    while (1)
    {
        g_contaStat.wait();
        g_contaLocker.lock();
        if (g_contaList.empty())
        {
            g_contaLocker.unlock();
            continue;
        }
        // T_CONTA_ALL_INFO containfo = g_contaList.front();
        T_CONTA_ALL_INFO_V2 containfo = g_contaList.front();
        g_contaList.pop_front();
        g_contaLocker.unlock();

        char cpYearMonDay[64] = {0};
        struct tm* tmNow;
        time_t tmTime = time(NULL);
        tmNow = localtime(&tmTime);
        sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);

        char szMsg[4096] = {0};
        snprintf(szMsg, sizeof (szMsg), "2|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s",
                containfo.CHNL_NO.c_str(),
                containfo.SEQ_NO.c_str(),
                containfo.CONTA_ID_F.c_str(),
                containfo.CONTA_MODEL_F.c_str(),
                containfo.CONTA_ID_B.c_str(),
                containfo.CONTA_MODEL_B.c_str(),
                cpYearMonDay,
                containfo.LOCAL_PIC_F.c_str(),
                containfo.LOCAL_PIC_B.c_str(),
                containfo.LOCAL_PIC_LF.c_str(),
                containfo.LOCAL_PIC_RF.c_str(),
                containfo.LOCAL_PIC_LB.c_str(),
                containfo.LOCAL_PIC_RB.c_str(),
                
                containfo.FTP_PIC_F.c_str(),
                containfo.FTP_PIC_B.c_str(),
                containfo.FTP_PIC_LF.c_str(),
                containfo.FTP_PIC_RF.c_str(),
                containfo.FTP_PIC_LB.c_str(),
                containfo.FTP_PIC_RB.c_str());

        szMsg[4095] = '\0';

        if (zmq_send(pSock, szMsg, sizeof (szMsg), 0) < 0)
        {
            syslog(LOG_ERR, "send message faild\n");
            continue;
        }

        //		static int i = 0;
        //		char szMsg[1024] = {0};
        //		pthread_mutex_lock(&g_StatusMutex);
        //		BuildStatusString(&g_ObjectStatus, szMsg, i++);
        //		pthread_mutex_unlock(&g_StatusMutex);
        //
        //		//_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
        //		//printf("Enter to send...\n");
        //		syslog(LOG_DEBUG, "Enter to send...\n");
        //		if(zmq_send(pSock, szMsg, sizeof(szMsg), 0) < 0)
        //		{
        //			//fprintf(stderr, "send message faild\n");
        //			syslog(LOG_ERR, "send message faild\n");
        //			continue;
        //		}
        //		syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
        //		// getchar();
        //		usleep (5000 * 1000);
    }

    return 0;
}


void *pthread_worker_rt3(void *arg)
{
    char szPublisherIp[64] = {0};
    signal(SIGPIPE, SIG_IGN);

    void * pCtx = NULL;
    void * pSock = NULL;

    snprintf(szPublisherIp, sizeof (szPublisherIp), "tcp://%s:%d", CSimpleConfig::m_publisher_server_ip.c_str(), CSimpleConfig::m_publisher_server_port);
    szPublisherIp[63] = '\0';
    const char * pAddr = szPublisherIp; // "tcp://192.168.1.101:7766";

    if ((pCtx = zmq_ctx_new()) == NULL)
    {
        return 0;
    }

    if ((pSock = zmq_socket(pCtx, ZMQ_DEALER)) == NULL)
    {
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    int iSndTimeout = 5000; // millsecond

    if (zmq_setsockopt(pSock, ZMQ_RCVTIMEO, &iSndTimeout, sizeof (iSndTimeout)) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }

    if (zmq_connect(pSock, pAddr) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }

    while (1)
    {
        g_positionListStat.wait();
        g_positionListLocker.lock();
        if (g_positionList.empty())
        {
            g_positionListLocker.unlock();
            continue;
        }
        T_RECOGNIZE_RESULT_POSITON position = g_positionList.front();
        g_positionList.pop_front();
        g_positionListLocker.unlock();

        char cpYearMonDay[64] = {0};
        struct tm* tmNow;
        time_t tmTime = time(NULL);
        tmNow = localtime(&tmTime);
        sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);

        char szMsg[512] = {0};
//        snprintf(szMsg, sizeof (szMsg), "2|%s|%s|%s|%s|%s|%s|%s|%s",
//                containfo.CHNL_NO.c_str(),
//                containfo.SEQ_NO.c_str(),
//                containfo.IMAGE_IP.c_str(),
//                containfo.CONTA_ID.c_str(),
//                containfo.CONTA_MODEL.c_str(),
//                cpYearMonDay,
//                containfo.PIC_TYPE.c_str(),
//                containfo.IMAGE_PATH.c_str());
//        snprintf(szMsg, sizeof (szMsg), "3|%s|%s|%s|%s|%s",
//                CSimpleConfig::GATHER_CHANNE_NO,
//                position.CONTA_ID.c_str(),
//                position.CONTA_MODEL.c_str(),
//                cpYearMonDay,
//                position.IMAGE_PATH.c_str());        
//        char szMsg[512] = {0};
        
        std::string strChno = CSimpleConfig::GATHER_AREA_ID;
        strChno += CSimpleConfig::GATHER_CHANNE_NO;
        
        snprintf(szMsg, sizeof (szMsg), "M|%s|%s|%s|%s|%s|%s|%d|%s|%s", strChno.c_str(), \
"", position.CONTA_ID.c_str(), position.CONTA_MODEL.c_str(), "", "", \
0, cpYearMonDay, position.IMAGE_PATH.c_str());
        szMsg[511] = '\0';
        
        
        szMsg[511] = '\0';

        if (zmq_send(pSock, szMsg, sizeof (szMsg), 0) < 0)
        {
            syslog(LOG_ERR, "send message faild\n");
            continue;
        }

        //		static int i = 0;
        //		char szMsg[1024] = {0};
        //		pthread_mutex_lock(&g_StatusMutex);
        //		BuildStatusString(&g_ObjectStatus, szMsg, i++);
        //		pthread_mutex_unlock(&g_StatusMutex);
        //
        //		//_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
        //		//printf("Enter to send...\n");
        //		syslog(LOG_DEBUG, "Enter to send...\n");
        //		if(zmq_send(pSock, szMsg, sizeof(szMsg), 0) < 0)
        //		{
        //			//fprintf(stderr, "send message faild\n");
        //			syslog(LOG_ERR, "send message faild\n");
        //			continue;
        //		}
        //		syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
        //		// getchar();
        //		usleep (5000 * 1000);
    }

    return 0;
}

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor*) arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

//int rsmain()
int main(int argc, char* argv[])
{
    openlog("BoxRecogServer", LOG_PID, LOG_LOCAL1);
    pthread_cond_init(&g_ImageDataCond, NULL);
    pthread_mutex_init(&g_ImageDataMutex, NULL);
    CSimpleConfig cg;
    cg.get_config();
    char *pRecoIP = CSimpleConfig::RecogServerIP; // "192.168.1.154";
    
    if (pRecoIP[0] != 0)
        RecoInit(pRecoIP, CSimpleConfig::RecogServerPort); // 19000);
    SetRecoResultCallback(RecvRecoResult);
    ProcessImage processImage;
    processImage.Init();

    //    {
    //        void* device_capture = dlopen("/usr/lib/libJZPicCapture.so", RTLD_LAZY);
    //        if (!device_capture)
    //        {
    //            //printf("load library fail! dll name is  %s \n", "libJZPicCapture.so");
    //            //		lprintf(g_log, FATAL, "load library fail! dll name is  %s \n", "libJZPicCapture.so");
    //                        
    //			//ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t)load library fail! dll name is  %s \n"), "libJZPicCapture.so"));
    //			syslog(LOG_DEBUG, "load library fail! dll name is  %s \n", "libJZPicCapture.so");
    //            return -1;
    //        }
    //        else
    //        {
    //            //printf("load library succ! dll name is  %s \n", "libJZPicCapture.so");
    //	//		lprintf(g_log, INFO, "load library succ! dll name is  %s \n", "libJZPicCapture.so");
    //            //ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)load library succ! dll name is  %s \n"), "libJZPicCapture.so"));  
    //			syslog(LOG_DEBUG, "load library succ! dll name is  %s \n", "libJZPicCapture.so");
    //        }
    //        // reset errors
    //        dlerror();
    //        // load the symbols
    //        create_t_p* create_capture	= (create_t_p*) dlsym(device_capture, "create");
    //        const char* dlsym_error		= dlerror();
    //        if (dlsym_error)
    //        {
    //            //printf("Cannot load symbol create:  %s \n", dlsym_error);
    //	//		lprintf(g_log, FATAL, "Cannot load symbol create:  %s \n", dlsym_error);
    //            //ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)Cannot load symbol create:  %s \n"), dlsym_error));
    //			syslog(LOG_DEBUG, "Cannot load symbol create:  %s \n", dlsym_error);
    //            return 1;
    //        }
    //        if (create_capture)
    //        {
    //            //printf("get create function pointer %p succ !\n", create_capture);
    //            //		lprintf(g_log, INFO, "get create function pointer %p succ !\n", create_capture);
    //            //ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)get create function pointer %p succ !\n"), create_capture));
    //			syslog(LOG_DEBUG, "get create function pointer %p succ !\n", create_capture);
    //        }
    //        // create an instance of the class
    //        CCapturePic* m_pCapture = create_capture();
    //        
    //        if(m_pCapture)
    //        {
    //            m_pCapture->SetSnapDataCallback(recv_snap_pic);
    //			//m_pCapture->SetSnapServer(CSimpleConfig::CameraFront, 2233, "123", "123", "127.0.0.1", 9999);
    //			//m_pCapture->SetSnapServer(CSimpleConfig::CameraLeft, 2233, "123", "123", "127.0.0.1", 9999);
    //			//m_pCapture->SetSnapServer(CSimpleConfig::CameraRight, 2233, "123", "123", "127.0.0.1", 9999);
    //			//m_pCapture->SetSnapServer(CSimpleConfig::CameraBack, 2233, "123", "123", "127.0.0.1", 9999);
    //                        
    //			m_pCapture->SetSnapServer(CSimpleConfig::CameraFront, CSimpleConfig::CAMERA_FRONT_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort); // "192.168.1.151", 9999);
    //			m_pCapture->SetSnapServer(CSimpleConfig::CameraLeft,  CSimpleConfig::CAMERA_LEFT_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);
    //			m_pCapture->SetSnapServer(CSimpleConfig::CameraRight, CSimpleConfig::CAMERA_RIGHT_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);
    //			m_pCapture->SetSnapServer(CSimpleConfig::CameraBack,  CSimpleConfig::CAMERA_BACK_PORT, "123", "123",	CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);                       
    //                        
    //                        
    //            m_pCapture->SetListenPort(CSimpleConfig::SnapshotServerPort);
    //          
    //        }
    //        
    //		//printf("========================================\n");
    //		//lprintf(g_log, INFO, "========================================\n");
    //        
    //        // ACE_DEBUG((LM_INFO,  ACE_TEXT("(%P|%t)========================================\n")));
    //		 syslog(LOG_DEBUG, "========================================\n");
    //    }


    CCameraCapturePic* m_pHikIPC = NULL;
    //    CCapturePic* m_pHikIPC1=NULL;
    //    CCapturePic* m_pHikIPC2=NULL;
    //    CCapturePic* m_pHikIPC3=NULL;
    //    CCapturePic* m_pHikIPC4=NULL;
    {

        void* proxy_so = dlopen("/usr/lib/libCameraCapture.so", RTLD_LAZY);
        if (!proxy_so)
        {

            syslog(LOG_DEBUG, "********load library fail! dll name is  %s ,error %s %d \n", "libCameraCapture.so", dlerror(), errno);
            return 1;
        }
        else
        {
            syslog(LOG_DEBUG, "load library succ! dll name is  %s \n", "libCameraCapture.so");
        }


        // reset errors
        dlerror();

        // load the symbols
        create_t* create = (create_t*) dlsym(proxy_so, "create");
        const char* dlsym_error = dlerror();
        if (dlsym_error)
        {
            syslog(LOG_DEBUG, "Cannot load symbol create:  %s \n", dlsym_error);
            return 1;
        }

        if (create)
        {
            syslog(LOG_DEBUG, "get create function pointer %p succ !\n", create);
        }

        // create an instance of the class
        m_pHikIPC = create();

        if (m_pHikIPC)
        {
            //m_pHikIPC->SetSnapServer("192.168.1.188",8000,"admin","12345","127.0.0.1",8000);            
            //m_pHikIPC->SetListenPort(7200);
            //m_pHikIPC->SetCameraType("HIK");
            //m_pHikIPC->SetCameraType("ANA");
            //m_pHikIPC->SetCameraType("HIK");
            m_pHikIPC->SetCameraType("DAH");
            m_pHikIPC->SetSnapDataCallback(RECVPASS_VEHICLE_INFO);
            m_pHikIPC->SetListenPort(CSimpleConfig::SnapshotServerPort);

            m_pHikIPC->SetSnapServer(CSimpleConfig::CameraFront, CSimpleConfig::CAMERA_FRONT_PORT, "admin", "admin", CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort); // "192.168.1.151", 9999);
            m_pHikIPC->SetSnapServer(CSimpleConfig::CameraLeft, CSimpleConfig::CAMERA_LEFT_PORT, "admin", "admin", CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);
            m_pHikIPC->SetSnapServer(CSimpleConfig::CameraRight, CSimpleConfig::CAMERA_RIGHT_PORT, "admin", "admin", CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);
            m_pHikIPC->SetSnapServer(CSimpleConfig::CameraBack, CSimpleConfig::CAMERA_BACK_PORT, "admin", "admin", CSimpleConfig::SnapshotServerIP, CSimpleConfig::SnapshotServerPort);
        }
    }

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    


    /* Ignore signals generated when a connection is broken unexpectedly. */
    ACE_Sig_Action sig((ACE_SignalHandler) SIG_IGN, SIGPIPE);
    ACE_UNUSED_ARG(sig);
    //    ACE_UNUSED_ARG(argc);
    //   ACE_UNUSED_ARG(argv);
    //get config details
    CMyLog::Init();
    /* get default instance of ACE_Reactor  */
    ACE_Select_Reactor *select_reactor;
    ACE_NEW_RETURN(select_reactor, ACE_Select_Reactor, 1);
    ACE_Reactor *reactor;
    ACE_NEW_RETURN(reactor, ACE_Reactor(select_reactor, 1), 1);
    ACE_Reactor::close_singleton();
    ACE_Reactor::instance(reactor, 1);
    /*  start event and control thread   */
    ACE_Thread_Manager::instance()->spawn(event_loop, reactor);
    //ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up controlcenter daemon\n"));
    syslog(LOG_DEBUG, "starting up controlcenter daemon\n");
    //ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up reactor event loop ...\n"));
    //syslog(LOG_DEBUG, "starting up reactor event loop ...\n");
    //MSG_HANDLE_CENTER::instance()->open();
    /*
     * read config from config file,and open port ,listen
     */
    ACE_Reactor* reactorptr = ACE_Reactor::instance();




    //processImage.Run();


    if (CSimpleConfig::m_nTestMode == 1)
    {
        pthread_t tid_worker_rt;

        pthread_create(&tid_worker_rt, NULL, pthread_worker_rt, NULL);
        syslog(LOG_DEBUG, "create the worker rt pthread OK!\n");
    }


    pthread_t tid_worker_rt2;

    pthread_create(&tid_worker_rt2, NULL, pthread_worker_rt2, NULL);
    syslog(LOG_DEBUG, "create the worker rt2 pthread OK!\n");
    
    
    pthread_t tid_worker_rt3;
    pthread_create(&tid_worker_rt3, NULL, pthread_worker_rt3, NULL);
    syslog(LOG_DEBUG, "create the worker rt3 pthread OK!\n");    


    //MSG_HANDLE_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();
    ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down controlcenter daemon\n"));
    closelog();
    return 0;
}
//
//int Daemon()
//{
//    pid_t pid;
//    pid = fork();
//    if (pid < 0)
//    {
//        return -1;
//    }
//    else if (pid != 0)
//    {
//        exit(0);
//    }
//    setsid();
//    return 0;
//}
//#define SOFTWARE_VERSION  "version 1.0.0.0"         //软件版本号
//
//int main(int argc, char *argv[])
//{
//    int iRet;
//    int iStatus;
//    pid_t pid;
//    //显示版本号
//    if (argc == 2)
//    {
//        //如果是查看版本号
//        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "-V") || !strcmp(argv[1], "-Version") || !strcmp(argv[1], "-version"))
//        {
//            printf("%s  %s\n", argv[0], SOFTWARE_VERSION);
//            return 0;
//        }
//    }
//
//    Daemon();
//
//createchildprocess:
//    //开始创建子进程
//    syslog(LOG_DEBUG, "begin to create	the child process of %s\n", argv[0]);
//
//    int itest = 0;
//    switch (fork())//switch(fork())
//    {
//        case -1: //创建子进程失败
//            syslog(LOG_DEBUG, "cs创建子进程失败\n");
//            return -1;
//        case 0://子进程
//            syslog(LOG_DEBUG, "cs创建子进程成功\n");
//            rsmain();
//            return -1;
//        default://父进程
//            pid = wait(&iStatus);
//            syslog(LOG_DEBUG, "子进程退出，5秒后重新启动......\n");
//            sleep(5);
//            goto createchildprocess; //重新启动进程
//            break;
//    }
//    return 0;
//}



