/*
 *
 * Header
 *
 */

#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include "types.h"

#define BASE64TABLE "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
#define VALID_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

/* Move to const.h */
#define DEFAULT_SMTP_SERVER "smtp.del.ufrj.br"
#define DEFAULT_SMTP_SERVER_PORT 25

#define BUFFER_SIZE 1024

#define MAX_CHAR 1000
#define RANDOM_STRING_SIZE 16

#define CMD_HELO "HELO"
#define CMD_DATA "DATA"
#define CMD_RCPT "RCPT To:"
#define CMD_FROM "MAIL From:"

unsigned establishConnection (char *, unsigned short, boolean, int *);

unsigned sendData (int, byte *, unsigned long long, boolean);

unsigned receiveData (int, byte *, unsigned long long *, boolean);

void closeConnection (int, boolean);

unsigned base64encode (char *, char *);

unsigned base64decode (char *, char *);

unsigned createRandomString (char *, unsigned, char *);

unsigned mailFrom(int, const char *, boolean);

unsigned sendmail(char *, unsigned, const char *, const char *, const char *, const char *, const char *, const char *, const char *, boolean);

#endif
