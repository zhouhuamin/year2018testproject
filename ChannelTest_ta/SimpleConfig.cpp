#include "SimpleConfig.h"
#include <errno.h>
#include "MyLog.h"
#include <string>
using namespace std;

char CSimpleConfig::GATHERGATE_TYPE[8];
int  CSimpleConfig::IS_VIRTUAL = 0;
int  CSimpleConfig::LOCAL_LISTEN_PORT;
int  CSimpleConfig::SERVER_MONITOR_PORT;


char CSimpleConfig::CENTER_CONTROLER_IP[32];
int CSimpleConfig::CENTER_CONTROLER_PORT;

char CSimpleConfig::GATHER_AREA_ID[32];
char CSimpleConfig::GATHER_CHANNE_NO[32];
char CSimpleConfig::GATHER_I_E_TYPE[8];
char CSimpleConfig::GATHER_CHNL_TYPE[8];

std::vector<T_Sequence_Control*> CSimpleConfig::m_StaSequenceControlVec;
std::vector<T_Sequence_Exception*> CSimpleConfig::m_StaSequenceExceptionVec;

std::map<string, T_Gather_Device*> CSimpleConfig::m_StaDeviceMap;
std::map<string, T_Gather_Device*> CSimpleConfig::m_StaTagDeviceMap;

char CSimpleConfig::EXE_FULL_PATH[256];
std::string CSimpleConfig::m_publisher_server_ip;
int			CSimpleConfig::m_publisher_server_port;


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
        CMyLog::m_pLog->_XGSysLog("********get process absolute path failed,errno:%d !\n", errno);
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

    sprintf(cpPath, "%sChannelConfig.xml", strFullExeName.c_str());

    //简单配置部分，一层，用辅助类直接读

    XMLConfig xmlConfig(cpPath);

    strcpy(GATHERGATE_TYPE, xmlConfig.GetValue("GATHERGATE_TYPE").c_str());

    string is_virtual = xmlConfig.GetValue("IS_VIRTUAL");
    IS_VIRTUAL = atoi(is_virtual.c_str());

    string conntroler_port = xmlConfig.GetValue("CHANNEL_CONTROLER_PORT");
    LOCAL_LISTEN_PORT = atoi(conntroler_port.c_str());


    strcpy(CENTER_CONTROLER_IP, xmlConfig.GetValue("CENTER_CONTROLER_IP").c_str());

    string center_port = xmlConfig.GetValue("CENTER_CONTROLER_PORT");
    CENTER_CONTROLER_PORT = atoi(center_port.c_str());

    string monitor_port=xmlConfig.GetValue("SERVER_MONITOR_PORT");
    SERVER_MONITOR_PORT=atoi(monitor_port.c_str());


    strcpy(GATHER_AREA_ID, xmlConfig.GetValue("GATHER_AREA_ID").c_str());
    strcpy(GATHER_CHANNE_NO, xmlConfig.GetValue("GATHER_CHANNE_NO").c_str());
    strcpy(GATHER_I_E_TYPE, xmlConfig.GetValue("GATHER_I_E_TYPE").c_str());
    strcpy(GATHER_CHNL_TYPE, xmlConfig.GetValue("GATHER_CHNL_TYPE").c_str());

    m_publisher_server_ip	= xmlConfig.GetValue("PUBLISHER_SERVER_IP");
    string strPublishPort	= xmlConfig.GetValue("PUBLISHER_SERVER_PORT");
    m_publisher_server_port	= atoi(strPublishPort.c_str());


    //设备配置部分
    //这里采用TiXML来读节点数据
    TiXmlDocument doc;
    bool bopen = doc.LoadFile(cpPath);

    TiXmlElement* pRootElem = doc.RootElement();
    TiXmlElement* item_device = pRootElem->FirstChildElement("DEVICE");
    if (!item_device)
    {
        return;
    }

    TiXmlElement* item_device_id = item_device->FirstChildElement("DEVICEID");
    if (!item_device_id)
    {
        return;
    }

    T_Gather_Device* pGatherDevice = new T_Gather_Device;
    pGatherDevice->is_data_full=1;
    pGatherDevice->is_active=0;

    char* pText = (char*) item_device_id->GetText();

    if (pText)
    {
        strcpy(pGatherDevice->device_id, pText);
        pGatherDevice->last_keeplive = 0;
    }


    pText = (char*) item_device_id->Attribute("IS_CAPTURE_DEVICE");
    if (pText)
    {
        pGatherDevice->is_capture_device = atoi(pText);
    }


    pText = (char*) item_device_id->Attribute("TAG");
    if (pText)
    {
        strcpy(pGatherDevice->device_tag, pText);
    }

    m_StaDeviceMap[string(pGatherDevice->device_id)] = pGatherDevice;
    m_StaTagDeviceMap[string(pGatherDevice->device_tag)] = pGatherDevice;

    CMyLog::m_pLog->_XGSysLog("read device id %s,device tag %s\n",pGatherDevice->device_id,pGatherDevice->device_tag);



    while ((item_device_id = item_device_id->NextSiblingElement()) != NULL)
    {
        T_Gather_Device* pGatherDevice = new T_Gather_Device;
        pGatherDevice->is_data_full=1;
        pGatherDevice->is_active=0;
        
        char* pText = (char*) item_device_id->GetText();

        if (pText)
        {
            strcpy(pGatherDevice->device_id, pText);
            pGatherDevice->last_keeplive = 0;
        }


        pText = (char*) item_device_id->Attribute("IS_CAPTURE_DEVICE");
        if (pText)
        {
            pGatherDevice->is_capture_device = atoi(pText);
        }

        pText = (char*) item_device_id->Attribute("TAG");
        if (pText)
        {
            strcpy(pGatherDevice->device_tag, pText);
        }

        m_StaDeviceMap[string(pGatherDevice->device_id)] = pGatherDevice;
        m_StaTagDeviceMap[string(pGatherDevice->device_tag)] = pGatherDevice;

        CMyLog::m_pLog->_XGSysLog("read device id %s,device tag %s\n",pGatherDevice->device_id,pGatherDevice->device_tag);
    }

    //CONTROL SEQUENCE
    TiXmlElement* item_sequence = pRootElem->FirstChildElement("CONTROL_SEQUENCE");
    if (!item_sequence)
    {
        return;
    }

    TiXmlElement* item_event = item_sequence->FirstChildElement("EVENT");
    if (!item_event)
    {
        return;
    }

    T_Sequence_Control* pSequenceControl = new T_Sequence_Control;
    pSequenceControl->recv_event_num = 0;

    pText = (char*) item_event->Attribute("DESC");

    if (pText)
    {
        strcpy(pSequenceControl->event_desc, pText);
    }

    pText = (char*) item_event->Attribute("ASSOC");

    if (pText)
    {
        pSequenceControl->event_assoc = atoi(pText);
    }


    TiXmlElement* item_recv = item_event->FirstChildElement("RECV_EVENT");
    if (item_recv)
    {
        pText = (char*) item_recv->GetText();
        if (pText)
        {
            pSequenceControl->recv_event[pSequenceControl->recv_event_num].is_recv_event = 0;
            strcpy(pSequenceControl->recv_event[pSequenceControl->recv_event_num].event_id, pText);
        }

        pSequenceControl->recv_event_num++;
    }

    CMyLog::m_pLog->_XGSysLog("item recv is %p \n",item_recv);
    
    if(!item_recv)
    {
        CMyLog::m_pLog->_XGSysLog("please check config file...... \n");
        return;
    }
    
    while ((item_recv = item_recv->NextSiblingElement()) != NULL)
    {
        pText = (char*) item_recv->Value();
        if (pText && strcmp(pText, "RECV_EVENT") == 0)
        {

            pText = (char*) item_recv->GetText();
            if (pText)
            {
                pSequenceControl->recv_event[pSequenceControl->recv_event_num].is_recv_event = 0;
                strcpy(pSequenceControl->recv_event[pSequenceControl->recv_event_num].event_id, pText);
            }

            pSequenceControl->recv_event_num++;
        }
        else
        {
            break;
        }

    }


    pSequenceControl->post_event_num = 0;
    TiXmlElement* item_post = item_event->FirstChildElement("POST_EVENT");
    pText = (char*) item_post->GetText();
    if (pText)
    {
        strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].event_id, pText);
    }

    pText = (char*) item_post->Attribute("TAG");
    if (pText)
    {
        strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].post_tag, pText);
    }

    pSequenceControl->post_event_num++;

    while ((item_post = item_post->NextSiblingElement()) != NULL)
    {
        pText = (char*) item_post->GetText();
        if (pText)
        {
            strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].event_id, pText);
        }

        pText = (char*) item_post->Attribute("TAG");
        if (pText)
        {
            strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].post_tag, pText);
        }

        pSequenceControl->post_event_num++;
    }


    m_StaSequenceControlVec.push_back(pSequenceControl);

    while ((item_event = item_event->NextSiblingElement()) != NULL)
    {


        T_Sequence_Control* pSequenceControl = new T_Sequence_Control;
        pSequenceControl->recv_event_num = 0;

        pText = (char*) item_event->Attribute("DESC");

        if (pText)
        {
            strcpy(pSequenceControl->event_desc, pText);
        }

        pText = (char*) item_event->Attribute("ASSOC");

        if (pText)
        {
            pSequenceControl->event_assoc = atoi(pText);
        }

        TiXmlElement* item_recv = item_event->FirstChildElement("RECV_EVENT");
        if (item_recv)
        {
            pText = (char*) item_recv->GetText();
            if (pText)
            {
                pSequenceControl->recv_event[pSequenceControl->recv_event_num].is_recv_event = 0;
                strcpy(pSequenceControl->recv_event[pSequenceControl->recv_event_num].event_id, pText);
            }

            pSequenceControl->recv_event_num++;
        }

        while ((item_recv = item_recv->NextSiblingElement()) != NULL)
        {
            pText = (char*) item_recv->Value();
            if (pText && strcmp(pText, "RECV_EVENT") == 0)
            {

                pText = (char*) item_recv->GetText();
                if (pText)
                {
                    pSequenceControl->recv_event[pSequenceControl->recv_event_num].is_recv_event = 0;
                    strcpy(pSequenceControl->recv_event[pSequenceControl->recv_event_num].event_id, pText);
                }

                pSequenceControl->recv_event_num++;
            }
            else
            {
                break;
            }

        }


        pSequenceControl->post_event_num = 0;
        TiXmlElement* item_post = item_event->FirstChildElement("POST_EVENT");
        pText = (char*) item_post->GetText();
        if (pText)
        {
            strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].event_id, pText);
        }

        pText = (char*) item_post->Attribute("TAG");
        if (pText)
        {
            strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].post_tag, pText);
        }

        pSequenceControl->post_event_num++;

        while ((item_post = item_post->NextSiblingElement()) != NULL)
        {
            pText = (char*) item_post->GetText();
            if (pText)
            {
                strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].event_id, pText);
            }

            pText = (char*) item_post->Attribute("TAG");
            if (pText)
            {
                strcpy(pSequenceControl->post_event[pSequenceControl->post_event_num].post_tag, pText);
            }

            pSequenceControl->post_event_num++;
        }


        m_StaSequenceControlVec.push_back(pSequenceControl);

    }

    //EXCEPTION part 
    TiXmlElement* item_exception_sequence = pRootElem->FirstChildElement("EXCEPTION_SEQUENCE");
    if (!item_exception_sequence)
    {
        return;
    }

    TiXmlElement* item_exception_event = item_exception_sequence->FirstChildElement("EVENT");
    if (!item_exception_event)
    {
        return;
    }

    T_Sequence_Exception* pSequenceException = new T_Sequence_Exception;
    pText = (char*) item_exception_event->Attribute("DESC");

    if (pText)
    {
        strcpy(pSequenceException->exception_desc, pText);
    }

    TiXmlElement* item_exception_recv = item_exception_event->FirstChildElement("RECV_EVENT");
    pText = (char*) item_exception_recv->GetText();
    if (pText)
    {
        strcpy(pSequenceException->recv_event, pText);
    }


    TiXmlElement* item_exception_wait = item_exception_event->FirstChildElement("WAIT_EVENT");
    pText = (char*) item_exception_wait->GetText();
    if (pText)
    {
        strcpy(pSequenceException->wait_event, pText);
    }


    TiXmlElement* item_exception_post = item_exception_event->FirstChildElement("POST_EVENT");
    pText = (char*) item_exception_post->GetText();
    if (pText)
    {
        strcpy(pSequenceException->post_event, pText);
    }


    TiXmlElement* item_exception_max = item_exception_event->FirstChildElement("MAX_WAIT");
    pText = (char*) item_exception_max->GetText();
    if (pText)
    {
        pSequenceException->wait_time = atoi(pText);
    }

    m_StaSequenceExceptionVec.push_back(pSequenceException);

    while ((item_exception_event = item_exception_event->NextSiblingElement()) != NULL)
    {

        TiXmlElement* item_exception_sequence = pRootElem->FirstChildElement("EXCEPTION_SEQUENCE");
        if (!item_exception_sequence)
        {
            return;
        }

        TiXmlElement* item_exception_event = item_exception_sequence->FirstChildElement("EVENT");
        if (!item_exception_event)
        {
            return;
        }

        T_Sequence_Exception* pSequenceException = new T_Sequence_Exception;
        char* pText = (char*) item_exception_event->Attribute("DESC");

        if (pText)
        {
            strcpy(pSequenceException->exception_desc, pText);
        }

        TiXmlElement* item_exception_recv = item_exception_event->FirstChildElement("RECV_EVENT");
        pText = (char*) item_exception_recv->GetText();
        if (pText)
        {
            strcpy(pSequenceException->recv_event, pText);
        }


        TiXmlElement* item_exception_wait = item_exception_event->FirstChildElement("WAIT_EVENT");
        pText = (char*) item_exception_wait->GetText();
        if (pText)
        {
            strcpy(pSequenceException->wait_event, pText);
        }


        TiXmlElement* item_exception_post = item_exception_event->FirstChildElement("POST_EVENT");
        pText = (char*) item_exception_post->GetText();
        if (pText)
        {
            strcpy(pSequenceException->post_event, pText);
        }


        TiXmlElement* item_exception_max = item_exception_event->FirstChildElement("MAX_WAIT");
        pText = (char*) item_exception_max->GetText();
        if (pText)
        {
            pSequenceException->wait_time = atoi(pText);
        }

        m_StaSequenceExceptionVec.push_back(pSequenceException);


    }

}


