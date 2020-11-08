/* Project : Simple HTTP Client */

#include <limits.h> /* PATH_MAX */
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include <sys/stat.h>

#include "util.h"
#include "request.h"

typedef struct
{
  char *hostname;
  char *identifier;
  int port;
  int count;
} thread_param_t;

pthread_mutex_t read_byte_size_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t downloaded_file_mutex = PTHREAD_MUTEX_INITIALIZER;

// to print HTTP response into file
void printResponse(char *buffer, int rc, int sd, int base)
{

  // ...
  int size = rc - (strstr(buffer, "\r\n\r\n") - buffer + 4);

  // ...
  int remaining = CONTENT_LENGTH - size;

  pthread_mutex_lock(&read_byte_size_mutex);
  //printf("  %8s  %8s %8s\n", "So far", "bytes", "Remaining");

  //fseek base

  fseek(DOWNLOADED_FILE, base, SEEK_SET);
  fwrite(&buffer[strstr(buffer, "\r\n\r\n") - buffer + 4], 1, size, DOWNLOADED_FILE);
  pthread_mutex_unlock(&read_byte_size_mutex);

  rc = 0;

  do
  {
    pthread_mutex_lock(&read_byte_size_mutex);
    memset(buffer, 0x0, BUFFER_SIZE); //  init line
    if (remaining < BUFFER_SIZE)
    {
      rc = read(sd, buffer, remaining);
    }
    else
    {
      rc = read(sd, buffer, BUFFER_SIZE);
    }

    if (rc > 0)
    {
      // printf ("%s", buffer);
      fseek(DOWNLOADED_FILE, base + size, SEEK_SET);
      fwrite(buffer, 1, rc, DOWNLOADED_FILE);

      size += rc;
      remaining -= rc;
      printf("  %8d  %8d %8d\n", size, rc, remaining);
    }
    pthread_mutex_unlock(&read_byte_size_mutex);
    // printf("  %d  ",size);

  } while ((rc > 0) && (remaining > 0));

  printf("\n   Total recieved response bytes: %d\n", size);
}

// where to downloaded files stored
char *DOWNLOAD_PATH = "./downloads/";

void *thread_function(void *arg)
{

  thread_param_t *thread_param = (thread_param_t *)arg;
  //----------------------------------------------------

  int thread_size = CONTENT_LENGTH / THREAD_COUNT;
  if (thread_param->count == THREAD_COUNT - 1)
  {
    thread_size += CONTENT_LENGTH % THREAD_COUNT;
  }

  int rc;
  // open and connect socket
  int sd = create_socket(thread_param->hostname, thread_param->port);

  // send request to get info
  pthread_mutex_lock(&read_byte_size_mutex);

  // ...
  int base = READ_BYTE_SIZE;
  int limit = (REMAINING_BYTE_SIZE < thread_size) ? REMAINING_BYTE_SIZE : thread_size;
  range_request(thread_param->hostname, thread_param->identifier, base, limit, sd, thread_param->count);

  // ...
  printf("\n-- Thread[%d]", thread_param->count);
  printf(" downloading %d bytes\n", thread_size);

  // ...
  printf("---TOTAL READ BYTE SIZE :");
  READ_BYTE_SIZE += thread_size;
  clprint(READ_BYTE_SIZE, thread_size);

  // ...
  printf("---TOTAL REMAINING BYTE SIZE :");
  REMAINING_BYTE_SIZE -= thread_size;
  clprint(REMAINING_BYTE_SIZE, -thread_size);

  pthread_mutex_unlock(&read_byte_size_mutex);

  // display response
  char buffer[BUFFER_SIZE];
  memset(buffer, 0x0, BUFFER_SIZE); //  init line
  if ((rc = read(sd, buffer, BUFFER_SIZE)) < 0)
  {
    perror("read");
    exit(1);
  }
  printResponse(buffer, rc, sd, base);

  /*
  pthread_mutex_lock(&read_byte_size_mutex);
  if (REMAINING_BYTE_SIZE < THREAD_BUFFER_SIZE)
  {
    // ...
    int base = READ_BYTE_SIZE;
    int limit = (REMAINING_BYTE_SIZE < THREAD_BUFFER_SIZE) ? REMAINING_BYTE_SIZE : THREAD_BUFFER_SIZE;
    range_request(thread_param->hostname, thread_param->identifier, base, limit, sd, thread_param->count);
  }
  pthread_mutex_unlock(&read_byte_size_mutex);
*/

  pthread_exit(0);
}

int start_threads(char *hostname, char *identifier, int port)
{
  int cnt;
  pthread_t p_thread[THREAD_COUNT];

  for (cnt = 0; cnt < THREAD_COUNT; cnt++)
  {

    /* Returns 0 on successful creation of the thread.  The second
       parameter, left NULL for this example,  allows for options
       like CPU scheduling to be set. */

    thread_param_t *thread_param = malloc(sizeof(thread_param_t *));

    thread_param->hostname = hostname;
    thread_param->identifier = identifier;
    thread_param->port = port;
    thread_param->count = cnt;

    if (pthread_create(&p_thread[cnt], NULL, thread_function, (void *)thread_param))
    {
      fprintf(stderr, "Error creating the thread");
    }
  }

  // Cycle through each thread and wait until it is completed.
  for (cnt = 0; cnt < THREAD_COUNT; cnt++)
  {
    // Waits for p_thread[cnt] to finish.
    pthread_join(p_thread[cnt], NULL);
  }

  fprintf(stdout, "\nAll threads completed.\n");

  return 0;
}

/*-------------------------------------------------------*/

int main(int argc, char *argv[])
{
  char url[MAX_STR_LEN]; //="http://popeye.cs.iastate.edu:9798/index.html/";
  char hostname[MAX_STR_LEN];
  int port;
  char identifier[MAX_STR_LEN];
  char filename[MAX_STR_LEN];

  int rc, i;

  char line[MAX_MSG];
  int size;

  parse_command(argc, argv, url, hostname, &port, identifier, filename);

  // create direction if not exists
  struct stat st = {0};
  if (stat(DOWNLOAD_PATH, &st) == -1)
  {
    mkdir(DOWNLOAD_PATH, 0700);
  }

  // open file and write data inside
  DOWNLOAD_PATH = concat(DOWNLOAD_PATH, filename);

  DOWNLOADED_FILE = fopen(DOWNLOAD_PATH, "w+b");

  // open and connect socket
  int sd = create_socket(hostname, port);

  // send request to get info
  standart_request(hostname, identifier, sd);

  // display response
  char buffer[BUFFER_SIZE];
  memset(buffer, 0x0, BUFFER_SIZE); //  init line
  if ((rc = read(sd, buffer, BUFFER_SIZE)) < 0)
  {
    perror("read");
    exit(1);
  }

  // ...
  CONTENT_LENGTH = get_content_length(buffer);
  //  THREAD_BUFFER_SIZE = CONTENT_LENGTH / thread_count;

  READ_BYTE_SIZE = 0;
  REMAINING_BYTE_SIZE = CONTENT_LENGTH;

  // testing without threads
  //test(buffer, rc, sd);

  // starting threads...
  start_threads(hostname, identifier, port);

  fclose(DOWNLOADED_FILE);

  // return
  close(sd);

  char buf[PATH_MAX + 1]; /* not sure about the "+ 1" */
  char *res = realpath(DOWNLOAD_PATH, buf);
  if (res)
  {
    printf("You can find your file at :\n");
    printf("\n-- " ANSI_COLOR_YELLOW "%s\n\n" ANSI_COLOR_RESET, buf);
  }
  else
  {
    perror("realpath");
    exit(EXIT_FAILURE);
  }

  return SUCCESS;
}

/* Threading */
