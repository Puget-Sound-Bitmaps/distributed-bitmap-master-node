/**
 * Slave Remote Query Handling
 */

// #include "slave_rq.h"
#include "../rpc/gen/rq.h"
// #include "../rpc/gen/rq_xdr.c"
// #include "../rpc/gen/rq_svc.c"

#include <stdio.h>
#include <stdlib.h>

int result;

int *rq_pipe_1_svc(struct rq_pipe query, struct svc_req *req)
{
    /* TODO:
     * Return the vector, not the vector's ID.
     */
    unsigned int id = query.vec_id;
    query_op op = query.op;
    struct rq_pipe *next = query.next;

    if (next == NULL) return id;

    /* Build the client. */
    CLIENT *client = clnt_create("127.0.1.1", RQ_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
    if (client == NULL) {
        printf("Error: Could not connect to server.\n");
        return EXIT_FAILURE;
    }

    /* Send the query. */
    int *result = rq_pipe_1(*next, client);
    if (result == NULL) {
        printf("Error: RPC Failure\n");
        return EXIT_FAILURE;
    }

    if (op == AND) return id & *result;
    else if (op == OR) return id | *result;
    else {
        printf("Error: Unknown Operator\n");
        return EXIT_FAILURE;
    }
}

int *rq_root_1_svc(struct rq_root query, struct svc_req *req)
{
    return EXIT_SUCCESS;
}
