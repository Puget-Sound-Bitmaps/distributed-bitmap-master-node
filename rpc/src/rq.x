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
    char *next_machine;
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
 *
 *  TODO:
 *  There are two ways we could handle the two arrays:
 *
 *  1.  We could require the two arrays to be the same length; in this case,
 *      we alternate queries with ops and end with the final value being 0.
 *      In this case, ops[n] would almost undoubtedly be OR.
 *
 *          pipes[0] ops[0] pipes[1] ops[1] . . . pipes[n] ops[n] 0
 *
 *  2.  We could require that `ops` is one less than `pipes`; in this case,
 *      we simply alternate the queries and ops.
 *
 *          pipes[0] ops[0] pipes[1] ops[1] . . . pipes[n-1] ops[n-1] pipes[n]
 *
 *  Regardless, there will still be an array of each.
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
