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
    query_op op;
    char next_machine[16]; // ip address, max 15-chars + null terminator.
    struct rq_pipe *next;
};

program REMOTE_QUERY_PIPE {
    version REMOTE_QUERY_PIPE_V1 {
        int RQ_PIPE(struct rq_pipe *) = 1;
    } = 1;
} = 0x20;

/**
 *  Root Query
 *
 *  Thus named as it is the root of the query.
 *  This is the query that creates the coordinator.
 *
 *  This is the query sent from the master-node to the coordinator.
 *  The coordinator then invokes the `pipe_query`s and combines the results.
 *  We require len(ops) = len(pipes) - 1 so that ops fit between ranges.
 */
struct rq_root {
    struct rq_pipe *pipes<>;
    query_op *ops<>;
};

program REMOTE_QUERY_ROOT {
    version REMOTE_QUERY_ROOT_V1 {
        int RQ_ROOT(struct rq_root *) = 1;
    } = 1;
} = 0x10;
