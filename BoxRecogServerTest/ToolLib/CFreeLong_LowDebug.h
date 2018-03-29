// CFreeLong_LowDebug.h: interface for the CCFreeLong_LowDebug class.
//
//////////////////////////////////////////////////////////////////////
#if !defined _FREELONG_LOW_DEBUG_H_
#define _FREELONG_LOW_DEBUG_H_

#include "MutexLock.h"

#define DEBUG_BUFFER_LENGTH            1024                         //最大缓冲区大小
#define DEBUG_MAX_FILENAME_LENGTH      256                          //最大文件名大小
#define DEBUG_MAX_FILE_SIZE            (1*1024*1024*1024)           //最大文件大小1G，超过了就覆盖原来的大小
/*
 *	提供一个回调函数，让用户可以获取打印输出的内容
 */
typedef void(*_APP_INFO_OUT_CALLBACK)(char* szInfo,void* pCallbackParam);

class CCFreeLong_LowDebug  
{
public:
	static void DeleteAFile(char* szFileName);                 //删除一个文件
	static char* GetTrueFileName(char* szBuffer);              //获取文件的真实名称
public:
	int Debug2File(char* szFormat,...);                        //输出字符串到文件或者控制台。返回字节数，不包括'\0'
	void Debug2File4Bin(char* pBuffer,int nLength);            //二进制输出一段内存区
public:
	CCFreeLong_LowDebug(char* szPathName,                             //路径名
						char* szAppName,                              //文件名
						bool  bPrint2TTYFlag=false,                   //是否打印到控制台屏
						_APP_INFO_OUT_CALLBACK pInfoOutCallback=NULL, //输出回调
						void* pInfoOutCallbackParam=NULL);            //回调函数参数
	
	 ~CCFreeLong_LowDebug();

public:
	_APP_INFO_OUT_CALLBACK m_pInfoOutCallback;
	void* m_pInfoOutCallbackParam;
private:
	bool       m_bPrint2TTYFlag;                                      //控制台输出标志
	char       m_szFileName[DEBUG_MAX_FILENAME_LENGTH];               //拼接好的路径名+文件名
	CMutexLock m_Lock;                                                //线程安全锁
	long       m_lWriteSize;
};

#endif 
