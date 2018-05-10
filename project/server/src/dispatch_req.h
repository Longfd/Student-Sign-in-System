
#ifndef DISPATCH_REQ_H

#define DISPATCH_REQ_H

int deal_client_req(CommThreadInfo* thread_info);

int dispatch_req(CommThreadInfo* thread_info,
		unsigned short func_no, int data_len, 
		unsigned char* reqdata);

#endif


