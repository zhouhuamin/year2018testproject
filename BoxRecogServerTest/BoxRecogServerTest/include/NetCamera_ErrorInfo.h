//2012-07-16
//V2.2.2
#ifndef _NET_CAMERA_ERROR_INFO_H_
#define _NET_CAMERA_ERROR_INFO_H_

#ifndef _WIN32
#define NC_ERROR_EXT
#else

#ifndef NETCAMERA_ERROR_EXPORTS
#define NC_ERROR_EXT	__declspec(dllimport)
#else
#define NC_ERROR_EXT	__declspec(dllexport)
#endif

#endif

class NC_ERROR_EXT NetCamera_Error  
{
public:
	static void UnInit();
	static void FormatErrorCode(int nErrorCode,char* pErrorInfo);
	static void Init();
};

#endif 
