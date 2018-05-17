
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


#include "comm_thread.h"
#include "config.h"
#include "dispatch_req.h"
#include "commun.h"
#include "loginfo.h"

CommThreadInfo* g_comm_thread_list = NULL;
pthread_t g_comm_listen_thread_handle = 0;
int g_comm_listen_sock = -1;

pthread_attr_t g_thread_attr;
int g_exit_flag = 0;

void set_exit_flag(int exit_flag)
{
	g_exit_flag = exit_flag;
}

int get_exit_flag()
{
	return g_exit_flag;
}

int init_thread_attr()
{
	int result;

	result = pthread_attr_init(&g_thread_attr);
	if (result != 0)
	{
		write_error_log(__FILE__, __LINE__, 
				"调用pthread_attr_init错误, 错误信息:%s", 
				strerror(result));
		return -1;
	}

	result = pthread_attr_setdetachstate(&g_thread_attr, 
			PTHREAD_CREATE_JOINABLE);
	if (result != 0)
	{
		write_error_log(__FILE__, __LINE__, 
				"调用pthread_attr_setdetachstate错误, 错误信息:%s", 
				strerror(result));
		return -2;
	}

	result = pthread_attr_setstacksize(&g_thread_attr, 512*1024);
	if (result != 0)
	{
		write_error_log(__FILE__, __LINE__, 
				"调用pthread_attr_setstacksize错误, 错误信息:%s", 
				strerror(result));
		return -3;
	}

	return 0;
}

/** 功能: 初始化通信线程缓冲
 *     返回: 0-成功, <0-失败
 *     */
int init_comm_thread_list()
{
	int i;
	int thread_list_bytes;

	thread_list_bytes = sizeof(CommThreadInfo)*g_cfg_param.max_connect;
	g_comm_thread_list = (CommThreadInfo*)malloc(thread_list_bytes);
	if (NULL == g_comm_thread_list)
	{
		write_error_log(__FILE__, __LINE__, "分配通信内存失败!");
		return -1;
	}

	memset(g_comm_thread_list, 0, thread_list_bytes);
	for (i=0; i<g_cfg_param.max_connect; i++)
	{
		g_comm_thread_list[i].thread_handle = 0;
		g_comm_thread_list[i].comm_sock = -1;
		g_comm_thread_list[i].thread_id = i;
	}

	return 0;
}

/** 功能: 释放通信线程缓冲
 * */
void uninit_comm_thread_list()
{
	if (NULL == g_comm_thread_list)
	{
		return;
	}

	free(g_comm_thread_list);
	g_comm_thread_list = NULL;
}

CommThreadInfo* get_free_comm_thread()
{
	int i;

	for (i=0; i<g_cfg_param.max_connect; i++)
	{
		if (-1 == g_comm_thread_list[i].comm_sock)
		{
			if (0 == g_comm_thread_list[i].thread_handle)
			{
				return &g_comm_thread_list[i];
			}
		}
	}

	return NULL;
}

CommThreadInfo* get_comm_thread(int thread_no)
{
	if (thread_no<0 || thread_no>=g_cfg_param.max_connect)
	{
		return NULL;
	}
	return &g_comm_thread_list[thread_no];
}

/** 计算可用的通信线程数量
 * */
unsigned short count_free_connect()
{
	unsigned short free_count = 0;
	unsigned short i;

	for (i=0; i<g_cfg_param.max_connect; i++)
	{
		if (-1 == g_comm_thread_list[i].comm_sock)
		{
			free_count++;
		}
	}

	return free_count;
}

int is_system_busy(int max_thread)
{
	int m = 0;
	int i;

	for (i=0; i<g_cfg_param.max_connect; i++)
	{
		if (-1 != g_comm_thread_list[i].comm_sock)
		{
			m++;
			if (m > max_thread)
			{
				return 1;
			}
		}
	}

	return 0;
}

/** 功能: 等待通信侦听线程退出
 * */
void wait_comm_listen_thread_exit()
{
	if (0 == g_comm_listen_thread_handle)
	{
		return;
	}

	pthread_join(g_comm_listen_thread_handle, NULL);
}

/** 功能: 等待通信线程退出
 * */
void wait_comm_thread_exit()
{
	int i;

	if (NULL == g_comm_thread_list)
	{
		return;
	}

	for (i=0; i<g_cfg_param.max_connect; i++)
	{
		if (0 != g_comm_thread_list[i].thread_handle)
		{
			write_info_log("等待通信线程[%d]退出...", i);
			sleep(g_cfg_param.time_out+2);
			if (0 != g_comm_thread_list[i].thread_handle)
			{
				write_info_log("通信线程[%d]可能未退出!", i);
			}
			else
			{
				write_info_log("等待通信线程[%d]退出成功!", i);
			}
		}
	}
}

/** 功能: 启动通信侦听线程
 *     返回: 0-成功, 非0-失败
 *     */
int start_comm_listen_thread()
{
	int result;

	result = pthread_create(&g_comm_listen_thread_handle, 
			&g_thread_attr, comm_listen_thread_routine, NULL);
	if (result != 0)
	{
		g_comm_listen_thread_handle = 0;
	}
	return result;
}

int g_start_date = 0;
int g_start_time = 0;
int should_reset_proc()
{
	time_t t;
	struct tm now;
	int today;
	int cur_time;

	if (0 == g_cfg_param.reload_flag)
	{
		/* 不重装 */
		return 0;
	}

	/* 取当前时间 */
	time(&t);
	localtime_r(&t, &now);

	if (now.tm_hour!=g_cfg_param.load_hour
			|| now.tm_min!=g_cfg_param.load_minute)
	{
		/* 不是重装时间 */
		return 0;
	}

	today = now.tm_mon*100+now.tm_mday;
	cur_time = now.tm_hour*3600+now.tm_min*60+now.tm_sec;
	switch (g_cfg_param.reload_flag)
	{
		case 3:/*每日重装*/
			{
				if (today!=g_start_date
						|| (cur_time-g_start_time)>60)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		case 1:/*单日重装*/
			{
				if (today!=g_start_date 
						|| (cur_time-g_start_time)>60)
				{
					if ((now.tm_mday%2) != 0)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		case 2:/*双日重装*/
			{
				if (today!=g_start_date
						|| (cur_time-g_start_time)>60)
				{
					if ((now.tm_mday%2) == 0)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		default:/*配置无效,不重装*/
			return 0;
	}
}

void* comm_listen_thread_routine(void* arg)
{
	int result;
	int data_sock;
	char client_addr[21];
	unsigned short client_port;
	CommThreadInfo* free_comm_thread = NULL;

	while(1)
	{
		result = wait_event(g_comm_listen_sock, 100);
		switch (result)
		{
			case -1:
				{
					char errmsg[301];
					strcpy(errmsg, strerror(errno));
					close_connect(g_comm_listen_sock);
					write_error_log(__FILE__, __LINE__, 
							"等待客户端连接错误, 返回码为:%s", errmsg);
					return arg;
				}
			case 0:
				/* 定时重启服务 */
				if (should_reset_proc() != 0) 
				{
					write_info_log("重新启动指纹认证服务...");
					g_exit_flag = 1;
					return NULL;
				}
				if (0 != g_exit_flag)
				{
					write_info_log("通信侦听线程退出!");
					return NULL;
				}
				continue;
			default:
				break;
		}

		free_comm_thread = get_free_comm_thread();
		if (NULL == free_comm_thread)
		{
			write_error_log(__FILE__, __LINE__, 
					"没有空闲的线程缓冲区接受客户端连接!");
			continue;
		}

		data_sock = accept_connect(g_comm_listen_sock, 
				client_addr, &client_port);
		if (-1 == data_sock)
		{
			char errmsg[301];
			strcpy(errmsg, strerror(errno));
			write_error_log(__FILE__, __LINE__, 
					"接受客户端连接错误(%s)!", errmsg);
			continue;
		}    

		memset(free_comm_thread->peer_addr, 0, 
				sizeof(free_comm_thread->peer_addr));
		free_comm_thread->comm_sock = data_sock;
		memset(free_comm_thread->peer_addr, 0, 
				sizeof(free_comm_thread->peer_addr));
		strncpy(free_comm_thread->peer_addr, client_addr, 
				sizeof(free_comm_thread->peer_addr)-1);
		free_comm_thread->peer_port = client_port;

		result = pthread_create(&free_comm_thread->thread_handle, 
				&g_thread_attr, comm_thread_routine, free_comm_thread);
		if (result != 0)
		{
			write_error_log(__FILE__, __LINE__, "创建通信线程错误!");
			close_connect(data_sock);
			free_comm_thread->comm_sock = -1;
			free_comm_thread->thread_handle = 0;
		}   

		/* 定时重启服务 */
		if (should_reset_proc() != 0)
		{
			write_info_log("重新启动指纹认证服务...");
			g_exit_flag = 1;
			return NULL;
		}
		if (0 != g_exit_flag)
		{
			write_info_log("通信侦听线程退出!");
			return NULL;
		}
	}

	return NULL; 
}

void* comm_thread_routine(void* arg)
{
	int max_wait_times;
	int time_out_count;
	CommThreadInfo* thread_info;

	max_wait_times = g_cfg_param.time_out*10;
	time_out_count = 0;
	thread_info = (CommThreadInfo*)arg;

	while(1)
	{
		switch(wait_event(thread_info->comm_sock, 100))
		{
			case -1:/*出错*/
				{
					close_connect(thread_info->comm_sock);
					write_error_log(__FILE__, __LINE__, "等待客户端请求错误!");
					thread_info->comm_sock = -1;

					pthread_detach(thread_info->thread_handle);
					thread_info->thread_handle = 0;

					return arg;
				}
			case 0:/*超时*/
				{
					if (time_out_count >= max_wait_times)
					{
						close_connect(thread_info->comm_sock);
						thread_info->comm_sock = -1;

						pthread_detach(thread_info->thread_handle);
						thread_info->thread_handle = 0;

						return arg;
					}
					time_out_count++;
					continue;
				}
			default:/*有数据到达*/
				break;
		}

		switch(deal_client_req(thread_info))
		{
			case -1:/*出错*/
				{
					close_connect(thread_info->comm_sock);
					thread_info->comm_sock = -1;

					pthread_detach(thread_info->thread_handle);
					thread_info->thread_handle = 0;

					return arg;       
				}
			case 0:/*客户端断开*/
				{
					close_connect(thread_info->comm_sock);
					thread_info->comm_sock = -1;

					pthread_detach(thread_info->thread_handle);
					thread_info->thread_handle = 0;

					return arg;
				}
			default:/*处理成功*/
				break;
		}
		wait_event(thread_info->comm_sock, 3000);
		close_connect(thread_info->comm_sock);
		thread_info->comm_sock = -1;

		pthread_detach(thread_info->thread_handle);
		thread_info->thread_handle = 0;

		return arg;
	}   

	close_connect(thread_info->comm_sock);
	thread_info->comm_sock = -1;

	pthread_detach(thread_info->thread_handle);
	thread_info->thread_handle = 0;
	return arg;
}

/** 功能: 启动指纹比对服务
 *     返回: 0-成功, <0-失败
 *     */
int start_server()
{
	int result;
	time_t t;
	struct tm now;

	write_info_log("开始启动签到服务器...");

	write_info_log("开始连接数据库...");
	g_exit_flag = 0; 
	init_thread_attr();
	result = init_database();
	if (result != 0)
	{
		write_info_log("连接数据库错误, 错误码为:%d!", result);
		sleep(2);
		return -1;
	}
	write_info_log("连接数据库成功!");

	write_info_log("开初始化通信线程缓冲区...");
	uninit_comm_thread_list();
	if (init_comm_thread_list() != 0)
	{
		write_info_log("初始化通信线程缓冲区错误!");
		uninit_database();
		sleep(2);
		return -3;
	}
	write_info_log("初始化通信线程缓冲区成功!");

	write_info_log("开始启动通信侦听服务...");
	g_comm_listen_sock = start_server(g_cfg_param.listen_port);  
	if (-1 == g_comm_listen_sock)
	{
		uninit_database();
		uninit_comm_thread_list();
		write_info_log("启动通信侦听服务失败!");
		sleep(2);
		return -5;
	} 
	write_info_log("启动通信侦听服务成功!");

	write_info_log("开始启动通信侦听服务线程...");
	if (start_comm_listen_thread() != 0)
	{
		uninit_database();
		uninit_comm_thread_list();
		write_info_log("启动通信侦听服务线程失败!");
		sleep(2);
		return -6;
	}
	write_info_log("启动通信侦听服务线程成功！");

	time(&t);
	localtime_r(&t, &now);

	g_start_date = now.tm_mon*100+now.tm_mday;
	g_start_time = now.tm_hour*3600+now.tm_min*60+now.tm_sec;

	write_info_log("启动签到服务器成功!");

	return 0;
}

/** 功能: 连接指纹数据库
 *     返回: 0-成功, <0-失败
 *     */
int init_database()
{
	int result;
	int i;
	int loc_opened_count;
	int db_count;

	loc_opened_count = 0;
	db_count = g_cfg_param.max_db_conn;

	result = create_conn_list(db_count);
	if (result != 0)
	{
		return -2;
	}

	write_info_log("共有%d个数据库连接!", db_count);
	for (i=0; i<db_count; i++)
	{    
		result = open_finger_db(i, g_cfg_param.username, 
				g_cfg_param.password, g_cfg_param.data_server,
				g_cfg_param.db_port, g_cfg_param.dbname);
		if (result != 0)
		{
			break;
		}
		loc_opened_count++;
		write_info_log("打开数据库连接%d成功!", loc_opened_count);
	}

	if (loc_opened_count < db_count)
	{
		write_info_log("数据库连接未能完全打开!");
		for (i=0; i<loc_opened_count; i++)
		{
			close_finger_db(i);
		}
		return -3;
	}
	return 0;
}

/** 功能: 关闭所有数据库连接
 *     返回: 0-成功
 *     */
int uninit_database()
{
	int i; 
	for (i=0; i<g_cfg_param.max_db_conn; i++)
	{ 
		close_finger_db(i);
	}   
	free_conn_list();
	return 0;
}


