#include <pthread.h>
#include <stdio.h>
#include "tpc_master.h"
#include "vote.h"
#include "tpc.h"

pthread_mutex_t lock;
int successes;

typedef struct push_vec_args {
    vecptr vector;
    slvptr slave;
} push_vec_args;

/*
 * Commit the given vid->vector mappings to the given slaves.
 */
int commit_vector(vecptr vec, slvptr slave)
{
    /* 2PC Phase 1 on all Slaves */
    pthread_t tids[num_slaves];
    successes = 0;
    pthread_mutex_init(&lock, NULL);
    slavptr start_slv = slave;

    int i, num_slaves;
    i = 0;
    num_slaves = 0;

    while (slave != NULL) {
        pthread_create(&tids[i++], NULL, get_commit_resp,
            (void *)slave->slave_addr);
        slave = slave->next;
        num_slaves++;
    }

    for (i = 0; i < num_slaves; i++) {
        pthread_join(tids[i], NULL);
    }

    /* check if all slaves can commit */
    if (successes != num_slaves) return 1;

    /* TODO: 2PC Phase 2 */
    slave = start_slv;
    i = 0;
    while (slave != NULL) {
        struct push_vec_args* ptr = (struct push_vec_args*)
            malloc(sizeof(struct push_vec_args));
        ptr->vector = vector;
        ptr->slave = slave;
        pthread_create(&tids[i++], NULL, push_vector, (void*) ptr);
        slave = slave->next;
        vector = vector->next;
    }
    pthread_mutex_destroy(&lock);
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
        return (void*) 1;
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
    struct push_vec_args *args = (struct push_vec_args*) thread_arg;
    CLIENT* cl = clnt_create(args->slave->slave_addr, TWO_PHASE_COMMIT_VEC,
        TPC_COMMIT_VEC_V1, "tcp");

    if (cl == NULL) {
        printf("Error: could not connect to slave %s.\n", );
        return (void*) 1;
    }

    struct commit_vec_args* a = (struct commit_vec_args*)
        malloc(sizeof(struct commit_vec_args));
    a->vec_id = args->vector->vec_id;
    a->vec = args->vector->bit_vector;
    int *result = commit_vec_1(a, cl);
    free(a);

    if (result == NULL) {
        printf("Commit failed.\n");
        return (void*) 1;
    }
    printf("Commit succeeded.\n");
    return (void*) 0;
}
