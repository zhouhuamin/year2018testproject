// CFreeLong_LowDebug.h: interface for the CCFreeLong_LowDebug class.
//
//////////////////////////////////////////////////////////////////////
#if !defined _FREELONG_LOW_DEBUG_H_
#define _FREELONG_LOW_DEBUG_H_

#include "MutexLock.h"

#define DEBUG_BUFFER_LENGTH            1024                         //��󻺳�����С
#define DEBUG_MAX_FILENAME_LENGTH      256                          //����ļ�����С
#define DEBUG_MAX_FILE_SIZE            (1*1024*1024*1024)           //����ļ���С1G�������˾͸���ԭ���Ĵ�С
/*
 *	�ṩһ���ص����������û����Ի�ȡ��ӡ���������
 */
typedef void(*_APP_INFO_OUT_CALLBACK)(char* szInfo,void* pCallbackParam);

class CCFreeLong_LowDebug  
{
public:
	static void DeleteAFile(char* szFileName);                 //ɾ��һ���ļ�
	static char* GetTrueFileName(char* szBuffer);              //��ȡ�ļ�����ʵ����
public:
	int Debug2File(char* szFormat,...);                        //����ַ������ļ����߿���̨�������ֽ�����������'\0'
	void Debug2File4Bin(char* pBuffer,int nLength);            //���������һ���ڴ���
public:
	CCFreeLong_LowDebug(char* szPathName,                             //·����
						char* szAppName,                              //�ļ���
						bool  bPrint2TTYFlag=false,                   //�Ƿ��ӡ������̨��
						_APP_INFO_OUT_CALLBACK pInfoOutCallback=NULL, //����ص�
						void* pInfoOutCallbackParam=NULL);            //�ص���������
	
	 ~CCFreeLong_LowDebug();

public:
	_APP_INFO_OUT_CALLBACK m_pInfoOutCallback;
	void* m_pInfoOutCallbackParam;
private:
	bool       m_bPrint2TTYFlag;                                      //����̨�����־
	char       m_szFileName[DEBUG_MAX_FILENAME_LENGTH];               //ƴ�Ӻõ�·����+�ļ���
	CMutexLock m_Lock;                                                //�̰߳�ȫ��
	long       m_lWriteSize;
};

#endif 
