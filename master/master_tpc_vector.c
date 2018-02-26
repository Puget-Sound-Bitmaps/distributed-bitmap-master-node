#include <pthread.h>
#include <stdio.h>
#include "tpc_master.h"
#include "vote.h"
#include "../rpc/src/tpc.h"
#include "../types/types.h"

pthread_mutex_t lock;
int successes;

typedef struct push_vec_args {
    vec_t vector;
    vec_id_t vec_id;
    char *slave_addr;
} push_vec_args;
/*
 * Commit the given vid->vector mappings to the given slaves.
 */
int commit_vector(vec_id_t vec_id, vec_t vector, char **slaves, int num_slaves)
{
    /* 2PC Phase 1 on all Slaves */
    pthread_t tids[num_slaves];
    successes = 0;
    pthread_mutex_init(&lock, NULL);
    slavptr start_slv = slave;

    int i;
    void *status = 0;
    for (i = 0; i < num_slaves; i++) {
        pthread_create(&tids[i], NULL, get_commit_resp, (void *)slaves[i]);
    }

    for (i = 0; i < num_slaves; i++) {
        pthread_join(tids[i], &status);
        if (status == (void *) 1) {
            printf("Failed to connect to all slaves.");
            return 1;
        }
    }

    /* check if all slaves can commit */
    if (successes != num_slaves) return 1;

    /* 2PC Phase 2 */
    slave = start_slv;
    i = 0;
    for (i = 0; i < num_slaves; i++) {
        push_vec_args* ptr = (push_vec_args*) malloc(sizeof(push_vec_args));
        ptr->vector = vector;
        ptr->slave = slave;
        pthread_create(&tids[i], NULL, push_vector, (void*) ptr);
    }
    for (i = 0; i < num_slaves; i++) {
        pthread_join(tids[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    return 0;
}

void *get_commit_resp(void *slv_addr_arg)
{
    char *slv_addr = (char *) slv_name_arg;
    CLNT* clnt = clnt_create(slv_addr, TWO_PHASE_COMMIT_VOTE,
        TWO_PHASE_COMMIT_VOTE_V1, "tcp");

    /* give the request a time-to-live */
    struct timeval tv;
    tv.tv_sec = TIME_TO_VOTE;
    tv.tv_usec = 0;
    clnt_control(clnt, CLSET_TIMEOUT, &tv);

    if (clnt == NULL) {
        printf("Error: could not connect to slave %s.\n", );
        pthread_exit((void*) 1);
    }
    int *result = commit_msg_1(NULL, clnt);

    if (result == NULL || *result == VOTE_ABORT) {
        printf("Couldn't commit at slave %s.\n", );
    }
    else {
        pthread_mutex_lock(&lock);
        successes++;
        pthread_mutex_unlock(&lock);
    }
    return (void*) 0;
}

void *push_vector(void *thread_arg)
{
    push_vec_args *args = (push_vec_args *) thread_arg;
    CLIENT* cl = clnt_create(args->slave_addr, TWO_PHASE_COMMIT_VEC,
        TPC_COMMIT_VEC_V1, "tcp");

    if (cl == NULL) {
        printf("Error: could not connect to slave %s.\n", );
        pthread_exit((void*) 1);
    }

    commit_vec_args *a = (commit_vec_args*) malloc(sizeof(commit_vec_args));
    a->vec_id = args->vec_id;
    a->vector = args->vector;
    int *result = commit_vec_1(a, cl);

    if (result == NULL) {
        printf("Commit failed.\n");
        return (void*) 1;
    }
    printf("Commit succeeded.\n");
    return (void*) 0;
}
