#if !defined _SOCKET_INCLUDE_H_
#define _SOCKET_INCLUDE_H_

#ifdef WIN32
	#include <winsock.h>
	#define Linux_Win_SOCKET SOCKET
	#define Linux_Win_InvalidSOCKET INVALID_SOCKET
#else
	#define Linux_Win_SOCKET int
	#define Linux_Win_InvalidSOCKET -1
#endif



#endif