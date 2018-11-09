#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_PORT 47500

/* Flag represents type of IR protocol */
#define FLAG_HELLO		((unsigned char)(0x01 << 7))
#define FLAG_INSTRUCTION	((unsigned char)(0x01 << 6))
#define FLAG_RESPONSE		((unsigned char)(0x01 << 5))
#define FLAG_TERMINATE		((unsigned char)(0x01 << 4))

/* OP field designates type of operation */
#define OP_ECHO			((unsigned char)(0x00))
#define OP_INCREMENT		((unsigned char)(0x01))
#define OP_DECREMENT		((unsigned char)(0x02))

struct hw_packet {
	unsigned char flag;		// HIRT-4bits, reserved-4bits
	unsigned char operation;	// 8-bits operation
	unsigned short data_len;	// 16 bits (2 bytes) data length
	unsigned int seq_num;		// 32 bits (4 bytes) sequence number
	char data[1024];		// optional data
};

int main(void) {
	struct hostent *hp;
	struct sockaddr_in sin;
	char host[] = "127.0.0.1";
	int s;
	int len;

	/* 
	Translate host name into IP address & check IP address. 
	I already set host variable's value to IP address,
	so there is no translation. Just make hostent struct. 
	*/
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "simple-talk: unknown host: %s\n", host);
		exit(1);
	}

	/* Build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;	//connect internet
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(SERVER_PORT);

	/* Active open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simple-talk: socket");
		exit(1);
	}

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("simplex-talk : connect");
		close(s);
		exit(1);
	}

	printf("*** starting ***\n\n");

	struct hw_packet req_buf_struct;
	struct hw_packet res_buf_struct;
	unsigned int value;

	req_buf_struct.flag = FLAG_HELLO;
	req_buf_struct.operation = OP_ECHO;
	req_buf_struct.data_len = 4;
	req_buf_struct.seq_num = 0;
	value = 2016320198;

	memcpy(req_buf_struct.data, &value, sizeof(unsigned int));	
	
	printf("sending first hello msg...\n");
	send(s, &req_buf_struct, sizeof(req_buf_struct), 0);

	recv(s, &res_buf_struct, sizeof(res_buf_struct), 0);
	printf("received hello message from the server!\n");

	printf("waiting for the first instruction message...\n");

	while(1) {
		recv(s, &res_buf_struct, sizeof(res_buf_struct), 0);
		
		if (res_buf_struct.flag == FLAG_INSTRUCTION) {
			printf("received instruction message! received data_len : %d bytes\n", res_buf_struct.data_len);
			
			if (res_buf_struct.operation == OP_ECHO) {
				printf("operation type is echo.\n");
				printf("echo : %s\n", res_buf_struct.data);
				
				req_buf_struct.flag = FLAG_RESPONSE;
				req_buf_struct.operation = OP_ECHO;
				req_buf_struct.data_len = res_buf_struct.data_len;
				req_buf_struct.seq_num = res_buf_struct.seq_num;
				memcpy(req_buf_struct.data, &res_buf_struct.data, sizeof(res_buf_struct.data));

				send(s, &req_buf_struct, sizeof(req_buf_struct), 0);
				printf("sent response msg with seq.num. %d to server.\n\n", req_buf_struct.seq_num);
			} else if (res_buf_struct.operation == OP_INCREMENT) {
				printf("operation type is increment.\n");
				
				memcpy(&value, res_buf_struct.data, sizeof(unsigned int));
				value += 1;

				req_buf_struct.flag = FLAG_RESPONSE;
				req_buf_struct.operation = OP_ECHO;
				req_buf_struct.data_len = 4;
				req_buf_struct.seq_num = res_buf_struct.seq_num;
				memcpy(req_buf_struct.data, &value, sizeof(unsigned int));
				printf("increment : %d\n", value);

				
				send(s, &req_buf_struct, sizeof(req_buf_struct), 0);
				printf("sent response msg with seq.num. %d to server.\n\n", req_buf_struct.seq_num);
			} else if (res_buf_struct.operation == OP_DECREMENT) {
				printf("operation type is decrement.\n");
				
				memcpy(&value, res_buf_struct.data, sizeof(unsigned int));
				value -= 1;

				req_buf_struct.flag = FLAG_RESPONSE;
				req_buf_struct.operation = OP_ECHO;
				req_buf_struct.data_len = 4;
				req_buf_struct.seq_num = res_buf_struct.seq_num;
				memcpy(req_buf_struct.data, &value, sizeof(unsigned int));
				printf("decrement : %d\n", value);

				
				send(s, &req_buf_struct, sizeof(req_buf_struct), 0);
				printf("sent response msg with seq.num. %d to server.\n\n", req_buf_struct.seq_num);	
			}	
				

		} else if (res_buf_struct.flag == FLAG_TERMINATE) {
			printf("received terminate msg! terminating...\n");
			break;
		}
	}


	close(s);
}
