#include "glo.h"

/*---------------------------------------------------------------------------*
 * 
 * to concatanete two strings each other
 * 
 *---------------------------------------------------------------------------*/
char *concat(const char *s1, const char *s2)
{
  const size_t len1 = strlen(s1);
  const size_t len2 = strlen(s2);
  char *result = malloc(len1 + len2 + 1); //+1 for the null-terminator
  //in real code you would check for errors in malloc here
  memcpy(result, s1, len1);
  memcpy(result + len1, s2, len2 + 1); //+1 to copy the null-terminator
  return result;
}

/*---------------------------------------------------------------------------*
 * 
 * Parses an url into hostname, port number and resource identifier.
 *
 *---------------------------------------------------------------------------*/
void parse_URL(char *url, char *hostname, int *port, char *identifier, char *filename)
{
  char protocol[MAX_STR_LEN], scratch[MAX_STR_LEN], *ptr = 0, *nptr = 0;

  strcpy(scratch, url);
  ptr = (char *)strchr(scratch, ':');
  if (!ptr)
  {
    fprintf(stderr, "Wrong url: no protocol specified\n");
    exit(ERROR);
  }

  strcpy(ptr, "\0");
  strcpy(protocol, scratch);
  /* if (strcmp (protocol, "http"))
     {
     fprintf (stderr, "Wrong protocol: %s\n", protocol);
     exit (ERROR);
     }
   */
  strcpy(scratch, url);
  ptr = (char *)strstr(scratch, "//");
  if (!ptr)
  {
    fprintf(stderr, "Wrong url: no server specified\n");
    exit(ERROR);
  }
  ptr += 2;

  strcpy(hostname, ptr);
  nptr = (char *)strchr(ptr, ':');
  if (!nptr)
  {
    *port = 80; /* use the default HTTP port number */
    nptr = (char *)strchr(hostname, '/');
  }
  else
  {
    sscanf(nptr, ":%d", port);
    nptr = (char *)strchr(hostname, ':');
  }

  if (nptr)
    *nptr = '\0';

  nptr = (char *)strchr(ptr, '/');

  if (!nptr)
  {
    fprintf(stderr, "Wrong url: no file specified\n");
    exit(ERROR);
  }

  strcpy(identifier, nptr);

  ptr = strrchr(nptr, '/');
  ptr++;
  ptr = concat("file_", ptr);
  strcpy(filename, ptr);
}

// ..
void parse_command(int argc, char *argv[], char *url, char *hostname, int *port, char *identifier, char *filename)
{
  int i;
  int url_flag = 0, thread_flag = 0;

  for (i = 1; i < argc; i++)
  {

    if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-H") == 0))
    {
      printf("\n Usage: [-u client] [-t thread_count] \n\n");
      printf("\t -u source URL\n");
      printf("\t -t thread count\n\n");
      exit(SUCCESS);
    }
    else if ((strcmp(argv[i], "-u") == 0) || (strcmp(argv[i], "-U") == 0))
    {
      //  printf("\n--------------------\n%s\n--------\n",argv[i + 1]);
      strcpy(url, argv[i + 1]);
      url_flag = 1;
    }
    else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "-T") == 0))
    {
      THREAD_COUNT = atoi(argv[i + 1]);
      thread_flag = 1;
    }
  }

  if (!url_flag)
  {
    strcpy(url, "http://cse.akdeniz.edu.tr/wp-content/uploads/2017/03/Taner_Danisman.jpg");
  }
  if (!thread_flag)
  {
    THREAD_COUNT = 10;
  }

  // In client.c, you need to parse the <URL> given in the command line, connect to
  // the server, construct an HTTP request based on the options specified in the
  // command line, send the HTTP request to server, receive an HTTP response and
  // display the response on the screen.
  parse_URL(url, hostname, port, identifier, filename);

  printf("\n-- Hostname = %s , Port = %ls , Identifier = %s\n", hostname, port, identifier);
}

/* socket */

int create_socket(char *hostname, int port)
{

  int sd, rc;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;

  h = gethostbyname(hostname);
  if (h == NULL)
  {
    printf("unknown host: %s \n ", hostname);
    exit(ERROR);
  }

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(port); //(LOCAL_SERVER_PORT);

  // create socket
  //printf("-- Create socket...    ");
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0)
  {
    perror("cannot open socket ");
    exit(ERROR);
  }

  //  bind port number
  //printf("Bind port number...  ");

  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);

  rc = bind(sd, (struct sockaddr *)&localAddr, sizeof(localAddr));
  if (rc < 0)
  {
    printf("%s: cannot bind port TCP %u\n", "file", port); //(LOCAL_SERVER_PORT);
    perror("error ");
    exit(ERROR);
  }

  // connect to server
  //printf("Connect to server...\n");
  rc = connect(sd, (struct sockaddr *)&servAddr, sizeof(servAddr));
  if (rc < 0)
  {
    perror("cannot connect ");
    exit(ERROR);
  }

  return sd;
}

/* getting content length header */
int get_content_length(char *response)
{
  char tag[100];
  int filesize;
  strcpy(tag, "Content-Length: ");
  sprintf(tag, "%s", strstr(response, tag) + strlen(tag));
  filesize = atoi(tag);
  //printf("Size: %d\n", filesize);
  return filesize;
}

void clprint(int val, int diff)
{
  int oldVal = val - diff;
  char sign;
  if (diff < 0)
  {
    sign = '-';
    diff *= -1;
  }
  else
  {
    sign = '+';
  }
  printf(ANSI_COLOR_CYAN " %d" ANSI_COLOR_YELLOW " %c %d = " ANSI_COLOR_RED "%d\n" ANSI_COLOR_RESET, oldVal, sign, diff, val);
}

// counting digits of an int
int count_digits(int num)
{
  if (num < 0)
  {
    perror("invalid int value");
    exit(ERROR);
  }

  // counting digits
  int digits = 0;

  // ..
  do
    digits++;
  while ((num = num / 10) != 0);

  return digits;
}

// integer to string
char *itoa(int num)
{
  int size = (count_digits(num) + 1) * sizeof(char);
  char *result = malloc(size);
  snprintf(result, size, "%d", num);
  return result;
}
