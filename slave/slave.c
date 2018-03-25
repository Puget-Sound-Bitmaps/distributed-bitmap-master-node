/**
 * Slave Remote Query Handling
 */

#include "../rpc/gen/slave.h"
#include "../rpc/vote.h"

#include "../master/slavelist.h"

#include "../../bitmap-engine/BitmapEngine/src/seg-util/SegUtil.h"
#include "../../bitmap-engine/BitmapEngine/src/wah/WAHQuery.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

char *machine_failure_msg(char *);

query_result *get_vector(u_int vec_id)
{
    /* Turn vec_id into the filename "vec_id.dat" */

    char filename[16];
    snprintf(filename, 16, "v_%u.dat", vec_id);
    /* Necessary Variables */
    FILE *fp = NULL;
    u_int64_t *vector_val = NULL;
    u_int vector_len = 0;
    u_int exit_code = EXIT_SUCCESS;
    char *error_message = NULL;
    /* Open file in binary mode. */
    fp = fopen(filename, "r");
    if (fp == NULL) {
        exit_code = EXIT_FAILURE;
        error_message = (char *) malloc(sizeof(char) * 256);
        snprintf(error_message, 256, "Error: Failed to find vector %u\n",
            vec_id);
    }
    else {
        // XXX: should be empirically determined average vector length
        int num_elts = 4;
        vector_val = (u_int64_t *) malloc(sizeof(u_int64_t) * num_elts);
        char buf[32];
        while (fgets(buf, 32, fp) != NULL) {
            if (vector_len > num_elts) {
                num_elts *= 2;
                vector_val = (u_int64_t *) realloc(vector_val,
                    num_elts * sizeof(u_int64_t));
            }
            vector_val[vector_len++] = (u_int64_t) strtol(buf, NULL, 10);
        }
        fclose(fp);
    }
    query_result *res = (query_result *) malloc(sizeof(query_result));
    res->vector.vector_val = vector_val;
    res->vector.vector_len = vector_len;
    res->exit_code = exit_code;
    res->error_message = error_message;
    return res;
}

query_result *rq_pipe_1_svc(rq_pipe_args query, struct svc_req *req)
{
    query_result *this_result;
    query_result *next_result = NULL;

    u_int exit_code = EXIT_SUCCESS;
    this_result = get_vector(query.vec_id);
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
            /* give the request a time-to-live */
            struct timeval tv;
            tv.tv_sec = TIME_TO_VOTE;
            tv.tv_usec = 0;
            clnt_control(client, CLSET_TIMEOUT, &tv);
            if (next_result == NULL) {
                clnt_perror(client, "call failed:");
                this_result->exit_code = EXIT_FAILURE;
                this_result->error_message =
                    machine_failure_msg(query.machine_addr);
                return this_result;
            }

            clnt_destroy(client);
        }
    }

    /* Something went wrong with the recursive call. */
    if (next_result->exit_code != EXIT_SUCCESS) {
        this_result->exit_code = next_result->exit_code;
        this_result->error_message = next_result->error_message;
        return this_result;
    }

    /* Our final return values. */
    u_int64_t *result_val = (u_int64_t *)
        malloc(sizeof(u_int64_t) * this_result->vector.vector_len);
    u_int result_len = 0;

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
    free(this_result);
    free(next_result);
    return vector;
}

query_result **results;
typedef struct coord_thread_args {
    rq_pipe_args *args;
    int query_result_index;
} coord_thread_args;

void *init_coordinator_thread(void *coord_args) {
    coord_thread_args *args = (coord_thread_args *) coord_args;

    CLIENT *clnt = clnt_create(args->args->machine_addr,
        REMOTE_QUERY_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(args->args->machine_addr);
    }
    query_result *res = rq_pipe_1_svc(*(args->args), clnt);
    /* give the request a time-to-live */
    struct timeval tv;
    tv.tv_sec = TIME_TO_VOTE;
    tv.tv_usec = 0;
    clnt_control(clnt, CLSET_TIMEOUT, &tv);
    if (res == NULL) {
        clnt_perror(clnt, "call failed: ");
        /* Report that this machine failed */
        res = (query_result *) malloc(sizeof(query_result));
        res->exit_code = EXIT_FAILURE;
        res->error_message = machine_failure_msg(args->args->machine_addr);
    }
    results[args->query_result_index] = res;
    clnt_destroy(clnt);
    return (void *) 0;
}

query_result *
rq_range_root_1_svc(rq_range_root_args query, struct svc_req *req)
{
    int num_threads = query.num_ranges;
    unsigned int *range_array = query.range_array.range_array_val;
    pthread_t tids[num_threads];

    results = (query_result **) malloc(sizeof(query_result *) * num_threads);
    int i;
    int array_index = 0;
    for (i = 0; i < num_threads; i++) {
        int num_nodes = range_array[array_index++];
        rq_pipe_args *pipe_args = (rq_pipe_args *)
            malloc(sizeof(rq_pipe_args) * num_nodes);
        rq_pipe_args *head_args = pipe_args;
        int j;
        for (j = 0; j < num_nodes; j++) {
            pipe_args->machine_addr = SLAVE_ADDR[range_array[array_index++]];
            pipe_args->vec_id = range_array[array_index++];
            pipe_args->op = '|';
            if (j < num_nodes - 1) {
                rq_pipe_args *next_args = (rq_pipe_args *)
                    malloc(sizeof(rq_pipe_args) * num_nodes);
                pipe_args->next = next_args;
                pipe_args = next_args;
            }
            else {
                pipe_args->next = NULL;
            }
        }

        /* allocate the appropriate number of args */
        coord_thread_args *thread_args = (coord_thread_args *)
            malloc(sizeof(coord_thread_args));
        thread_args->query_result_index = i;
        thread_args->args = head_args;

        pthread_create(&tids[i], NULL, init_coordinator_thread,
            (void *) thread_args);
    }

    query_result *res = (query_result *) malloc(sizeof(query_result));

    /*
     * Conclude the query. Join each contributing thread,
     * and in doing so, report error if there is one, or report largest vector size
     */
    u_int64_t *result_vector = NULL;
    u_int result_vector_len = 0;
    u_int largest_vector_len = 0;
    for (i = 0; i < num_threads; i++) {
        pthread_join(tids[i], NULL);
        /* assuming a single point of failure, report on the failed slave */
        if (results[i]->exit_code != EXIT_SUCCESS) {
            return results[i];
        }
        largest_vector_len = (u_int) fmax((double) largest_vector_len,
            (double) results[i]->vector.vector_len);
    }
    /* all results found! */
    res->exit_code = EXIT_SUCCESS;
    res->error_message = "";
    if (num_threads == 1) { /* there are no vectors to AND together */
        res->vector = results[0]->vector;
        return res;
    }
    result_vector = (u_int64_t *)
        malloc(sizeof(u_int64_t) * largest_vector_len);

    /* AND the first 2 vectors together */
    result_vector_len = AND_WAH(result_vector,
        results[0]->vector.vector_val, results[0]->vector.vector_len,
        results[1]->vector.vector_val, results[1]->vector.vector_len);

    /* AND the subsequent vectors together */
    for (i = 2; i < num_threads; i++) {
        u_int64_t *new_result_vector = (u_int64_t *)
            malloc(sizeof(u_int64_t) * largest_vector_len);
        result_vector_len = AND_WAH(new_result_vector, result_vector,
            result_vector_len, results[i]->vector.vector_val,
            results[i]->vector.vector_len);
        free(result_vector);
        result_vector = new_result_vector;
    }
    res->vector.vector_val = result_vector;
    res->vector.vector_len = result_vector_len;
    return res;
}

#define TESTING_SLOW_PROC 0

int result;

int *commit_msg_1_svc(int message, struct svc_req *req)
{
	printf("SLAVE: VOTING\n");
    int ready = 1; // test value
    if (ready) {
        result = VOTE_COMMIT;
    }
    else {
        /* possible reasons: lack of memory, ... */
        result = VOTE_ABORT;
    }
    /* make the process run slow, to test failure */
    if (TESTING_SLOW_PROC) {
        sleep(TIME_TO_VOTE + 1);
    }
    return &result;

}

int *commit_vec_1_svc(struct commit_vec_args args, struct svc_req *req)
{
	printf("SLAVE: Putting vector %u\n", args.vec_id);
    FILE *fp;
    char filename_buf[128];
    snprintf(filename_buf, 128, "v_%d.dat", args.vec_id); // XXX: function to get vector filename
    fp = fopen(filename_buf, "wb");
    char buffer[128];
    int i;
    for (i = 0; i < args.vector.vector_len; i++) {
        snprintf(buffer, 128, "%llu", args.vector.vector_val[i]);
        fprintf(fp, "%s\n", buffer);
    }
    fclose(fp);
    result = EXIT_SUCCESS;
    return &result;
}

char *machine_failure_msg(char *machine_name) {
    char *error_message = (char *) malloc(sizeof(char) * 256);
    snprintf(error_message, 256,
        "Error: No response from machine %s\n", machine_name);
    return error_message;
}
