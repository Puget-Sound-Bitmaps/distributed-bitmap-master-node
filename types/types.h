/**
 * Types to be used by every module.
 */
#ifndef TYPES_H
#define TYPES_H
typedef struct vec_t {
    unsigned long long *vector;
    unsigned int vector_length;
} vec_t;
typedef unsigned int vec_id_t;
#endif
