#if !defined _SYS_INCLUDE_H_
#define _SYS_INCLUDE_H_

//ͨ�ð���
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

#ifdef WIN32                                //WIndows��
    #include <conio.h>
    #include <windows.h>
    #include <process.h>
    #include <winsock.h>
#else                                        //linux��
    #include <pthread.h>
    #include <sys/time.h>

#endif



#endif
