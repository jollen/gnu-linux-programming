/*
   web.c,
   a dirty embedded Web server.
*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
#include "http.h"

#define CR '\r'
#define LF '\n'

#define MAX 4096

static int http_get;
static struct http_operations *http_ops[MAX_OPS];

/*
 * Global Pipes
 */
int fds[2];	

/*
 * Thread functions
 */
static void *http_main(int n);
static void *http_io(int n);

static int http_parser(char *s)
{
    if (s[0] == CR && s[1] == LF)
	return -1;
    else if (strncmp(s, "GET", 3) == 0)
	http_get = 1;
    return 0;
}

static void *http_main(int n)
{
	struct http_operations *ops;
	char buf[MAX];
	int ret;
	FILE *stream;
   	pthread_t thread_id2;

	ops = (struct http_operations *)http_data[n].fops;

	/* blocking open */
	printf("ops->open\n");
	if (ops == NULL)
	   printf("ops = NULL\n");
	if (ops->open) {
	   ops->open(ops);
	} else {
	   printf("ops->open = NULL\n");
	}
	printf("exit ops->open\n");

	/*
	 * Create pipes. Close input pipe. Read output pipe.
 	 */
	pipe(fds);

    while (1) {
	   printf("ops->read\n");
	   ret = ops->read(ops, buf);
	   printf("client: %s [%d]\n", buf, ret);
       http_parser(buf);
	   if (buf[0] == CR && buf[1] == LF)
	      break;
	}

	if (http_get) {
	    pthread_create(&thread_id2, NULL, &http_io, SOCKET_OPS);

		stream = fdopen(fds[1], "w");
		fprintf(stream, "HTTP/1.1 200 OK\n");
		fprintf(stream, "Content-Type: text/html\n");
		fprintf(stream, "\n");
		fprintf(stream, "<h1>hihi</h1>");
		fprintf(stream, "\n\n");
		fflush(stream);
	}

	printf("ops->close\n");
	ops->close(ops);

	return NULL;
}

static void *http_io(int n)
{
	struct http_operations *ops;
	char buffer[1024];
	FILE *stream;

	ops = (struct http_operations *)http_data[n].fops;

	stream = fdopen(fds[0], "r");

	while (!feof(stream) && !ferror(stream) &&
          fgets(buffer, sizeof(buffer), stream) != NULL) {
   		printf("[pipe_read ] %s\n", buffer);
		ops->write(ops, buffer, strlen(buffer));
	}
}

/* API implementation */
int http_register(struct http_operations *ops, int opsno)
{
   	pthread_t thread_id1;

	switch (opsno) {
		case SOCKET_OPS:
			   http_data[SOCKET_OPS].fops = ops;
			   pthread_create(&thread_id1, NULL, &http_main, SOCKET_OPS);
			break;
		case FILE_OPS:
			http_ops[FILE_OPS] = 
			       (struct http_operations *)ops;
			break;
		default:
			printf("error\n");
			return -1;
	}
	printf("exit http_register\n");

	return 0;
}
