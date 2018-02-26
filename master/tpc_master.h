
typedef struct slave *slvptr;

/* used to represent a slave, connected in a linked-list. */
typedef struct slave {
    int slave_id;
    char *slave_addr;
    slvptr next;
} slave;

/* master.c */


/* master_tpc_vector.c functions */
int commit_vector(int vecptr, int slaveptr);
void *get_commit_resp(void*);
void *commit_bit_vector(void*);
