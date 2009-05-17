#include "socket.h"

int
twid_serv_listen(const char *path){
	int listenfd;
	struct sockaddr_un sa;
	
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		syslog(LOG_ERR, "Failed to open socket");
		return -1;
	}
	
	unlink(path);	/* unlink the socket file */
	memset(&sa, 0, sizeof(sa));	/* clear the memory block */
	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, path, sizeof(sa.sun_path) -1);
	
	/* bind fd to socket */
	if ((bind(listenfd, (struct sockaddr*)&sa, sizeof(sa))) < 0){
		syslog(LOG_ERR, "Failed to bind socket");
		close(listenfd);
		return -2;
	}
	
	if ((listen(listenfd, QUEUE_LEN)) < 0){
		syslog(LOG_ERR, "Couldn't listen to socket");
		close(listenfd);
		return -3;
	}
	
	return listenfd;
}

int
twid_serv_accept(int listenfd){
	int clifd;
	pid_t pid;
	
	if ((clifd = accept(listenfd, NULL, NULL)) < 0){
		syslog(LOG_ERR, "Failed to accept request from client");
		return -1;
	}
	
	/* Request came, fork a child process to deal with the packets */
	if ((pid = fork()) < 0){
		syslog(LOG_ERR, "Failed to fork a process to deal with packets");
		close(clifd);
		return -2;
	}
	
	if (pid == 0){
		/* child process */
		twid_serv_handler(clifd);
		close(clifd);
		exit(0);
	}
	else{
		/* parent process */
		close(clifd);
	}
}

void
twid_serv_respond(int clifd, const char *msg){
	int n, len;
	len = strlen(msg) + 1;
	if ((n = write(clifd, msg, len)) < len){
		syslog(LOG_ERR, "Error writing to twi client.");
		return;
	}
}

void
twid_serv_handler(int clifd){
	/* only one packet is acceptable, any other packet will be discarded */
	int n;
	struct twid_packet_upstream packet_s;
	while((n = read(clifd, &packet_s, sizeof(packet_s))) > 0){
		if (n == sizeof(packet_s)){
			if (packet_s.flag == 0xee){
				if (packet_s.op & TWID_OP_NEW_TWEET){
					if (*packet_s.tweet && strlen(packet_s.tweet)){
						syslog(LOG_ERR, "New tweet: %s\n", packet_s.tweet);
						twid_serv_respond(clifd, "Hi from twid!");
						if (twid_twitter_new_tweet(packet_s.tweet)){
							twid_serv_respond(clifd, "got it.");
						}
						else{
							twid_serv_respond(clifd,
							"Failed to publish your tweet.");
						}
					}
					else{
						syslog(LOG_ERR, "Invalid tweet to publish.");
					}
				}
				
				if (packet_s.op & TWID_OP_QUOTA){
				}
				
				if (packet_s.op & TWID_OP_LIST){
				}
			}
			else{
				syslog(LOG_INFO, 
					"Invalid packet received with FLAG '%d'",
					packet_s.flag
				);
				return;
			}
		}
	}
	
	/* finished */
	
}
