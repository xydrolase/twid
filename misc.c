#include "misc.h"

void
twid_concat_auth_body(TWITTER_USER twitter_user)
{
	twitter_user->userpwd = (char *)malloc(MAX_LEN);
	memset(twitter_user->userpwd, 0, MAX_LEN);	/* initialize the memory of string to zero */
	
	snprintf(twitter_user->userpwd, MAX_LEN, "%s:%s", 
			twitter_user->username, twitter_user->password);
	
}

int
twid_compile_regex(pcre *re, const char *pattern){
	const char *error;
	int err_ind; 
	
	re = pcre_compile(pattern, 0, &error, &err_ind, NULL);
	if (re == NULL){
		/* failed to compile */
		fprintf(stderr, "Failed to compile pcre regex: %s", error);
		
		return 0;
	}
	
	return 1;
}

