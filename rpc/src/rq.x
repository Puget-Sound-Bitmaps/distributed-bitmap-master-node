/**
 *  RPCs for [R]emote [Q]uerying (RQ)
 */

/**
 *  Query Operations
 *
 *  The operations permitted in queries.
 *  For now we only allow AND and OR.
 */
enum query_op {AND, OR};

/**
 *  Pipe Query
 *
 *  Thus named as the query is piped from the end back to the initial caller.
 *
 *  This query is formulated as [vec op [pipe_query]].
 *  When [pipe_query] is null the above query is read as [vec op 0].
 *
 *  This can also be used for point queries by sending [vec OR null].
 */
struct rq_pipe {
    unsigned int vec_id;
    unsigned hyper int *vector;
    query_op op;
    char next_machine[16]; /* ip address, max 15-chars + null terminator. */
    struct rq_pipe *next;
};

struct query_result {
    unsigned hyper int *vector;
    unsigned int vector_length;
    unsigned int exit_code;
};

program REMOTE_QUERY_PIPE {
    version REMOTE_QUERY_PIPE_V1 {
        query_result RQ_PIPE(struct rq_pipe) = 1;
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
 *  We require len(ops) = num_ranges - 1 so that ops fit between ranges.
 */
struct rq_root_args {
    unsigned int range_array<>;
    unsigned int num_ranges;
    char ops<>;
};

program REMOTE_QUERY_ROOT {
    version REMOTE_QUERY_ROOT_V1 {
        int RQ_ROOT(rq_root_args) = 1;
    } = 1;
} = 0x10;
