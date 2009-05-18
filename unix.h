#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include "const.h"

int twid_instance_exists();
void twid_daemonize();void twid_getopt(int argc, char *argv[], TWITTER_USER tuser);
int twid_lockfile(int fd);
int twid_getopt_cli(int argc, char *argv[]);
void twid_sig_chld(int signo);

void twid_query_password(TWITTER_USER tuser);
void twid_query_username(TWITTER_USER tuser);

typedef void (*ptr_pipe_callback)(const char *, int);
