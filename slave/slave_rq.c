/**
 * Slave Remote Query Handling
 */

#include "../rpc/gen/rq.h"
#include "../rpc/gen/rq_xdr.c"
#include "../rpc/gen/rq_svc.c"
#include "../rpc/gen/rq_clnt.c"

#include <stdio.h>
#include <stdlib.h>

int *rq_pipe_1_svc(struct rq_pipe query, struct svc_req *req)
{
    unsigned int id = query.vec_id;
    query_op op = query.op;
    struct rq_pipe *next = query.next;

    /*  TODO:
     *  Use consistent hashing instead of localhost.
     */

    int result;

    /* Recursive Query */
    if (next) {
        CLIENT *client = clnt_create("localhost",
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
    if      (op == OR)  result = result | id;
    else if (op == AND) result = result & id;
    else {
        printf("Error: Unknown Operator\n");
        result = EXIT_FAILURE;
    }

    return &result;
}

int *rq_root_1_svc(struct rq_root query, struct svc_req *req)
{
    return EXIT_SUCCESS;
}
