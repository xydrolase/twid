#include "twi.h"

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
		
	if (argc < 2)
	{
		printf("Usage: [OPTIONS] %s <your tweet>\n", argv[0]);
		return 0;
	}
	else
	{
		
		/* process for publishing tweet only */
		twid_op = twid_getopt_cli(argc, argv);
		if (optind == argc - 1){
			tweet_raw_bytes = argv[optind];
		}

		if (tweet_raw_bytes){
			tweet_post_body = (char *)twid_normalize_tweet(tweet_raw_bytes);
		}
	}
	
	/* Operational twi process */
	if (tweet_post_body){
		twid_op |= TWID_OP_NEW_TWEET;
	}
	
	/* Connect daemon process via UNIX socket */
	int clifd = twid_cli_init(TWID_CLI_SOCKET_PATH, TWID_SERV_SOCKET_PATH);
	if (clifd < 0){
		fprintf(stderr, "Your tweet will not be published.\n");
		exit(0);
	}
	
	if (twid_cli_request(clifd, twid_op, tweet_post_body) < 0){
		fprintf(stderr, "Failed to communicate with twid server.\n");
		exit(0);
	}
	
	return 0;
}
