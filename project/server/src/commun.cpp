#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/times.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#include "commun.h"

/** 功能：连接服务器
 *  输入参数：server_ip-服务器IP地址，server_port-服务器端口
 *  返回值：成功：连接套接字句柄(>0)，-1-失败。
*/
int connect_server(char* server_ip, unsigned short server_port)
{
	int sock;
	int result;
	struct sockaddr_in server_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sock)
	{
		return -1;
	}

	result = config_sock(sock);
	if (-1 == result)
	{
		close_connect(sock);
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	result = connect(sock, (struct sockaddr*)&server_addr,
		sizeof(server_addr));
	if (-1 == result)
	{
		close_connect(sock);
		return -1;
	}

	return sock;
}

/** 功能：断开连接
 *  输入参数：sock-要关闭的连接
 *  返回值：无。
*/
void close_connect(int sock)
{
	shutdown(sock, SHUT_RDWR);
	close(sock);
}

/** 功能：启动服务器
 *  输入参数：server_port-服务器侦听端口号
 *  返回值：成功：服务器的侦听套接字句柄(>0)，-1-失败。
*/
int start_server(unsigned short server_port)
{
	int sock;
	int result;
	struct sockaddr_in listen_addr;
	int reuse_addr_flag;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sock)
	{
		return -1;
	}

	result = config_sock(sock);
	if (-1 == result)
	{
		close_connect(sock);
		return -1;
	}

	reuse_addr_flag = 1;
	result = setsockopt(sock, SOL_SOCKET,
		SO_REUSEADDR, &reuse_addr_flag,
		sizeof(reuse_addr_flag));
	if (-1 == result)
	{
		close_connect(sock);
		return -1;
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(server_port);
	listen_addr.sin_addr.s_addr = htonl(0);

	result = bind(sock, (struct sockaddr*)&listen_addr,
		sizeof(listen_addr));
	if (-1 == result)
	{
		close_connect(sock);
		return -1;
	}

	result = listen(sock, 1024);
	if (-1 == result)
	{
		close_connect(sock);
		return -1;
	}

	return sock;
}

/** 功能：接受连接
 *  输入参数：listen_sock-侦听套接字，client_ip-客户端IP地址，
 *  client_port-客户端端口
 *  返回值：成功：数据套接字句柄(>0), -1-失败。
*/
int accept_connect(int listen_sock, char* client_ip,
	unsigned short* client_port)
{
	struct sockaddr_in client_addr;
	unsigned int addr_len;
	int sock;

	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	addr_len = sizeof(client_addr);

	sock = accept(listen_sock, (struct sockaddr*)&client_addr,
		&addr_len);
	if (-1 == sock)
	{
		return -1;
	}

	if (NULL != client_ip)
	{
		strcpy(client_ip, (char*)inet_ntoa(client_addr.sin_addr));
	}

	if (NULL != client_port)
	{
		*client_port = ntohs(client_addr.sin_port);
	}

	return sock;
}

/** 功能：等待连接或等待数据
 *  输入参数：sock-等待套接字，timeout-超时时间(毫秒)
 *  返回值：>0-成功，0-超时，-1-失败。
*/
int wait_event(int sock, int timeout)
{
	struct timeval tm_out;
	int result;
	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(sock, &read_fds);

	tm_out.tv_sec = timeout / 1000;
	tm_out.tv_usec = 1000 * (timeout % 1000);

	result = select(sock + 1, &read_fds, NULL,
		NULL, &tm_out);
	switch (result)
	{
	case -1:
		return -1;
	case 0:
		return 0;
	default:
		break;
	}

	return 1;
}

/** 功能：发送数据
 *  输入参数：sock-发送数据的连接，buf-要发送的数据, data_len-数据长度
 *  返回值：>0成功，0-连接已断开，-1-失败。
*/
int send_data(int sock, const char* buf, int data_len)
{
	if (NULL == buf || data_len <= 0)	return -1;

	int left_bytes = data_len;
	int send_bytes = 0;
	int send_per_times = -1;

	while (1)
	{
		if (send_bytes == data_len) return send_bytes;
		if (send_bytes > data_len) return -3;

		if (left_bytes > 8192)
		{
			send_per_times = send(sock, &buf[send_bytes], 8192, 0);
		}
		else
		{
			send_per_times = send(sock, &buf[send_bytes], left_bytes, 0);
		}

		switch (send_per_times)
		{
		case -1:
			return -2;
		case 0:
			return 0;

		default:
			break;
		}

		send_bytes += send_per_times;
		left_bytes -= send_per_times;
	}
}

/** 功能：接收数据
 *  输入参数：sock-接收数据的连接，
 *  buf-接收缓冲区，
 *  buf_len-接收缓冲区大小
 *  recv_len-收到的数据长度
 *  输出参数:
 *  buf-接收缓冲区，
 *  recv_len-收到的数据长度
 *  返回值：>0-成功，0-连接已断开，-1-失败。
 */
int recv_data(int sock, unsigned char* buf,
	int buf_len, int* recv_len)
{
	int result;
	int left_bytes;

	if (NULL == buf || buf_len <= 0)
	{
		return -1;
	}

	*recv_len = 0;
	while (1)
	{
		left_bytes = buf_len - (*recv_len);
		if (left_bytes > 8192)
		{
			result = recv(sock, &buf[*recv_len], 8192, 0);
		}
		else
		{
			result = recv(sock, &buf[*recv_len], left_bytes, 0);
		}

		switch (result)
		{
		case -1:
			if (NULL != recv_len)
			{
				*recv_len = 0;
			}
			return -2;
		case 0:
			if (NULL != recv_len)
			{
				*recv_len = 0;
			}
			return 0;
		default:
			if (NULL != recv_len)
			{
				*recv_len += result;
			}

			if (*recv_len >= buf_len)
			{
				return 1;
			}
		}
	}
}

/** 功能：配置连接
 *  参数：sock-要配置的连接句柄
 *  返回值：1-成功，-1-失败。
*/
int config_sock(int sock)
{
	int buf_size;
	int result;

	buf_size = 8192;

	result = setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		(char*)&buf_size, sizeof(buf_size));
	if (-1 == result)
	{
		return -1;
	}

	result = setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
		(char*)&buf_size, sizeof(buf_size));
	if (-1 == result)
	{
		return -1;
	}

	return 1;
}

/* 功能：将短整数转换为字节数组
 * 输入参数：val-主机字节序短整数
 * 输出参数：字节数组(2字节)
 * 返回值: 无
*/
void dll_ustobytes(unsigned short val,
	unsigned char bytes[2])
{
	bytes[1] = (unsigned char)(val % 256);
	bytes[0] = (unsigned char)(val / 256);
}

/* 功能：将字节数组转换为短整数
 * 输入参数：bytes-字节数组(2字节)
 * 返回值：短整数
*/
unsigned short dll_bytestous(unsigned char bytes[2])
{
	return bytes[0] * 256 + bytes[1];
}

/* 功能：将整数转换为字节数组
 * 输入参数：val-主机字节序短整数
 * 输出参数：字节数组(4字节)
 * 返回值: 无
*/
void dll_ultobytes(unsigned long val,
	unsigned char bytes[4])
{
	bytes[3] = (unsigned char)(val % 256);
	bytes[2] = (unsigned char)((val % 65536) / 256);
	bytes[1] = (unsigned char)((val / 65536) % 256);
	bytes[0] = (unsigned char)((val / 65536) / 256);
}

/* 功能：将字节数组转换为整数
 * 输入参数：bytes-字节数组(4字节)
 * 返回值：整数
*/
unsigned long dll_bytestoul(unsigned char bytes[4])
{
	return bytes[0] * 256 * 256 * 256 + bytes[1] * 256 * 256
		+ bytes[2] * 256 + bytes[3];
}

