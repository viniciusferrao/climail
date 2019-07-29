/*
 *
 * Implementacao
 *
 */

#include <stdio.h>
#include <stdlib.h> /*srand(), rand()*/
#include <time.h> /*time()*/
#include <netdb.h> /*gethostbyname()*/
#include <arpa/inet.h> /*inet_ntoa()*/
#include <string.h> /*memset()*/
#include <unistd.h> /*close()*/
#include <errno.h> /*strerror()*/
#include <sys/types.h>
#include <sys/socket.h>

/*damnit BSD*/
#include <netinet/in.h>

#include "functions.h"
#include "errors.h"

unsigned establishConnection (char *name, unsigned short port, boolean verbose, int *socketDescriptor) {	
	
	int i;
	struct hostent *data;
	struct in_addr **addr_list; /*dont know why use this*/
	struct sockaddr_in socketAddr;
	
	if ((data = gethostbyname(name)) == NULL) {
		herror("gethostbyname");
		return FAILED_GETHOSTBYNAME;
	}
	
	if (verbose == true) {
		printf("Nome oficial do host: %s\n", data->h_name);
		printf("Enderecos IP: ");
		addr_list = (struct in_addr **)data->h_addr_list;
		for (i = 0; addr_list[i] != NULL; i++) {
			printf("%s ", inet_ntoa(*addr_list[i]));
		}
		printf("\n");
	}
	
	if ((*socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Falha ao abrir socket!\n\n");
		return FAILED_OPEN_SOCKET;
	}
	
	if (verbose == true) {
		printf("Socket aberto!\n\n");
	}
	
	memset(&socketAddr, 0, sizeof(socketAddr));
	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(port);
	socketAddr.sin_addr.s_addr = *(u_long*)data->h_addr;
	
	if ((connect(*socketDescriptor, (struct sockaddr*)&socketAddr, sizeof(socketAddr))) < 0) {
		perror("Erro na funcao connect().\n");
		return FAILED_CONNECT;
	}

	return OK;

}

unsigned sendData (int sock, byte *buffer, unsigned long long size, boolean verbose) {
	
	unsigned long long sent;	
	byte *aux = buffer;
	
	if (buffer == NULL) return NO_ARGS;
	
	while ((sent = send(sock, aux, size, 0))) {
		size -= sent;
		aux += sent;
	}

	if (verbose) printf(">> %s", buffer);

	return OK;
	
}

unsigned receiveData (int sock, byte *buffer, unsigned long long *size, boolean verbose) {
	
	if (buffer == NULL) return NO_ARGS;
	memset(buffer, '\0', *size);	

	*size = recv(sock, buffer, *size, 0);
	if (verbose) printf("<< %s", buffer);

	return OK;
	
}

void closeConnection (int sock, boolean verbose) {
	
	if (verbose == true) {
		printf("Tentando fechar socket... ");
	}
	
	if (close(sock) != OK) {
		printf("Falha: %s.\n",strerror(errno));
	} else if (verbose == true) {
		printf("OK!\n");
	}
	
}

unsigned base64encode (char *inputFile, char *outputFile) {

	FILE *input, *output;
	int read;
	int counter;
	byte ent[3];
	byte ind[4];

	if (!inputFile || !outputFile) {
		return NO_ARGS;
	}

	if (!(input = fopen(inputFile, "rb"))) {
		return ERROR_OPENING_INPUTFILE;
	}

	if (!(output = fopen(outputFile, "w"))) {
		return ERROR_OPENING_OUTPUTFILE;
	}

	counter = 0;
	while ((read = fread(ent, sizeof(byte), 3, input))) {
		if (read < 2) ent[1] = 0x00;
		if (read < 3) ent[2] = 0x00;
		
		ind[0] = (ent[0] >> 2) & 0x3F;
		ind[1] = ((ent[0] << 4) & 0x30) | ((ent[1] >> 4) & 0x0F);
		ind[2] = ((ent[1] << 2) & 0x3C) | ((ent[2] >> 6) & 0x03);
		ind[3] = ent[2] & 0x3F;

		fprintf(output, "%c%c%c%c", BASE64TABLE[ind[0]], BASE64TABLE[ind[1]], (read < 2) ? '=': BASE64TABLE[ind[2]], (read < 3) ? '=' : BASE64TABLE[ind[3]]);
		counter += 4;
		if (counter == 76) {
			fprintf(output, "\r\n");
			counter = 0;
		}
	}
	
	fclose (input);
	fclose (output);
	return OK;
	
}

unsigned base64decode (char *inputFile, char *outputFile) {
/*check error handling*/	
	FILE *input, *output;
	int read;
	int counter;
	byte ent[3];
	byte ind[4];

	if (!inputFile || !outputFile) {
		return NO_ARGS;
	}

	if (!(input = fopen(inputFile, "r"))) {
		return OK;
	}

	if (!(output = fopen(outputFile, "wb"))) {
		return OK;
	}

	counter = 0;
	while ((read = fread(ind, sizeof(byte), 4, input))) {
		ind[0] = strchr(BASE64TABLE, ind[0]) - BASE64TABLE;
		ind[1] = strchr(BASE64TABLE, ind[1]) - BASE64TABLE;
		ind[2] = (ind[2] == '=') ? read-- : strchr(BASE64TABLE, ind[2]) - BASE64TABLE;
		ind[3] = (ind[3] == '=') ? read-- : strchr(BASE64TABLE, ind[3]) - BASE64TABLE;

		ent[0] = ((ind[0] << 2) & 0xFC) | ((ind[1] >> 4) & 0x03);
		ent[1] = ((ind[1] << 4) & 0xF0) | ((ind[2] >> 2) & 0x0F);
		ent[2] = ((ind[2] << 6) & 0xC0) | (ind[3] & 0x3F);
		
		fwrite(ent, sizeof(byte), read - 1, output);
		
		if (read != 4) {
			break;
		}
		
		counter += read;
		if (counter == 76) {
			read = fread(ind, sizeof(byte), 1, input);
			counter = 0;
		}
	}

	fclose (input);
	fclose (output);
	return OK;

}

unsigned createRandomString (char *validChars, unsigned strLenght, char *strReturn) {

	unsigned counter;
	
	if (!validChars || !strReturn)
		return NO_ARGS;

	srand((unsigned)time(NULL));
	for (counter=0; counter<strLenght; counter++)
		strReturn[counter]=validChars[rand()%strlen(validChars)];
	strReturn[counter]='\0'; /*Screw you fucking end of string*/
	return OK;

}

/* Stupid Function; this should be inside sendmail() */
unsigned mailFrom (int socketDescriptor, const char *emailFrom, boolean verbose) {
	
	byte buffer[BUFFER_SIZE];
	unsigned long long transferBytes;
	unsigned errorlevel;
	
	memset(buffer, 0, BUFFER_SIZE);
	snprintf((char *)buffer, BUFFER_SIZE, "%s %s\r\n", CMD_FROM, emailFrom);
	if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char*)buffer), verbose))) 
		return errorlevel;
		
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0, BUFFER_SIZE);
	if ((errorlevel = receiveData(socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;
	
	if (buffer[0]!='2') {
		if (verbose) 
			printf("Resposta inválida do servidor!\n");
		return ERROR_SMTP_FROM;
	}

	return OK;

}

unsigned sendRcpt (int socketDescriptor, const char *emailTo, boolean verbose) {
	
	byte buffer[BUFFER_SIZE];
	unsigned long long transferBytes;
	unsigned errorlevel;
	
	memset(buffer, 0x00, BUFFER_SIZE);	
	snprintf((char *)buffer, BUFFER_SIZE, "%s %s\r\n", CMD_RCPT, emailTo);
	
	if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose)))
		return errorlevel;

	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;

	if (buffer[0]!='2') {
		if (verbose) 
			printf("Resposta inválida do servidor!\n");
		return ERROR_SMTP_TO;
	}
	
	return OK;
	
}

unsigned sendmail (char *ip, unsigned port, const char *emailFrom, const char *emailTo, const char *emailSubject, const char *emailBcc, const char *emailCc, const char *emailBody, const char *emailAttach, boolean verbose) {
	
	unsigned errorlevel;
	int socketDescriptor;
	unsigned long long transferBytes;
	unsigned bytesRead=0;
	byte buffer[BUFFER_SIZE];
	char emailToAux[MAX_CHAR];
	char emailCcAux[MAX_CHAR];
	char emailBccAux[MAX_CHAR];
	char emailAttachAux[MAX_CHAR];
	char emailToData[MAX_CHAR];
	char emailCcData[MAX_CHAR];
	char randomString[RANDOM_STRING_SIZE+1];
	char encodedFile[RANDOM_STRING_SIZE+1];
	FILE *pFile;
	
	/*Lets try to reach the smtp server*/
	if ((errorlevel = establishConnection(ip, port, verbose, &socketDescriptor))) {
		return errorlevel;
	}
		
	/*Receive data from server*/
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose))) {
		return errorlevel;
	}
	if (buffer[0]!='2') {
		printf("Unhandled Exception\n");
		return ERROR_SMTP_SERVER;
	}
	
	/*Let's try to send a hello to server*/
	memset(buffer, 0x00, BUFFER_SIZE);
	snprintf((char *)buffer, BUFFER_SIZE, "%s %s\r\n", CMD_HELO, ip);

	if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose)))
		return errorlevel;
	
	/*Getting response from hello command*/	
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;
	if (buffer[0]!='2') {
		printf("Unhandled Exception\n");
		return ERROR_SMTP_HELO;
	}
		
	/*Sending the mail from command to server*/
	if ((errorlevel = mailFrom(socketDescriptor, emailFrom, verbose)))
		return errorlevel;

	/*Try to send recipient command to smpt server for emailTo*/
	strcpy(emailToAux, emailTo);
	strcpy(emailToData, emailTo);
	emailTo = strtok(emailToAux, ","); /*strtok has been deprecated; reimplement this*/
	while (emailTo != NULL) {
		if ((errorlevel = sendRcpt(socketDescriptor, emailTo, verbose)))
			return errorlevel;
		emailTo = strtok (NULL, ",");
	}

	/*Try to send a new recipient command to smtp server for emailCc*/
	if (strcmp(emailCc,"") != 0) {
		strcpy(emailCcAux, emailCc);
		strcpy(emailCcData, emailCc);
		emailCc = strtok(emailCcAux, ","); /*strtok has been deprecated; reimplement this*/
		while (emailCc != NULL) {
			if ((errorlevel = sendRcpt(socketDescriptor, emailCc, verbose)))
				return errorlevel;
			if (verbose)
				printf("emailCc: %s\n",emailCc);
			emailCc = strtok (NULL, ",");
		}
	}

	/*Finally try to send a new^2 recipient command to smtp server for emailCCo*/
	if (strcmp(emailBcc,"") != 0) {
		strcpy(emailBccAux, emailBcc);
		emailBcc = strtok(emailBccAux, ",");
		while (emailBcc != NULL) {
			if ((errorlevel = sendRcpt(socketDescriptor, emailBcc, verbose)))
				return errorlevel;
			if (verbose) 
				printf("emailBcc: %s\n",emailBcc);
			emailBcc = strtok (NULL, ",");
		}
	}
	
	/*Let's put some data over here to play with the smtp server*/
	memset(buffer, 0x00, BUFFER_SIZE);
	snprintf((char *)buffer, BUFFER_SIZE, "%s\r\n", CMD_DATA);
		
	if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose)))
		return errorlevel;
	
	transferBytes = BUFFER_SIZE;
	memset(buffer, 0x00, BUFFER_SIZE);
	if ((errorlevel = receiveData(socketDescriptor, buffer, &transferBytes, verbose)))
		return errorlevel;
	if (buffer[0]!='3') {
		return ERROR_SMTP_DATA;
		printf("Unhandled Exception\n");
	}
		
	/*Try to send body command without attachments*/
	if (strcmp(emailAttach,"") == 0) {
		memset(buffer, 0x00, BUFFER_SIZE);
		sprintf((char *)buffer, "From: %s\r\nTo: %s\r\nCc: %s\r\nSubject: %s\r\nContent-type: text/plain\r\n\r\n%s\r\n.\r\n", emailFrom, emailToData, emailCcData, emailSubject, emailBody);
	
		if (verbose)
			printf("Enviando eMail sem anexos:\n%s\n", buffer);
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) 
			return errorlevel;
		
		transferBytes = BUFFER_SIZE;
		memset(buffer, 0x00, BUFFER_SIZE);
		if ((errorlevel = receiveData (socketDescriptor, buffer, &transferBytes, verbose)))
			return errorlevel;
		if (buffer[0]!='2') {
			printf("Unhandled Exception\n");
			return ERROR_SMTP_BODY;
		}
		closeConnection(socketDescriptor, verbose);	
		return errorlevel;
	} else {
		memset(buffer, 0x00, BUFFER_SIZE);
		sprintf((char *)buffer, "From: %s\r\nTo: %s\r\nCc: %s\r\nSubject: %s\r\n", emailFrom, emailToData, emailCcData, emailSubject);
	
		if (verbose)
			printf("Enviando eMail com anexos:\n%s\n", buffer);
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose))) 
			return errorlevel;
		
		createRandomString(VALID_CHARS, RANDOM_STRING_SIZE, randomString);
		memset(buffer, 0x00, BUFFER_SIZE);
		sprintf((char *)buffer, "content-type: Multipart/Mixed; boundary=\"%s\"\r\n", randomString);
		if (verbose)
			printf("%s",buffer);
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose)))
			return errorlevel;
		
		/*Try to send body command with attachment*/
		memset(buffer, 0x00, BUFFER_SIZE);
		sprintf((char *)buffer,  "--%s\ncontent-type: text/plain\n\n %s\r\n", randomString,emailBody);
		if (verbose)
			printf("%s",buffer);
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose)))
			return errorlevel;

		/*The attachment*/
		strcpy(emailAttachAux,emailAttach);
		emailAttach = strtok (emailAttachAux, ",");

		while (emailAttach != NULL) {
			memset(buffer, 0x00, BUFFER_SIZE);
			sprintf((char *)buffer, "--%s\ncontent-type: Application/octet-stream; name=\"%s\"\n", randomString, emailAttach);
			strcat((char *)buffer, "Content-Transfer-Encoding: base64\n\n");

			if (verbose)
				printf("%s",buffer);
			if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char*)buffer), verbose)))
				return errorlevel;

			createRandomString(VALID_CHARS, RANDOM_STRING_SIZE, encodedFile);
			base64encode((char *)emailAttach, (char *)encodedFile);

    		if ((pFile = fopen(encodedFile, "rb")) == NULL)
        		return ERROR_OPENING_FILE;

    		while ((bytesRead = fread(buffer, 1, 1, pFile))) {
				if ((errorlevel = sendData(socketDescriptor, buffer, bytesRead, verbose))) {
					fclose(pFile);
					return errorlevel;
				}
			}

			fclose(pFile);
			remove(encodedFile);
			strcpy((char *)buffer, "\r\n");
			
			if (verbose)
				printf("%s",buffer);
			if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char*)buffer), verbose)))
				return errorlevel;
			if (verbose)
				printf ("emailAttach: %s\n", emailAttach);
			emailAttach = strtok(NULL, ","); /*remember to change this to strsep*/
		}
		
		memset(buffer, 0x00, BUFFER_SIZE);
		sprintf((char *)buffer, "--%s--\r\n\r\n.\r\n", randomString);
		if (verbose)
			printf("%s",buffer);
		if ((errorlevel = sendData(socketDescriptor, buffer, strlen((char *)buffer), verbose)))
			return errorlevel;
		if (verbose)
			printf("Enviado.\n");
		
		transferBytes = BUFFER_SIZE;
		memset(buffer, 0x00, BUFFER_SIZE);
		if ((errorlevel = receiveData(socketDescriptor, buffer, &transferBytes, verbose)))
			return errorlevel;
		if (buffer[0]!='2') {
			printf("Unhandled Exception!\n");
			return ERROR_SMTP_BODY;
		}
		closeConnection(socketDescriptor, verbose);
		return errorlevel;
	}
	
	/*I will never get here :(*/
		
}
