#include "broadcast.h"
#include "CRC.h"
#include "common.h"
#include "config.h"
#include "includes.h"
#include "broadcast.h"
#include "packet_format.h"

#include "jzLocker.h"
#include "ProcessBroadcast.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <errno.h>

label_list_t label_list;

std::vector<structPrinterMsg2>            g_MsgVect;
locker                              g_MsgVectLock;

static LED_frame_s led_frame_src;
static LED_frame_s led_frame_send;
static LED_frame_s led_frame_recv;

static int led_frame_attach(LED_frame_s *p_src, LED_frame_s *p_dst);
static int led_frame_check(LED_frame_s *p_src, LED_frame_s *p_dst);

int clean_label_list_cnt = 0;
static char data_buf[512];

static int broadcast_request_attach(char *data_buf, int max_len, int rsn,
		int priority, int media_no, int vol);
static int dev_read_timeout(int fd_dev, char *buf, int len, int time_out);

int send_msg_to_main_pthread(void)
{
	syslog(LOG_DEBUG, "Enter send_msg_to_main_pthread, label_list.label_cnt=%d\n", label_list.label_cnt);
	if (((get_system_ms() - dev_unit.dev_scan_ok_start_systick)
			< gSetting_system.DEV_scan_duration_time)
			&& (label_list.label_cnt < 2))
	{
		return -1;
	}

	syslog(LOG_DEBUG, "label_cnt:%d, msg_send_flag:%d\n", label_list.label_cnt, label_list.msg_send_flag);
	if (label_list.label_cnt > 0)
	{
		if (0 == label_list.msg_send_flag)
		{
			dev_send_msg(msg_DEV_NEW_DATA, 0);
			//label_list.msg_send_flag = 1;
		}
	}

	return 0;
}



int clean_list(void)
{
	memset(&label_list, 0, sizeof(label_list_t));
	return 0;
}

int led_dev_open(int *p_fd_led)
{
	int fd_com;
	struct sockaddr_in server_addr;
	struct hostent *host;
	int flag;

	host = gethostbyname(gSetting_system.com_ip);
	if ( NULL == host)
	{
		syslog(LOG_DEBUG, "get hostname error!\n");
		return -1;
	}

	fd_com = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == fd_com)
	{
		syslog(LOG_DEBUG, "socket error:%s\a\n!\n", strerror(errno));
		return -2;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(gSetting_system.com_port);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);

	if (connect(fd_com, (struct sockaddr *) (&server_addr),
			sizeof(struct sockaddr_in)) == -1)
	{
		syslog(LOG_DEBUG, "connect error:%s", strerror(errno));
		close(fd_com);
		return -3;
	}

	flag = fcntl(fd_com, F_GETFL, 0);
	fcntl(fd_com, F_SETFL, flag | O_NONBLOCK);

	*p_fd_led = fd_com;
	return 0;
}

int led_print_item_ID(int fd_led, int item_id)
{
	u8 *p_send;
	int ret;

	p_send = led_frame_src.buf;
	memset(&led_frame_src, 0, sizeof(LED_frame_s));

	*p_send++ = 0x55;
	//cmd
	*p_send++ = 0x44;
	*p_send++ = 0x00;
	//srcADDR
	*p_send++ = 0x0;
	//dstADDR
	*p_send++ = 0x0;
	//item_id
	*(u32*) p_send = item_id;
	p_send += 4;

	led_frame_src.len = p_send - led_frame_src.buf;
	Modbus_CRC_attach(&led_frame_src);
	led_frame_src.buf[led_frame_src.len++] = 0xAA;
	led_frame_attach(&led_frame_src, &led_frame_send);
	tcflush(fd_led, TCIOFLUSH);

	write(fd_led, led_frame_send.buf, led_frame_send.len);

	ret = led_get_respond_package(fd_led, 200);

	return ret;
}

int led_print_string(int fd_led, int item_id, char *str)
{
	u8 *p_send;
	int str_len;
	int ret;

	p_send = led_frame_src.buf;
	if( NULL == str)
	{
		syslog(LOG_DEBUG, "%s",__LINE__);
		return -1;
	}

	memset(&led_frame_src, 0, sizeof(LED_frame_s));

	*p_send++ = 0x55;
	//cmd
	*p_send++ = 0x1D;
	*p_send++ = 0x00;
	//srcADDR
	*p_send++ = 0x0;
	//dstADDR
	*p_send++ = 0x0;
	//item_id
	*(u32*) p_send = item_id;
	p_send += 4;

	str_len = strlen(str);
	if ((p_send - led_frame_src.buf) + str_len >= 990)
	{
		syslog(LOG_DEBUG, "%s",__LINE__);
		return -1;
	}
	str_len += 1;
	memcpy(p_send, str, str_len);

	if (0 != (str_len % 4))
	{
		p_send += (str_len + 4 - (str_len % 4));
	}
	else
	{
		p_send += str_len;
	}

	led_frame_src.len = p_send - led_frame_src.buf;
	Modbus_CRC_attach(&led_frame_src);
	led_frame_src.buf[led_frame_src.len++] = 0xAA;
	led_frame_attach(&led_frame_src, &led_frame_send);

	tcflush(fd_led, TCIOFLUSH);

	write(fd_led, led_frame_send.buf, led_frame_send.len);
	syslog(LOG_DEBUG, "package len = %d\n", led_frame_send.len);
	ret = led_get_respond_package(fd_led, 200);

	return ret;
}


static int led_frame_attach(LED_frame_s *p_src, LED_frame_s *p_dst)
{
	int len = 0;
	int i;

	p_dst->buf[len++] = p_src->buf[0];
	for (i = 1; i < p_src->len - 1; i++)
	{
		if (0x55 == p_src->buf[i])
		{
			p_dst->buf[len] = 0xBB;
			len++;
			p_dst->buf[len] = 0x56;
		}
		else if (0xAA == p_src->buf[i])
		{
			p_dst->buf[len] = 0xBB;
			len++;
			p_dst->buf[len] = 0xAB;
		}
		else if (0xBB == p_src->buf[i])
		{
			p_dst->buf[len] = 0xBB;
			len++;
			p_dst->buf[len] = 0xBC;
		}
		else
		{
			p_dst->buf[len] = p_src->buf[i];
		}
		len++;
	}
	p_dst->buf[len++] = p_src->buf[p_src->len - 1];
	p_dst->len = len;

//	memcpy(p_dst, p_src, sizeof(LED_frame_s));
	return 0;
}

static int led_frame_check(LED_frame_s *p_src, LED_frame_s *p_dst)
{
	int len = 0;
	int i;
	int flag = 0;

	p_dst->buf[len++] = p_src->buf[0];
	for (i = 1; i < p_src->len - 1; i++)
	{
		if (flag)
		{
			flag = 0;
			switch (p_src->buf[i])
			{
			case 0x56:
				p_dst->buf[len] = 0x55;
				break;
			case 0xAB:
				p_dst->buf[len] = 0xAA;
				break;
			case 0xBC:
				p_dst->buf[len] = 0xBB;
				break;
			default:
				break;
			}
			len++;
		}
		else
		{
			if (0xBB == p_src->buf[i])
			{
				flag = 1;
			}
			else
			{
				p_dst->buf[len] = p_src->buf[i];
				len++;
			}
		}

	}
	p_dst->buf[len++] = p_src->buf[p_src->len - 1];
	p_dst->len = len;

	return 0;
}

int led_get_respond_package(int fd_led, int time_out)
{
	int ret;
	int r_len = 0;
	int state = 0;
	int start_time_ms;

	u8 *p_recv = led_frame_recv.buf;

	start_time_ms = get_system_ms();
	while (1)
	{
		ret = read(fd_led, p_recv, 1);
		if (ret > 0)
		{
			if (0x55 == p_recv[0])
			{
				r_len++;
				break;
			}
			else
			{
				state = 0;
			}
		}
		else
		{
			usleep(1000);
		}

		if ((get_system_ms() - start_time_ms) >= time_out)
		{
			return -99;
		}
	}

	while (1)
	{
		ret = read(fd_led, p_recv + r_len, 1);

		if (ret > 0)
		{
			if (0xAA == p_recv[r_len])
			{
				r_len++;
				break;
			}
			else
			{
				r_len += ret;
			}
		}
		else
		{
			usleep(1000);
		}

		time_out--;

		if (time_out <= 0)
		{
			return -99;
		}
	}

	led_frame_recv.len = r_len;
	led_frame_check(&led_frame_recv, &led_frame_src);

	if (Modbus_CRC_check(&led_frame_src))
	{
		syslog(LOG_DEBUG, "crc error! len = %d\n", led_frame_src.len);
		return -3;
	}

	return 0;
}

int broadcast_dev_open(int *p_fd_led)
{
	int fd_com;
	struct sockaddr_in server_addr;
	struct hostent *host;
	int flag;

	host = gethostbyname(gSetting_system.com_ip);
	if ( NULL == host)
	{
		syslog(LOG_DEBUG, "get hostname error!\n");
		return -1;
	}

	fd_com = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == fd_com)
	{
		syslog(LOG_DEBUG, "socket error:%s\a\n!\n", strerror(errno));
		return -2;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(gSetting_system.com_port);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);

	if (connect(fd_com, (struct sockaddr *) (&server_addr),
			sizeof(struct sockaddr_in)) == -1)
	{
		syslog(LOG_DEBUG, "connect error:%s", strerror(errno));
		close(fd_com);
		return -3;
	}

	flag = fcntl(fd_com, F_GETFL, 0);
	fcntl(fd_com, F_SETFL, flag | O_NONBLOCK);

	*p_fd_led = fd_com;
	return 0;
}

int broadcast_item_ID(int fd_broadcast, int item_id)
{
    syslog(LOG_DEBUG, "REQ:\t<Play> %d.mp3\n", item_id);
    static int rsn = 1;

    memset(data_buf, 0x00, sizeof(data_buf));
    // broadcast_request_attach(data_buf, sizeof(data_buf), rsn++, 0, item_id, gSetting_system.Broadcast_volume);
    ProcessBroadcast broadcast;
    int nRet = broadcast.SocketWrite(fd_broadcast, data_buf, sizeof(data_buf), 100);

    return 0;
}

static int broadcast_request_attach(char *data_buf, int max_len, int rsn,
		int priority, int media_no, int vol)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	xmlChar * xml_buf;
	int xml_len;
	char str_tmp[50];

	/***********************************XML***************************************/
	doc = xmlNewDoc(BAD_CAST "1.0");

	root_node = xmlNewNode(NULL, BAD_CAST "AUDIO");

	xmlDocSetRootElement(doc, root_node);

	sprintf(str_tmp, "%d", rsn);
	xmlNewChild(root_node, NULL, BAD_CAST "RSN", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", priority);
	xmlNewChild(root_node, NULL, BAD_CAST "PRIORITY", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", media_no);
	xmlNewChild(root_node, NULL, BAD_CAST "MEDIA_NO", BAD_CAST (str_tmp));

	sprintf(str_tmp, "%d", vol);
	xmlNewChild(root_node, NULL, BAD_CAST "VOL", BAD_CAST (str_tmp));

	xmlDocDumpFormatMemory(doc, &xml_buf, &xml_len, 1);

	if (xml_len > (max_len - 1))
	{
		xml_len = max_len - 1;
	}

	memcpy(data_buf, xml_buf, xml_len);
	data_buf[xml_len] = 0;

	xmlFree(xml_buf);
	xmlFreeDoc(doc);

	xmlCleanupParser();

	xmlMemoryDump();

	return 0;
}

static int dev_read_timeout(int fd_dev, char *buf, int len, int time_out)
{
	int ret;
	int r_len = 0;

	while (1)
	{
		ret = read(fd_dev, buf + r_len, len - r_len);
                //syslog(LOG_DEBUG, "read ret=%d\n", ret);

		if (ret > 0)
		{
			r_len += ret;
			break;
		}
		else if (ret == 0) //server close
		{
                        close(fd_dev);
			return -99;
		}
                else
                {
                    if (errno == EAGAIN)
                    {
                        
                    }
                    else
                    {
                        close(fd_dev);
			return -99;                       
                    }
                }

		usleep(1000);
		time_out--;
		if (time_out <= 0)
		{
			break;
		}
	}

	return r_len;
}

