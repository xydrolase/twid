#include "socket.h"

int
twid_cli_init(const char *cli_path, const char *serv_path){
	int clifd;
	struct sockaddr_un sa;
	
	if ((clifd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Failed to open socket for transmitting.\n");
		return -1;
	}
	
	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_UNIX;
	/* this cast should be safe since the string is considerably small to be 
	safe enough without overflow
	*/
	sprintf(sa.sun_path, "%s.%05d", cli_path, getpid());
	unlink(sa.sun_path);	/* unlink previous sock file */
	
	if (bind(clifd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
		fprintf(stderr, "Failed to bind socket.\n");
		close(clifd);
		return -2;
	}
	
	/* fill the sockaddr with the server's socket path */
	memset(&sa, 0, sizeof(sa)); 	/* erase the memory block again */
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, serv_path, sizeof(sa.sun_path) - 1);
	
	if (connect(clifd, (struct sockaddr *)&sa, sizeof(sa)) < 0){
		fprintf(stderr, "Couldn't connect to twid server.\n");
		close(clifd);
		return -3;
	}
	
	return clifd;
}

int
twid_cli_request(int clifd, int op, char *tweet_post_body){
	if (op < 0){
		fprintf(stderr, "No valid operations.\n");
		return -1;
	}
	int packet_len, n;
	char recv[MAXLINE];
	memset(recv, 0, MAXLINE);
	
	/* bulid the packet */
	struct twid_packet_upstream packet_s;
	packet_s.flag = 0xee;
	packet_s.op = op;
	strncpy(packet_s.tweet, tweet_post_body, sizeof(packet_s.tweet));
	
	packet_len = sizeof(packet_s);
	
	/* transmission */
	n = write(clifd, &packet_s, packet_len);
	if (n < packet_len){
		fprintf(stderr, 
			"Error occurred when transmitting packet to twid server.\n"
		);
	}

	/* receiving response */
	while((n = read(clifd, recv, MAXLINE)) > 0){
		fputs(recv, stdout);
		memset(recv, 0, MAXLINE);	/* clear buffer */
	}
	
	if (n < 0){
		fprintf(stderr, "Reading error.\n");
		return -2;
	}
	
	return 1;
}
