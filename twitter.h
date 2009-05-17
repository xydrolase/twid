#include <pcre.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "glib-object.h"
#include "json-glib.h"
#include "const.h"


extern TWITTER_USER twid_twitter_user;	/* reference here */

#define CURL_PERFORM_TIMEOUT 30
#define MAX_LEN 1024

void twid_twitter_cache_sessionid(char *buffer);
int twid_twitter_is_auth_valid(PSTREAM chunk);
int twid_twitter_new_tweet(char *tweet_post_body);
void twid_twitter_end_session();
int twid_twitter_authenticate();
void twid_twitter_get_limits();
