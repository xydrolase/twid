#include <curl/curl.h>
#include "unix.h"
#include "twitter.h"

/* structures */
struct twitter_user{
	char *username;
	char *password;
	unsigned int quota_remaining;
	unsigned int quota_hourly;
};

