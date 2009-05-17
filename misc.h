#include <pcre.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include "const.h"

#define MAX_LEN 1024

PSTREAM twid_init_stream_chunk();
int twid_compile_regex(pcre *re, const char *pattern);
void twid_concat_auth_body(TWITTER_USER twitter_user);
