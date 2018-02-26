#ifndef TREE_MAP_H
#define TREE_MAP_H

#define NIL_HV 0

typedef int cache_id;
typedef int hash_value;

typedef enum {RED, BLACK} rbt_node_color;

typedef struct node *node_ptr;
typedef struct rbt *rbt_ptr;

typedef struct node {
    hash_value hv; /* key used to identify the node */
    cache_id cid;
    rbt_node_color color;
    node_ptr left;
    node_ptr right;
    node_ptr parent;
} node;

typedef struct rbt {
    node_ptr root;
    node_ptr nil;
    int size;
} rbt;


typedef struct cache {
    int cache_id;
    char *cache_name;
    int replication_factor;
} cache;

void insert_cache(rbt_ptr, struct cache*);
cache_id get_machine_for_vector(rbt_ptr, unsigned int);
rbt_ptr new_rbt();
void print_tree(rbt_ptr, node_ptr);


#endif
