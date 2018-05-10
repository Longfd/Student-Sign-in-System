
#ifndef COMM_THREAD_H

#define COMM_THREAD_H

#include <pthread.h>
#include "SignInSys_db.h"

#define COMM_BUF_SIZE	200*1024

typedef struct tagCommThreadInfo
{
	pthread_t thread_handle;
	int thread_id;
	int comm_sock;
	char peer_addr[21];
	unsigned peer_port;
	unsigned char comm_buf[COMM_BUF_SIZE];
}CommThreadInfo;

int init_comm_thread_list();
void uninit_comm_thread_list();
CommThreadInfo* get_free_comm_thread();
CommThreadInfo* get_comm_thread(int thread_no);
void wait_comm_listen_thread_exit();
void wait_comm_thread_exit();

int start_comm_listen_thread();

void* comm_listen_thread_routine(void* arg);
void* comm_thread_routine(void* arg);

int start_finger_server();
int init_database();
int uninit_database();

void set_exit_flag(int exit_flag);
int get_exit_flag();
unsigned short count_free_connect();
int is_system_busy(int max_thread);

#endif


