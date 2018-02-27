/* 2PC server (slave) */

#include "../rpc/vote.h"
#include "../rpc/gen/tpc.h"
#include <stdio.h>
#include <stdlib.h>

#define TESTING_SLOW_PROC 0

int result;

int *commit_msg_1_svc(int *message, struct svc_req *req)
{
    int ready = 1; // test value
    if (ready) {
        result = VOTE_COMMIT;
    }
    else {
        /* possible reasons: lack of memory, ... */
        result = VOTE_ABORT;
    }
    /* make the process run slow, so that */
    if (TESTING_SLOW_PROC) {
        sleep(TIME_TO_VOTE + 1);
    }
    return &result;

}

int *commit_vec_1_svc(struct commit_vec_args *args, struct svc_req *req)
{
    //printf("Commiting vector #%d: %lu\n", args->vec_id, args->vec);
    FILE *fp;
    char filename_buf[128];
    snprintf(filename_buf, 128, "vector_%d.dat", args->vec_id);
    fp = fopen(filename_buf, "a");
    char buffer[128];
    int i;
    for (i = 0; i < args->vector_length; i++)
        snprintf(buffer, 128, "%lu", args->vec[i]);

    fprintf(fp, "%s\n", buffer);
    fclose(fp);
    result = 0;
    return &result;
}
