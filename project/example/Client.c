#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "packet.h"

#define BUF_SIZE 8192
#define PACK_TYPE 1002
#define IP "127.0.0.1"
#define PORT 7000

int main(int argc, char* argv[])
{

	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == client_fd){
		perror("socket error");
		return -2;
	}
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	if(0 == inet_aton(IP, &server_addr.sin_addr)){
		perror("inet_aton error");
		return -3;
	}
	server_addr.sin_port = htons(PORT);

	socklen_t sockAddr_len = sizeof(struct sockaddr);
	int iRet = connect(client_fd, (struct sockaddr*)&server_addr, sockAddr_len);
	if(-1 == iRet){
		perror("connect error");
		return -4;
	}

	char* send_buf = (char*)calloc(BUF_SIZE, sizeof(char));	
	if(NULL == send_buf){
		perror("calloc error");
		exit(EXIT_FAILURE);
	}

	//debug
	const char* p = "{"
		"\"userId\":\"00001\","
		"\"userName\":\"张三\","
		"\"role\":\"1\","
		"\"pwd\":\"123456\""
		"}";


	size_t buf_size = BUF_SIZE;
	while(getline(&send_buf, &buf_size, stdin)){
		unsigned char head[11] = { 0 };
		unsigned char pack[8192] = { 0 };
		//memset(send_buf, 0, sizeof(send_buf));
		//strcpy(send_buf, p);
		int send_len = send_packet(client_fd, PACK_TYPE, strlen(send_buf), (unsigned char*)send_buf);
		//int send_len = send(client_fd, send_buf, BUF_SIZE, 0);
		if(send_len <= 0){
			perror("send error");
			return -5;
		}
		printf("client: send_len:%d, buf:%s\n", send_len, send_buf);


		memset(head, 0, 11);
		unsigned short pack_type = 0;
		int data_len = 0;
		int recved_len = 0;
		char read_buf[BUF_SIZE] = {0};

		int result = recv_data(client_fd, head, PACK_HEAD_LEN, &recved_len);
		if (result <= 0){
			if (recved_len > 0){
				int i;
				char message[100] = {0};
				for (i = 0; i < recved_len; i++)
					sprintf(message + strlen(message), "%02x ", head[i]);
				printf("接收包头错误!数据:[%s]\n", message);
			}
			else{
				printf("接收返回包头错误!");
			}
			return result;
		}

		if (recved_len != PACK_HEAD_LEN){
			printf("包头长度错误! recv_len:%d\n", recved_len);
			return -1;
		}

		result = parse_pack_head(head, &pack_type, &data_len);
		if (0 != result){
			int i;
			char message[100] = { 0 };
			for (i = 0; i < recved_len; i++)
				sprintf(message + strlen(message), "%02x ", head[i]);
			printf("解析包头错误!数据[%s]\n", message);
			return -1;
		}

		result = recv_data(client_fd, (unsigned char*)read_buf, data_len, &recved_len);
		if (result <= 0){
			printf("接收包体错误! result:%d\n", result);
			return -1;
		}

		if (recved_len != data_len){
			printf("接收包体长度错误! result:%d\n", result);
			return -1;
		}

		printf("server: pack_type[%d], pack_body:%s\n", pack_type, read_buf);
	}

	close(client_fd);
	return 0;
}
