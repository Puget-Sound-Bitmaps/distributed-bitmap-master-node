#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "dbms.h"

/**
 * Database Management System (DBMS)
 *
 * Imitate the interactions coming from a true DBMS.
 */
int main(int argc, char *argv[])
{
    bool master_exists = false;

    pid_t master_pid = -1;

    if (!master_exists) switch (master_pid = fork()) {
        case -1:
            printf("Master process creation failed.\n");
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            master_exists = true;
            printf("Master process creation succeeded.\n");
            char *argv[] = {MASTER_EXECUTABLE, REPLICATION_FACTOR};
            int master_exit_status = execv(MASTER_EXECUTABLE, argv);
            exit(master_exit_status);
        default:
            master_exists = true;
    }

    /* Do IPC */

    waitpid(master_pid, NULL, 0);
    printf("Master process reaping succeeded.\n");
    return EXIT_SUCCESS;
}
