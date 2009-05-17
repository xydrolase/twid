#include "curl.h"
#include <syslog.h>

CURLcode
twid_curl_perform(const char *url, const char *post_body, int auth, 
	PSTREAM pstream_header, PSTREAM pstream_body){
		
		CURL *curl;
		CURLcode code;
		
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		
		if (post_body && *post_body){
			/* POST body found */
			
			curl_easy_setopt(curl, CURLOPT_POST, 1);	/* use POST method */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body);
		}
		
		if (auth){
			/* HTTP Basic Authentication */
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);  
			/* POST body */
			curl_easy_setopt(curl, CURLOPT_USERPWD, twid_twitter_user->userpwd);
		}
		
		if (twid_twitter_user->sessionid){
			/* Cookie / Session holds by twid */
			curl_easy_setopt(curl, CURLOPT_COOKIE,
				twid_twitter_user->sessionid);  
		}
		
		if (pstream_body){
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pstream_body);
			/* register callback entry */
			curl_easy_setopt(
				curl, 
				CURLOPT_WRITEFUNCTION,
				twid_write_stream_callback
			);
		}
		
		if (pstream_header){
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, pstream_header);
			/* register callback entry */
			curl_easy_setopt(
				curl, 
				CURLOPT_HEADERFUNCTION,
				twid_write_stream_callback
			);
		}
		
		code = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		
		return code;
}

/* normalize the tweet to be escaped, truncated string in post format with 
	prefix which is ready to submit to the Twitter API to publish */
	
char *
twid_normalize_tweet(char *tweet_raw_bytes){
	
	CURL *curl;
	curl = curl_easy_init();
	
	char *tweet_post_body = NULL;
	
	/* considering the utf-8 encoding where one character takes 3 bytes */ 
 	/* the memory needed should be 140 * 3 = 420 bytes */
	char *tweet_truncated = (char *)malloc(TWEET_MAX_LEN * 3 + 1);
	strncpy(tweet_truncated, tweet_raw_bytes, TWEET_MAX_LEN * 3);
	
	/* escape the post body */
	char *tweet_escaped = curl_easy_escape(curl, tweet_truncated, 0);
	if (tweet_escaped){
		int tweet_length = strlen(tweet_escaped);
		/* allocate memory for store the whole POST body */
		tweet_post_body = (char *)malloc(tweet_length + 7);
		memset(tweet_post_body, 0, tweet_length + 7);
		
		/* a safe cast of string duplication */
		strncpy(tweet_post_body, "status=", 7);
		strncat(tweet_post_body, tweet_escaped, tweet_length);
		/* concat the two strings */
	}
	
	curl_easy_cleanup(curl);
	return tweet_post_body;
}

PSTREAM
twid_init_stream_chunk(){
	PSTREAM chunk = (PSTREAM)malloc(sizeof(struct twid_stream));
	chunk->text = NULL;
	chunk->size = 0;
	
	return chunk;
}

static size_t
twid_write_stream_callback(void *ptr, size_t size, size_t nmemb, void *data){
	PSTREAM chunk = (PSTREAM) data;
	size_t full_size = size * nmemb;
	
	/* reallocate memory */
	if (chunk->text == NULL){
		/* 1 char for '\0' */
		chunk->text = (char *)malloc(chunk->size + full_size + 1);
	}
	else {
		chunk->text = (char *)realloc(chunk->text, chunk->size + full_size + 1);
	}
	
	if (chunk->text){
		/* concatenate two strings */
		memcpy(&(chunk->text[chunk->size]), ptr, full_size);
		chunk->size += full_size;
		chunk->text[chunk->size] = 0;	/* string terminator */
	}
	
	return full_size;
}
