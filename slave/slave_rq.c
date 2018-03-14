/**
 * Slave Remote Query Handling
 */

#include "../rpc/gen/rq.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../master/slavelist.h"

// TODO Jahrme
query_result *rq_pipe_1_svc(rq_pipe_args *query, struct svc_req *req)
{
    // int result = 0;
    // unsigned int id = query.vec_id;
    // query_op op = query.op;
    // struct rq_pipe_args *next = query.next;
    //
    // int result;
    //
    // /* Recursive Query */
    // if (next) {
    //     CLIENT *client = clnt_create(next->machine_name,
    //         REMOTE_QUERY_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
    //     if (client) result = *rq_pipe_1(*next, client);
    //
    //     if (!client) {
    //         printf("Error: Could not connect to server.\n");
    //         result = EXIT_FAILURE;
    //         return &result;
    //     }
    //
    //     if (!result) {
    //         printf("Error: RPC Failure\n");
    //         result = EXIT_FAILURE;
    //         return &result;
    //     }
    // }
    //
    // else {
    //     result = 0;
    // }
    //
    // /*  TODO:
    //  *  Return the vector, not the vector's ID.
    //  *  Use WAH_OR and WAH_AND functions instead of bitwise.
    //  */
    // if      (op == '|')  result = result | id;
    // else if (op == '&') result = result & id;
    // else {
    //     printf("Error: Unknown Operator\n");
    //     result = EXIT_FAILURE;
    // }
    // //TO be done by Jahrme, not used
    query_result *res = (query_result *) malloc(sizeof(query_result));
    return res;
}

query_result **results;
typedef struct coord_thread_args {
    rq_pipe_args *args;
    int query_result_index;
} coord_thread_args;

void *init_coordinator_thread(void *coord_args) {
    // TODO: get a query result
    coord_thread_args *args = (coord_thread_args *) coord_args;

    // Testing Zone
    printf("In thread %d", args->query_result_index);
    rq_pipe_args *pipe_args = args->args;
    while (pipe_args != NULL) {
        printf("Machine: %s\n", pipe_args->machine_addr);
        pipe_args = pipe_args->next;
    }
    printf("\n");
    // END TEST

    CLIENT *clnt = clnt_create(args->args->machine_addr, REMOTE_QUERY_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
    query_result *res = rq_pipe_1_svc(args->args, clnt);
    if (res == NULL) {
        printf("Query to %s failed\n", args->args->machine_addr);
        return (void *) 1;
    }
    else
        results[args->query_result_index] = res;
    return (void *) 0;
}

// TODO Sam
query_result *rq_root_1_svc(rq_range_root_args *query, struct svc_req *req)
{
    // TODO: spawn a thread for each range
    int num_threads = query->num_ranges;
    unsigned int *range_array = query->range_array.range_array_val;
    pthread_t tids[num_threads];
    // the results to be anded together (in general, OP)
    results = (query_result **) malloc(sizeof(query_result *) * num_threads);
    int i;
    int array_index = 0;
    for (i = 0; i < num_threads; i++) {
        // coordinator arguments
        // TODO:
        int num_nodes = range_array[array_index];
        rq_pipe_args *pipe_args = (rq_pipe_args *) malloc(sizeof(rq_pipe_args) * num_nodes);
        int j;
        for (j = 0; j < num_nodes; j++) {
            pipe_args->machine_addr = SLAVE_ADDR[range_array[array_index++]];
            pipe_args->vec_id = range_array[array_index++];
            pipe_args->op = '|';
            if (j < num_nodes - 1) {
                rq_pipe_args *next_args = (rq_pipe_args *) malloc(sizeof(rq_pipe_args) * num_nodes);
                pipe_args->next = next_args;
                pipe_args = next_args;
            }
            else
                pipe_args->next = NULL;
        }

        coord_thread_args *thread_args = (coord_thread_args *) malloc(sizeof(coord_thread_args));
        thread_args->query_result_index = i;
        thread_args->args = pipe_args;

        // allocate the appropriate number of args
        pthread_create(&tids[i], NULL, init_coordinator_thread, (void *) thread_args);
    }
    // TODO AND all the results together and just print it out for now.
    query_result *res = (query_result *) malloc(sizeof(query_result));
    return res;
}
