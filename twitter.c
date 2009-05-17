#include "twitter.h"

int
twid_twitter_authenticate(){
	
	PSTREAM pstream_header = (PSTREAM)twid_init_stream_chunk();
	PSTREAM pstream_body = (PSTREAM)twid_init_stream_chunk();
	
	CURLcode code = twid_curl_perform(
		"http://twitter.com/account/verify_credentials.json", 
		NULL, 1, pstream_header, pstream_body);
		
	if (code != CURLE_OK){
		/* error process */
		fprintf(stderr, 
			"CURL Error[%d] occurred \
while submitting the authentication request, user: %s\n", 
			code, twid_twitter_user->username);
		
		free(pstream_body);
		free(pstream_header);
		return 0;
	}
	
	/* done calling curl, now deal with the response */
	if (twid_twitter_is_auth_valid(pstream_body)){
		printf("auth valid\n");
		twid_twitter_cache_sessionid(pstream_header->text);
		twid_twitter_get_limits();
	}
	else{
		free(pstream_body);
		free(pstream_header);
		
		fprintf(stderr,
				"Failed to verify your account: %s\n",
				twid_twitter_user->username);
			
		return 0;
	}
	
	free(pstream_body);
	free(pstream_header);
	
	return 1;
	
}

void
twid_twitter_end_session(){
	CURLcode code = twid_curl_perform(
		"API here",
		NULL, 0, NULL, NULL	);
}

void
twid_twitter_get_limits(){
	printf("getting limits..., sess=%s\n", twid_twitter_user->sessionid);
	PSTREAM pstream = (PSTREAM)twid_init_stream_chunk();
	
	CURLcode code = twid_curl_perform(
		"http://twitter.com/account/rate_limit_status.json",	/* API entry */
		NULL,										/* NO POST */
		0,											/* No authentication */
		NULL,										/* No header callback */
		pstream										/* Response callback */
	);
	
	if (code != CURLE_OK){
		fprintf(stderr,
			"CURL Error[%d], which causes the failure of fetching rate limits\n",
			code
		);
		return;
	}
	
	JsonParser *parser = json_parser_new();
	printf("BODY: %s\n", pstream->text);
	JsonNode *root = (JsonNode *)twid_json_get_root(pstream->text, parser);
	
	if (root == NULL){
		fprintf(stderr,
			"Failed to retrieve rate limits via API, \
which means your tweet might be omitted if you update so fast :p.\n"
		);
	}
		
	GValue quota_hourly = {0, }, quota_remaining = {0, };
	
	if (twid_json_get_value_by_name(root, "remaining_hits", &quota_remaining) &&
		twid_json_get_value_by_name(root, "hourly_limit", &quota_hourly)){
		if (G_VALUE_HOLDS_INT(&quota_hourly)){
			twid_twitter_user->quota_hourly = g_value_get_int(&quota_hourly);
		}
		
		if (G_VALUE_HOLDS_INT(&quota_remaining)){
			twid_twitter_user->quota_remaining = 
				g_value_get_int(&quota_remaining);
		}
		
	}
	else{
		fprintf(stderr,
			"Failed to parse API response to get rate limits, \
which means your tweet might be omitted if you update so fast :p.\n"
		);
	}
	
	free(pstream);
	g_object_unref(parser);
}

int
twid_twitter_new_tweet(char *tweet_post_body){
	PSTREAM pstream_body = (PSTREAM)twid_init_stream_chunk();
	
	CURLcode code = twid_curl_perform(
		"http://twitter.com/statuses/update.json",	/* API entry */
		tweet_post_body,							/* POST body */
		0,											/* No authentication */
		NULL,										/* No header callback */
		pstream_body								/* Response callback */
	);
	
	if (code != CURLE_OK){
		/* error process here */
		syslog(LOG_ERR, "CURL ERROR while publishing a new tweet");
		free(pstream_body);
		return 0;
	}
	
	free(pstream_body);
	return 1;
}

/* functions below for parsing JSON packets */

int
twid_twitter_is_auth_valid(PSTREAM chunk){
	JsonParser *parser = json_parser_new();
	JsonNode *root = (JsonNode *)twid_json_get_root(chunk->text, parser);
	if (root == NULL){
		return 0;
	}
	
	GValue gv_tmp = {0, };
	
	if (twid_json_get_value_by_name(root, "error", &gv_tmp)){
		json_node_free(root);
		g_object_unref(parser);
		return 0;
	}
	else{
		/* some methods are omitted here */
		json_node_free(root);
		g_object_unref(parser);
		return 1;
	}
}

void
twid_twitter_cache_sessionid(char *buffer){
	if (buffer){
		int matches, i;
		int ovector[30];
		const char *error;
		int erroffset;
		
		pcre *re = pcre_compile(
			"Set-Cookie: (_twitter_sess=[^;]+;).+",
			0,
			&error,
			&erroffset,
			NULL
			);
		
		matches = pcre_exec(re, NULL, 
			buffer, strlen(buffer), 0, 0, ovector, 30);
		
		if (matches < 0){
			if (matches == PCRE_ERROR_NOMATCH){
				fprintf(stderr, "Couldn't match Set-Cookie in HTTP header.\n");
			}
			else{
				fprintf(stderr, "Failed to execute regex.\n");
			}
		}
		
		printf("matches: %d\n", matches);
		if (matches == 2){
			/* $1 for the session id */
			i = 1;
			char *session_start = buffer + ovector[i * 2];
			int session_len = ovector[i * 2 + 1] - ovector[i * 2];
			twid_twitter_user->sessionid = (char *)malloc(session_len + 1);
			/* copy session id to the user structure */
			strncpy(twid_twitter_user->sessionid, session_start, session_len);
		}
		
	}
}
