#ifdef WIN32
#include "stdafx.h"
#else
#endif

#include "SysUtil.h"
#include "SysMessage.h"
#define  MAX_HOST_DATA       (4096)

SysUtil::SysUtil()
{
}

SysUtil::~SysUtil()
{
}

int SysUtil::CreateSocket()
{
    int nSocket;
    nSocket = (int) socket(PF_INET, SOCK_STREAM, 0);
    return nSocket;
}

int SysUtil::CheckSocketResult(int nResult)
{
    //	check result;
    if (nResult == -1)
        return 0;
    else
        return 1;
}

int SysUtil::CheckSocketValid(int nSocket)
{
    //	check socket valid
    if (nSocket == -1)
        return 0;
    else
        return 1;
}

int SysUtil::BindPort(int nSocket, int nPort)
{
    int rc;
    int optval = 1;
    rc = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR,
            (const char *) & optval, sizeof (int));

    if (!CheckSocketResult(rc))
    {
        return 0;
    }

    sockaddr_in name;
    memset(&name, 0, sizeof (sockaddr_in));
    name.sin_family = AF_INET;
    name.sin_port = htons((unsigned short) nPort);
    name.sin_addr.s_addr = INADDR_ANY;

    rc = bind(nSocket, (sockaddr *) & name, sizeof (sockaddr_in));
    if (!CheckSocketResult(rc))
    {
        return 0;
    }

    return 1;
}

int SysUtil::ConnectSocket(int nSocket, const char * szHost, int nPort)
{
    hostent *pHost = NULL;
#ifdef WIN32
	pHost = gethostbyname(szHost);
    if (pHost == 0)
    {
        return 0;
    }
#else
    hostent localHost;
    char pHostData[MAX_HOST_DATA];
    int h_errorno = 0;
#endif

#ifdef	WIN32
    pHost = gethostbyname(szHost);
    if (pHost == 0)
    {
        return 0;
    }
#else
    int h_rc = gethostbyname_r(szHost, &localHost, pHostData, MAX_HOST_DATA, &pHost, &h_errorno);
    if ((pHost == 0) || (h_rc != 0))
    {
        return 0;
    }
#endif

    struct in_addr in;
    memcpy(&in.s_addr, pHost->h_addr_list[0], sizeof (in.s_addr));
    sockaddr_in name;
    memset(&name, 0, sizeof (sockaddr_in));
    name.sin_family = AF_INET;
    name.sin_port = htons((unsigned short) nPort);
    name.sin_addr.s_addr = in.s_addr;
#ifdef WN32
    int rc = connect((SOCKET) nSocket, (sockaddr *) & name, sizeof (sockaddr_in));
#else
    int rc = connect(nSocket, (sockaddr *) & name, sizeof (sockaddr_in));
#endif
    if (rc >= 0)
        return 0;
    return -1;

}

int SysUtil::CloseSocket(int nSocket)
{
    int rc = 1;
    if (!CheckSocketValid(nSocket))
    {
        return rc;
    }
#ifdef WIN32
    shutdown((SOCKET) nSocket, SD_BOTH);
    closesocket((SOCKET) nSocket);
#else
    shutdown(nSocket, SHUT_RDWR);
    close(nSocket);
#endif
    rc = 0;
    return rc;
}

int SysUtil::ListenSocket(int nSocket, int nMaxQueue)
{
    int rc = 0;
    rc = listen(nSocket, nMaxQueue);
    return CheckSocketResult(rc);
}

void SysUtil::SetSocketNotBlock(int nSocket)
{
    //	�ı��ļ����Ϊ������ģʽ
#ifdef WIN32
    unsigned long block = 1;
    ioctlsocket(nSocket, FIONBIO, &block);

#else
    int iFlags = fcntl(nSocket, F_GETFL);
    fcntl(nSocket, F_SETFL, iFlags | ~O_NONBLOCK);


#endif


}

int SysUtil::CheckSocketError(int nResult)
{
    //	���������׽��ִ���
    if (nResult > 0)
        return 0;
    if (nResult == 0)
        return 1;

#ifdef WIN32
    if (WSAGetLastError() == WSAEWOULDBLOCK)
        return 0;
    else
        return 1;
#else
    if (errno == EAGAIN)
        return 0;
    return 1;
#endif


}

int SysUtil::SocketWrite(int nSocket, char * pBuffer, int nLen, int nTimeout)
{
    int nOffset = 0;
    int nWrite;
    int nLeft = nLen;
    int nLoop = 0;
    int nTotal = 0;
    int nNewTimeout = nTimeout * 10;

    /*
     *	�����nTimeout��ʵ���ط�������������ʵ�ʵ�ʱ��
     */

    while ((nLoop <= nNewTimeout) && (nLeft > 0))
    {
        nWrite = send(nSocket, pBuffer + nOffset, nLeft, 0);
        if (nWrite == 0)
        {
            return -1;
        }
#ifdef WIN32
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
            //	��ʱ100ms
            SysSleep(100);
        }
        nLoop++;
    }


    return nTotal;



}

int SysUtil::SocketRead(int nSocket, void * pBuffer, int nLen)
{
    if (nSocket == -1)
        return -1;
    int len = 0;
#ifdef WIN32
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
#ifdef WIN32
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


int  SysUtil::SocketRead_n(int nSocket,void * pBuffer,int nLen)
{
	int nRecvlen = 0;
	int nSum     = 0;
	do 
	{
		nRecvlen = SocketRead(nSocket, pBuffer + nSum, nLen - nSum);
		if (nRecvlen < 0)
			return -1;

		nSum += nRecvlen;
	} while (nSum < nLen);
	return nSum;	
}


void SysUtil::SysSleep(long ms)
{

#ifdef WIN32
    Sleep(ms);
#else
    //	unix/linux����
    timespec localTimeSpec;
    timespec localLeftSpec;
    localTimeSpec.tv_sec = ms / 1000;
    localTimeSpec.tv_nsec = (ms % 1000)*1000000;
    nanosleep(&localTimeSpec, &localLeftSpec);
#endif

}

int SysUtil::ConnectSocket(int nSocket, unsigned long serverip, int port)
{
    struct sockaddr_in ServerAddress;
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = serverip;
    ServerAddress.sin_port = ntohs(port);
#ifdef WIN32
    int rc = connect((SOCKET) nSocket, (sockaddr *) & ServerAddress, sizeof (sockaddr_in));
#else
    int rc = connect(nSocket, (sockaddr *) & ServerAddress, sizeof (sockaddr_in));
#endif

    if (rc >= 0)
        return 0;
    return -1;
}

int SysUtil::InitNetLib()
{
#ifdef WIN32
    WSADATA wsd;
    int nRet = ::WSAStartup(MAKEWORD(2, 2), &wsd);
    if (nRet != 0)
    {
        return -1;
    }
#else
#endif
    return 0;
}

int SysUtil::ReleaseNetLib()
{
#ifdef WIN32
    ::WSACleanup();
#else
#endif

    return 0;
}

int SysUtil::ReceiveTimer(int fd)
{
    int iRet = 0;
    fd_set rset;
    struct timeval tv;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    tv.tv_sec = 0;
    tv.tv_usec = 500 * 1000;

    iRet = select(fd + 1, &rset, NULL, NULL, &tv);
    return iRet;
}

int SysUtil::SearchHeadPos(char *chBuffer, int nDataLen)
{
    const char szHead[4 + 1] = {(char)0xE2,(char)0x5C,(char)0x4B,(char)0x89, 0x00};

    int end, i, j;
    end = nDataLen;

    if (end > 0)
    {
        for (i = 0; i <= end; i++)
        {
            //ѭ���Ƚ�
            for (j = i; chBuffer[j] == szHead[j - i]; j++)
            {
                if (szHead[j - i + 1] == '\0')
                {
                    return i; /* �ҵ������ַ���   */
                }
            }
        }
    }

    return -1;
}

int SysUtil::SearchTailPos(char *chBuffer, int nDataLen)
{
    int end, i, j;
    end = nDataLen; /* �������λ��*/
    
    unsigned char szXmlLen[4] = {0};
    szXmlLen[0] = chBuffer[4];
    szXmlLen[1] = chBuffer[5];
    szXmlLen[2] = chBuffer[6];
    szXmlLen[3] = chBuffer[7];
    int nAllLength = 0;
    nAllLength = (unsigned int)szXmlLen[0] + (unsigned int)(szXmlLen[1] << 8) + (unsigned int)(szXmlLen[2] << 16) + (unsigned int)(szXmlLen[3] << 24);

    if (nDataLen < nAllLength)
    {
        return -1;
    }    
    return nAllLength;
    
}
//
//int SysUtil::SearchHeadPos(char *chBuffer, int nDataLen)
//{
//    int nHeadFlag   = -1;
//    int end, i, j;
//    //end = nDataLen - 8;  //�������λ��
//    end = nDataLen - 1;  //�������λ��
//
//    if (end > 0)
//    {
////        for (i = 0; i <= end; i++)
//        {
////            //ѭ���Ƚ�
////            for (j = i; chBuffer[j] == SYS_NET_MSGHEAD[j - i]; j++)
////            {
////                if (SYS_NET_MSGHEAD[j - i + 1] == '\0')
////                {
////                    return i; // �ҵ������ַ���   
////                }
////            }
//            
//            int ncount      = 0;           
//            for (int k = 0; k < 8; ++k)
//            {
//                if (chBuffer[k] == SYS_NET_MSGHEAD[k])
//                {
//                    ++ncount;
//                }
//                if (k == 7 && ncount == 8)
//                {
//                    nHeadFlag = 0;
//                }
//            }            
//        }
//    }
//
//    return nHeadFlag;
//}
//
//int SysUtil::SearchTailPos(char *chBuffer, int nDataLen)
//{
//    int nEndFlag   = -1;
//    int end, i;
//    //end = nDataLen - 8; // �������λ��
//    end = nDataLen; // �������λ��
//
//    if (end > 0)
//    {
//        for (i = 0; i < end; i += 1)
//        {
////            //ѭ���Ƚ�
////            for (j = i; chBuffer[j] == SYS_NET_MSGTAIL[j - i]; j++)
////            {
////                if (SYS_NET_MSGTAIL[j - i + 1] == '\0')
////                {
////                    return i; // �ҵ������ַ���   
////                }
////            }
//            
//            int ncount      = 0;           
//            for (int k = 0, j = i; k < 8 && j < end; ++k,++j)
//            {
//                if (chBuffer[j] == SYS_NET_MSGTAIL[k])
//                {
//                    ++ncount;
//                }
//                if (k == 7 && ncount == 8)
//                {
//                    nEndFlag = 0;
//                }
//            }
//            
//            if (nEndFlag == 0)
//            {
//                nEndFlag= i;
//                break;
//            }
//        }
//    }
//
//    return nEndFlag;
//
//}

