#include "SimpleConfig.h"

#ifdef WIN32 

//#include   <Windows.h>

#endif

#include "MyLog.h"
#include "tinyXML/tinyxml.h"

#include <syslog.h>
#include <string>

using namespace std;

int CSimpleConfig::m_nTestMode;

int CSimpleConfig::CHANNEL_CONTROLER_PORT;

char CSimpleConfig::DEVICE_ID[32];

//int CSimpleConfig::COM_PORT;

//int CSimpleConfig::BAUD_RATE;

//char CSimpleConfig::DEVICE_FACTORY[256];

//char CSimpleConfig::DEVICE_SO[64];

//char CSimpleConfig::DEVICE_NAME[128];

char CSimpleConfig::EXE_FULL_PATH[256];

char CSimpleConfig::DEVICE_TAG[32];

char CSimpleConfig::CameraFront[50];
char CSimpleConfig::CameraLeft[50];
char CSimpleConfig::CameraRight[50];
char CSimpleConfig::CameraBack[50];

char CSimpleConfig::DI_BIT[9];
char CSimpleConfig::DI_ENABLE[9];


char CSimpleConfig::SnapshotServerIP[32];
int CSimpleConfig::SnapshotServerPort;

char CSimpleConfig::RecogServerIP[32];
int CSimpleConfig::RecogServerPort;

std::vector<JZ_PUBLISH_EVENT_STRUCT> CSimpleConfig::m_eventVect;
std::string CSimpleConfig::m_strGatherControlIP;
int CSimpleConfig::m_nGatherControlPort;
std::string CSimpleConfig::m_strChannelControlIP;

std::string CSimpleConfig::DI_BIT_REVERSE;



int CSimpleConfig::CAMERA_FRONT_PORT;
int CSimpleConfig::CAMERA_LEFT_PORT;
int CSimpleConfig::CAMERA_RIGHT_PORT;
int CSimpleConfig::CAMERA_BACK_PORT;

std::string CSimpleConfig::m_publisher_server_ip;
int CSimpleConfig::m_publisher_server_port;

char CSimpleConfig::GATHER_AREA_ID[32];
char CSimpleConfig::GATHER_CHANNE_NO[32];
char CSimpleConfig::GATHER_I_E_TYPE[8];

std::string CSimpleConfig::LOCAL_FILE_PATH;
std::string CSimpleConfig::REMOTE_FILE_PATH;

std::string CSimpleConfig::m_strBackServerIP;
int	    CSimpleConfig::m_nBackServerPort;

CSimpleConfig::CSimpleConfig()
{

}

CSimpleConfig::~CSimpleConfig()
{

}

int CSimpleConfig::GetProcessPath(char* cpPath)
{
    int iCount;
    iCount = readlink("/proc/self/exe", cpPath, 256);

    if (iCount < 0 || iCount >= 256)
    {
        //char szLog[1024] = {0};
        syslog(LOG_DEBUG, "********get process absolute path failed,errno:%d !\n", errno);
        //CMyLog::m_pLog->_XGSysLog(szLog);
        return -1;
    }

    cpPath[iCount] = '\0';
    return 0;
}

void CSimpleConfig::get_config()
{
    char exeFullPath[256];

    GetProcessPath(exeFullPath); //得到程序模块名称，全路径

    string strFullExeName(exeFullPath);

    int nLast = strFullExeName.find_last_of("/");

    strFullExeName = strFullExeName.substr(0, nLast + 1);

    strcpy(EXE_FULL_PATH, strFullExeName.c_str());

    char cpPath[256] = {0};

    char temp[256] = {0};

    sprintf(cpPath, "%sBoxNumberConfigTest.xml", strFullExeName.c_str());

    //简单配置部分，一层，用辅助类直接读
    XMLConfig xmlConfig(cpPath);

    string strTEST_MODE = xmlConfig.GetValue("IS_TEST_MODE");
    m_nTestMode = atoi(strTEST_MODE.c_str());

    m_strGatherControlIP = xmlConfig.GetValue("GATHER_CONTROLER_IP");
    string strGatherPort = xmlConfig.GetValue("GATHER_CONTROLER_PORT");
    m_nGatherControlPort = atoi(strGatherPort.c_str());

    m_strChannelControlIP = xmlConfig.GetValue("CHANNE_CONTROLER_IP");
    string conntroler_port = xmlConfig.GetValue("CHANNE_CONTROLER_PORT");
    CHANNEL_CONTROLER_PORT = atoi(conntroler_port.c_str());

    strcpy(DEVICE_ID, xmlConfig.GetValue("DEVICEID").c_str());

    m_publisher_server_ip = xmlConfig.GetValue("PUBLISHER_SERVER_IP");
    string strPublishPort = xmlConfig.GetValue("PUBLISHER_SERVER_PORT");
    m_publisher_server_port = atoi(strPublishPort.c_str());


    DI_BIT_REVERSE = xmlConfig.GetValue("DI_BIT_REVERSE");

    strcpy(GATHER_AREA_ID,      xmlConfig.GetValue("GATHER_AREA_ID").c_str());
    strcpy(GATHER_CHANNE_NO,    xmlConfig.GetValue("GATHER_CHANNE_NO").c_str());
    strcpy(GATHER_I_E_TYPE,     xmlConfig.GetValue("GATHER_I_E_TYPE").c_str());    
    
    
    LOCAL_FILE_PATH = xmlConfig.GetValue("LOCAL_FILE_PATH");  
    REMOTE_FILE_PATH = xmlConfig.GetValue("REMOTE_FILE_PATH");        
    
    m_strBackServerIP           = xmlConfig.GetValue("BACK_SERVER_IP");
    string strBackServerPort    = xmlConfig.GetValue("BACK_SERVER_PORT");
    m_nBackServerPort = atoi(strBackServerPort.c_str());    
        
    //string comport = xmlConfig.GetValue("COMPORT");

    //COM_PORT = atoi(comport.c_str());

    //string strbaudrate = xmlConfig.GetValue("BAUDRATE");

    //BAUD_RATE = atoi(strbaudrate.c_str());

    //strcpy(DEVICE_FACTORY, xmlConfig.GetValue("FACTORYNAME").c_str());

    //strcpy(DEVICE_SO, xmlConfig.GetValue("DEVICESONAME").c_str());

    //strcpy(DEVICE_NAME, xmlConfig.GetValue("DEVICENAME").c_str());

    strcpy(DEVICE_TAG, xmlConfig.GetValue("DEVICE_TAG").c_str());

    syslog(LOG_DEBUG, "%s:%s:%d\n", m_strChannelControlIP.c_str(), m_strGatherControlIP.c_str(), m_nGatherControlPort);
    ReadConfig(cpPath);

}

void CSimpleConfig::ReadConfig(std::string strFileName)
{
    TiXmlDocument doc(strFileName.c_str());
    doc.LoadFile();

    TiXmlElement *xmlRootElement = NULL;
    TiXmlElement *xmlSubElement = NULL;
    TiXmlNode *pNode = NULL;
    TiXmlNode *pNodeTmp = NULL;
    TiXmlNode *pConfigNode = NULL;
    TiXmlNode *pCameraConfigNode = NULL;
    TiXmlNode *pSnapshotConfigNode = NULL;
    TiXmlNode *pRecogConfigNode = NULL;

    pConfigNode = doc.FirstChild("Config");
    if (pConfigNode == NULL)
    {
        syslog(LOG_DEBUG, "pConfigNode:%p\n", pConfigNode);
        return;
    }

    xmlRootElement = pConfigNode->ToElement();
    syslog(LOG_DEBUG, "xmlRootElement:%p\n", xmlRootElement);
    if (xmlRootElement)
    {
        pCameraConfigNode = xmlRootElement->FirstChild("CAMERA_CONFIG");
        syslog(LOG_DEBUG, "pCameraConfigNode:%p\n", pCameraConfigNode);
        if (pCameraConfigNode != NULL)
        {
            string strCameraFront = "";
            string strCameraLeft = "";
            string strCameraRight = "";
            string strCameraBack = "";

            string strCameraFrontPort = "";
            string strCameraLeftPort = "";
            string strCameraRightPort = "";
            string strCameraBackPort = "";

            xmlSubElement = pCameraConfigNode->ToElement();
            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_FRONT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraFront = pNodeTmp->ToElement()->GetText();

            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_FRONT_PORT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraFrontPort = pNodeTmp->ToElement()->GetText();

            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_LEFT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraLeft = pNodeTmp->ToElement()->GetText();

            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_LEFT_PORT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraLeftPort = pNodeTmp->ToElement()->GetText();


            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_RIGHT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraRight = pNodeTmp->ToElement()->GetText();


            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_RIGHT_PORT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraRightPort = pNodeTmp->ToElement()->GetText();


            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_BACK");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraBack = pNodeTmp->ToElement()->GetText();

            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("CAMERA_BACK_PORT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strCameraBackPort = pNodeTmp->ToElement()->GetText();

            syslog(LOG_DEBUG, "%s,%s,%s,%s\n", strCameraFront.c_str(), strCameraLeft.c_str(), strCameraRight.c_str(), strCameraBack.c_str());

            memcpy(CameraFront, strCameraFront.c_str(), strCameraFront.size());
            memcpy(CameraLeft, strCameraLeft.c_str(), strCameraLeft.size());
            memcpy(CameraRight, strCameraRight.c_str(), strCameraRight.size());
            memcpy(CameraBack, strCameraBack.c_str(), strCameraBack.size());

            CAMERA_FRONT_PORT = atoi(strCameraFrontPort.c_str());
            CAMERA_LEFT_PORT = atoi(strCameraLeftPort.c_str());
            CAMERA_RIGHT_PORT = atoi(strCameraRightPort.c_str());
            CAMERA_BACK_PORT = atoi(strCameraBackPort.c_str());

        }


        TiXmlNode *pControlNode = NULL;
        TiXmlNode *pDI_BITNode = NULL;
        pControlNode = xmlRootElement->FirstChild("GATHER_CONTROLLER_CONFIG");
        if (pControlNode != NULL)
        {
            int i = 7;
            for (pDI_BITNode = pControlNode->FirstChild("DI_BIT"); pDI_BITNode != NULL; pDI_BITNode = pDI_BITNode->NextSibling("DI_BIT"))
            {
                string strDI_BIT = "";
                string strEnable = "";
                xmlSubElement = pDI_BITNode->ToElement();

                if (pDI_BITNode != NULL && pDI_BITNode->ToElement() != NULL && pDI_BITNode->ToElement()->GetText() != 0)
                {
                    strDI_BIT = pDI_BITNode->ToElement()->GetText();
                    syslog(LOG_DEBUG, "strDI_BIT:%s", strDI_BIT.c_str());
                    if (i >= 0)
                        DI_BIT[i] = strDI_BIT[0];
                }

                if (xmlSubElement != NULL)
                {
                    if (xmlSubElement->Attribute("Enable") != NULL)
                    {
                        strEnable = xmlSubElement->Attribute("Enable");
                        if (i >= 0)
                            DI_ENABLE[i] = strEnable[0];
                    }
                    syslog(LOG_DEBUG, "strEnable:%s", strEnable.c_str());

                }
                --i;
                syslog(LOG_DEBUG, "\n");
            }
        }


        pSnapshotConfigNode = xmlRootElement->FirstChild("SNAPSHOT_SERVER_CONFIG");
        syslog(LOG_DEBUG, "pSnapshotConfigNode:%p\n", pSnapshotConfigNode);
        if (pSnapshotConfigNode != NULL)
        {
            std::string strSnapshotServerIP = "";
            std::string strSnapshotServerPort = "";

            xmlSubElement = pSnapshotConfigNode->ToElement();
            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("SNAPSHOT_SERVER_IP");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strSnapshotServerIP = pNodeTmp->ToElement()->GetText();

            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("SNAPSHOT_SERVER_PORT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strSnapshotServerPort = pNodeTmp->ToElement()->GetText();

            syslog(LOG_DEBUG, "snapshot,port:%s,%s\n", strSnapshotServerIP.c_str(), strSnapshotServerPort.c_str());
            memcpy(SnapshotServerIP, strSnapshotServerIP.c_str(), strSnapshotServerIP.size());
            SnapshotServerPort = atoi(strSnapshotServerPort.c_str());
        }



        pRecogConfigNode = xmlRootElement->FirstChild("RECOGNITION_SERVER_CONFIG");
        syslog(LOG_DEBUG, "pCameraConfigNode:%p\n", pRecogConfigNode);
        if (pRecogConfigNode != NULL)
        {
            std::string strRecogServerIP = "";
            std::string strRecogServerPort = "";

            xmlSubElement = pRecogConfigNode->ToElement();
            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("RECOGNITION_SERVER_IP");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strRecogServerIP = pNodeTmp->ToElement()->GetText();

            if (xmlSubElement != NULL)
                pNodeTmp = xmlSubElement->FirstChildElement("RECOGNITION_SERVER_PORT");
            if (pNodeTmp != NULL && pNodeTmp->ToElement() != NULL && pNodeTmp->ToElement()->GetText() != 0)
                strRecogServerPort = pNodeTmp->ToElement()->GetText();

            syslog(LOG_DEBUG, "recog,port:%s,%s\n", strRecogServerIP.c_str(), strRecogServerPort.c_str());
            memcpy(RecogServerIP, strRecogServerIP.c_str(), strRecogServerIP.size());
            RecogServerPort = atoi(strRecogServerPort.c_str());
        }


        TiXmlNode *pEventSequenceNode = NULL;
        TiXmlNode *pEventNode = NULL;
        pEventSequenceNode = xmlRootElement->FirstChild("EVENT_SEQUENCE");
        if (pEventSequenceNode != NULL)
        {
            for (pEventNode = pEventSequenceNode->FirstChild("EVENT"); pEventNode != NULL; pEventNode = pEventNode->NextSibling("EVENT"))
            {

                JZ_PUBLISH_EVENT_STRUCT tmpEvent = {0};
                string strTAG = "";
                string strEVENT_ID = "";
                string strSEQUENCE_NO = "";

                xmlSubElement = pEventNode->ToElement();

                //if (pEventNode != NULL && pEventNode->ToElement() != NULL && pEventNode->ToElement()->GetText() != 0)
                //{
                //	strDI_BIT = pEventNode->ToElement()->GetText();
                //	printf("strDI_BIT:%s", strDI_BIT.c_str());
                //	if (i >= 0)
                //		DI_BIT[i]	= strDI_BIT[0];
                //}

                if (xmlSubElement != NULL)
                {
                    if (xmlSubElement->Attribute("TAG") != NULL)
                    {
                        strTAG = xmlSubElement->Attribute("TAG");
                    }

                    if (xmlSubElement->Attribute("EVENT_ID") != NULL)
                    {
                        strEVENT_ID = xmlSubElement->Attribute("EVENT_ID");
                    }

                    if (xmlSubElement->Attribute("SEQUENCE_NO") != NULL)
                    {
                        strSEQUENCE_NO = xmlSubElement->Attribute("SEQUENCE_NO");
                    }

                    strncpy(tmpEvent.device_tag, strTAG.c_str(), strTAG.size() < 32 ? strTAG.size() : 31);
                    strncpy(tmpEvent.event_id, strEVENT_ID.c_str(), strEVENT_ID.size() < 32 ? strEVENT_ID.size() : 31);
                    strncpy(tmpEvent.sequence_no, strSEQUENCE_NO.c_str(), strSEQUENCE_NO.size() < 32 ? strSEQUENCE_NO.size() : 31);
                    m_eventVect.push_back(tmpEvent);

                    syslog(LOG_DEBUG, "tag,eventid,sequenceno:%s,%s,%s\n", strTAG.c_str(), strEVENT_ID.c_str(), strSEQUENCE_NO.c_str());
                }
            }
        }
    }
}
