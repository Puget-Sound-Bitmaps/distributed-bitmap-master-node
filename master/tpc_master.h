typedef unsigned int vid_t;
typedef unsigned long long vec_t;
typedef struct slave *slvptr;
typedef struct vector_kv *vecptr;

/* used to represent a slave, connected in a linked-list. */
typedef struct slave {
    int slave_id;
    char *slave_addr;
    slvptr next;
} slave;

typedef struct vector_kv {
    vid_t vec_id;
    vec_t bit_vector;
    vecptr next;
} push_vec_args;

/* master.c */


/* master_tpc_vector.c functions */
void *get_commit_resp(void*);
void *commit_bit_vector(void*);
