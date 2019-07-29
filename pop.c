/*
 * 
 * Popclient.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "errors.h"
#include "functions.h"
#include "types.h"

/*fuck you strcasestr*/
#define _GNU_SOURCE

#define CMD_USERNAME "USER"
#define CMD_PASSWORD "PASS"
#define CMD_LISTMAIL "LIST"
#define CMD_RETRIEVE "RETR"
#define CMD_DELETE "DELE"
#define CMD_QUIT "QUIT"

void discardstdin (void) {
	char c;
	while((c = getchar()) != '\n' && c != EOF);
}

/* This functions is not working as expected */
/*unsigned sendQuitCommand (boolean verbose) {
	
	unsigned errorlevel;
	int socketDescriptor;
	byte buffer[BUFFER_SIZE];
	unsigned long long transferBytes;
	
	*//*Sending the QUIT Command*//*
	memset(buffer, 0x00, BUFFER_SIZE);
	snprintf((char *)buffer, BUFFER_SIZE, "%s\r\n", CMD_QUIT);

	if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) {
		return errorlevel;
	}
	
	*//*Getting responde from QUIT command*//*
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;
	if (buffer[0]!='+') {
		printf("Unhandled Exception\n");
		return -1;
	}
	
	return OK;
	
}*/

int main (int argc, char **argv) {
	
	boolean verbose = false;
	unsigned errorlevel;
	
	char ip[BUFFER_SIZE];
	char username[BUFFER_SIZE];
	char password[BUFFER_SIZE];
	
	unsigned port=110;
	
	/*Control variables*/
	boolean list = false;
	boolean retrieve = false;
	int messageToRetrieve = 0;
	FILE *tempAttachFile;
	
	/*Retrieve HELL!!!!!!!!!*/
	int caseOf;
	char *auxiliary;
	char boundary[BUFFER_SIZE+1];
	char filename[BUFFER_SIZE+1];
	char *headers[] = {
		"From: ",
		"To: ",
		"Date: ",
		"Subject",
		""
	};
	unsigned counter;
	char attachBuffer[BUFFER_SIZE+1];
	unsigned getch;
	FILE *attachment;
	boolean saveAttachment = false;
	
	/*Delete variables*/
	boolean delete = false;
	int messageToDelete = 0;
	
	/*Responses*/
	int socketDescriptor;
	unsigned long long transferBytes;
	byte buffer[BUFFER_SIZE];
	
	/*Getopt variables*/
	int option;
	int indexOptions = 0;
	static char shortOptions[] = "hSPuplrdv";
	char server[BUFFER_SIZE];
	opterr = 0;

	static struct option longOptions[]={
		{"help",0,0,'h'},
		{"server",0,0,'S'},
		{"port",0,0,'P'},
		{"username",0,0,'u'},
		{"password",0,0,'p'},
		{"list",0,0,'l'},
		{"retrieve",0,0,'r'},
		{"delete",0,0,'d'},
		{"verbose",0,0,'v'},
		{0,0,0,0},
	};
	
	/*Program start*/
	if (argc == 1) {
		printf("Type %s -h for more information.\n",argv[0]);
		return -1;
	}
	
	while ((option = getopt_long(argc, argv, shortOptions, longOptions, &indexOptions)) != -1)
		switch(option) {
			case 'h': {
				printf("Usage: %s [-%s]\n", argv[0], shortOptions);
				printf("\n");
				printf("       -h --help     | Display this help\n");
				printf("       -S --server   | Server address\n");
				printf("       -P --port     | Server port\n");
				printf("       -u --username | Username to login\n");
				printf("       -p --password | Password to validade\n");
				printf("       -l --list     | List emails from server\n");
				printf("       -r --retrieve | Retrieve selected emails\n");
				printf("       -d --delete   | Delete selected message\n");
				printf("       -v --verbose  | Verbosity for pseudo-nerds\n");
				printf("\n");
				printf("Example: %s -S pop.address.com -u username -p plain-password -l\n", argv[0]);
				exit(OK);
				break;
			}
			
			case 'S': {
				if (argv[optind])
					strcpy(ip,argv[optind]);
				break;
			}
			
			case 'P': {
				if (argv[optind])
					port=atoi(argv[optind]);
				break;
			}	
			
			case 'u': {	
				if (argv[optind])
					strcpy(username,argv[optind]);
				break;
			}
			
			case 'p': {
				if (argv[optind])
					strcpy(password,argv[optind]);
				break;
			}
			
			case 'l': {
				list = true;
				break;
			}

			case 'r': {
				retrieve = true;
				if (argv[optind] == NULL) {
					printf("Must request a message number to proceed!\n");
					return -1;
				}
				messageToRetrieve = atoi(argv[optind]);
				break;
			}
			
			case 'd': {
				delete = true;
				if (argv[optind] == NULL) {
					printf("Must request a message number to proceed!\n");
					return -1;
				}
				messageToDelete = atoi(argv[optind]);
				break;
			}
			
			case 'v': {
				verbose = true;
				break;
			}
			
			default:
				printf("Invalid option %s\n",argv[optind-1]);
		}
	
	/*Control section, to check if the user entered server, login, password*/	
	if (strlen(ip) == 0) {
		printf("No specified server address.\n");
		return -1; /*Change this*/
	}
	
	if (!port) {
		port = 110; /*Change this shit too*/
	}
	
	if (strlen(username) == 0) {
		printf("No specified username.\n");
		return -1;
	}
	
	if (strlen(password) == 0) {
		printf("No specified password to login.\n");
		return -1;
	}
	
	/*Since we have settings, let's connect to pop server*/
	printf("Trying to connect... ");
	if ((errorlevel = establishConnection(ip, port, verbose, &socketDescriptor))) {
		return errorlevel;
	}

	/*Receive data from server*/
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;
	if (buffer[0]!='+') {
		printf("Unhandled Exception\n");
		return -1;
	}

	/*Let's try to send the username to server*/
	memset(buffer, 0x00, BUFFER_SIZE);
	snprintf((char *)buffer, BUFFER_SIZE, "%s %s\r\n", CMD_USERNAME, username);

	if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) {
		return errorlevel;
	}
	
	/*Getting response from username command*/	
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;
	if (buffer[0]!='+') {
		printf("Unhandled Exception\n");
		return -1;
	}
	
	/*Let's try to send the password to server*/
	memset(buffer, 0x00, BUFFER_SIZE);
	snprintf((char *)buffer, BUFFER_SIZE, "%s %s\r\n", CMD_PASSWORD, password);

	if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) {
		return errorlevel;
	}
	
	/*Getting response from password command*/	
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;
	if (buffer[0]!='+') {
		printf("Unhandled Exception\n");
		return -1;
	}
	printf("OK!\n");
	
	/*If list is asked then here we go*/
	if (list) {	
		printf("Listing messages on server:\n");
		
		/*Let's list messages from server*/
		memset(buffer, 0x00, BUFFER_SIZE);
		snprintf((char *)buffer, BUFFER_SIZE, "%s\r\n", CMD_LISTMAIL);
	
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) {
			return errorlevel;
		}
		
		/*Getting response from list command*/	
		transferBytes = BUFFER_SIZE;
		memset(buffer, 0x00, BUFFER_SIZE);
		if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
			return errorlevel;
		if (buffer[0]!='+') {
			printf("Unhandled Exception\n");
			return -1;
		}
		printf("%s",buffer); /*Poor listing mode*/
		
		/*Send quit command to be cool with the pop server*/
		//return (sendQuitCommand(verbose));
	
	}
	
	/*If retrieve command was asked let's search for messages*/
	if (retrieve) {
		printf("Retrieving message %i from server:",messageToRetrieve);
		
		/*Let's get some messages*/
		memset(buffer, 0x00, BUFFER_SIZE);
		snprintf((char *)buffer, BUFFER_SIZE, "%s %i\r\n", CMD_RETRIEVE, messageToRetrieve);
	
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) {
			return errorlevel;
		}
	
		/*Getting response from retrieve command*/	
		transferBytes = BUFFER_SIZE;
		memset(buffer, 0x00, BUFFER_SIZE);
		tempAttachFile = fopen("attach.tmp", "w");
		do {
			if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
				return errorlevel;
			fprintf(tempAttachFile, "%s", buffer);
		} while (transferBytes == BUFFER_SIZE);
		fclose(tempAttachFile);
		
		tempAttachFile = fopen("attach.tmp", "r");
		memset(attachBuffer, 0x00, BUFFER_SIZE);
		memset(boundary, 0x00, BUFFER_SIZE);
		memset(filename, 0x00, BUFFER_SIZE);
				
		/* Bacalho do Alberto para ler os mails!!!!!! */
		caseOf = 1;
		while (caseOf && (fgets(attachBuffer, BUFFER_SIZE, tempAttachFile) != NULL)) {
			switch (caseOf) {
				case 1:
					if (attachBuffer == strstr(attachBuffer, "content-type: ")) {
						auxiliary = strstr(attachBuffer, "boundary");
						if (auxiliary == NULL) 
							break;
						auxiliary = strchr(auxiliary, '=');
						if (auxiliary == NULL) 
							break;
						while (auxiliary[0] == '=' || auxiliary[0] == ' ' || auxiliary[0] == '"') {
							auxiliary++;
						}
						strncpy(boundary, auxiliary, BUFFER_SIZE);
						counter = strlen(boundary);
						while (counter && (boundary[counter-1] == '\n' || boundary[counter-1] == '\r' || boundary[counter-1] == ' ' || boundary[counter-1] == '"')) {
							boundary[counter-1] = '\0';
							counter = strlen(boundary);
						}

						snprintf(attachBuffer, BUFFER_SIZE, "--%s", boundary);
						strncpy(boundary, attachBuffer, BUFFER_SIZE);

						if (verbose) {
							printf("Boundary: %s\n", boundary);
						}
					}

					counter = 0;
					while (strlen(headers[counter])) {
						if (attachBuffer == strstr(attachBuffer, headers[counter])) {
							printf("%s", attachBuffer);
						}
						counter++;
					}

					if (attachBuffer[0] == '\r' || attachBuffer[0] == '\n') {
						printf("\n");
						caseOf = (strlen(boundary)) ? 3 : 2;
					}
					break;

				case 2:
					if (attachBuffer != strstr(attachBuffer, ".\r\n")) {
						printf("%s", attachBuffer);
					}
					break;

				case 3:
					if (attachBuffer == strstr(attachBuffer, boundary)) {
						if (verbose) {
							printf("Boundary found.\n");
						}

						memset(filename, 0x00, BUFFER_SIZE);
						caseOf = 4;
					}
					break;

				case 4:
					if (attachBuffer == strstr(attachBuffer, "content-type: ")) {
						auxiliary = strstr(attachBuffer, "name");
						if (auxiliary == NULL) 
							break;

						auxiliary = strchr(auxiliary, '=');
						
						if (auxiliary == NULL) 
							break;

						while (auxiliary[0] == '=' || auxiliary[0] == ' ' || auxiliary[0] == '"') {
							auxiliary++;
						}
						strncpy(filename, auxiliary, BUFFER_SIZE);

						counter = strlen(filename);
						while (counter && (filename[counter-1] == '\n' || filename[counter-1] == '\r' || filename[counter-1] == ' ' || filename[counter-1] == '"')) {
							filename[counter-1] = '\0';
							counter = strlen(filename);
						}

						if (verbose) {
							printf("Filename: %s\n", filename);
						}
					}

					if (attachBuffer[0] == '\r' || attachBuffer[0] == '\n') {
						if (verbose) {
							printf("Terminei de verificar os headers de um multipart.\n");
						}
						caseOf = (strlen(filename)) ? 5 : 6;
					}			
					break;

				case 5:
					saveAttachment = false;

					printf("Do you want to dump attachment %s and save? [Y/N]: ", filename);
					getch = getchar();
					discardstdin();

					if (getch == 'y' || getch == 'Y') {
						saveAttachment = true;

						if (verbose) {
							printf("Saving file: %s.\n", filename);
						}

						attachment = fopen("attach.tmp", "w");
						if (attachment == NULL) {
							saveAttachment = false;
						}

						fprintf(attachment, "%s", attachBuffer);
					}

					caseOf = 7;

					break;

				case 6: 
					if (attachBuffer != strstr(attachBuffer, boundary)) {
						printf("%s", attachBuffer);
					} else {
						if (verbose) {
							printf("Boundary found.\n");
						}
						memset(filename, 0x00, BUFFER_SIZE);
						caseOf = 4;
					}
					break;

				case 7:
					if (attachBuffer != strstr(attachBuffer, boundary)) {
						if (saveAttachment) {
							if (verbose) 
								printf("Saving!\n");
							fprintf(attachment, "%s", attachBuffer);
						}
					} else {
						if (verbose) {
							printf("Boundary Found.\n");
						}

						if (saveAttachment) {
							fclose(attachment);
							base64decode("attach.tmp", filename);
							remove("attach.tmp");
						}

						memset(filename, 0x00, BUFFER_SIZE);
						caseOf = 4;
					}

					break;
			}
		}
		
		/*Send quit command to be cool with the pop server*/
		//return (sendQuitCommand(verbose));
		
	}
	
	/*Delete messages from server*/ /*Not working correctly*/
	if (delete) {
		printf("Deleting message %i from server... ", messageToDelete);
		
		/*Let's select message to deletion*/
		memset(buffer, 0x00, BUFFER_SIZE);
		snprintf((char *)buffer, BUFFER_SIZE, "%s %i\r\n", CMD_DELETE, messageToDelete);
	
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) {
			return errorlevel;
		}
	
		/*Getting response from delete command*/	
		transferBytes = BUFFER_SIZE;
		memset(buffer, 0x00, BUFFER_SIZE);
		if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
			return errorlevel;
		if (buffer[0]!='+') {
			printf("Unhandled Exception\n");
			return -1;
		}
		printf("OK!\n");
		
		/*Send quit command to be cool with the pop server*/
		
		memset(buffer, 0x00, BUFFER_SIZE);
		snprintf((char *)buffer, BUFFER_SIZE, "%s\r\n", CMD_QUIT);
	
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) {
			return errorlevel;
		}
		
		//sendQuitCommand(verbose);
		
	}
	
	return OK;
}
