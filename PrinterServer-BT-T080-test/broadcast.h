#pragma once
#include "includes.h"
#include <time.h>

#define LABEL_LIST_SIZE     5
#define LABEL_SIZE_MAX		32


typedef struct
{
	int len;
	u8 data[LABEL_SIZE_MAX];
}label_t;

typedef struct
{
	int label_cnt;
	int msg_send_flag;
	int cur_index;
	label_t label[LABEL_LIST_SIZE];
}label_list_t;

struct strutLabelModifiers
{
	int		nCount;
	time_t	nTime;
	int		nSendFlag;
};

int send_msg_to_main_pthread(void);
int clean_list(void);

int led_dev_open(int *p_fd_led);
int led_print_item_ID(int fd_led, int item_id);
int led_print_string(int fd_led, int item_id, char *str);
int led_get_respond_package(int fd_led, int time_out);

int broadcast_dev_open(int *p_fd_led);
int broadcast_item_ID(int fd_led, int item_id);
int broadcast_trans_handle(int fd_dev);
