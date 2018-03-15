/*
 * TODO: instead of being able to send just one vector at a time,
 * send a linked list of vec_args structs
 */

struct commit_vec_args {
    unsigned int vec_id;
    unsigned hyper int *vector;
    unsigned int vector_length;
};

program TWO_PHASE_COMMIT_VOTE {
    version TWO_PHASE_COMMIT_VOTE_V1 {
        int COMMIT_MSG(int x) = 1;
    } = 1;
} = 0x30;

program TWO_PHASE_COMMIT_VEC {
    version TPC_COMMIT_VEC_V1 {
        int COMMIT_VEC(struct commit_vec_args) = 1;
    } = 1;
} = 0x40;
