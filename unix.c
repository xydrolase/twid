#include <unistd.h>

#define MAXLEN 1024;

/* Parse GNU style parameters/options */
void
twid_getopt(int argc, char *argv[], struct twitter_user *user){
	char op;
	while( op = getopt(argc, argv, "up") != -1 ){
		switch(op){
			case 'u':
				/* username */
				break;
			case 'p':
				/* password */
				break;
			case 'q':
				/* quiet mode without notify */
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

void
twid_already_running(){
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
    else (pid != 0){
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
}

/*	
	Open a pipe, later open by fdopen() as a stream, registering as a callback
	stream for CURLOPT_WRITEDATA.
	
	void *callback is the callback function pointer which processes the HTML
	response after calling the Twitter API
*/
int
twid_open_pipe(void (*callback)(const char *)){
	int pipe_fd[2], pid;
	
	if (pipe(pipe_fd) < 0){
		/* Daemonized already, output error to log file */
	}
	
	if ((pid = fork()) < 0){
		/* Failed to fork */
	}
	else if (pid == 0){
		/* child process: open the pipe, waiting for reading while blocked */
		close(pipe_fd[1]);	/* close the writing fd */
		
		char response[MAXLEN];
		int n = read(pipe_fd[0], response, MAXLEN);
		
		callback(response);	/* callback */
		
		exit(0);	/* child process finished workload, exit here */
		
	}
	else{
		/* parent process here */
		close(pipe_fd[0]);
		
		return pipe_fd[1];
	}
}
