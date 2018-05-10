#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "packet.h"

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

unsigned char* get_next_field(int* data_len,
		unsigned char* data, char* field, int field_len)
{
	int length = 0;

	if (*data_len <2)
	{
		return NULL;
	}
	length = dll_bytestous(data);
	*data_len -= 2;
	data += 2;

	/* 字段值 */
	if (*data_len < length)
	{
		return NULL;
	}
	if (length > 0)
	{
		if (field_len > length)
		{
			field_len = length;
		}
		memcpy(field, data, field_len);
	}
	*data_len -= length;
	data += length;

	return data;
}

/** 功能：生成包头
 * */
void make_pack_head(unsigned short pack_type, 
		int data_len, unsigned char* pack_head)
{
	int cur_pos = 0;
	unsigned short check_sum = 0;
	int i = 0;

	dll_ultobytes(pack_type, &pack_head[cur_pos]);
	cur_pos += 4;

	dll_ultobytes(data_len, &pack_head[cur_pos]);
	cur_pos += 4;

	for (i=0; i<cur_pos; i++)
	{
		check_sum += ((unsigned char)pack_head[i]);
	}

	dll_ustobytes(check_sum, &pack_head[cur_pos]);
	cur_pos += 2;
}

/** 功能：分析包头
 * */
int parse_pack_head(unsigned char* pack_head, 
		unsigned short* pack_type, int* data_len)
{
	unsigned short check_sum1 = 0; 
	unsigned short check_sum2 = 0;
	int i = 0;

	for (i=0; i<PACK_HEAD_LEN-2; i++)
	{
		check_sum1 += ((unsigned char)pack_head[i]);
	}

	check_sum2 = dll_bytestous(&pack_head[PACK_HEAD_LEN-2]);
	if (check_sum2 != check_sum1)
	{
		return -1;
	}
	*pack_type = dll_bytestoul(pack_head);
	*data_len = dll_bytestoul(pack_head+4);

	return 0;
}

/** 功能：发送数据包
 * */
int send_packet(int sock, unsigned short pack_type,
		int data_len, const unsigned char* data)
{
	int field_pos = 0;
	unsigned short check_sum = 0;
	int i = 0;
	unsigned char buf[10*1024];

	make_pack_head(pack_type, data_len+2, buf);
	field_pos += PACK_HEAD_LEN;

	memcpy(&buf[field_pos], data, data_len);
	field_pos += data_len;

	for (i=0; i<data_len; i++)
	{
		check_sum += ((unsigned char)data[i]);
	}
	dll_ustobytes(check_sum, &buf[field_pos]);
	field_pos += 2;
	
	return send_data(sock, (const char*)buf, field_pos);
}

int check_packet_body(unsigned char* data, int data_len)
{
	unsigned short check_sum1 = 0;
	unsigned short check_sum2 = 0;
	int i;

	for (i=0; i<data_len-2; i++)
	{
		check_sum1 += ((unsigned char)data[i]);
	}

	check_sum2 = dll_bytestous(&data[data_len-2]);
	if (check_sum1 != check_sum2)
	{
		return -1;
	}

	return 0;
}


