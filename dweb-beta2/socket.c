/*
   web.c,
   a dirty embedded Web server.
*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
#include "socket.h"
#include "http.h"

#define CR '\r'
#define LF '\n'

#define MAX 4096

/* supporting functions */
int socket_listen(struct http_operations *ops)
{
    struct socket_data *priv= (struct socket_data *)ops->priv;
    int sockfd = priv->sockfd;
    int client_sockfd;
    struct sockaddr_in client_addr;
    pid_t self;
    int len = priv->len;

    /* Start Web server. */
  repeat:
    listen(sockfd, 25);

    client_sockfd = accept(sockfd, &client_addr, &len);
#ifdef __USE_FORK__
    /* fork a child to handle client's request */
    self = fork();
    if (self != 0)
	goto repeat;
#else
    pthread_t thread_id1;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (client_sockfd == -1) {
        perror("accept:");
        goto err;
    }

    // private data
    priv->client_sockfd = client_sockfd;

    // HTTP main thread
    pthread_create(&thread_id1, &attr, http_response, &ops);
#endif

err:
    goto repeat;

// NOT REACH
}

/* callback functions */
int http_write(struct http_operations *ops, char *buf)
{
    struct socket_data *priv = (struct socket_data *)ops->priv;
    int fd = priv->client_sockfd;
    int n;

    n = strlen(buf);
    printf("[Responsing: %d] %s\n", n, buf);

    write(fd, buf, n);
}

int http_read(struct http_operations *ops, char *netread)
{
    struct socket_data *priv = (struct socket_data *)ops->priv;
    int fd;

    int len;
    char readch;
    ssize_t n;

    fd = priv->client_sockfd;

    len = 0;
    netread[0] = CR;
    netread[1] = LF;

    while (len < MAX) {
	n = read(fd, &readch, 1);
	if (n <= 0)
	    break;

	netread[len++] = readch;
	if (readch == LF)
	    break;
    }

    netread[len] = '\0';

    return len;
}

int http_open(struct http_operations *ops)
{
    int sockfd;
    int len;
    struct sockaddr_in server_addr;
    struct socket_data *priv = ops->priv;

    printf("in ops->open\n");
    /* 1. Create a socket. */
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
	perror("socket:");
	return -1;
    }

    /* 2. Bind an address to the socket. */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, &server_addr, sizeof(struct sockaddr_in)) < 0) {
	perror("bind:");
	return -1;
    }
    len = sizeof(struct sockaddr_in);

    priv->sockfd = sockfd;
    priv->len = len;

    printf("socket:socket_listen\n");
    socket_listen(ops);
}

void http_close(struct http_operations *ops)
{
    struct socket_data *priv = (struct socket_data *)ops->priv;
    int client_sockfd = priv->client_sockfd;
    int sockfd =  priv->sockfd;

    shutdown(client_sockfd, SHUT_RDWR);
    close(client_sockfd);
#ifdef __USE_FORK__
    // close the local socket
    close(sockfd);

    // terminate the child process
    exit(0);
#endif
}

struct http_operations http_ops =
{
	open:	http_open,
	read:	http_read,
	write:	http_write,
	close:	http_close,
};

#ifdef __USE_FORK__
void sigchld_handler()
{
   int status;
   printf("Receive SIGCHLD signal.\n");

   wait(&status);
}
#endif

int main(int argc, char *argv[])
{
#ifdef __USE_FORK__
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigchld_handler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);
#endif
    pthread_t tid;

    http_ops.priv = (struct socket_data *)malloc(sizeof(struct socket_data));  
    http_register(&http_ops, SOCKET_OPS, &tid);

    pthread_join(tid, NULL);
}

