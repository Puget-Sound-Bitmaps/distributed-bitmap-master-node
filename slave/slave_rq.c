/**
 * Slave Remote Query Handling
 */

#include "../rpc/gen/rq.h"
#include "../master/slavelist.h"

#include "../../bitmap-engine/BitmapEngine/src/seg-util/SegUtil.h"
#include "../../bitmap-engine/BitmapEngine/src/wah/WAHQuery.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

query_result get_vector(u_int vec_id) {
    /* Turn vec_id into the filename "vec_id.dat" */
    int number_size = (vec_id == 0 ? 1 : (int) (log10(vec_id) + 1));
    int filename_size = number_size + 4; /* ".dat" */
    char filename[filename_size];
    snprintf (filename, filename_size * sizeof(char), "%u.dat", vec_id);

    /* Necessary Variables */
    FILE *file_pointer = NULL;
    u_int64_t *vector_val = NULL;
    u_int vector_len = 0;
    u_int exit_code = EXIT_SUCCESS;

    /* Open file in binary mode. */
    file_pointer = fopen(filename, "rb");

    if (file_pointer == NULL) {
        exit_code = EXIT_FAILURE ^ vec_id;
    }
    else {
        /* Jump to end of file. */
        fseek(file_pointer, 0, SEEK_END);
        /* Get offset in the file (as byte). */
        vector_len = ftell(file_pointer) / 8;
        /* Return to start of file. */
        rewind(file_pointer);

        vector_val = (u_int64_t *) malloc((vector_len + 1) * sizeof(u_int64_t));
        fread(vector_val, vector_len, 1, file_pointer);
        fclose(file_pointer);
    }

    query_result vector = { {vector_len, vector_val}, exit_code };
    return vector;
}

query_result *rq_pipe_1_svc(rq_pipe_args query, struct svc_req *req)
{
    query_result *this_result = NULL;
    query_result *next_result = NULL;

    u_int exit_code = EXIT_SUCCESS;

    *this_result = get_vector(query.vec_id);

    /* Something went wrong with reading the vector. */
    if (this_result->exit_code != EXIT_SUCCESS) {
        return this_result;
    }

    /* We are the final call. */
    else if (query.next == NULL) {
        return this_result;
    }

    /* Recursive Query */
    else {
        char *host = query.machine_addr;

        CLIENT *client;
        client = clnt_create(host,
            REMOTE_QUERY_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");

        if (client == NULL) {
            clnt_pcreateerror(host);
            exit_code = EXIT_FAILURE;
        }
        else {
            next_result = rq_pipe_1_svc(*(query.next), client);

            if (next_result == NULL) {
                clnt_perror(client, "call failed:");
            }

            clnt_destroy(client);
        }
    }

    /* Something went wrong with the recursive call. */
    if (next_result->exit_code != EXIT_SUCCESS) {
        this_result->exit_code = next_result->exit_code;
        return this_result;
    }

    /* Our final return values. */
    query_result *result_val = NULL;
    u_int result_len = 0;

    /*  TODO:
     *  Return the vector, not the vector's ID.
     *  Use WAH_OR and WAH_AND functions instead of bitwise.
     */
    if (query.op == '|') {
        result_len = OR_WAH(result_val,
            this_result->vector.vector_val, this_result->vector.vector_len,
            next_result->vector.vector_val, next_result->vector.vector_len);
    }
    else if (query.op == '&') {
        result_len = AND_WAH(result_val,
            this_result->vector.vector_val, this_result->vector.vector_len,
            next_result->vector.vector_val, next_result->vector.vector_len);
    }
    else {
        printf("Error: Unknown Operator\n");
    }

    query_result *vector = (query_result *) malloc(sizeof(query_result));
    vector->vector.vector_len = result_len;
    vector->vector.vector_val = result_val;
    vector->exit_code = exit_code;

    return vector;
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
    query_result *res = rq_pipe_1_svc(*(args->args), clnt);
    if (res == NULL) {
        printf("Query to %s failed\n", args->args->machine_addr);
        return (void *) 1;
    }
    else
        results[args->query_result_index] = res;
    return (void *) 0;
}

// TODO Sam
query_result *rq_range_root_1_svc(rq_range_root_args query, struct svc_req *req)
{
    // TODO: spawn a thread for each range
    int num_threads = query.num_ranges;
    unsigned int *range_array = query.range_array.range_array_val;
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
