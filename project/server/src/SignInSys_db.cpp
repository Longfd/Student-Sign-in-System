#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "SignInSys_db.h"
#include "oradb.h"
#include "loginfo.h"

int g_conn_count = 0;
OraConn* g_conn_list = NULL;

pthread_mutex_t g_conn_list_mutex = PTHREAD_MUTEX_INITIALIZER;

int create_conn_list(int count)
{
	int i;

	init_db_env();
	g_conn_list = (OraConn*)malloc(sizeof(OraConn)*count);
	if (g_conn_list == NULL)
	{
		return -1;
	} 

	for (i=0; i<count; i++)
	{
		g_conn_list[i].free_flag = 1;
	} 

	g_conn_count = count;
	return 0;
}

int query_free_conn()
{
	int i;

	for (i=1; i<g_conn_count; i++)
	{
		if (g_conn_list[i].free_flag == 1)
		{
			return i;
		}
	}
	return -1;
}

int get_free_conn()
{
	int i;
	int free_no = -1;
	pthread_mutex_lock(&g_conn_list_mutex);
	for (i=0; i<20; i++)
	{
		free_no = query_free_conn();
		if (free_no != -1)
		{
			g_conn_list[free_no].free_flag = 0;
			break;
		}
		usleep(100*1000);
	}

	pthread_mutex_unlock(&g_conn_list_mutex);
	return free_no;
}

int free_conn(int conn_no)
{
	g_conn_list[conn_no].free_flag = 1;
	return 0;
}

int free_conn_list()
{
	uninit_db_env();
	free(g_conn_list);
	g_conn_list = NULL;
	g_conn_count = 0;
	return 0;
}

int open_finger_db(int conn_no, char* user_name, 
		char* pass_word, char* server_name, 
		int dbport, char* dbname)
{
	int result;

	if (conn_no<0 ||conn_no>=g_conn_count)
	{
		return -1;
	}

	if (NULL == g_conn_list)
	{
		return -1;
	}

	result = open_db_conn(user_name,
			pass_word, server_name, dbport,
			dbname, &g_conn_list[conn_no]);
	if (result > 0)
	{ 
		char errbuf[601];
		int error_no;

		get_conn_error(&g_conn_list[conn_no],
				&error_no, errbuf, 600);
		write_error_log(__FILE__, __LINE__,
				"连接数据库[%s/%s]错误,错误码:%d, 详细信息:%s",
				user_name, pass_word, error_no, errbuf);
	}  
	return result;
}

int close_finger_db(int conn_no)
{
	if (conn_no<0 ||conn_no>=g_conn_count)
	{
		return -1;
	}

	if (NULL == g_conn_list)
	{
		return -1;
	}

	close_db_conn(&g_conn_list[conn_no]);
	return 0;
}

void get_datetime(char* datestr, char* timestr)
{
	time_t t;
	struct tm tm_now;

	time(&t);
	localtime_r(&t, &tm_now);
	sprintf(datestr, "%04d-%02d-%02d", tm_now.tm_year+1900,
			tm_now.tm_mon+1, tm_now.tm_mday);
	sprintf(timestr, "%02d:%02d:%02d", tm_now.tm_hour,
			tm_now.tm_min, tm_now.tm_sec);
}


int commit_trans(int conn_no)
{
	if (NULL==g_conn_list || conn_no<0
			|| conn_no>=g_conn_count)
	{
		return -1;
	}

	return commit(&g_conn_list[conn_no]);
}

int rollback_trans(int conn_no)
{
	if (NULL==g_conn_list || conn_no<0
			|| conn_no>=g_conn_count)
	{
		return -1;
	}

	return rollback(&g_conn_list[conn_no]);
}
int count_table_record(int conn_no, char* table_name, 
		char* where, int* count)
{
	int result;
	char sql_stmt[201];
	OraQuery ora_query;
	char val[51];

	if (NULL==g_conn_list || conn_no<0
			|| conn_no>=g_conn_count)
	{
		return -1;
	}

	sprintf(sql_stmt, "SELECT COUNT(*) as CNT FROM %s %s",
			table_name, where);  
	memset(&ora_query, 0, sizeof(ora_query));
	result = prepare_query(&g_conn_list[conn_no],
			sql_stmt, &ora_query);
	if (result != 0)
	{
		if (result > 0)
		{
			char errbuf[601];
			int error_no;

			get_query_error(&ora_query, &error_no,
					errbuf, 600);
			write_error_log(__FILE__, __LINE__,
					"统计数据记录数错误,错误码:%d,详细信息:%s",
					error_no, errbuf);
			if (2006 == error_no)
			{
				write_info_log("数据库连接中断，重新启动服务!");
				exit(error_no);
			}
		}
		close_query(&ora_query);
		return result;
	}

	result = open_query(&ora_query);
	if (result != 0)
	{
		if (result > 0)
		{
			char errbuf[600];
			int error_no;

			get_query_error(&ora_query, &error_no,
					errbuf, 600);
			write_error_log(__FILE__, __LINE__,
					"统计数据记录数错误,错误码:%d,详细信息:%s",
					error_no, errbuf);
			if (2006 == error_no)
			{
				write_info_log("数据库连接中断，重新启动服务!");
				exit(error_no);
			}
		}
		close_query(&ora_query);
		return result;
	}

	result = move_to_next(&ora_query);
	if (result != 0)
	{
		if (result > 0)
		{
			char errbuf[300];
			int error_no;
			get_query_error(&ora_query, &error_no, errbuf, 300);
			write_error_log(__FILE__, __LINE__,
					"统计数据记录数错误,错误码:%d,详细信息:%s", 
					error_no, errbuf);
			if (2006 == error_no)
			{
				write_info_log("数据库连接中断，重新启动服务!");
				exit(error_no);
			}
		}
		close_query(&ora_query);
		return result;
	}

	memset(val, 0, sizeof(val));
	result = get_field_by_pos(&ora_query, 0, val, sizeof(val));
	if (result != 0)
	{
		close_query(&ora_query);
		return result;
	}
	*count = atol(val);
	close_query(&ora_query);
	return 0;
}

int get_last_insert_id(int conn_no, 
		char* insert_id, int length)
{
	OraQuery ora_query;
	char sql_stmt[256] = {0};
	strcpy(sql_stmt, "SELECT LAST_INSERT_ID()");
	int result;

	if (NULL==g_conn_list || conn_no<0 
			|| conn_no>=g_conn_count)
	{
		return -1;
	}

	memset(&ora_query, 0, sizeof(ora_query));
	result = prepare_query(&g_conn_list[conn_no], 
			sql_stmt, &ora_query);
	if (result != 0)
	{
		if (result > 0)
		{
			char errbuf[600];
			int error_no;

			get_query_error(&ora_query, &error_no,
					errbuf, 600);
			write_error_log(__FILE__, __LINE__, 
					"获取自增长ID错误,错误码:%d,详细信息:%s",
					error_no, errbuf);
			if (2006 == error_no) 
			{
				write_info_log("数据库连接中断，重新启动服务!");
				exit(error_no);
			}
		}
		close_query(&ora_query);
		return result;
	}

	result = open_query(&ora_query);
	if (result != 0)
	{
		if (result > 0)
		{
			char errbuf[600];
			int error_no;
			get_query_error(&ora_query, &error_no,
					errbuf, 600);
			write_error_log(__FILE__, __LINE__, 
					"获取自增长ID错误,错误码:%d,详细信息:%s",
					error_no, errbuf);

			if (2006 == error_no) 
			{
				write_info_log("数据库连接中断，重新启动服务!");
				exit(error_no);
			}
		}
		close_query(&ora_query);
		return result;
	}

	result = move_to_next(&ora_query);
	if (result != 0)
	{
		if (result > 0)
		{
			char errbuf[600];
			int error_no;
			get_query_error(&ora_query, &error_no, errbuf, 600);
			write_error_log(__FILE__, __LINE__,
					"获取自增长ID错误,错误码:%d,详细信息:%s", 
					error_no, errbuf);
		}
		close_query(&ora_query);
		return result;
	}

	get_field_by_pos(&ora_query, 0, insert_id, length);
	close_query(&ora_query);
	return 0;
}




