#include "master.h"
#include "tpc_master.h"
#include "../ipc/messages.h"
#include "slavelist.h"
#include "../consistent-hash/ring/src/tree_map.h"
#include "../types/types.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/types.h>

/**
 * Sort machine-vector tuples in ascending order of machine ID.
 * Source: https://en.wikipedia.org/wiki/Qsort
 */
int compare_machine_vec_tuple(const void *p, const void *q) {
    unsigned int **x = (const unsigned int **)p;
    unsigned int **y = (const unsigned int **)q;

    if (**x < **y)
        return -1;
    else if (**x > **y)
        return 1;

    return 0;
}

/**
 * Master Process
 *
 * Coordinates/delegates tasks for the system.
 */
int main(int argc, char *argv[])
{
    // XXX: running with defaults for now (r = 3, and hardcoded slave addresses)
    /*
    if (argc < 3) {
        printf("Usage: [-r repl_factor] -s slave_addr1 [... slave_addrn]\n");
        return 1;
    }
    */

    /* Connect to message queue. */
    int msq_id = msgget(MSQ_KEY, MSQ_PERMISSIONS | IPC_CREAT);
    /* Container for messages. */
    struct msgbuf *request;
    struct msqid_ds buf;
    int rc;

    bool dying = false;

    /*
     * insert slaves into the tree
     */
     rbt_ptr chash_table = new_rbt();
     int i;
     for (i = 0; i < NUM_SLAVES; i++) {
        struct cache *cptr = (struct cache*) malloc(sizeof(struct cache));
        cptr->cache_id = i;
        cptr->cache_name = SLAVE_ADDR[i];
        cptr->replication_factor = 1;
        insert_cache(chash_table, cptr);
     }


    while (true) {
        msgctl(msq_id, IPC_STAT, &buf);

        if (buf.msg_qnum > 0) {


            request = (struct msgbuf *) malloc(sizeof(msgbuf));

            /* Grab from queue. */
            rc = msgrcv(msq_id, request, sizeof(msgbuf), 0, 0);

            /* Error Checking */
            if (rc < 0) {
                perror( strerror(errno) );
                printf("msgrcv failed, rc = %d\n", rc);
                return EXIT_FAILURE;
            }

            /* Check for death signal. XXX: should be done w/ SIGKILL */
            /*
            if (request->mtype == mtype_kill_master) {
                dying = true;
                break;
            }
            */
            if (request->mtype == mtype_put) {
                //printf("Master PUT %i := 0x%llx\n", request->vector.vec_id,
                //    request->vector.vec);
                vec_id_t vec_id_mult = request->vector.vec_id *
                    replication_factor;
                vec_id_t vec_id_1 = vec_id_mult;
                vec_id_t vec_id_2 = vec_id_mult + 1;
                vec_id_t vec_id_3 = vec_id_mult + 2;
                char *slave_1 = SLAVE_ADDR[
                    get_machine_for_vector(chash_table, vec_id_1)
                ];
                char *slave_2 = SLAVE_ADDR[
                    get_machine_for_vector(chash_table, vec_id_2)
                ];
                char *slave_3 = SLAVE_ADDR[
                    get_machine_for_vector(chash_table, vec_id_3)
                ];
                // TODO ensure slave_1 != slave_2 != slave_3
                // TODO: call commit_vector RPC function here
                //commit_vector(request->vector.vec_id, request->vector.vec,
                //    {slave_1, slave_2, slave_3}, 3);
            }
            else if (request->mtype == mtype_range_query) {
                range_query_contents contents = request->range_query;
                #define NUM_SLAVES 3
                int i;
                for (i = 0; i < contents.num_ranges; i++) {
                    unsigned int *range = contents.ranges[i];
                    vec_id_t j;
                    unsigned int **machine_vec_ptrs = (unsigned int **)
                        malloc(sizeof(int *) * (range[1] - range[0] + 1));
                    for (j = range[0]; j <= range[1]; j++) {
                        unsigned int *tuple = (unsigned int *)
                            malloc(sizeof(unsigned int) * 2);
                        tuple[0] = get_machine_for_vector(chash_table, j);
                        tuple[1] = j;
                        printf("Tuple[0] = %d, Tuple[1] = %d\n", tuple[0], tuple[1]);
                        machine_vec_ptrs[j - range[0]] = tuple;
                    }

                    qsort(machine_vec_ptrs, range[1] - range[0], sizeof(unsigned int) * 2,
                        compare_machine_vec_tuple);
                    printf("Printing tuples\n");
                    for (j = 0; j < range[1] - range[0]; j++) {
                        printf("Machine = %d, vector_id = %d\n", machine_vec_ptrs[j][0], machine_vec_ptrs[j][1]);
                    }
                }

                /* TODO: Call Jahrme function here */
            }
            else if (request->mtype == mtype_point_query) {
                /* TODO: Call Jahrme function here */
            }

        }

    }

    // if (dying) {
    //     struct put_msgbuf signal = {mtype_master_dying, {0,0} };
    //     msgsnd(msq_id, &signal, sizeof(struct put_msgbuf), 0);
    // }

    return EXIT_SUCCESS;
}
