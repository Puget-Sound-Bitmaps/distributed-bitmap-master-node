#include "dbms.h"
#include "../ipc/messages.h"
#include "../types/types.h"

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
    printf("Ready to fork...\n");
    if (!master_exists) switch (master_pid = fork()) {
        case -1:
            printf("Master process creation failed.\n");
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            printf("Becomming master...\n");
            char *argv[3] = {MASTER_EXECUTABLE, REPLICATION_FACTOR};
            int master_exit_status = execv(MASTER_EXECUTABLE, argv);
            exit(master_exit_status);
        default:
            printf("Resuming DBMS role...\n");
            master_exists = true;
    }

    /* SETUP IPC */

    /* Create message queue. */
    int msq_id = msgget(MSQ_KEY, MSQ_PERMISSIONS | IPC_CREAT);

    printf("Ready to send vectors...\n");
    // put_vector(msq_id, 1, 0x00ab);
    // put_vector(msq_id, 2, 0x10c0);
    // put_vector(msq_id, 3, 0x0781);
    // put_vector(msq_id, 4, 0x99ff);

    int master_result = 0;
    wait(&master_result);

    /* CLEANING UP */

    /* Destroy message queue. */
    msgctl(msq_id, IPC_RMID, NULL);
    /* Reap master. */

    printf("Master returned: %i\n", master_result);

    printf("Stopping DBMS...\n");
    return EXIT_SUCCESS;
}

int put_vector(int queue_id, vec_id_t vec_id, vec_t vec)
{
    msgbuf *put = (msgbuf *) malloc(sizeof(msgbuf));
    put->mtype = mtype_put;
    put->vector = (assigned_vector) { vec_id, vec };
    msgsnd(queue_id, put, sizeof(msgbuf), 0);
    return EXIT_SUCCESS;
}

int point_query(int queue_id, char *query_str)
{
    msgbuf *point = (msgbuf *) malloc(sizeof(msgbuf));
    point->mtype = mtype_point_query;
    point->point_vec_id = (vec_id_t) atoi(strtok(query_str, "P:"));
    msgsnd(queue_id, point, sizeof(point), 0);
    return EXIT_SUCCESS;
}

/*
 * for now, assume that all interbin ops are &
 * Format: "R:[b1,b2]<op1>[b3,b4]<op2>..."
 * where op is & or |
 */
int range_query(int queue_id, char *query_str)
{
    msgbuf *range = (msgbuf *) malloc(sizeof(msgbuf));
    range->mtype = mtype_range_query;
    // first pass: figure out how many ranges are in the query, to figure out
    // how much memory to allocate
    int i;
    unsigned int num_ranges = 0;
    i = 0;
    while (query_str[i] != '\0') {
        if (query_str[i++] == ',') num_ranges++;
    }
    static char delim[] = "[,]";
    char *token = strtok(query_str, "R:");
    token = strtok(token, delim); // grab first element
    char *ops = (char *) malloc(sizeof(char) * num_ranges);
    /* Allocate 2D array of range pointers */
    vec_id_t **ranges = (vec_id_t **) malloc(sizeof(vec_id_t *) * num_ranges);
    /* Allocate range arrays. */
    for (i = 0; i < num_ranges; i++) {
        ranges[i] = (vec_id_t *) malloc(sizeof(vec_id_t *) * 2);
    }

    /* parse out ranges and operators */
    num_ranges = 0;
    int r1, r2;
    while (token != NULL) {
        r1 = atoi(token);
        token = strtok(NULL, delim);
        r2 = atoi(token);
        vec_id_t *bounds = (vec_id_t *) malloc(sizeof(vec_id_t) * 2);
        bounds[0] = r1;
        bounds[1] = r2;
        ranges[num_ranges] = bounds;
        token = strtok(NULL, delim);
        if (token != NULL)
            ops[num_ranges] = token[0];
        else {
            ops[num_ranges] = 0;
            ranges[num_ranges + 1] = NULL;
            break;
        }
    }
    range_query_contents *contents = (range_query_contents *)
        malloc(sizeof(range_query_contents));
    contents->ranges = ranges;
    contents->ops = ops;
    contents->num_ranges = num_ranges;
    range->range = contents;
    msgsnd(queue_id, contents, sizeof(range), 0);
    return EXIT_SUCCESS;
}
