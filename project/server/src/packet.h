
#ifndef _FINGER_PROTO_H

#define _FINGER_PROTO_H

#define PACK_HEAD_LEN 10

void make_pack_head(unsigned short pack_type, 
		int data_len, unsigned char* pack_head);
int parse_pack_head(unsigned char* pack_head,
		unsigned short* pack_type, int* data_len);
int send_packet(int sock, unsigned short pack_type,
	int data_len, const unsigned char* data);

unsigned char* get_next_field(int* data_len,
		unsigned char* data, char* field, int field_len);

int check_packet_body(unsigned char* data, int data_len);

#endif

