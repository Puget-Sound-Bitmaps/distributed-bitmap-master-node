/**
 *  RPCs for [R]emote [Q]uerying (RQ)
 */

/**
 *  Pipe Query
 *
 *  Thus named as the query is piped from the end back to the initial caller.
 *
 *  Querying works like this: each element in this linked list is a machine to
 *  visit to carry out this query. the result_vector contains the result of
 *  performing the query thus far. receiving this argument is a command to
 *  get the vector on this machine with the given vector id and OP it
 *  with the result_vector. then if next is nonnull recurse, otherwise return the result
*/
struct rq_pipe_args {
    unsigned int vec_id;
    char op;
    string machine_addr<16>; /* ip address, max 15-chars + null terminator. */
    struct rq_pipe_args *next;
};

struct query_result {
    unsigned hyper int vector<>;
    unsigned int exit_code;
};

program REMOTE_QUERY_PIPE {
    version REMOTE_QUERY_PIPE_V1 {
        query_result RQ_PIPE(rq_pipe_args) = 1;
    } = 1;
} = 0x20;

/**
 *  Root Query
 *
 *  Thus named as it is the root of the query.
 *  This is the query that tells the coordinator to carry out the given
 *  query.
 *
 *  This is the query sent from the master-node to the coordinator.
 *  The coordinator then invokes the `pipe_query`s and combines the results.
*/
struct rq_range_root_args {
    unsigned int range_array<>;
    unsigned int num_ranges;
    char ops<>;
};

program REMOTE_QUERY_ROOT {
    version REMOTE_QUERY_ROOT_V1 {
        query_result RQ_RANGE_ROOT(rq_range_root_args) = 1;
    } = 1;
} = 0x10;
