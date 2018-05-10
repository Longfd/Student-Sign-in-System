#include <string.h>
#include <stdio.h>

#include "comm_thread.h"
#include "commun.h"
#include "packet.h"
#include "dispatch_req.h"
#include "loginfo.h"
#include "SignInSys_Function.h"

int deal_client_req(CommThreadInfo* thread_info)
{
	int recved_len = 0;
	int result = 0;
	unsigned short pack_type = 0;
	int data_len = 0;
	unsigned char head[PACK_HEAD_LEN];
	unsigned char comm_buf[20 * 1024];

	write_debug_log("Recv packet from:[%s]:[%d]", thread_info->peer_addr, thread_info->peer_port);
	memset(comm_buf, 0, sizeof(comm_buf));
	result = recv_data(thread_info->comm_sock, head,
		PACK_HEAD_LEN, &recved_len);
	if (result <= 0)
	{
		if (recved_len > 0)
		{
			int i;
			char message[100];
			strcpy(message, "");
			for (i = 0; i < recved_len; i++)
			{
				sprintf(message + strlen(message), "%02x ", head[i]);
			}
			write_error_log(__FILE__, __LINE__,
				"接收请求包头错误!数据:[%s]", message);
		}
		else
		{
			write_error_log(__FILE__, __LINE__, "接收请求包头错误!");
		}
		return result;
	}

	if (recved_len != PACK_HEAD_LEN)
	{
		write_error_log(__FILE__, __LINE__, "请求包头长度错误!");
		return -1;
	}

	result = parse_pack_head(head, &pack_type, &data_len);
	if (0 != result)
	{
		int i;
		char message[100];
		strcpy(message, "");
		for (i = 0; i < recved_len; i++)
		{
			sprintf(message + strlen(message), "%02x ", head[i]);
		}
		write_error_log(__FILE__, __LINE__,
			"解析请求包头错误!数据[%s]", message);
		return -1;
	}

	result = recv_data(thread_info->comm_sock, comm_buf,
		data_len, &recved_len);
	if (result <= 0)
	{
		write_error_log(__FILE__, __LINE__, "接收请求包体错误!");
		return -1;
	}

	if (recved_len != data_len)
	{
		write_error_log(__FILE__, __LINE__, "请求包体长度错误!");
		return -1;
	}

	return dispatch_req(thread_info, pack_type,
		data_len, comm_buf);
}

int dispatch_req(CommThreadInfo* thread_info,
	unsigned short func_no, int data_len,
	unsigned char* reqdata)
{
	unsigned short check_sum1 = 0;
	unsigned short check_sum2 = 0;
	int i;

	for (i = 0; i < data_len - 2; i++)
	{
		check_sum1 += ((unsigned char)reqdata[i]);
	}

	check_sum2 = dll_bytestous(&reqdata[data_len - 2]);
	if (check_sum1 != check_sum2)
	{
		write_error_log(__FILE__, __LINE__,
			"包体校验和错误(%d,%d)!", check_sum1,
			check_sum2);
		return -1;
	}
	write_debug_log("请求包校验OK! 包类型:%d", func_no);
	reqdata[data_len - 2] = '\0';

	switch (func_no)
	{
	case CLIENT_REQ_REGISTER:
		return userRegister(thread_info, reqdata);

	case	CLIENT_REQ_LOG_IN:
		return userSignUp(thread_info, reqdata);

	case CLIENT_REQ_ADD_CLASS:
		return addClassRequest(thread_info, reqdata);

	case CLIENT_REQ_JOIN_CLASS:
		return joinClassRequest(thread_info, reqdata);
		
	default:
		write_debug_log("收到不认识的请求包,包类型码:%d!", func_no);
		break;
	}
	return -1;
}


