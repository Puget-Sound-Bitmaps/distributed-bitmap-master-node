#ifndef DBMS_H
#define DBMS_H

#include "../types/types.h"

/* XXX: executables should be in the same directory? */
static char *MASTER_EXECUTABLE = "./master";
static char *REPLICATION_FACTOR = "3";

int put_vector(int queue_id, vec_id_t vec_id, vec_t vec);

#endif /* DBMS_H */
