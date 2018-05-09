#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "loginfo.h"
#include "config.h"

char g_module_path[255];
FILE* g_debug_log = NULL;
int g_debug_count = 0;

/*
 * int get_module_path()
 * {
 *   char proc_name[300];
 *     unsigned int count;
 *       char* p;
 *         
 *           memset(proc_name, 0, sizeof(proc_name));
 *             count = readlink("/proc/self/exe", proc_name, sizeof(proc_name));
 *               if (count<0 || count>=sizeof(proc_name))
 *                 {
 *                     return -1;
 *                       }
 *                         
 *                           p = strrchr(proc_name, '/');
 *                             if (NULL == p)
 *                               {
 *                                   return -1;
 *                                     }
 *                                       *p = 0;
 *                                         strcpy(g_module_path, proc_name);
 *                                           return 0;
 *                                           }
 *                                           */

int get_module_path()
{
	char proc_name[300];
	char proc_buf[100];
	int count;

	memset(proc_name, 0, sizeof(proc_name));
	sprintf(proc_buf, "/proc/%d/cwd", getpid());
	count = readlink(proc_buf, proc_name, sizeof(proc_name));
	if (count<0 || count>=(int)sizeof(proc_name))
	{
		return -1;
	}

	strcpy(g_module_path, proc_name);
	return 0;
}

void get_curtime(char* datestr, char* timestr)
{
	time_t t;
	struct tm tm_now;

	time(&t);
	localtime_r(&t, &tm_now);
	sprintf(datestr, "%04d%02d%02d", tm_now.tm_year+1900,
			tm_now.tm_mon+1, tm_now.tm_mday);
	sprintf(timestr, "%02d:%02d:%02d", tm_now.tm_hour,
			tm_now.tm_min, tm_now.tm_sec);
}

int get_time()
{
	time_t t;
	time(&t);
	return t;
}

void write_error_log(const char* file, int line, const char* format, ...)
{
	va_list vl;
	char log_file[201];
	char loc_date[11];
	char loc_time[11];
	FILE* fp_log;

	get_curtime(loc_date, loc_time);
	sprintf(log_file, "%s/log/v_err%s.log", 
			g_module_path, loc_date);
	fp_log = fopen(log_file, "a");
	if (NULL == fp_log)
	{
		return;
	}

	fprintf(fp_log, "[%s][%s,%d]", 
			loc_time, file, line);
	va_start(vl, format);
	vfprintf(fp_log, format, vl);
	fprintf(fp_log, "\n");
	va_end(vl);

	fclose(fp_log);
}

void write_info_log(const char* format, ...)
{
	va_list vl;
	char log_file[201];
	char loc_date[11];
	char loc_time[11];
	FILE* fp_log;

	get_curtime(loc_date, loc_time);
	sprintf(log_file, "%s/log/v_info%s.log", 
			g_module_path, loc_date);
	fp_log = fopen(log_file, "a");
	if (NULL == fp_log)
	{
		return;
	}

	fprintf(fp_log, "[%s]", loc_time);
	va_start(vl, format);
	vfprintf(fp_log, format, vl);
	fprintf(fp_log, "\n");
	va_end(vl);

	fclose(fp_log);
}

void write_debug_log(const char* format, ...)
{
	va_list vl;
	char log_file[201];
	char loc_date[11];
	char loc_time[11];
	FILE* fp_log;

	if (g_cfg_param.debug_level == 0)
	{
		return;
	}

	get_curtime(loc_date, loc_time);
	sprintf(log_file, "%s/log/v_debug%s.log", 
			g_module_path, loc_date);
	fp_log = fopen(log_file, "a");
	if (NULL == fp_log)
	{
		return;
	}

	fprintf(fp_log, "[%s]", loc_time);
	va_start(vl, format);
	vfprintf(fp_log, format, vl);
	fprintf(fp_log, "\n");
	va_end(vl);

	fclose(fp_log);
}

