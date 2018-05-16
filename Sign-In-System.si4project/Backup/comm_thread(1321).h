/*
comm_thread.h
文件说明: 初始化服务
包括: 
	1.启动监听线程
	2.初始化数据库连接
	3.初始化子线程通信缓冲区
	4.定义子线程处理消息函数
	5.各资源的释放
*/
#ifndef COMM_THREAD_H

#define COMM_THREAD_H

#include <pthread.h>
#include "SignInSys_db.h"

#define COMM_BUF_SIZE	200*1024

//客户端连接结构体
typedef struct tagCommThreadInfo
{
	pthread_t thread_handle;//通信线程ID
	int thread_id;//结构体缓冲区所处位置标记
	int comm_sock;//客户端socket
	char peer_addr[21];//客户端IP
	unsigned peer_port;//客户端端口号
	unsigned char comm_buf[COMM_BUF_SIZE];//客户端发送内容
}CommThreadInfo;

//启动服务
int start_server();

//初始化通信缓冲区
int init_comm_thread_list();
//释放通信缓冲区
void uninit_comm_thread_list();


//初始化数据库连接
int init_database();
//释放数据库连接
int uninit_database();


//开启 主线程监听
int start_comm_listen_thread();
//等待 监听主线程 退出
void wait_comm_listen_thread_exit();
//等待 子线程 退出
void wait_comm_thread_exit();

//监听主线程处理函数
void* comm_listen_thread_routine(void* arg);
//子线程(负责通信)处理函数
void* comm_thread_routine(void* arg);


//获取空闲通信缓冲区
CommThreadInfo* get_free_comm_thread();
//通过缓冲区所处位置标记, 获取指定通信缓冲区
CommThreadInfo* get_comm_thread(int thread_no);


//设置系统退出(重启)标志
void set_exit_flag(int exit_flag);
//查看系统退出(重启)标志
int get_exit_flag();

//计算可用的通信线程数
unsigned short count_free_connect();
//是否有可用通信线程
int is_system_busy(int max_thread);

#endif


