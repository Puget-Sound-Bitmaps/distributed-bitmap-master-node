#include "dbms.h"
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
 * Database Management System (DBMS)
 *
 * Imitate the interactions coming from a true DBMS.
 */
int main(int argc, char *argv[])
{
    printf("Starting DBMS...\n");

    /* SPAWN MASTER */

    bool master_exists = false;
    pid_t master_pid = -1;
    if (!master_exists) switch (master_pid = fork()) {
        case -1:
            printf("Master process creation failed.\n");
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            master_exists = true;
            char *argv[] = {MASTER_EXECUTABLE, REPLICATION_FACTOR};
            int master_exit_status = execv(MASTER_EXECUTABLE, argv);
            exit(master_exit_status);
        default:
            master_exists = true;
    }

    /* SETUP IPC */

    /* Create message queue. */
    int msq_id = msgget(MSQ_KEY, MSQ_PERMISSIONS | IPC_CREAT);

    put_vector(msq_id, 1, 0x00ab);
    put_vector(msq_id, 2, 0x10c0);
    put_vector(msq_id, 3, 0x0781);
    put_vector(msq_id, 4, 0x99ff);

    /* CLEANING UP */

    /* Destroy message queue. */
    msgctl(msq_id, IPC_RMID, NULL);
    /* Reap master. */
    int master_result;
    waitpid(master_pid, &master_result, 0);
    printf("Master returned: %i\n", master_result);

    printf("Stopping DBMS...\n");
    return EXIT_SUCCESS;
}


int put_vector(int queue_id, int vec_id, unsigned long long vec)
{
    struct put_msgbuf put = {mtype_put, { vec_id, vec } };
    msgsnd(queue_id, &put, sizeof(struct put_msgbuf), 0);
    return EXIT_SUCCESS;
}

