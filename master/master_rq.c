/**
 * Master Remote Query Handling
 */

#include "master_rq.h"
#include "../rpc/gen/rq.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    hande_query("P:1");
    hande_query("P:11");
    hande_query("P:33");
    hande_query("P:1154");
    hande_query("R:[1,2]&[3,4]");

    return 0;
}

int hande_query(char *query_string)
{
    if (query_string[0] == 'P') {
        printf("Point Query: %s\n", query_string);
        handle_point_query(query_string);
    }
    else if (query_string[0] == 'R') {
        printf("Range Query: %s\n", query_string);
        handle_range_query(query_string);
    }
    else {
        printf("Unknown query type.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int handle_point_query(char *query_string)
{
    /* Strip "P:" from start of string. */
    size_t query_length = strlen(query_string);
    char *vec_id_str = (char *) malloc((query_length - 2) * sizeof(char));
    memcpy(vec_id_str, &query_string[2], query_length - 1);

    /* Parse as vector identifier. */
    unsigned long vec_id = strtol(vec_id_str, NULL, 10);
    printf("Vector id: %lu\n", vec_id);

    /* Build the query. */
    struct rq_pipe *query = malloc(sizeof(rq_pipe));
    query->vec_id = vec_id;
    query->op = OR;
    query->next = NULL;

    /* Build the client. */
    CLIENT *client = clnt_create("127.0.1.1", RQ_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
    if (client == NULL) {
        printf("Error, could not connect to server.\n");
        return EXIT_FAILURE;
    }

    /* Send the query. */
    int *result = rq_pipe_1(*query, client);
    if (result == NULL) {
        printf("Error: RPC failed.\n");
        return EXIT_FAILURE;
    }

    /* Return the result. */
    return *result;
}

int handle_range_query(char *query_string)
{
    return EXIT_SUCCESS;
}
