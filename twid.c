#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "twid.h"

#define MAX_LEN 1024

/* twid, a flexible twitter service */


/* return: fd for writing in the parent process */
int
init_pipe(){
	/* FIFO processing of HTTP downloading stream */
	/* TEST ONLY */
	
	/* initialize the pipe first */
	int pipe_fd[2];
	if (pipe(pipe_fd) < 0){
		printf("[ERROR] Failed create a pipeline\n");
		exit(0);
	}
	
	int pid = fork();	/* fork a child process */
	if (pid == 0){
		/* child process here, open pipe for reading */
		/* close the writing fd */
		close(pipe_fd[1]);
		
		/* buffer for reading */
		/* read for only 50 bytes as a test of truncating string */
		char line[50];	
		int n = read(pipe_fd[0], line, 50);
		
		printf("[PIPE READ]%s\n", line);
		printf("[PIPE READ]%d bytes read\n", n);
		
		/* finished reading, exit child process */
		exit(0);
	}
	else {
		/* parent process here */
		close(pipe_fd[0]);
		return pipe_fd[1];	/* return pipe fd here */
	}
}

CURLcode twid_publish_tweet()
{
	
}



const char*
twid_concat_auth_body(struct twitter_user* user)
{
	char *auth_body = (char *)malloc(MAX_LEN);
	memset(auth_body, 0, MAX_LEN);	/* initialize the memory of string to zero */
	
	snprintf(auth_body, MAX_LEN, "%s:%s", 
			user->username, user->password);
	
	return auth_body;
}

int
main(int argc, char *argv[])
{
	CURL *curl;
	CURLcode code;
	
	/* twid_getopt */
	
	char auth_userpwd[] = "killkeeper:219078imyinxin";
	char *tweet_post_body = 0, *tweet_escaped = 0;
	char *tweet_raw_bytes = 0;
	
	if (argc < 2)
	{
		printf("Usage: %s <your tweet>\n", argv[0]);
		return 0;
	}
	else
	{
		/* considering the utf-8 encoding where one character takes 3 bytes */ 
 		/* the memory needed should be 140 * 3 = 420 bytes */
		tweet_raw_bytes = (char *)malloc(140 * 3);
		strncpy(tweet_raw_bytes, argv[1], 420);	
		/* truncate the string to 420 bytes */
		
		tweet_escaped = curl_easy_escape(curl, tweet_raw_bytes, 0);
		/* escape the post body */
		if (tweet_escaped)
		{
			int tweet_length = strlen(tweet_escaped);
			tweet_post_body = (char *)malloc(tweet_length + 7 + 1);
			/* allocate memory for store the whole POST body */
			
			/* a safe cast of string duplication */
			strncpy(tweet_post_body, "status=", 7);
			strncat(tweet_post_body, tweet_escaped, tweet_length);
			/* concat the two strings */
		}
		else
		{
			printf("[ERROR] Escape failed\n");
			return 0;
		}
	}
	
	int pfd = init_pipe();
	if (pfd < 0){
		printf("Failed to create pipe\n");
		exit(0);
	}
	
	/* open the fd as a stream for writing, which is supported by cURL */
	FILE *fp = fdopen(pfd, "w");
	
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://twitter.com/statuses/update.json");
		curl_easy_setopt(curl, CURLOPT_POST, 1);	/* use POST method */
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);  
		/* HTTP Basic Authentication */
		curl_easy_setopt(curl, CURLOPT_USERPWD, auth_userpwd);	/* POST body */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, tweet_post_body);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);	/* callback stream */
		
		code = curl_easy_perform(curl);
 		if (code != CURLE_OK)
 		{
 			printf("[ERROR] Failed to call Twitter API...\n");
 		}
 
		/* always cleanup */
		curl_easy_cleanup(curl);
		
		/* free the memory allocated for storing strings */
		free(tweet_post_body);
		free(tweet_escaped);
	}
	return 0;
}
