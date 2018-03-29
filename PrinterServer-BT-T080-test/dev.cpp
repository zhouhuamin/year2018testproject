#include "includes.h"
#include "reader1000api.h"
#include "dev.h"
#include "broadcast.h"
#include "packet_format.h"
#include "starup_handle.h"
#include "broadcast.h"
#include "signal.h"

#include <iconv.h>
#include <time.h>
#include <zmq.h>
#include "ace/Log_Priority.h"
#include "ace/Log_Msg.h"
#include "ace/SOCK_Connector.h"
#include "ace/Signal.h"
#include "SysMessage.h"
#include "MSGHandleCenter.h"
#include "jzLocker.h"
#include "ProcessBroadcast.h"
#include "tinyxml/tinyxml.h"



#include <map>
#include <algorithm>
#include <iterator>
#include <boost/timer.hpp>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/std/utility.hpp>
#include <boost/lexical_cast.hpp>

extern int gFd_dev;
static net_packet_head_t packet_head;
static u8 data_buf[0x800];
static char tmp_str[150];
int g_nArrivedFlag = 0; // 0Ϊ������״̬, 1Ϊ����״̬
locker g_nArrivedFlagLock;

extern int dev_retry_connect_handle(int *pFd_dev);
extern std::vector<structPrinterMsg2> g_MsgVect;
extern locker g_MsgVectLock;
extern sem g_StartEventSem;
extern sem g_StoppedEventSem;
extern sem g_BroadcastEventSem;
static int msg_LED;

static char gb_buf[200];

int broadcast_trans_handle(int fd_dev);

int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);

void SplitString(std::string source, std::vector<std::string>& dest, const std::string &division)
{
    if (source.empty())
    {
        return;
    }

    int pos = 0;
    int pre_pos = 0;
    while (-1 != pos)
    {
        pre_pos = pos;

        pos = source.find_first_of(division, pos);
        if (pos == -1)
        {
            std::string str = source.substr(pos + 1);
            dest.push_back(str);
            break;
        }

        std::string tmp = source.substr(pre_pos, (pos - pre_pos));
        dest.push_back(tmp);

        source = source.substr(pos + 1);
        pos = 0;
    }
}

void SplitStringByLen(const std::string &strSouce, std::vector<std::string> &strVect)
{
    std::string soure = strSouce;
    std::string str1 = "";
    int nLen = soure.size();
    int index = 0;
    int nConst = 36;
    int nIter = 0;
    //std::vector<std::string> strVect;
    while (1)
    {
        if (nLen <= 0)
        {
            break;
        }
        else if (nLen <= 38)
        {
            str1 = "";
            str1 = soure.substr(index, nLen);
            strVect.push_back(str1);
            break;
        }
        else
        {
            if (soure[nIter + index] & 0x80)
            {
                if (nIter < 38)
                {
                    nIter += 2;
                    continue;
                }
                else
                {
                    nIter += 2;
                    str1 = soure.substr(index, nIter);
                    strVect.push_back(str1);
                    index += nIter;
                    nLen -= nIter;
                    nIter = 0;
                    continue;

                }
            }
            else
            {
                if (nIter < 38)
                {
                    nIter += 1;
                    continue;
                }
                else
                {
                    nIter += 1;
                    str1 = soure.substr(index, nIter);
                    strVect.push_back(str1);
                    index += nIter;
                    nLen -= nIter;
                    nIter = 0;
                    continue;

                }
            }
        }
    }
}

int broadcast_msg_play(char *xml_buf, int xml_buf_len)
{
    //	xmlDocPtr doc;
    //	xmlNodePtr root_node, cur_node;
    //	xmlChar *led_msg, *str_tmp;
    int led_code = 0;
    char log_str[160];
    int len;
    int i;
    //	char * p_inter;
    //
    //	p_inter = strstr(xml_buf, "<BROADCAST>");
    //	xml_buf_len -= (p_inter - xml_buf);
    char *pXmlData = new char[xml_buf_len + 1];
    memcpy(pXmlData, xml_buf, xml_buf_len);
    pXmlData[xml_buf_len] = '\0';
    std::string strXmlDataFront = "<?xml version=\"1.0\" encoding=\"GB2312\"?>";
    std::string strXmlDataBack = pXmlData;
    std::string strXmlData = strXmlDataFront;
    //strXmlData      += strXmlDataBack;

    char *pItemStart = strstr(pXmlData, "<PRINTER>");
    char *pItemEnd = strstr(pXmlData, "</PRINTER>");
    if (pItemStart != NULL && pItemEnd != NULL && pItemStart < pItemEnd)
    {
        int nFront = pItemEnd - pItemStart + 12;
        char *pTmpData = new char[nFront + 1];
        memcpy(pTmpData, pItemStart, nFront);
        pTmpData[nFront] = '\0';
        strXmlDataBack = pTmpData;
        delete []pTmpData;
        strXmlData += strXmlDataBack;
    }
    else
    {
        delete []pXmlData;
        return 0;
    }
    delete []pXmlData;

    // ======================================================================================
    TiXmlDocument doc;
    doc.Parse(strXmlData.c_str());

    TiXmlHandle docHandle(&doc);
    TiXmlHandle COUNTERHandle = docHandle.FirstChildElement("PRINTER").ChildElement("COUNTER", 0).FirstChild();
    TiXmlHandle DATAHandle = docHandle.FirstChildElement("PRINTER").ChildElement("DATA", 0).FirstChild();

    std::string COUNTER = "";
    std::string DATA = "";

    if (COUNTERHandle.Node() != NULL)
        COUNTER = COUNTERHandle.Text()->Value();
    if (DATAHandle.Node() != NULL)
        DATA = DATAHandle.Text()->Value();

    std::vector<std::string> printVect;
    std::size_t pos1 = DATA.find("$");
    if (pos1 != std::string::npos)
    {
        std::string source = DATA;
        std::string division = "$";
        SplitString(source, printVect, division);
    }
    else
    {
        printVect.push_back(DATA);
    }




    for (int k = 0; k < printVect.size(); ++k)
    {
        structPrinterMsg2 tmpMsg;
        tmpMsg.strCount = COUNTER;
        // ========================================================================================   
        std::string source = printVect[k]; // DATA;
        std::vector<std::string> dest;
        std::string division = "%";
        SplitString(source, dest, division);
        tmpMsg.itemVect = dest;

        syslog(LOG_DEBUG, "source:%s\n", source.c_str());

        for (int i = 0; i < dest.size(); ++i)
        {
            syslog(LOG_DEBUG, "x:%s\n", dest[i].c_str());
        }


        g_MsgVectLock.lock();
        g_MsgVect.push_back(tmpMsg);
        g_MsgVectLock.unlock();



        g_BroadcastEventSem.post();

        sleep(1);
    }


    //xmlFree(led_msg);
    //xmlFreeDoc(doc);
    return 0;
}

int print_buys_info(const structPrinterMsg2 &tmpMsg)
{
    int nLen = 0;
    ProcessBroadcast broadcast;
    int socket_ = broadcast.CreateSocket();
    if (socket_ == -1)
    {
        syslog(LOG_DEBUG, "Can't create socket\n");
        sleep(3);
        socket_ = broadcast.CreateSocket();
        if (socket_ == -1)
        {
            return -1;
        }
    }

    struct timeval timeo = {15, 0};
    socklen_t len = sizeof (timeo);
    timeo.tv_sec = 5;
    setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);

    // gSetting_system.com_ip, gSetting_system.com_port
    std::string strIP = gSetting_system.com_ip;
    if (broadcast.ConnectSocket(socket_, strIP.c_str(), gSetting_system.com_port) <= 0)
    {
        if (errno == EINPROGRESS)
        {
            syslog(LOG_DEBUG, "Time Out!\n");
            return -1;
        }
        broadcast.CloseSocket(socket_);
        return -1;
    }
    broadcast.set_keep_live(socket_, 3, 30);

    // 0A(����ӡλ����������)
    char szFirstRow[1 + 1] = {0x0A, 0x00};
    int nRet = broadcast.SocketWrite(socket_, szFirstRow, 1, 100);
    broadcast.SysSleep(100);

    //	// 1) ��ʼ����ӡ�� 1B 40��
    //	char szInitialPrinter[2 + 1]	= {0x1B, 0x40, 0x00};
    //	nRet=broadcast.SocketWrite(socket_, szInitialPrinter, 2, 100);
    //	broadcast.SysSleep(100);

    // 2) ģʽѡ�� 1B 53 �����׼ģʽ��Ĭ�ϣ���1B 4C ����ҳģʽ��
    char szModeSelect[2 + 1] = {0x1B, 0x53, 0x00};
    nRet = broadcast.SocketWrite(socket_, szModeSelect, 2, 100);
    broadcast.SysSleep(100);

    // ѡ���ӡģʽ
    char szBold1[3 + 1] = {0x1B, 0x21, 0x08, 0x00};
    nRet = broadcast.SocketWrite(socket_, szBold1, 3, 100);

    // ѡ���ַ���С
    char szSize[3 + 1] = {0x1D, 0x21, 0x11, 0x00};
    nRet = broadcast.SocketWrite(socket_, szSize, 3, 100);

    {
        // ѡ���ַ�����ģʽ-�м����
        char szPosition1[3 + 1] = {0x1B, 0x61, 0x01, 0x00};
        nRet = broadcast.SocketWrite(socket_, szPosition1, 3, 100);
        broadcast.SysSleep(100);
        {
            char szLarge[3 + 1] = {0x1C, 0x21, 0x0C, 0x00};
            nRet = broadcast.SocketWrite(socket_, szLarge, 3, 100);
            broadcast.SysSleep(100);
        }

//        {
//            std::string strSinglePound = "                     ";
//            std::string strValue1 = "";
//            strSinglePound += strValue1;
//            nRet = broadcast.SocketWrite(socket_, (char *) strSinglePound.c_str(), strSinglePound.size(), 100);
//            char szNextPosition11[1 + 1] = {0x0A, 0x00}; // ��ӡ����ֽһ��
//            nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);
//        }
//        {
//            std::string strSinglePound = "                     ";
//            std::string strValue1 = "";
//            strSinglePound += strValue1;
//            nRet = broadcast.SocketWrite(socket_, (char *) strSinglePound.c_str(), strSinglePound.size(), 100);
//            char szNextPosition11[1 + 1] = {0x0A, 0x00}; // ��ӡ����ֽһ��
//            nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);
//        }


        std::string strSinglePound = "  ��ͨ�ǻ�բ��СƱ";
        std::string strValue1 = "";
        strSinglePound += strValue1;
        nRet = broadcast.SocketWrite(socket_, (char *) strSinglePound.c_str(), strSinglePound.size(), 100);
        char szNextPosition11[1 + 1] = {0x0A, 0x00}; // ��ӡ����ֽһ��
        nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);

        {
            std::string strSinglePound = "                     ";
            std::string strValue1 = "";
            strSinglePound += strValue1;
            nRet = broadcast.SocketWrite(socket_, (char *) strSinglePound.c_str(), strSinglePound.size(), 100);
            char szNextPosition11[1 + 1] = {0x0A, 0x00}; // ��ӡ����ֽһ��
            nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);
        }
        //                {
        //                    std::string strSinglePound	= "                     ";
        //                    std::string strValue1		= "";
        //                    strSinglePound += strValue1;
        //                    nRet=broadcast.SocketWrite(socket_, (char *)strSinglePound.c_str(), strSinglePound.size(), 100);
        //                    char szNextPosition11[1 + 1]		= {0x0A, 0x00};				// ��ӡ����ֽһ��
        //                    nRet=broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);                    
        //                }                  

    }

    {
        char szPosition1[3 + 1] = {0x1B, 0x61, 0x00, 0x00}; // ѡ���ַ�����ģʽ
        nRet = broadcast.SocketWrite(socket_, szPosition1, 3, 100);
        broadcast.SysSleep(100);
    }


    //        {
    //            // bold
    //            char szSize[3 + 1]			= {0x1D, 0x21, 0x01, 0x00};
    //            nRet=broadcast.SocketWrite(socket_, szSize, 3, 100);                
    //        }


    {
        // ѡ���ַ���С
        char szSize[3 + 1] = {0x1D, 0x21, 0x00, 0x00};
        nRet = broadcast.SocketWrite(socket_, szSize, 3, 100);
    }

    // ============================================================================================
    syslog(LOG_DEBUG, "itemVect size:%d\n", tmpMsg.itemVect.size());
    int nLine = 0;
    int nSize = tmpMsg.itemVect.size();
    for (int i = 0; i < nSize; ++i)
    {
        if (tmpMsg.itemVect[i].empty())
            continue;

        std::vector<std::string> strVect;
        SplitStringByLen(tmpMsg.itemVect[i], strVect);


        //            char szPosition1[4 + 1]			= {0x1B, 0x5C, 0x19, 0x00, 0x00};
        //            nRet=broadcast.SocketWrite(socket_, szPosition1, 4, 100);
        //            broadcast.SysSleep(100);            
        ++nLine;
        char szItem[100] = {0};
        sprintf(szItem, "��Ŀ%d: ", nLine);
        std::string ID = ""; // szItem;

        std::string strMode1 = "���";
        std::string strMode2 = "����";
        std::string strMode3 = "��λ";
        
        std::string strMode4 = "������";

        for (int j = 0; j < strVect.size(); ++j)
        {
            // ѡ���ַ���С
            char szSize[3 + 1] = {0x1D, 0x21, 0x01, 0x00};
            nRet = broadcast.SocketWrite(socket_, szSize, 3, 100);

            char szPosition1[4 + 1] = {0x1B, 0x5C, 0x19, 0x00, 0x00}; // ������Ժ����ӡλ��
            nRet = broadcast.SocketWrite(socket_, szPosition1, 4, 100);
            //broadcast.SysSleep(100);                 
            if (j == 0)
            {
                ;
            }
            else
            {
                ID = "";
            }
            std::string strValue1 = strVect[j]; // tmpMsg.itemVect[i];
            ID += strValue1;


            if (ID.find(strMode1) != std::string::npos)
            {
                // ѡ���ַ���С
                char szSize[3 + 1] = {0x1D, 0x21, 0x12, 0x00};
                nRet = broadcast.SocketWrite(socket_, szSize, 3, 100);

                nRet = broadcast.SocketWrite(socket_, (char *) ID.c_str(), ID.size(), 100);
                char szNextPosition11[1 + 1] = {0x0A, 0x00};
                nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);

                continue;
            }
            
            
            if (ID.find(strMode2) != std::string::npos)
            {
                if (ID.find(strMode4) != std::string::npos)
                {
                    std::string strLeft  = "";
                    std::string strRight = "";
                    std::size_t found = ID.find(strMode2);
                    strLeft = ID.substr(0, found + 6);
                    if (ID.size() > found + 6)
                        strRight = ID.substr(found + 6);
                    // ѡ���ַ���С
                    char szSize[3 + 1] = {0x1D, 0x21, 0x12, 0x00};
                    nRet = broadcast.SocketWrite(socket_, szSize, 3, 100);
                    nRet = broadcast.SocketWrite(socket_, (char *) strLeft.c_str(), strLeft.size(), 100);                    
                    char szNextPosition11[1 + 1] = {0x0A, 0x00};
                    nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);     
                    
                    if (!strRight.empty())
                    {
                        char szPosition12[4 + 1] = {0x1B, 0x5C, 0x19, 0x00, 0x00}; // ������Ժ����ӡλ��
                        nRet = broadcast.SocketWrite(socket_, szPosition12, 4, 100);    
                        nRet = broadcast.SocketWrite(socket_, (char *) strRight.c_str(), strRight.size(), 100);
                        char szNextPosition11[1 + 1] = {0x0A, 0x00};
                        nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);                           
                    }
                }
                else
                {
                    // ѡ���ַ���С
                    char szSize[3 + 1] = {0x1D, 0x21, 0x12, 0x00};
                    nRet = broadcast.SocketWrite(socket_, szSize, 3, 100);

                    nRet = broadcast.SocketWrite(socket_, (char *) ID.c_str(), ID.size(), 100);
                    char szNextPosition11[1 + 1] = {0x0A, 0x00};
                    nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);                    
                }
                continue;
            }            
            
            if (ID.find(strMode3) != std::string::npos)
            {
                // ѡ���ַ���С
                char szSize[3 + 1] = {0x1D, 0x21, 0x12, 0x00};
                nRet = broadcast.SocketWrite(socket_, szSize, 3, 100);

                nRet = broadcast.SocketWrite(socket_, (char *) ID.c_str(), ID.size(), 100);
                char szNextPosition11[1 + 1] = {0x0A, 0x00};
                nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);

                continue;
            }   
            

            nRet = broadcast.SocketWrite(socket_, (char *) ID.c_str(), ID.size(), 100);
            char szNextPosition11[1 + 1] = {0x0A, 0x00};
            nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);
        }


        //broadcast.SysSleep(100);
    }

    for (int i = nSize; i < nSize + 1; ++i)
    {
        char szItem[100] = {0};
        sprintf(szItem, "��Ŀ%d: ", i + 1);
        char szPosition1[4 + 1] = {0x1B, 0x5C, 0x19, 0x00, 0x00};
        nRet = broadcast.SocketWrite(socket_, szPosition1, 4, 100);
        broadcast.SysSleep(100);
        std::string ID = "           "; // szItem;
        std::string strValue1 = "";
        ID += strValue1;
        nRet = broadcast.SocketWrite(socket_, (char *) ID.c_str(), ID.size(), 100);
        char szNextPosition11[1 + 1] = {0x0A, 0x00};
        nRet = broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);
        broadcast.SysSleep(100);
    }

    //        // ================================================================================================
    //        {
    //            char szPosition1[4 + 1]			= {0x1B, 0x5C, 0x19, 0x00, 0x00};
    //            nRet=broadcast.SocketWrite(socket_, szPosition1, 4, 100);
    //            broadcast.SysSleep(100);
    //            std::string ID	= "������:";
    //            std::string strValue1		= tmpMsg.buysInfo.CARD_TYPE;
    //            ID += strValue1;
    //            nRet=broadcast.SocketWrite(socket_, (char *)ID.c_str(), ID.size(), 100);
    //            char szNextPosition11[1 + 1]		= {0x0A, 0x00};
    //            nRet=broadcast.SocketWrite(socket_, szNextPosition11, 1, 100);
    //            broadcast.SysSleep(100);
    //        }
    //
    //
    //	char szPosition2[4 + 1]			= {0x1B, 0x5C, 0x19, 0x00, 0x00};
    //	nRet=broadcast.SocketWrite(socket_, szPosition2, 4, 100);
    //	broadcast.SysSleep(100);
    //	std::string PRINT_DATE	= "����:";
    //	std::string strValue2			= tmpMsg.buysInfo.PRINT_DATE;
    //	PRINT_DATE += strValue2;
    //	nRet=broadcast.SocketWrite(socket_, (char *)PRINT_DATE.c_str(), PRINT_DATE.size(), 100);
    //	char szNextPosition12[1 + 1]		= {0x0A, 0x00};
    //	nRet=broadcast.SocketWrite(socket_, szNextPosition12, 1, 100);
    //	broadcast.SysSleep(100);
    //        
    //
    //
    //	char szPosition3[4 + 1]			= {0x1B, 0x5C, 0x19, 0x00, 0x00};
    //	nRet=broadcast.SocketWrite(socket_, szPosition3, 4, 100);
    //	broadcast.SysSleep(100);
    //	std::string DEPARTMENT			= "��λ: ";
    //	std::string strValue3			= tmpMsg.buysInfo.DEPARTMENT;
    //	DEPARTMENT += strValue3;
    //	nRet=broadcast.SocketWrite(socket_, (char *)DEPARTMENT.c_str(), DEPARTMENT.size(), 100);
    //	char szNextPosition13[1 + 1]		= {0x0A, 0x00};
    //	nRet=broadcast.SocketWrite(socket_, szNextPosition13, 1, 100);
    //	broadcast.SysSleep(100);




    char szFeedPaper[3 + 1] = {0x1B, 0x64, 0x08, 0x00};
    nRet = broadcast.SocketWrite(socket_, szFeedPaper, 3, 100);
    broadcast.SysSleep(100);

    // CutPaper
    //char szCutPaper[3 + 1]			= {0x1D, 0x56, 0x00, 0x00};
    char szCutPaper[3 + 1] = {0x1D, 0x56, 0x01, 0x00};
    nRet = broadcast.SocketWrite(socket_, szCutPaper, 3, 100);
    broadcast.SysSleep(100);

    //SysUtil::CloseSocket(socket_);
    broadcast.SysSleep(1000);

    // ״̬��ѯ

    // DLE EOT n ʵʱ״̬����
    char szDLE_EOT1[3 + 1] = {0x10, 0x04, 0x01, 0x00}; // n = 1�������ӡ��״̬��
    nRet = broadcast.SocketWrite(socket_, szDLE_EOT1, 3, 100);
    char szRecvBuffer1[1 + 1] = {0};
    int nRecvLen1 = broadcast.SocketRead(socket_, szRecvBuffer1, 1);
    broadcast.SysSleep(100);

    BT_T080_STATUS tmpStatus1;
    tmpStatus1.byteStatus = (BYTE) szRecvBuffer1[0];

    std::string strStatusMessage = "";
    if (tmpStatus1.status.b0 == 0)
    {
        strStatusMessage = "�̶�Ϊ0";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus1.status.b1 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus1.status.b2 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus1.status.b3 == 0)
    {
        strStatusMessage = "����";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus1.status.b3 == 1)
    {
        strStatusMessage = "�ѻ�";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus1.status.b4 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus1.status.b7 == 0)
    {
        strStatusMessage = "�̶�Ϊ0";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }


    char szDLE_EOT2[3 + 1] = {0x10, 0x04, 0x02, 0x00}; // n = 2���ѻ�״̬��
    nRet = broadcast.SocketWrite(socket_, szDLE_EOT2, 3, 100);
    char szRecvBuffer2[1 + 1] = {0};
    int nRecvLen2 = broadcast.SocketRead(socket_, szRecvBuffer2, 1);
    broadcast.SysSleep(100);

    BT_T080_STATUS tmpStatus2;
    tmpStatus2.byteStatus = (BYTE) szRecvBuffer2[0];

    strStatusMessage = "";
    if (tmpStatus2.status.b0 == 0)
    {
        strStatusMessage = "�̶�Ϊ0";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus2.status.b1 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus2.status.b2 == 0)
    {
        strStatusMessage = "��ӡͷ�պ�";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus2.status.b2 == 1)
    {
        strStatusMessage = "��ӡͷ̧��";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }


    if (tmpStatus2.status.b3 == 0)
    {
        strStatusMessage = "δ����ֽ��";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus2.status.b3 == 1)
    {
        strStatusMessage = "������ֽ��";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus2.status.b4 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus2.status.b5 == 0)
    {
        strStatusMessage = "��ӡ����ȱֽ";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus2.status.b5 == 1)
    {
        strStatusMessage = "��ӡ��ȱֽ��ֹͣ��ӡ";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus2.status.b6 == 0)
    {
        strStatusMessage = "û�г������";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus2.status.b6 == 1)
    {
        strStatusMessage = "�д������";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus2.status.b7 == 0)
    {
        strStatusMessage = "�̶�Ϊ0";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }


    char szDLE_EOT3[3 + 1] = {0x10, 0x04, 0x03, 0x00}; // n = 3������״̬��
    nRet = broadcast.SocketWrite(socket_, szDLE_EOT3, 3, 100);
    char szRecvBuffer3[1 + 1] = {0};
    int nRecvLen3 = broadcast.SocketRead(socket_, szRecvBuffer3, 1);

    broadcast.SysSleep(100);

    BT_T080_STATUS tmpStatus3;
    tmpStatus3.byteStatus = (BYTE) szRecvBuffer3[0];

    strStatusMessage = "";
    if (tmpStatus3.status.b0 == 0)
    {
        strStatusMessage = "�̶�Ϊ0";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus3.status.b1 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus3.status.b2 == 0)
    {
        strStatusMessage = "����ֽģ�����ӡ������";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus3.status.b2 == 1)
    {
        strStatusMessage = "����ֽģ�����п�ֽ����";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }


    if (tmpStatus3.status.b3 == 0)
    {
        strStatusMessage = "�е��޴���";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus3.status.b3 == 1)
    {
        strStatusMessage = "�е��д���";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus3.status.b4 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus3.status.b5 == 0)
    {
        strStatusMessage = "�޲��ɻָ�����";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus3.status.b5 == 1)
    {
        strStatusMessage = "�в��ɻָ�����";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus3.status.b6 == 0)
    {
        strStatusMessage = "��ӡͷ�¶ȡ���ѹ�����Ҵ�ӡͷѹ��";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus3.status.b6 == 1)
    {
        strStatusMessage = "��ӡͷ�¶Ȼ��ѹ������Χ���ӡͷ̧��";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus3.status.b7 == 0)
    {
        strStatusMessage = "����ֽģ�����ӡ������";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }




    char szDLE_EOT4[3 + 1] = {0x10, 0x04, 0x04, 0x00}; // n = 4������ֽ������״̬��
    nRet = broadcast.SocketWrite(socket_, szDLE_EOT4, 3, 100);
    char szRecvBuffer4[1 + 1] = {0};
    int nRecvLen4 = broadcast.SocketRead(socket_, szRecvBuffer4, 1);
    broadcast.SysSleep(100);


    BT_T080_STATUS tmpStatus4;
    tmpStatus4.byteStatus = (BYTE) szRecvBuffer4[0];

    strStatusMessage = "";
    if (tmpStatus4.status.b0 == 0)
    {
        strStatusMessage = "�̶�Ϊ0";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus4.status.b1 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus4.status.b2 == 0 && tmpStatus4.status.b3 == 0)
    {
        strStatusMessage = "ֽ����������̽�⵽��ֽ";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus4.status.b2 == 1 && tmpStatus4.status.b3 == 1)
    {
        strStatusMessage = "ֽ����������̽�⵽��ֽ";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus4.status.b4 == 1)
    {
        strStatusMessage = "�̶�Ϊ1";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus4.status.b5 == 0 && tmpStatus4.status.b6 == 0)
    {
        strStatusMessage = "ȱֽ������̽�⵽��ֽ";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }
    if (tmpStatus4.status.b5 == 1 && tmpStatus4.status.b6 == 1)
    {
        strStatusMessage = "ȱֽ������̽�⵽ֽ��";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }

    if (tmpStatus4.status.b7 == 0)
    {
        strStatusMessage = "�̶�Ϊ0";
        syslog(LOG_DEBUG, "%s,%d\n", strStatusMessage.c_str(), __LINE__);
    }


    broadcast.CloseSocket(socket_);


    return 1;
}

int print_sales_info(const structPrinterMsg2 &tmpMsg)
{
    return 1;
}

int print_sales_info2(const structPrinterMsg2 &tmpMsg)
{
    return 1;
}

void ReceiveReaderData(const char* pXmlData, int nXmlLen, const char *pEventID)
{
    NET_PACKET_MSG* pMsg_ = NULL;
    CMSG_Handle_Center::get_message(pMsg_);
    if (!pMsg_)
    {
        return;
    }
    memset(pMsg_, 0, sizeof (NET_PACKET_MSG));
    pMsg_->msg_head.msg_type = SYS_MSG_PUBLISH_EVENT;
    pMsg_->msg_head.packet_len = sizeof (T_SysEventData);
    T_SysEventData* pReadData = (T_SysEventData*) pMsg_->msg_body;

    strcpy(pReadData->event_id, pEventID);
    pReadData->event_id[31] = '\0';
    strcpy(pReadData->device_tag, gSetting_system.gather_DEV_TAG);

    pReadData->is_data_full = 0; // 1;
    char szXMLGatherInfo[10 * 1024] = {0};
    strncpy(szXMLGatherInfo, pXmlData, nXmlLen);


    syslog(LOG_DEBUG, "%s\n", szXMLGatherInfo);
    strcpy(pReadData->xml_data, szXMLGatherInfo);
    pReadData->xml_data_len = strlen(szXMLGatherInfo) + 1;
    pMsg_->msg_head.packet_len = sizeof (T_SysEventData) + pReadData->xml_data_len;
    syslog(LOG_DEBUG, "pack len:%d\n", pMsg_->msg_head.packet_len);
    MSG_HANDLE_CENTER::instance()->put(pMsg_);
    return;
}

int broadcast_trans_handle(int fd_dev)
{
    int ret = 0;

    structPrinterMsg2 tmpMsg;

    g_MsgVectLock.lock();
    if (g_MsgVect.empty())
    {
        ret = 0;
    }
    else
    {
        ret = 1;
        tmpMsg = g_MsgVect[0];
        g_MsgVect.erase(g_MsgVect.begin());
    }
    g_MsgVectLock.unlock();

    if (ret > 0)
    {
        // ==========================================2016-8-5===================================================
        syslog(LOG_DEBUG, "==================================Play Start===============================\n");

        //if (tmpMsg.messageType == "13")
        {
            int nCounter = 0;
            nCounter = atoi(tmpMsg.strCount.c_str());
            for (int i = 0; i < nCounter; ++i)
                print_buys_info(tmpMsg);
        }

        syslog(LOG_DEBUG, "==================================Print Finished!===============================\n");

        std::string strXml = "";
        int nXmlLen = strXml.size();
        std::string strEventID = "EG_FINISHED_PRT";

        ReceiveReaderData(strXml.c_str(), nXmlLen, strEventID.c_str());
        return 1;
    }

    return 0;
}

void *pthread_dev_handle(void *arg)
{
    return 0;
}

void *pthread_dev_handle2(void *arg)
{
    int nResult = 0;
    signal(SIGPIPE, SIG_IGN);

    while (1)
    {
        nResult = g_BroadcastEventSem.wait();
        broadcast_trans_handle(gFd_dev);

        usleep(500 * 1000);
    }

    return 0;
}

void *pthread_dev_handle_test(void *arg)
{
    int nResult = 0;
    signal(SIGPIPE, SIG_IGN);

    while (1)
    {
        int ch = getchar();

        if (ch == 'p')
        {
            structPrinterMsg2 tmpMsg;
            tmpMsg.strCount = "1";

//std::string source = "��ͨ��ͷ�豸���ӵ� ���� IN%���1��RFSU2014398%����1��"
//        "29.04%�ƻ���λ1��A333021%����:JI HANG 77/����77%���Σ�1724N%%����/Ӫ����/����/����:"
//        "ZGXL/ZGXL/20GP/F%�ᵥ�ţ�ZHR1723NXMNT447%Ǧ��ţ�JF1209498%������/���ţ�YUXIONG/��DB8097%ж���ۣ�"
//        "CNTC1Ŀ�ĸۣ�%�쳣˵����%��բʱ�䣺2017-08-08 17:00:22%������ǩ���⾲ ��ͷǩ��:ZHZKAUTO%%% "
//        "��ͨ��ͷ�豸���ӵ� ���� IN%���2��HJLU1181284%����2��29.04%�ƻ���λ1��A333022%����:JI HANG "
//        "77/����77%���Σ�1724N%%����/Ӫ����/����/����:ZGXL/ZGXL/20GP/F%�ᵥ�ţ�ZHR1723NXMNT447%Ǧ��ţ�"
//        "JF1209486%������/���ţ�YUXIONG/��DB8097%ж���ۣ�CNTC1Ŀ�ĸۣ�%�쳣˵����%��բʱ�䣺2017-08-08 "
//        "17:00:22%������ǩ��:�⾲ ��ͷǩ��:ZHZKAUTO%%";
            
            
std::string source = "��ͨ��ͷ�豸���ӵ� ���� IN%���1��ZGXU2033975%����1��28.42%�ƻ���λ1��B351051%����:NAN HUI 17/�ϻ�17%���Σ�1739N%%����/Ӫ����/����/����:ZGXL/ZGXL/20GP/F%�ᵥ�ţ�NH1739NXMZG024%Ǧ��ţ�JF1147242%������/���ţ�GANGLE/��C87024%ж���ۣ�CNTC1Ŀ�ĸۣ�%�쳣˵����%��բʱ�䣺2017-11-14 14:47:06%������ǩ�� ��ͷǩ��:ZHZKAUTO%%%%%%%%$D2/NO;1;��ͨ��ͷ��������%����ͨ����%%���1��CXDU1650486%�ֳ�λ1��D213093%�ƻ���λ��OUT���ţ���C87024%����Ա��ZHZKAUTO2017-11-14 14:47:22%";            
            
            
            
        std::vector<std::string> dest;
        std::string division = "%";
        SplitString(source, dest, division);
        tmpMsg.itemVect = dest;            
            
            
//            tmpMsg.itemVect.push_back("��Ϣ����1");
//            tmpMsg.itemVect.push_back("��Ϣ����2");
//            tmpMsg.itemVect.push_back("��Ϣ����3");
//            tmpMsg.itemVect.push_back("��Ϣ����4");
//            tmpMsg.itemVect.push_back("��Ϣ����5");
//            tmpMsg.itemVect.push_back("��Ϣ����6");
//            tmpMsg.itemVect.push_back("��Ϣ����7");
//            tmpMsg.itemVect.push_back("��Ϣ����8");
//            tmpMsg.itemVect.push_back("��Ϣ����9");
//            tmpMsg.itemVect.push_back("��Ϣ����10");
//            tmpMsg.itemVect.push_back("��Ϣ����11");
//            tmpMsg.itemVect.push_back("��Ϣ����12");


            g_MsgVectLock.lock();
            g_MsgVect.push_back(tmpMsg);
            g_MsgVectLock.unlock();


            g_BroadcastEventSem.post();
        }

        sleep(3);
    }

    return 0;
}

int RFID_upload_arrived_data(int server_fd, const char *ic_number1, const char *ic_number2)
{
    syslog(LOG_DEBUG, "Enter RFID_upload_arrived_data\n");
    NET_gather_data *p_gather_data;
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;
    int w_len;
    xmlChar * xml_buf;
    char *p_send;
    int xml_len;
    int ret;

    packet_head.msg_type = MSG_TYPE_GATHER_DATA;
    strcpy(packet_head.szVersion, "1.0");

    p_gather_data = (NET_gather_data *) (data_buf + sizeof (net_packet_head_t)
            + 8);

    strcpy(p_gather_data->event_id, gSetting_system.arrived_DEV_event_id);
    sprintf(tmp_str, "ARRIVED:\t%s\t%s\n", ic_number1, ic_number2);

    ret = 0;
    memcpy(&(p_gather_data->is_data_full), &ret, sizeof (int));
    strcpy(p_gather_data->dev_tag, gSetting_system.gather_DEV_TAG);

    /**********************************XML*********************************/
    doc = xmlNewDoc(NULL);

    root_node = xmlNewNode(NULL, BAD_CAST "CAR");

    xmlDocSetRootElement(doc, root_node);

    xmlNewChild(root_node, NULL, BAD_CAST "VE_NAME", BAD_CAST "");
    xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO",
            BAD_CAST(ic_number1));

    xmlNewChild(root_node, NULL, BAD_CAST "CAR_EC_NO2", BAD_CAST(ic_number2));
    xmlNewChild(root_node, NULL, BAD_CAST "VE_CUSTOMS_NO", BAD_CAST "");

    xmlNewChild(root_node, NULL, BAD_CAST "VE_WT", BAD_CAST "");

    xmlDocDumpFormatMemoryEnc(doc, &xml_buf, &xml_len, "GB2312", 1);

    p_send = strstr((char*) xml_buf, "<CAR>");

    if (p_send == NULL)
    {
        return -1;
    }

    xml_len -= (p_send - (char*) xml_buf);

    p_gather_data->xml_data_len = xml_len + 1;
    memcpy(p_gather_data->xml_data, p_send, xml_len);
    p_gather_data->xml_data[xml_len] = 0;

    xmlFree(xml_buf);
    xmlFreeDoc(doc);

    xmlCleanupParser();

    xmlMemoryDump();
    /**********************************XML END*********************************/

    w_len = ((u8*) p_gather_data->xml_data - data_buf) + xml_len + 1;
    packet_head.packet_len = w_len - sizeof (net_packet_head_t) - 8;
    strcpy((char *) data_buf, "0XJZTECH");

    memcpy(data_buf + 8, &packet_head, sizeof (net_packet_head_t));
    strcpy((char*) (data_buf + w_len), "NJTECHJZ");
    w_len += 8;

    syslog(LOG_DEBUG, "upload arrived data w_len = %d\n", w_len);
    //	ReceiveReaderData(p_gather_data->xml_data, xml_len, p_gather_data->event_id);


    //ret = write(server_fd, data_buf, w_len);
    //if (ret != w_len)
    //{
    //	//LOG_M("upload gather data error! ret =%d\n", ret);
    //	syslog(LOG_DEBUG, "upload gather data error! ret =%d\n", ret);
    //}

    //log_send(tmp_str);

    return 0;
}

void GetCurTime(char *pTime)
{
    time_t t = time(0);
    struct tm *ld = NULL;
    char tmp[32] = {0};
    ld = localtime(&t);
    strftime(tmp, sizeof (tmp), "%Y-%m-%d %H:%M:%S", ld);
    memcpy(pTime, tmp, 64);
}

void BuildStatusString(struct struDeviceAndServiceStatus *pStatus, char *pMsg, int nCount)
{
    //        char szMsg[512] = {0};
    //		snprintf(szMsg, sizeof(szMsg), "NJJZTECH : |%s|%d|%s|%d|%s|%d|%s|%s|%s|%s|%d", \
//pStatus->szLocalIP, \
//pStatus->nLocalPort, \
//pStatus->szUpstreamIP, \
//pStatus->nUpstreamPort, \
//pStatus->szUpstreamObjectCode, \
//pStatus->nServiceType, \
//pStatus->szObjectCode, \
//pStatus->szObjectStatus, \
//pStatus->szConnectedObjectCode, \
//pStatus->szReportTime, \
//nCount);
    //		memcpy(pMsg, szMsg, sizeof(szMsg));

    using boost::property_tree::ptree;
    ptree pt;
    ptree pattr1;
    std::string AREA_ID = gSetting_system.GATHER_AREA_ID;
    std::string CHNL_NO = gSetting_system.GATHER_CHANNE_NO;
    std::string I_E_TYPE = gSetting_system.GATHER_I_E_TYPE;
    std::string strID = gSetting_system.register_DEV_ID;
    std::string NAME = "��������";
    std::string STATUS = "1";
    std::string REASON = "����";

    pattr1.add<std::string>("<xmlattr>.AREA_ID", AREA_ID);
    pattr1.add<std::string>("<xmlattr>.CHNL_NO", CHNL_NO);
    pattr1.add<std::string>("<xmlattr>.I_E_TYPE", I_E_TYPE);

    pt.add_child("DEVICE_STATUS_INFO", pattr1);
    pt.put("DEVICE_STATUS_INFO.DEVICES.ID", strID);
    pt.put("DEVICE_STATUS_INFO.DEVICES.NAME", NAME);
    pt.put("DEVICE_STATUS_INFO.DEVICES.STATUS", STATUS);
    pt.put("DEVICE_STATUS_INFO.DEVICES.REASON", REASON);

    // ��ʽ�������ָ�����루Ĭ��utf-8��
    boost::property_tree::xml_writer_settings<char> settings('\t', 1, "GB2312");
    //write_xml(filename, pt, std::locale(), settings);

    ostringstream oss;
    write_xml(oss, pt, settings);

    string strGatherXml = oss.str();

    memcpy(pMsg, strGatherXml.c_str(), strGatherXml.size());
    return;

}

void *pthread_worker_task(void *arg)
{
    char szPublisherIp[64] = {0};
    signal(SIGPIPE, SIG_IGN);

    void * pCtx = NULL;
    void * pSock = NULL;
    //使�??tcp??�?�?�???信�???�?�??��???????��??IP?��??�??92.168.1.2
    //??信使?��??�?�?�?�??�??766
    snprintf(szPublisherIp, sizeof (szPublisherIp), "tcp://%s:%d", gSetting_system.publisher_server_ip, gSetting_system.publisher_server_port);
    szPublisherIp[63] = '\0';
    const char * pAddr = szPublisherIp; // "tcp://192.168.1.101:7766";

    //??�?context
    if ((pCtx = zmq_ctx_new()) == NULL)
    {
        return 0;
    }
    //??�?socket
    if ((pSock = zmq_socket(pCtx, ZMQ_DEALER)) == NULL)
    {
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    int iSndTimeout = 5000; // millsecond
    //设置?��?��???
    if (zmq_setsockopt(pSock, ZMQ_RCVTIMEO, &iSndTimeout, sizeof (iSndTimeout)) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    //�??��????IP192.168.1.2�?�?�??766
    if (zmq_connect(pSock, pAddr) < 0)
    {
        zmq_close(pSock);
        zmq_ctx_destroy(pCtx);
        return 0;
    }
    //
    while (1)
    {
        static int i = 0;

        char szMsg[1024] = {0};


        //_snprintf(szMsg, sizeof(szMsg), "NJJZTECH : %3d", i++);
        //printf("Enter to send...\n");
        syslog(LOG_DEBUG, "Enter to send...\n");
        if (zmq_send(pSock, szMsg, sizeof (szMsg), 0) < 0)
        {
            //fprintf(stderr, "send message faild\n");
            syslog(LOG_ERR, "send message faild\n");
            continue;
        }
        syslog(LOG_DEBUG, "send message : [%s] succeed\n", szMsg);
        // getchar();
        //usleep (5000 * 1000);
        sleep(5);
    }

    return 0;
}

void *StartEventThread(void *arg)
{
    signal(SIGPIPE, SIG_IGN);

    int nResult = 0;
    while (1)
    {
        nResult = g_StartEventSem.wait();

        int nFlag = 0;
        g_MsgVectLock.lock();
        if (g_MsgVect.size() > 0)
        {
            nFlag = 1;
        }
        g_MsgVectLock.unlock();

        if (nFlag == 1)
        {
        }

        usleep(1000);
    }
    return 0;
}

int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen,
        char *outbuf, size_t outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}
