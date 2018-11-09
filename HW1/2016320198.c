#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_PORT 47500

int main(void) {
	struct hostent *hp;
	struct sockaddr_in sin;
	char host[] = "127.0.0.1";
	char buf[] = "2016320198";
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

	/* Send student ID number */
	len = strlen(buf) + 1;
	send(s, buf, len, 0);
	close(s);
}
