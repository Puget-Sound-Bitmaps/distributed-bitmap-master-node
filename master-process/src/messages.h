#ifndef MESSAGES_H
#define MESSAGES_H

#include <sys/msg.h>

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
static int mtype_quit = 666;
static int mtype_put = 1;

struct put_msgbuf {
    long mtype;
    struct put_request {
        int vec_id;
        unsigned long long vec;
    } vector;
};

#endif /* MESSAGES_H */
