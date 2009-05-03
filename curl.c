#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "twid.h"

#define MAX_LEN 1024

/* twid, a flexible twitter service */

struct twitter_user{
	char *username;
	char *password;
	unsigned int quota_remaining;
	unsigned int quota_hourly;
};

bool twid_already_running()
{
		
}

void twid_daemonize()
{
	
}

CURLcode twid_publish_tweet()
{
	
}

const char *twid_concat_auth_body(struct twitter_user*)
{
	char *auth_body = (char *)malloc(MAX_LEN);
	memset(auth_body, 0, MAX_LEN);	/* initialize the memory of string to zero */
	
	snprintf(auth_body, MAX_LEN, "%s:%s", 
			twitter_user->username, twitter_user->password);
	
	return auth_body;
}

int main(int argc, char *argv[])
{
	CURL *curl;
	CURLcode code;
	
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
	
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://twitter.com/statuses/update.json");
		curl_easy_setopt(curl, CURLOPT_POST, 1);	/* use POST method */
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);  /* HTTP Basic Authentication */
		curl_easy_setopt(curl, CURLOPT_USERPWD, auth_userpwd);	/* POST body */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, tweet_post_body);
		
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
