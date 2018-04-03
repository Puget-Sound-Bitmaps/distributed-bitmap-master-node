#ifndef MASTER_H
#define MASTER_H

#include <stdbool.h>
#include "../types/types.h"
#include "../ipc/messages.h"

/**
 * each slave will keep a linked list of vector IDs indicating which
 * vector it has (could also keep record of the vector itself)
 */
typedef struct slave_vector {
    unsigned int id;
    struct slave_vector *next;
} slave_vector;

typedef struct slave {
    unsigned int id;
    char *address;
    bool is_alive;
    slave_vector *vectors;
} slave;

typedef struct slave_ll {
    slave *slave_node;
    struct slave_ll *next;
} slave_ll;

/* will vary by system */
enum {RING_CH, JUMP_CH, STATIC_PARTITION}; /* parition types */
enum {UNISTAR, STARFISH, MULTISTAR, ITER_PRIM}; /* query plan algorithms */
/* The number of different machines a vector appears on */
static int replication_factor = 2;

/* master function prototypes */
int setup_slave(slave*);
slave *new_slave(char*);
void heartbeat(void);
int is_alive(char *);
unsigned int *get_machines_for_vector(unsigned int);
int send_vector(slave *, vec_id_t, slave*);
void reallocate(void);
int starfish(range_query_contents);

#endif /* MASTER_H */
