#include "master.h"
#include "messages.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/types.h>

/**
 * Master Process
 *
 * Coordinates/delegates tasks for the system.
 */
int main(int argc, char *argv[])
{
    /* Connect to message queue. */
    int msq_id = msgget(MSQ_KEY, MSQ_PERMISSIONS | IPC_CREAT);

    /* Hold next thing in queue. */
    struct put_msgbuf request;

    struct msqid_ds buf;
    int rc, num_messages;
    rc = msgctl(msq_id, IPC_STAT, &buf);
    num_messages = buf.msg_qnum;

    bool stop = false;

    FILE *fp;

    fp = fopen("hello-world-test.txt", "a");
    fprintf(fp, "Hello, World!\n");
    fclose(fp);


    fp = fopen("master_node_record.txt", "a");

    while (num_messages == 0) {
        while (num_messages > 0) {
            /* Grab from queue. */
            msgrcv(msq_id, &request, sizeof(struct put_msgbuf), mtype_put, 0);

            if (request.mtype == mtype_quit) {
                stop = true;
                break;
            }

            int vec_id = request.vector.vec_id;
            unsigned long long vec = request.vector.vec;

            fprintf(fp, "%i := %llx\n", vec_id, vec);

            rc = msgctl(msq_id, IPC_STAT, &buf);
            num_messages = buf.msg_qnum;
        }

        if (stop) break;

        rc = msgctl(msq_id, IPC_STAT, &buf);
        num_messages = buf.msg_qnum;
    }

    fclose(fp);

    //  while (queue is empty)
    //      while (queue is not empty)
    //          pop message from queue
    //          2-phase commit agreement w/ slaves
    //          if all agree, make RPC calls w/ message args
    return EXIT_SUCCESS;
}
