#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

#define TEXT_SIZE     128
#define	MQ_KEY	      13579

typedef struct msgbuf
{
    long    	     mtype;
    unsigned char    mtext[TEXT_SIZE];
};

int main()
{
    int mqid;
    struct msgbuf rcvbuffer;

    mqid = msgget(MQ_KEY, 0666);

    if (mqid < 0) {
        perror("failed to get message queue");
        exit(1);
    }

    // message type is 1
    if (msgrcv(mqid, &rcvbuffer, TEXT_SIZE, 1, 0) < 0) {
        perror("failed to receive message queue of type 1");
        exit(1);
    }

    printf("%s\n", rcvbuffer.mtext);

    return 0;
}
