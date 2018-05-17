/*
文件名称: loginfo.h
文件说明: 记录服务日志
包括: 
	1.提供记录各等级(debug, erro, info)日志写入函数
	2.常用功能函数
	
注:日志文件路径:server/log/v_infoxxx.log
				server/log/v_debugxxx.log
				server/log/v_errorxxx.log
*/

#ifndef LOGINFO_H

#define LOGINFO_H

/** 功能: 写错误日志
 *  输入参数: file-文件名, line-行号, format-日志信息
*/
void write_error_log(const char* file, int line, const char* format, ...);

/** 功能: 写信息日志
 *  输入参数: format-日志信息
 */
void write_info_log(const char* format, ...);

/** 功能: 写debug日志
 *  输入参数: format-日志信息
*/
void write_debug_log(const char* format, ...);

/** 功能: 获取比对服务所在目录
 *  返回: 0-成功, 非0-失败
 */
int get_module_path();

//获取系统当前时间
void get_curtime(char* datestr, char* timestr);
int get_time();
void get_date_time(char* str);

extern char g_module_path[255];

#endif

