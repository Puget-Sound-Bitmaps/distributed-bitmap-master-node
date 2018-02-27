/**
 * Master Remote Query Handling
 */

#include "master_rq.h"
#include "../rpc/gen/rq.h"
#include "../rpc/gen/rq_xdr.c"
#include "../rpc/gen/rq_clnt.c"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// For testing.
int main()
{
    int result;

    result = hande_query("P:1");
    printf("P:1;           RPC Response: %i\n", result);

    result = hande_query("P:2");
    printf("P:2;           RPC Response: %i\n", result);

    result = hande_query("P:3");
    printf("P:3;           RPC Response: %i\n", result);

    result = hande_query("P:4");
    printf("P:4;           RPC Response: %i\n", result);

    result = hande_query("R:[1,2]&[3,4]");
    printf("R:[1,2]&[3,4]; RPC Response: %i\n", result);

    return 0;
}

/**
 * Handles a query from the DBMS given as a string.
 */
int hande_query(char *query_string)
{
    int result;
    if (query_string[0] == 'P') {
        result = handle_point_query(query_string);
    }
    else if (query_string[0] == 'R') {
        result = handle_range_query(query_string);
    }
    else {
        printf("Error: Unknown query type.\n");
        return EXIT_FAILURE;
    }

    return result;
}

/**
 * Handles a point-query from the DBMS given as a string.
 *
 * Input is of the form "P:42"; output would be vector 42.
 */
int handle_point_query(char *query_string)
{
    /* Strip "P:" from start of string. */
    size_t query_length = strlen(query_string);
    char *vec_id_str = (char *) malloc((query_length - 2) * sizeof(char));
    memcpy(vec_id_str, &query_string[2], query_length - 2);

    /* Parse as vector identifier. */
    unsigned long vec_id = strtol(vec_id_str, NULL, 10);

    /* Build the query. */
    struct rq_pipe *query = malloc(sizeof(rq_pipe));
    query->vec_id = vec_id;
    query->op = OR;
    query->next = NULL;

    /* Send the Query */

    CLIENT *client;
    int *result;

    /*  TODO:
     *  Use consistent hashing instead of localhost.
     */
    client = clnt_create("localhost",
        REMOTE_QUERY_PIPE, REMOTE_QUERY_PIPE_V1, "tcp");
    if (client) result = rq_pipe_1(*query, client);

    if (!client) {
        printf("Error: Could not connect to server.\n");
        result = EXIT_FAILURE;
    }
    if (!result) {
        printf("Error: RPC failed.\n");
        result = EXIT_FAILURE;
    }

    free(vec_id_str);
    free(query);

    /* Return the result. */
    return *result;
}

/**
 * Handles a range-query from the DBMS given as a string.
 *
 * Input is of the form "P:[1,2]&[3,4]"; output would be vector 42.
 */
int handle_range_query(char *request)
{
    return EXIT_SUCCESS;
   // 
   //  /* Strip "R:" from start of string. */
   //  size_t request_length = strlen(request);
   //  char *query_string = (char *) malloc((request_length - 2) * sizeof(char));
   //  memcpy(query_string, &request[2], request_length - 1);
   //  size_t query_length = strlen(query_string);
   //
   //  /********************
   //   * Build the query. *
   //   ********************/
   //
   //  /* Count up OPs. */
   //  int and_count = 0, or_count = 0;
   //
   //  int i;
   //
   //  for (i = 0; i < query_length; ++i) {
   //      if (query_string[i] == '&') ++and_count;
   //      else if (query_string[i] == '|') ++or_count;
   //      else continue;
   //  }
   //
   //  int op_count = and_count + or_count;
   //
   //  query_op *ops = malloc(op_count * sizeof(query_op));
   //
   //  int ops_pos = 0;
   //  for (i = 0; i < query_length; ++i) {
   //      if (query_string[i] == '&') {
   //          ops[ops_pos] = AND;
   //          ++ops_pos;
   //      }
   //      else if (query_string[i] == '|') {
   //          ops[ops_pos] = OR;
   //          ++ops_pos;
   //      }
   //      else continue;
   //  }
   //
   //  /* TODO:
   //   * Split bit query into array of ranges
   //   * Convert each range into a `rq_pipe` query
   //   *
   //
   //
   // /* Split Query into ranges */
   //
   //
   //
   //
   //
   //
   //
   //
   //
   //  // struct rq_pipe *pipes[op_count + 1] = malloc((op_count + 1) * sizeof(rq_pipe));
   //  struct rq_root *query = malloc(sizeof(rq_root));
   //  // query->pipes = *pipes;
   //  // query->ops = *ops;
   //
   //
   //
   //
   //
   //
   //
   //  /* Send the Query */
   //
   //  CLIENT *client;
   //  int *result;
   //
   //  client = clnt_create("localhost",
   //      REMOTE_QUERY_ROOT, REMOTE_QUERY_ROOT_V1, "tcp");
   //  if (client) result = rq_root_1(*query, client);
   //
   //  if (!client) {
   //      printf("Error: Could not connect to server.\n");
   //      result = EXIT_FAILURE;
   //  }
   //  if (!result) {
   //      printf("Error: RPC failed.\n");
   //      result = EXIT_FAILURE;
   //  }
   //
   //  free(query);
   //
   //  /* Return the result. */
   //  return *result;
}
