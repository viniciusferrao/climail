/*
 * 
 * Programa de socket
 *
 */

#include <stdio.h>
#include <string.h> /* strlen(); */
#include <stdlib.h> /* malloc(); */
#include <getopt.h> /* opterr */

#include "errors.h"
#include "functions.h"

#define VERBOSE true

int main (int argc, char **argv) {

	int option;
	int indexOptions = 0;
	static char shortOptions[] = "hSpftcbsmav";
	char server[BUFFER_SIZE];
	int port = 0;
	unsigned errorlevel = 0;
	char emailFrom[BUFFER_SIZE];
	char emailTo[BUFFER_SIZE];
	char emailSubject[BUFFER_SIZE];
	char emailBody[BUFFER_SIZE];
	char emailBcc[BUFFER_SIZE];
	char emailCc[BUFFER_SIZE];
	char emailAttach[BUFFER_SIZE];
	boolean verbose=false;
	opterr = 0;

	static struct option longOptions[]={
		{"help",0,0,'h'},
		{"server",0,0,'S'},
		{"port",0,0,'p'},
		{"from",0,0,'f'},
		{"to",0,0,'t'},
		{"cc",0,0,'c'},
		{"bcc",0,0,'b'},
		{"subject",0,0,'s'},
		{"message",0,0,'m'},
		{"attach",0,0,'a'},
		{"verbose",0,0,'v'},
		{0,0,0,0},
	};
	
	while ((option = getopt_long(argc, argv, shortOptions, longOptions, &indexOptions)) != -1)
		switch(option) {
			case 'h': {
				printf("Usage: %s [-%s]\n", argv[0], shortOptions);
				printf("\n");
				printf("       -h --help    | Display this help\n");
				printf("       -S --server  | Server address\n");
				printf("       -p --port    | Server port\n");
				printf("       -f --from    | eMail address source\n");
				printf("       -t --to      | eMail address destination\n");
				printf("       -c --cc      | eMail address to send copy\n");
				printf("       -b --bcc     | eMail address to send hidden copy\n");
				printf("       -s --subject | Message subject\n");
				printf("       -m --message | Message field\n");
				printf("       -a --attach  | Attachment file\n");
				printf("       -v --verbose | Verbosity for pseudo-nerds\n");
				printf("\n");
				printf("Example: %s -S smtp.address.com -f mail@domain.com -t destiny@isp.com\n", argv[0]);
				break;
			}
			
			case 'S': {
				if (argv[optind])
					strcpy(server,argv[optind]);
				break;
			}
			
			case 'p': {
				if (argv[optind])
					port=atoi(argv[optind]);
				break;
			}	
			
			case 'f': {	
				if (argv[optind])
					strcpy(emailFrom,argv[optind]);
				break;
			}
			
			case 't': {
				if (argv[optind])
					strcpy(emailTo,argv[optind]);
				break;
			}
			
			case 'c': {
				if (argv[optind])
					strcpy(emailCc,argv[optind]);
				break;
			}

			case 'b': {
				if (argv[optind])
					strcpy(emailBcc,argv[optind]);
				break;
			}
			
			case 's': {
				if (argv[optind])
					strcpy(emailSubject,argv[optind]);
				break;
			}
			
			case 'm': {
				if (argv[optind])
					strcpy(emailBody,argv[optind]);
				break;
			}
			
			case 'a': {
				if (argv[optind])
					strcpy(emailAttach,argv[optind]);
				break;
			}
			
			case 'v': {
				verbose = true;
				break;
			}
			
			default:
				printf("Invalid option %s\n",argv[optind-1]);
		}

		/*Bad error treatment, shit!!!*/
		
		if (argc == 1) 
			printf("Type %s -h for more information.\n",argv[0]);
			
		if (server == NULL) strcpy(server, DEFAULT_SMTP_SERVER); /*set default server*/
		if (port == 0) port = DEFAULT_SMTP_SERVER_PORT; /*set default port*/
		
		if (argc > 2) {
			printf("\n");
			if ((errorlevel = sendmail(server, port, emailFrom, emailTo, emailSubject, emailBcc, emailCc, emailBody, emailAttach, verbose)) != OK)
				printf("Failed to send mail; check %s -h for help.\n",argv[0]);
				printf("Exited with errorcode: %i\n",errorlevel);
		}
	
	return OK;

}
