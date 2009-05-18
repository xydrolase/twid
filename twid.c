#include "twid.h"

/* 
	[twid]
		a flexible twitter service on *nix powered by command lines :p
		
	Author : killkeeper
	killkeeper AT gmail DOT com
	http://tremblefrog.org/
 */


int
main(int argc, char *argv[])
{
	int twid_is_daemon = 0;
	
	/* twid_getopt */
	
	char *tweet_post_body = 0, *tweet_escaped = 0;
	char *tweet_raw_bytes = 0;
	int twid_op = 0;
	
	/* twid_twitter_user */
	twid_twitter_user = (TWITTER_USER)malloc(sizeof(struct twitter_user));
	twid_twitter_user->username = NULL;
	twid_twitter_user->password = NULL;
	twid_twitter_user->sessionid = NULL;
		
	g_type_init ();
	
	if (argc < 1)
	{
		printf("Usage: [OPTIONS] %s <your tweet>\n", argv[0]);
		return 0;
	}
	else
	{
		if (1){
			/* first instance, register to be the daemon process */
			
			/* getopt */
			twid_getopt(argc, argv, twid_twitter_user);
			
			if (!twid_twitter_user->username || 
				!(*twid_twitter_user->username)){
				
				twid_query_username(twid_twitter_user);
				
				if (!twid_twitter_user->password || 
					!(*twid_twitter_user->password)){
					twid_query_password(twid_twitter_user);
				}
			}
			
			twid_concat_auth_body(twid_twitter_user);
			
			if (optind == argc - 1){
				/* the last argument, which is the tweet */
				tweet_raw_bytes = argv[optind];
			}
			else if (optind < argc - 1){
				fprintf(stderr, "Too many arguments.\n");
				exit(0);
			}
			
			/* regex */
			if (!twid_compile_regex(REGEX_SESSION, PATTERN_SESSION)){
				/* error process */
				exit(0);
			}
			
			/* login in */
			if (!twid_twitter_authenticate()){
				fprintf(stderr, 
						"Failed to authenticate your account: %s.\n",
						twid_twitter_user->username
				);
				exit(0);
			}
			
			fprintf(stdout, "%s, welcome to twid and Twitter :)\n",
				twid_twitter_user->username);
			
			/* all preparation work done, and we've logon onto Twitter
			service, now daemonize to be a daemon process */
			
			/* daemonize */
			twid_daemonize();
			twid_is_daemon = 1;
			
			/* register handler for SIGCHLD */
			signal(SIGCHLD, twid_sig_chld);
			
		}
		
		if (tweet_raw_bytes){
			tweet_post_body = (char *)twid_normalize_tweet(tweet_raw_bytes);
		}
	}
	
	if (twid_is_daemon){
		/* lock again */
		if (twid_instance_exists()){
			syslog(LOG_ERR, "Twid is already running.");
			exit(0);
		}
		
		/* This is the main daemon process */
		int listenfd = twid_serv_listen(TWID_SERV_SOCKET_PATH);
		if (listenfd < 0){
			syslog(LOG_ERR, "Failed to listen to requests, exiting now.");
			exit(0);
		}
		
		/* Do we have a tweet to be published ? */
		if (tweet_post_body){
			/* publish */
			twid_twitter_new_tweet(tweet_post_body);
		}
		
		/* loop, waiting for requests */
		for(;;)
			twid_serv_accept(listenfd);
			
		/* NOTE: we are not implementing the socket in a ASYNC mode now
		since the anticipated requests are considerably small, which indicates 
		that the processes being forked would rarely contribute to the 
		declination of performance.
		
		We might later rewrite this part in ASYNC :)
		*/
		
	}
	
	return 0;
}
