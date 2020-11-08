#include "glo.h"

/* request */

// sending request on socket
void send_request(char *request, int sd)
{
  // send request
  int rc = write(sd, request, strlen(request));
  if (rc < 0)
  {
    perror("cannot send data ");
    close(sd);
    exit(ERROR);
  }
}

// ...
void standart_request(char *hostname, char *identifier, int sd)
{

  char *request = 0; //[MAX_STR_LEN];
  char buf[MAX_STR_LEN];
  request = buf;

  strcpy(request, "GET ");
  request = (char *)strcat(request, identifier);
  request = (char *)strcat(request, " HTTP/1.1\r\nHOST: ");
  request = (char *)strcat(request, hostname);
  request = (char *)strcat(request, "\r\n");
  request = (char *)strcat(request, "\r\n");

  printf("\n-- Initial Request\n--\n%s\n--\n", request);
  send_request(request, sd);
}


void range_request(char *hostname, char *identifier, int base, int limit, int sd, int count)
{

  char *request = 0; //[MAX_STR_LEN];
  char buf[MAX_STR_LEN];
  request = buf;

  strcpy(request, "GET ");
  request = (char *)strcat(request, identifier);
  request = (char *)strcat(request, " HTTP/1.1\r\nHOST: ");
  request = (char *)strcat(request, hostname);
  request = (char *)strcat(request, "\r\n");

  char *low_s = itoa(base);
  int high = base + limit - 1;
  char *high_s = itoa(high);

  request = (char *)strcat(request, "Range: bytes=");
  request = (char *)strcat(request, low_s);
  request = (char *)strcat(request, "-");
  request = (char *)strcat(request, high_s);
  request = (char *)strcat(request, "\r\n");
  request = (char *)strcat(request, "\r\n");

  //printf("\n-- Thread[%d] Request\n--\n%s\n--\n", count, request);

  send_request(request, sd);
}
