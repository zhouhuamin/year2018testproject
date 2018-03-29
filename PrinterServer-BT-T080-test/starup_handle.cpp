#include "includes.h"
#include "port.h"
#include "packet_format.h"
#include <boost/timer.hpp>

extern int gFd_dev;
extern void GetCurTime(char *pTime);

int param_init_handle(void)
{
	int ret;

	syslog(LOG_DEBUG, "parame init...\n");

	do
	{
		ret = parame_init();
		if (ret < 0)
		{
			syslog(LOG_DEBUG, "parame init error!\n");
		}
		else
		{
			break;
		}
		sleep(1);
	} while (ret < 0);

	syslog(LOG_DEBUG, "LOG:\tparam init OK!\n");

	return 0;
}

int dev_open_handle(int &p_fd_com)
{
	int ret;
	syslog(LOG_DEBUG, "LOG:\topen the reader...\n");

	while (1)
	{
		ret = dev_open(&p_fd_com);
		if (ret < 0)
		{
			syslog(LOG_DEBUG, "open reader port error! %d\n", ret);
		}
		else
		{
			gReal_system.dev_open_flag = 1;
			break;
		}
		sleep(2);
	}

	syslog(LOG_DEBUG, "LOG:\topen the reader OK!\n");
	return 0;
}

int dev_connect_handle(int fd_dev)
{
    int nTmpFd = fd_dev;
	int ret;

	//log_send("LOG:\tconnect the dev...\n");
	syslog(LOG_DEBUG, "LOG:\tconnect the dev...\n");

	while (1) //
	{
		if (gReal_system.dev_open_flag)
		{
			ret = dev_connect(nTmpFd);
			if (ret != 0)
			{
				syslog(LOG_DEBUG, "connect read error! %d\n", ret);
			}
			else
			{
				gReal_system.dev_connect_flag = 1;
				break;
			}
		}

		sleep(2);
	}

	syslog(LOG_DEBUG, "LOG:\tconnect the dev OK!\n");
	return 0;
}

int server_connect_handle(int *pFd_ser)
{
	syslog(LOG_DEBUG, "LOG:\tconnect the server...\n");

	while (1)
	{
		*pFd_ser = connect_server(gSetting_system.server_ip,
				gSetting_system.server_port);
		if (*pFd_ser <= 0)
		{
			syslog(LOG_DEBUG, "connect to server error-1!\n");
		}
		else
		{
			gReal_system.server_connect_flag = 1;
			break;
		}
		sleep(2);
	}

	syslog(LOG_DEBUG, "LOG:\tconnect the server OK!\n");

	return 0;
}

int server_register_handle(int server_fd)
{
	int ret;

	syslog(LOG_DEBUG, "LOG:\tregister the device...\n");

	while (1)
	{
		if (gReal_system.server_connect_flag)
		{
			ret = net_dev_register_handle(server_fd);
			if (ret < 0)
			{
				syslog(LOG_DEBUG, "register dev error!\n");
			}
			else
			{
				gReal_system.server_register_flag = 1;
				break;
			}
		}

		sleep(2);
	}
	syslog(LOG_DEBUG, "LOG:\tregister the device OK\n");
	return 0;
}

int dev_retry_connect_handle(int *pFd_dev)
{
	int fd_tmp;
	int ret;
	syslog(LOG_DEBUG, "ERR:\tthe reader's connection lost \n");

	while (1)
	{
            boost::timer t;
		ret = dev_open(&fd_tmp);
                syslog(LOG_DEBUG, "ERR:\tdev_open ret :%.3lf\n", t.elapsed());
                
                
		if (ret >= 0)
		{
			gReal_system.dev_open_flag = 1;
			close(*pFd_dev);
			*pFd_dev = fd_tmp;
			syslog(LOG_DEBUG, "MSG:\tconnect the reader with new fd!\n");
			break;
		}
		else
		{
			ret = dev_connect(*pFd_dev);
			if (ret == 0)
			{
				gReal_system.dev_connect_flag = 1;
				syslog(LOG_DEBUG, "MSG:\tconnect the reader with old fd!\n");
				break;
			}
		}
		sleep(1);
	}
	return 0;
}

int server_retry_connect_handle(int *pFd_ser)
{
	int ret;

	close(*pFd_ser);

	syslog(LOG_DEBUG, "ERR:\tthe server's connection is lost!\n");
	syslog(LOG_DEBUG, "MSG:\tRetry to connect to the server...\n");

	//è¿??¥æ???¡ï¿½?	
	while (1)
	{
		int nTmpSocket = -1;
		nTmpSocket = connect_server(gSetting_system.server_ip,
				gSetting_system.server_port);
		syslog(LOG_DEBUG, "BBBBBBBBBBBBBBBBBB\n");
		if (nTmpSocket <= 0)
		{
			syslog(LOG_DEBUG, "connect to server error-2!\n");
			syslog(LOG_DEBUG, "XXXXXXXXXXXXXXX\n");
		}
		else
		{
			*pFd_ser = nTmpSocket;
			syslog(LOG_DEBUG, "MSG:\tconnected to the server!\n");
			ret = net_dev_register_handle(*pFd_ser);
			if (0 == ret)
			{
				syslog(LOG_DEBUG, "MSG:\tregister to the server!\n");
				break;
			}
			else
			{
				close(*pFd_ser);
			}
		}
		sleep(2);
	}
	return 0;
}

