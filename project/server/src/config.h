/*
文件名称: config.h
文件说明: 读取配置文件内容, 作为服务启动时所需参数
包括: 
	1.读整个配置文件配置
	2.读配置文件中的某项配置
	
注:配置文件路径:server/etc/server.cfg
*/

#ifndef CONFIG_H
#define CONFIG_H

/** 系统配置参数
 * */
typedef struct tagConfigParam
{
	int max_connect; /* 最大连接数 */
	int listen_port; /* 侦听端口 */
	int time_out; /* 超时(秒)*/
	int max_db_conn; /* 最大数据库连接数 */
	char data_server[31]; /* 数据库服务名 */
	char username[31]; /* 数据库用户 */
	char password[31]; /* 数据库密码 */
	int db_port;
	char dbname[51];
	int reload_flag; /* 指纹重装标志*/
	int load_hour; /* 重装时间(时)*/
	int load_minute; /* 重装时间(分)*/
	int debug_level; /*调试级别*/
	int log_to_db_flag; /* 是否写数据库日志*/
}ConfigParam;

/** 功能: 读系统配置
 *     输入参数: cfg_name-配置文件路径
 *         返回:0-成功, <0-失败
 *         */
int read_config(char* cfg_name);

/** 功能:读配置文件中的某项
 *  输入参数: cfg_name-配置文件路径, key-参数键
 *  value_size-值缓冲区大小
 *  输出参数: value-参数值
 *  返回:0-成功, <0-失败
 **/
int read_cfg_value(char* cfg_name, char* key,
		char* value, int value_size);

extern ConfigParam g_cfg_param;

#endif

