/*
文件名称: dispatch_req.h
文件说明: 分发客户端请求
*/

#ifndef DISPATCH_REQ_H

#define DISPATCH_REQ_H

//解析客户端请求是否合法
int deal_client_req(CommThreadInfo* thread_info);

//解析合法请求数据, 并派发给对应函数处理
int dispatch_req(CommThreadInfo* thread_info,
		unsigned short func_no, int data_len, 
		unsigned char* reqdata);

#endif


