/**
 * Slave Remote Query Handling
 */

#include "../rpc/gen/rq.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../master/slavelist.h"

// TODO Jahrme
query_result *rq_pipe_1_svc(struct rq_pipe_args *query, struct svc_req *req)
{
    int result = 0;
    unsigned int id = query.vec_id;
    query_op op = query.op;
    struct rq_pipe_args *next = query.next;



    int result;

    /* Recursive Query */
    if (next) {
        CLIENT *client = clnt_create(next->machine_name,
            REMOTE_QUERY_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
        if (client) result = *rq_pipe_1(*next, client);

        if (!client) {
            printf("Error: Could not connect to server.\n");
            result = EXIT_FAILURE;
            return &result;
        }

        if (!result) {
            printf("Error: RPC Failure\n");
            result = EXIT_FAILURE;
            return &result;
        }
    }

    else {
        result = 0;
    }

    /*  TODO:
     *  Return the vector, not the vector's ID.
     *  Use WAH_OR and WAH_AND functions instead of bitwise.
     */
    if      (op == '|')  result = result | id;
    else if (op == '&') result = result & id;
    else {
        printf("Error: Unknown Operator\n");
        result = EXIT_FAILURE;
    }

    return &result;
}

// TODO Sam
int *rq_root_1_svc(rq_root_args *query, struct svc_req *req)
{
    // TODO: spawn a thread for each range

    // TODO:
    return &EXIT_SUCCESS;
}
