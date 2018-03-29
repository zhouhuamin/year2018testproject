// FreeLongLog.h: interface for the CFreeLongLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _FREELONG_LOG_H_
#define _FREELONG_LOG_H_
#include "MutexLock.h"
#include <vector>
using namespace std;



#define LOG_FILE_SIZE_MAX                         (1*1024*1024*1024)   //������־�ļ���󳤶�
#define LOG_ITEM_LENGTH_MAX                       (2*1024)             //����LOG��󳤶�2K
#define LOG_FILE_CHANGE_NAME_PRE_SECONDS          (60*60*24 )           //һСʱ��һ������
#define LOG_FILE_INFO_BUFFER_SIZE                 (256*1024)           //��־Ŀ¼��󳤶�
#define LOG_FILE_DEFAULT_HOLD                     (12)                  //����3�����־�ļ�
#define LOG_TIME_STRING_MAX                       (128)                //ʱ����ַ�����
#define FILENAME_STRING_LENGTH 256
#define APP_INFO_OIT_STRING_MAX                   (2*1024)


typedef void (*_APP_INFO_OUT_CALLBACK)(char* szInfo,void* pCallparam);
class CFreeLongLog  
{
public:
	static int MakeATimeString(char* szBuffer,int nBufferSize);  //����ʱ����ַ���
public:
	CFreeLongLog(
		    char* szLogPath,
			char* szAppName,
			int nHoldFileMax=LOG_FILE_DEFAULT_HOLD,
			bool bSyslogFlag=true,
			bool bDebugFlag=true,
			bool bDebug2Flag=false,
			bool bDebug3Flag=false,
			bool bPrint2ScrFlag=true
			);

	 ~CFreeLongLog();
public:
	void _XGDebug4Bin(char* pBuffer,int nLength);
	int  _XGSysLog(char* szFormat,...);
	int  _XGDebug(char* szFormat,...);
	int  _XGDebug2(char* szFormat,...);
	int  _XGDebug3(char* szFormat,...);

public:
	int InitFileInfo();
	bool m_bSyslogFlag;
	bool m_bDebugFlag;
	bool m_bDebug2Flag;
	bool m_bDebug3Flag;

private:                                             //�ڲ����ܺ���
	int _Printf(char* szFormat,...);                 //���Ĵ�ӡ����
	void DeleteFirstFile();                          //ɾ�����ϵ��ļ�
	void FixFileInfo();                              //�޶��ļ���Ŀ¼����
	void MakeFileName();                             //����ʱ����ļ���С�������ļ���
	void GetFileName();                              //��õ�ǰ�����ļ���
private:
	CMutexLock    m_Lock;
	char m_szFilePath[FILENAME_STRING_LENGTH];       //�ļ�·��
	char m_szFileName[FILENAME_STRING_LENGTH];       //�ļ���
	unsigned long m_nFileSize;                       //��ǰ�ļ���С
	time_t        m_tFileNameMake;                   //�����ļ�����ʱ���
	int           m_nHoleFileMax;                    //�����ļ������������캯�����ݽ���
	_APP_INFO_OUT_CALLBACK m_pInfoOutCallBack;       //����ص�����
	void*         m_pInfoOutCallbackParam;
	bool          m_bPrint2ScrFlag;
	char          m_szFileInfoName[FILENAME_STRING_LENGTH];

	std::vector<char*> m_FileNameVec;
	char*         m_chFileName[LOG_FILE_DEFAULT_HOLD];
	int           m_nFileNameIndex;
};

#endif 
