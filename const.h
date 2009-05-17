#include <pcre.h>

/* socket packet op code */
#define TWID_OP_NEW_TWEET 1
#define TWID_OP_QUOTA 2
#define TWID_OP_LIST 4

#define QUEUE_LEN 5
#define MAXLINE 2048
#define MAX_LEN 1024

/* for lock file */
#define LOCKFILE "/tmp/twid.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

/* socket path */
#define TWID_SERV_SOCKET_PATH "/tmp/twid-serv.sock"
#define TWID_CLI_SOCKET_PATH "/tmp/twid-cli.sock"

/* structures */
typedef struct twitter_user{
	char *username;
	char *password;
	char *userpwd;
	char *sessionid;
	unsigned int quota_remaining;
	unsigned int quota_hourly;
	int quiet_mode;
} *TWITTER_USER;

TWITTER_USER twid_twitter_user;

/* for regular expressions */
pcre *REGEX_SESSION;
#define PATTERN_SESSION "Set-Cookie: (_twitter_sess=[^;]+;).+"

/* PSTERAM */
struct twid_stream {
	char *text;
	size_t size;
};

typedef struct twid_stream *PSTREAM;

typedef struct listNode {
	char *name;
	struct listNode *next;
} *PLIST;
