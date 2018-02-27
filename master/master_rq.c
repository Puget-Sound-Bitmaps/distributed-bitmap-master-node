/**
 * Master Remote Query Handling
 */

#include "master_rq.h"
#include "../rpc/gen/rq.h"
#include "../rpc/gen/rq_xdr.c"
#include "../rpc/gen/rq_clnt.c"
#include "slavelist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// For testing.
// int main()
// {
//     int result;
//
//     result = hande_query("P:1");
//     printf("P:1;           RPC Response: %i\n", result);
//
//     result = hande_query("P:2");
//     printf("P:2;           RPC Response: %i\n", result);
//
//     result = hande_query("P:3");
//     printf("P:3;           RPC Response: %i\n", result);
//
//     result = hande_query("P:4");
//     printf("P:4;           RPC Response: %i\n", result);
//
//     result = hande_query("R:[1,2]&[3,4]");
//     printf("R:[1,2]&[3,4]; RPC Response: %i\n", result);
//
//     return 0;
// }
//
// /**
//  * Handles a query from the DBMS given as a string.
//  */
// int hande_query(char *query_string)
// {
//     int result;
//     if (query_string[0] == 'P') {
//         result = handle_point_query(query_string);
//     }
//     else if (query_string[0] == 'R') {
//         result = handle_range_query(query_string);
//     }
//     else {
//         printf("Error: Unknown query type.\n");
//         return EXIT_FAILURE;
//     }
//
//     return result;
// }
//
// /**
//  * Handles a point-query from the DBMS given as a string.
//  *
//  * Input is of the form "P:42"; output would be vector 42.
//  */
// int handle_point_query(char *query_string)
// {
//     /* Strip "P:" from start of string. */
//     size_t query_length = strlen(query_string);
//     char *vec_id_str = (char *) malloc((query_length - 2) * sizeof(char));
//     memcpy(vec_id_str, &query_string[2], query_length - 2);
//
//     /* Parse as vector identifier. */
//     unsigned long vec_id = strtol(vec_id_str, NULL, 10);
//
//     /* Build the query. */
//     struct rq_pipe *query = malloc(sizeof(rq_pipe));
//     query->vec_id = vec_id;
//     query->op = OR;
//     query->next = NULL;
//
//     /* Send the Query */
//
//     CLIENT *client;
//     int *result;
//
//     /*  TODO:
//      *  Use consistent hashing instead of localhost.
//      */
//     client = clnt_create("localhost",
//         REMOTE_QUERY_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
//     if (client) result = rq_pipe_1(*query, client);
//
//     if (!client) {
//         printf("Error: Could not connect to server.\n");
//         result = EXIT_FAILURE;
//     }
//     if (!result) {
//         printf("Error: RPC failed.\n");
//         result = EXIT_FAILURE;
//     }
//
//     free(vec_id_str);
//     free(query);
//
//     /* Return the result. */
//     return *result;
// }


int init_range_query(unsigned int *range_array, int num_ranges, char *ops, int array_len)
{
    rq_root_args *root = (rq_root_args *) malloc(sizeof(rq_root_args));
    root->range_array.range_array_val = range_array;
    root->range_array.range_array_len = array_len;
    root->num_ranges = num_ranges;
    root->ops.ops_val = ops;
    root->ops.ops_len = num_ranges - 1;
    char *coordinator = SLAVE_ADDR[0]; // arbitrary for now
    CLIENT *cl = clnt_create(coordinator, REMOTE_QUERY_ROOT,
        REMOTE_QUERY_ROOT_V1, "tcp");
    if (cl == NULL) {
        printf("Error: could not connect to coordinator %s.\n", coordinator);
    }
    //int root = 0;
    rq_root_1(root, cl);

    free(root);
}
