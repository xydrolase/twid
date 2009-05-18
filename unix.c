#include "unix.h"

/* Parse GNU style parameters/options */
void
twid_getopt(int argc, char *argv[], TWITTER_USER tuser){
	char op;
	int length;
	tuser->username = NULL;
	tuser->password = NULL;
	
	while( (op = getopt(argc, argv, "u:p:q")) != -1 ){
		switch(op){
			case 'u':
				/* username */
				length = strlen(optarg);
				tuser->username = (char *)malloc(length + 1);
				strncpy(tuser->username, optarg, length);
				
				break;
			case 'p':
				/* password */
				length = strlen(optarg);
				tuser->password = (char *)malloc(length + 1);
				strncpy(tuser->password, optarg, length);
				
				break;
			case 'q':
				/* quiet mode without notify */
				tuser->quiet_mode = 1;
				
				break;
			case '?':
				if (isprint(optopt)){
					fprintf(stderr, "Unknown option: '-%c'\n", optopt);
					exit(0);
				}else {
					fprintf(stderr, "Unknown option.\n");
					exit(0);
				}
				break;
		}	
	}
}

int
twid_getopt_cli(int argc, char *argv[]){
	char c;
	int op = 0;
	while((c = getopt(argc, argv, "Ql")) != -1){
		switch(c){
			case 'Q':
				/* retrieve quota(rate limits) */
				op |= TWID_OP_QUOTA;
				break;
			case 'l':
				/* list tweets */
				op |= TWID_OP_LIST;
				break;
			case '?':
				fprintf(stderr, "Unknown option: '-%c'\n", optopt);
				exit(0);
				break;
		}
	}
	
	return op;
	
}

int
twid_instance_exists(){
	int fd, n;
	char buf[16];
	
	fd = open(LOCKFILE, O_CREAT|O_RDWR, LOCKMODE);
	if (fd < 0){
		syslog(LOG_ERR, "Can't open lock file: %s", LOCKFILE);
		exit(0);
	}
	
	if (twid_lockfile(fd) < 0) {
		if (errno = EACCES || errno == EAGAIN) {
			close(fd);
			return 1;
		}
		syslog(LOG_ERR, "Can't lock file: %s", LOCKFILE);
		exit(0);
	}
	
	if (ftruncate(fd, 0) < 0){
		syslog(LOG_ERR, "Can't truncate file: %s to zero length", LOCKFILE);
		exit(0);
	}
	sprintf(buf, "%ld", (long)getpid());
	if ((n = write(fd, buf, strlen(buf) + 1)) < strlen(buf) + 1){
		syslog(LOG_ERR, "Error writing PID to lockfile: %s", LOCKFILE);
		exit(0);
	}
	
	
	return 0;
}

void twid_sig_chld(int signo){
	/* waiting for the child process to exit */
	
	pid_t pid;
	int stat;
	
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		;	/* do nothing, just wait every child process to terminate normaly */
	
	return;
}

void twid_query_username(TWITTER_USER tuser){
	char buffer[256];
	int slen;
	
	printf("Twitter ID / Email: ");
	if (fgets(buffer, 255, stdin) <= 0){
		fprintf(stderr, "Invalid Twittier ID / Email.\n");
		exit(0);
	}
	
	if ((slen = (strlen(buffer) - 1)) > 1){
		buffer[slen] = 0;	/* override the LF */
		tuser->username = (char *)malloc(slen); /* ignore the LF */
		strncpy(tuser->username, buffer, slen);
	}
	else{
		fprintf(stderr, "Invalid Twittier ID / Email.\n");
		exit(0);
	}
}

void twid_query_password(TWITTER_USER tuser){

	struct termios term;
	int oldflag;
	char buffer[256];
	int slen;
	
	if (tcgetattr(1, &term) < 0){
		fprintf(stderr,
		"[ERROR] Failed to get TERMINAL I/O status, your password might be \
visible.\n");
	}
	else{
		oldflag = term.c_lflag;
		term.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);	/* set new flag */
	}
	
	if (tcsetattr(1, TCSAFLUSH, &term) < 0){
		fprintf(stderr,
		"[ERROR] Failed to set TERMINAL I/O status, your password might be \
visible.\n");
	}
	
	printf("Your password: ");
	fflush(stdout);	/* flush all data, and validate the new TERMIO status */
	if (fgets(buffer, 255, stdin) <= 0){
		fprintf(stderr, "Invalid Twittier ID / Email.\n");
		exit(0);
	}
	
	/* recover termio */
	term.c_lflag = oldflag;
	tcsetattr(1, TCSANOW, &term);
	
	if ((slen = (strlen(buffer) - 1)) > 1){
		buffer[slen] = 0;	/* override the LF */
		tuser->password = (char *)malloc(slen); /* ignore the LF */
		strncpy(tuser->password, buffer, slen);
	}
	else{
		fprintf(stderr, "Invalid password.\n");
		exit(0);
	}
}

int
twid_lockfile(int fd){
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}

/* Daemonize Twid as a daemon service to handle Twitter API */
void
twid_daemonize(){
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0); /* clear file creation mask */

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0){
        printf("[ERROR] Couldn't get file limit\n");
        exit(0);
    }

    /* become session leader and to lose TTY control */
    if ((pid = fork()) < 0){
        printf("[ERROR] Failed to fork()\n");
        exit(0);
    }
    else if (pid != 0){
        /* parent process */
        exit(0);
    }

    /* child process here */
    setsid();   /* to become the session leader */
    /* ignore the SIG_HUP */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0){
        printf("[ERROR] Couldn't ignore SIGHUP\n");
        exit(0);
    }

    /* fork the second child process */
    if ((pid = fork()) < 0){
        printf("[ERROR] Failed to fork()\n");
        exit(0);
    }
    else if (pid != 0){
        /* first child here */
        exit(0);
    }

    /* second process here */
    if (chdir("/") < 0){
        printf("[ERROR] Coudldn't chdir() to '/'\n");
        exit(0);
    }

    /* close all fd */
    if (rl.rlim_max == RLIM_INFINITY){
        rl.rlim_max = 1024; /* assume the maximum to be 1024 */
    }
    for(i = 0; i < rl.rlim_max; ++i){
        close(i);
    }

    fd0 = open("/dev/null", O_RDWR);    /* open /dev/null to forbid standard I/O */
    fd1 = dup(0);
    fd2 = dup(0);
    
    /* logging */
	openlog("twid", LOG_CONS, LOG_DAEMON);
	
	if (fd0 != 0 || fd1 != 1 || fd2 != 2){
		/* bad file descriptors */
		syslog(LOG_ERR, "Unexcepted file descriptors: %d %d %d", 
			fd0, fd1, fd2);
		
		exit(0);
	}
}
