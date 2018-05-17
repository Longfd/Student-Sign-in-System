/*
文件名称: commun.h
文件说明: 提供常用的socket功能
包括: 
	1.socket连接初始化
	2.socket连接断开
	3.socket接收数据
	4.socket发送数据
	5.socket配置
	等...
*/


#ifndef _COMMUN_H
#define _COMMUN_H


/** 功能：连接服务器
 *  输入参数：server_ip-服务器IP地址，server_port-服务器端口
 *  返回值：成功：连接套接字句柄(>0)，-1-失败。
 **/
int connect_server(char* server_ip, unsigned short server_port);

/** 功能：断开连接
 *  输入参数：sock-要关闭的连接
 *  返回值：无。
 **/
void close_connect(int sock);

/** 功能：启动服务器
 *  输入参数：server_port-服务器侦听端口号
 *  返回值：成功：服务器的侦听套接字句柄(>0)，-1-失败。
 **/
int start_server(unsigned short server_port);

/** 功能：接受连接
 *  输入参数：listen_sock-侦听套接字，client_ip-客户端IP地址，
 *  client_port-客户端端口
 *  返回值：成功：数据套接字句柄(>0), -1-失败。
 **/
int accept_connect(int listen_sock, char* client_ip, 
		unsigned short* client_port);

/** 功能：等待连接或等待数据
 *  输入参数：sock-等待套接字，timeout-超时时间(毫秒)
 *  返回值：>0-成功，0-超时，-1-失败。
 **/
int wait_event(int sock, int timeout);

/** 功能：发送数据
 *  输入参数：sock-发送数据的连接，buf-要发送的数据, data_len-数据长度
 *  返回值：>0成功，0-连接已断开，-1-失败。
 **/
int send_data(int sock, const char* buf, int data_len);

/** 功能：接收数据
 *  输入参数：sock-接收数据的连接，
 *  	buf-接收缓冲区，
 *  	buf_len-接收缓冲区大小
 *  	recv_len-收到的数据长度
 *  输出参数:
 *  	buf-接收缓冲区，
 *  	recv_len-收到的数据长度
 *  返回值：>0-成功，0-连接已断开，-1-失败。
 **/
int recv_data(int sock, unsigned char* buf, int buf_len, int* recv_len); 

/** 功能：配置连接
 *  参数：要配置的连接句柄
 *  返回值：1-成功，-1-失败。
 **/
int config_sock(int sock);

/* 功能：将短整数转换为字节数组
 * 输入参数：val-主机字节序短整数
 * 输出参数：字节数组(2字节)
 * 返回值: 无
 **/
void dll_ustobytes(unsigned short val,
		unsigned char bytes[2]);

/* 功能：将字节数组转换为短整数
 * 输入参数：bytes-字节数组(2字节)
 * 返回值：短整数
 **/
unsigned short dll_bytestous(unsigned char bytes[2]);

/* 功能：将整数转换为字节数组
 * 输入参数：val-主机字节序短整数
 * 输出参数：字节数组(4字节)
 * 返回值: 无
 **/
void dll_ultobytes(unsigned long val,
		unsigned char bytes[4]);

/* 功能：将字节数组转换为整数
 * 输入参数：bytes-字节数组(4字节)
 * 返回值：整数
 **/
unsigned long dll_bytestoul(unsigned char bytes[4]);

#endif

