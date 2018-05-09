#include <stdio.h>
#include <string.h>
#include <sys/times.h>

#include "packet.h"
#include "commun.h"


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


