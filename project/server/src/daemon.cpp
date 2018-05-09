#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "commun.h"
#include "config.h"
#include "loginfo.h"
#include "comm_thread.h"

int daemon_init()
{
	int i, maxfd;

	if (fork() != 0)
	{
		exit(0);
	}

	if (setsid() < 0)
	{
		return -1;
	}

	signal(SIGHUP, SIG_IGN);

	if (fork() != 0)
	{
		exit(0);
	}

	chdir("/");
	umask(0);

	maxfd = getdtablesize();
	for (i=0; i< maxfd; i++)
	{
		close(i);
	}

	open("/dev/null", O_RDWR);

	dup(0);
	dup(1);
	dup(2);

	return 0;
}

void term_handler(int signo)
{
	if (get_exit_flag() != 0)
	{
		return;
	}
	write_info_log("程序正常结束！");
	signal(SIGTERM, SIG_IGN);
	set_exit_flag(1);
}

void segv_handler(int signo)
{
	if (get_exit_flag() != 0)
	{
		return;
	}
	set_exit_flag(1);
	write_info_log("程序异常退出！");
	exit(0);
}

int main(int argc, char** argv)
{
	int result;
	char cfg_name[255];
	pid_t child_pid;

	get_module_path();
	sprintf(cfg_name, "%s/etc/server.cfg", g_module_path);
	result = read_config(cfg_name);
	if (result != 0)
	{
		printf("读取配置文件错误！\n");
		return -1;
	}

	signal(SIGCHLD, SIG_IGN);

	if (daemon_init() < 0)
	{
		printf("创建守护进程错误！\n");
		exit(1);
	}

	while (1)
	{
		child_pid = fork();
		if (0 == child_pid) /* 子进程 */
		{  
			signal(SIGTERM, term_handler);
			signal(SIGSEGV, segv_handler);
			signal(SIGPIPE, SIG_IGN);
			signal(SIGALRM, SIG_IGN);
			signal(SIGINT, SIG_IGN);

			result = start_finger_server();
			if (result != 0)
			{
				write_info_log("启动签到服务器错误!错误码为:%d", result);
				exit(-1);
			}

			wait_comm_listen_thread_exit();
			write_info_log("通信侦听线程已退出!");
			wait_comm_thread_exit();
			write_info_log("所有通信线程已退出!");

			uninit_database();
			write_info_log("所有数据库连接已关闭!");
			uninit_comm_thread_list();
			write_info_log("释放通信线程内存完毕!");

			write_info_log("签到服务器退出完毕!");
			exit(0);
		}
		else if (child_pid < 0) /* 创建子进程错误*/ 
		{
			write_info_log("创建子进程错误!");
			return -1;
		}
		else /* 父进程 */
		{
			waitpid(child_pid, NULL, 0);
		}
	}
	return 0;
}

