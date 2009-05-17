#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include "const.h"


struct twid_packet_upstream{
	int flag;
	int op;
	char tweet[424];	
	/* 140 * 3 + 1 + 3, in which the 3 bytes are allocated for alignment */
};

void twid_serv_handler(int clifd);
void twid_serv_respond(int clifd, const char *msg);
int twid_serv_accept(int listenfd);
int twid_serv_listen(const char *path);
int twid_cli_init(const char *cli_path, const char *serv_path);
int twid_cli_request(int clifd, int op, char *tweet_post_body);
