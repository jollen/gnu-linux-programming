#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TEXT_SIZE     256
#define MQ_KEY	      13579

struct msgbuf
{
    long    	     mtype;
    unsigned char    mtext[TEXT_SIZE];
};

int main()
{
    int mqid;
    int msgflg = IPC_CREAT | 0666;
    struct msgbuf mbuf;
    size_t buflen;

    mqid = msgget(MQ_KEY, msgflg);

    if (mqid  < 0) {
      perror("failed to get message queue");
      exit(1);
    }

    // message type
    mbuf.mtype = 1;

    printf("Enter your message: ");
    scanf("%[^\n]", mbuf.mtext);
    getchar();

    buflen = strlen(mbuf.mtext) + 1 ;

    if (msgsnd(mqid, &mbuf, buflen, IPC_NOWAIT) < 0)
    {
        printf ("%d, %d, %s, %d\n", mqid, mbuf.mtype, mbuf.mtext, buflen);
        perror("msgsnd");
	exit(1);
    }

    printf("Message Sent\n");

    return 0;
}
