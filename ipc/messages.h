#ifndef MESSAGES_H
#define MESSAGES_H

#include <sys/msg.h>
#include "../types/types.h"

/**
 * TODO:
 * Do not hard code key so that we can (hopefully) avoid conflicts.
 * Beej recommends using:
 *      key_t ftok(const char *path, int id);
 */
static key_t MSQ_KEY = 440440;

/* Equivalent to "rw-rw-rw-" */
static int MSQ_PERMISSIONS = 0666;

/* Message types */
enum {
    mtype_put,
    mtype_point_query,
    mtype_range_query,
    mtype_kill_master,
    mtype_slave_intro
};
// static long mtype_put = 1;
// static long mtype_point_query = 2;
// static long mtype_range_query = 3;
// static long mtype_kill_master = 98;
// static long mtype_master_dying = 99;

typedef struct assigned_vector {
    vec_id_t vec_id;
    vec_t vec;
} assigned_vector;

typedef struct range_query_contents {
    vec_id_t ranges[128][2];
    char ops[128];
    unsigned int num_ranges;
} range_query_contents;

typedef struct msgbuf {
    int mtype;
    assigned_vector vector;
    vec_id_t point_vec_id;       /* for point query */
    range_query_contents range_query; /* for range query */
} msgbuf;


#endif /* MESSAGES_H */
