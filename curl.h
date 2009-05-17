#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"

#define TWEET_MAX_LEN 140

CURLcode twid_curl_perform(const char *url, const char *post_body, int auth, 
	PSTREAM pstream_header, PSTREAM pstream_body);
	
char *twid_normalize_tweet(char *tweet_raw_bytes);
static size_t twid_write_stream_callback
	(void *ptr, size_t size, size_t nmemb, void *data);
PSTREAM twid_init_stream_chunk();
