#ifndef __SIGNINSYS_DB_H__
#define __SIGNINSYS_DB_H__

/** 功能: 创建数据库连接池
 *  输入参数: count-连接数
 *  返回: 0-成功, <0-失败
 */
 
int create_conn_list(int count);

/** 功能: 释放数据库连接池
 *  返回: 0-成功
*/
int free_conn_list();

/** 功能: 打开数据库连接
 *  输入参数: conn_no-连接号, user_name-用户名
 *  pass_word-密码, server_name-数据库
 *  返回: 0-成功, 非0-失败
*/
int open_finger_db(int conn_no, char* user_name, 
		char* pass_word, char* server_name, 
		int dbport, char* dbname);

/** 功能: 关闭数据库连接
 *  输入参数: conn_no-连接号
 *  返回: 0-成功
*/
int close_finger_db(int conn_no);

/** 功能: 获取空闲的数据库连接
 *  返回: >=0-成功(数据库连接号), <0-失败
*/
int get_free_conn();

/** 功能: 释放数据库连接
 *  输入参数: conn_no-连接号
 *  返回: 0-成功
*/
int free_conn(int conn_no);

int commit_trans(int conn_no);
int rollback_trans(int conn_no);

int get_last_insert_id(int conn_no, char* insert_id, int length);
void get_datetime(char* datestr, char* timestr);

#endif

