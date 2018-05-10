
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
ConfigParam g_cfg_param;

char* trim(char* str)
{
	char* p = NULL;
	int length = 0;

	if (NULL == str)
	{
		return str;
	}

	length = strlen(str);
	if (length == 0)
	{
		return str;
	}

	for (p=str; *p!=0; p++)
	{
		if (*p!=' ' && *p!='\t' && *p!='\r' && *p!='\n')
		{
			break;
		}
	}

	if (p != str)
	{
		strcpy(str, p);
	}

	length = strlen(str);
	if (length == 0)
	{
		return str;
	}

	for(p=&str[length-1]; p!=str; p--)
	{
		if (*p!=' ' && *p!='\t' && *p!='\r' && *p!='\n')
		{
			break;
		}
		*p = 0;
	}

	return str;
}

int read_config(char* cfg_name)
{
	FILE* fp_cfg;
	char line_data[201];
	char* p;
	int line_len, key_len;

	if (NULL==cfg_name)
	{
		return -1;
	}
	fp_cfg = fopen(cfg_name, "r");
	if (NULL == fp_cfg)
	{
		printf("打开文件 %s 错误!\n", cfg_name);
		return -1;
	}

	memset(&g_cfg_param, 0, sizeof(ConfigParam));
	while(1)
	{
		memset(line_data, 0, sizeof(line_data));
		if (NULL == fgets(line_data, sizeof(line_data)-1, fp_cfg))
		{
			break;
		}

		trim(line_data);
		line_len = strlen(line_data);
		if ('\n' == line_data[line_len-1])
		{
			line_data[line_len-1] = 0;
		}

		p = strchr(line_data, '=');
		if (NULL == p)
		{
			continue;
		}

		key_len = p-line_data;
		if (strncasecmp(line_data, "listen_port", key_len) == 0)
		{
			g_cfg_param.listen_port = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "max_connect", key_len) == 0)
		{
			g_cfg_param.max_connect = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "time_out", key_len) == 0)
		{
			g_cfg_param.time_out = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "load_hour", key_len) == 0)
		{
			g_cfg_param.load_hour = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "load_minute", key_len) == 0)
		{
			g_cfg_param.load_minute = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "debug_level", key_len) == 0)
		{
			g_cfg_param.debug_level = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "log_to_db_flag", key_len) == 0)
		{
			g_cfg_param.log_to_db_flag = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "max_db_conn", key_len) == 0)
		{
			g_cfg_param.max_db_conn = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "data_server", key_len) == 0)
		{
			strncpy(g_cfg_param.data_server, p+1, 
					sizeof(g_cfg_param.data_server)-1);
			trim(g_cfg_param.data_server);
		}
		else if (strncasecmp(line_data, "username", key_len) == 0)
		{
			strncpy(g_cfg_param.username, p+1, 
					sizeof(g_cfg_param.username)-1);
			trim(g_cfg_param.username);
		}
		else if (strncasecmp(line_data, "password", key_len) == 0)
		{
			strncpy(g_cfg_param.password, p+1, 
					sizeof(g_cfg_param.password)-1);
			trim(g_cfg_param.password);
		}
		else if (strncasecmp(line_data, "dbname", key_len) == 0)
		{
			strncpy(g_cfg_param.dbname, p+1, 
					sizeof(g_cfg_param.dbname)-1);
			trim(g_cfg_param.dbname);
		}
		else if (strncasecmp(line_data, "dbport", key_len) == 0)
		{
			g_cfg_param.db_port = atoi(trim(p+1));
		}
		else if (strncasecmp(line_data, "reload_flag", key_len) == 0)
		{
			g_cfg_param.reload_flag = atoi(trim(p+1));
		}
	}

	fclose(fp_cfg);
	return 0;
}

int read_cfg_value(char* cfg_name, char* key, 
		char* value, int value_size)
{
	FILE* fp_cfg;
	char line_data[201];
	char* p;
	int line_len, key_len;

	if (NULL==cfg_name)
	{
		return -1;
	}
	fp_cfg = fopen(cfg_name, "r");
	if (NULL == fp_cfg)
	{
		return -1;
	}

	while(1)
	{
		memset(line_data, 0, sizeof(line_data));
		if (NULL == fgets(line_data, sizeof(line_data)-1, fp_cfg))
		{
			break;
		}

		trim(line_data);
		line_len = strlen(line_data);
		if ('\n' == line_data[line_len-1])
		{
			line_data[line_len-1] = 0;
		}

		if ('#' == line_data[0])
		{
			continue;
		}

		p = strchr(line_data, '=');
		if (NULL == p)
		{
			continue;
		}

		key_len = p-line_data;
		if (strncasecmp(line_data, key, key_len)==0
				&& (int)strlen(key)==key_len)
		{
			strncpy(value, p+1, value_size-1);
			trim(value);
			fclose(fp_cfg);
			return 0;
		}
	}

	fclose(fp_cfg);
	return -1;
}


