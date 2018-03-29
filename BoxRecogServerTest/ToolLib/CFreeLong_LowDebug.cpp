// CFreeLong_LowDebug.cpp: implementation of the CCFreeLong_LowDebug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CFreeLong_LowDebug.h"
#include "SafePrint.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////		   
CCFreeLong_LowDebug::CCFreeLong_LowDebug(char* szPathName,
										 char* szAppName,
										 bool  bPrint2TTYFlag,
										 _APP_INFO_OUT_CALLBACK pInfoOutCallback,
										 void* pInfoOutCallbackParam)
{

	//��¼�ص�����ָ��
	m_pInfoOutCallback=pInfoOutCallback;
	m_pInfoOutCallbackParam=pInfoOutCallbackParam;

	//��¼�������Ļ�ı���
	m_bPrint2TTYFlag=bPrint2TTYFlag;

	//ƴ������ļ���
	if(szAppName)
	{
		FULL_NAME(szPathName,szAppName,m_szFileName,"dbg");
	}
	else
	{
		m_szFileName[0]='\0';
	}

	//��ɾ���ϴε����кۼ������������������
	DeleteAFile(m_szFileName);
	m_lWriteSize=0;
	Debug2File("CCFreeLong_LowDebug:Start!\n");

}

CCFreeLong_LowDebug::~CCFreeLong_LowDebug()
{
	Debug2File("CCFreeLong_LowDebug:Stop!\n");
}

char* CCFreeLong_LowDebug::GetTrueFileName(char* szBuffer)
{
	char* pRet=szBuffer;
	int nLen=strlen(szBuffer);
	int i=0;
	for(i=nLen-1;i>=0;i--)                //��������
	{
		if('\\'==*(szBuffer+i))           //Windows��·��
		{
			pRet=(szBuffer+i+1);
			break;
		}

		if('/'==*(szBuffer+i))            //Linux/Unix��·��
		{
			pRet=(szBuffer+i+1);
			break;
		}
	}

	return pRet;             //���ص����ҵ���λ�ã��������ﲻ��Ҫ�ͷŻ�����
}


void CCFreeLong_LowDebug::DeleteAFile(char* szFileName)
{
	remove(szFileName);
}

int CCFreeLong_LowDebug::Debug2File(char* szFormat,...)
{
	char szBuf[DEBUG_BUFFER_LENGTH]={0};
	char szTemp[DEBUG_BUFFER_LENGTH]={0};
	char szTime[DEBUG_BUFFER_LENGTH]={0};

	FILE* fp=NULL;
	int nListCount=0;
	va_list pArgList;
	time_t t;
	struct tm *pTM=NULL;
	int nLength=0;
	
	//�����ǹ���ʱ���
	GetATimeStamp(szTemp,DEBUG_BUFFER_LENGTH);
	SafePrint(szTime,DEBUG_BUFFER_LENGTH,"[%s]",szTemp);

	bool bOverrideFlag=false;      //�Ƿ񸲸Ǳ�־
	//ע�⣬���濪ʼ����������
	m_Lock.Lock();
	{
		va_start(pArgList,szFormat);
		nListCount+=Linux_Win_vsnprintf(szBuf+nListCount,
			                            DEBUG_BUFFER_LENGTH-nListCount,
										szFormat,
										pArgList);
		va_end(pArgList);

		if(nListCount>(DEBUG_BUFFER_LENGTH-1))
		{
			nListCount=DEBUG_BUFFER_LENGTH-1;
		}

		*(szBuf+nListCount)='\0';

		//��ʼ��ʵ�����
		fp=fopen(m_szFileName,"a+");

		if(fp)
		{
			//������ļ�
			nListCount=fprintf(fp,"%s%s",szTime,szBuf);
			m_lWriteSize+=nListCount;
			if(m_lWriteSize > DEBUG_MAX_FILE_SIZE)    //�ļ���С�������ƣ����븲��ԭ�����ļ�
			{
				m_lWriteSize=0;
				bOverrideFlag=true;
			}
			if(m_bPrint2TTYFlag)
			{
				//������Ҫ���������̨
				sprintf(szTemp,"%s%s\n",szTime,szBuf);
				printf(szTemp);

				if(m_pInfoOutCallback)
				{
					char szInfoOut[DEBUG_BUFFER_LENGTH*4];
					SafePrint(szInfoOut,DEBUG_BUFFER_LENGTH*4,"%s%s",szTime,szBuf);
					m_pInfoOutCallback(szInfoOut,m_pInfoOutCallbackParam);
				}	
			}

			fclose(fp);
		}
		else
		{
			nListCount=0;
		}
	}

	m_Lock.Unlock();

	if(bOverrideFlag)
	{
		fp=fopen(m_szFileName,"wb");              //��wb��ʽ���ļ������Ǹ���ԭ�����ļ�
		fclose(fp);
	}
	return nListCount;
}

void CCFreeLong_LowDebug::Debug2File4Bin(char* pBuffer,int nLength)
{
	m_Lock.Lock();
	{
//		dbg2file4bin(m_szFileName,"a+",pBuffer,nLength);
		if(m_bPrint2TTYFlag)
		{
			dbg_bin(pBuffer,nLength);
		}
	}
	m_Lock.Unlock();
}


