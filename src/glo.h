#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
 

#define LOCAL_SERVER_PORT 9798
#define MAX_MSG 100
#define END_LINE 0x0A

#define SUCCESS 0
#define ERROR 1

#define MAX_STR_LEN 512
#define BUFFER_SIZE 10000

// size of the file
int CONTENT_LENGTH;

// total amount of bytes read at the momoent
int READ_BYTE_SIZE; // = 0

// remaining bytes will be read by threads
int REMAINING_BYTE_SIZE; // = CONTENT_LENGTH

// file descriptor for downloaded file
FILE *DOWNLOADED_FILE;


//
int THREAD_COUNT;
