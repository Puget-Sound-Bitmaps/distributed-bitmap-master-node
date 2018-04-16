/**
 * Master Remote Query Handling
 */

#include "master_rq.h"
#include "master.h"
#include "../rpc/gen/slave.h"
#include "slavelist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include </usr/include/python2.7/Python.h> // XXX should be Python3.5 on Linux

btree_query_args *get_query_args(PyObject *);


int init_btree_range_query(range_query_contents contents)
{
    Py_Initialize();
    PySys_SetPath(".");
    // TODO: condition on plan type
    PyObject *pName, *pModule, *pValue;
    pName = PyString_FromString("mst_planner");
    pModule = PyImport_Import(pName);
    if (pModule != NULL) {
        PyObject *pyArg;
        pyArg = PyList_New(contents.num_ranges);
        // set the subranges
        int i;
        for (i = 0; i < contents.num_ranges; i++) {
            int range_len = contents.ranges[i][1] - contents.ranges[i][0];
            PyObject *sublist = PyList_New(range_len);
            PyList_SetItem(pyArg, i, sublist);;
            //unsigned int vec_ids[range_len];
            int j;
            for (j = contents.ranges[i][0]; j <= contents.ranges[i][1]; j++) {
                PyObject *t1, *t2;
                t1 = PyTuple_New(2);
                t2 = PyTuple_New(replication_factor);
                PyTuple_SetItem(t1, 0, PyInt_FromLong(j));
                PyTuple_SetItem(t1, 1, t2);
                unsigned int *tup = get_machines_for_vector(j, false);
                PyTuple_SetItem(t2, 0, PyInt_FromLong(tup[0]));
                PyTuple_SetItem(t2, 1, PyInt_FromLong(tup[1]));
                PyList_SetItem(sublist, contents.ranges[i][0] - j, t1);
            }
        }
        PyObject *pFunc = PyObject_GetAttrString(pModule, "iter_mst");
        if (pFunc != NULL && PyCallable_Check(pFunc)) {
            pValue = PyObject_CallFunctionObjArgs(pFunc, pyArg, NULL);
            if (pValue != NULL) {
                PyObject *res = PyObject_CallFunctionObjArgs(pFunc, pyArg, NULL);
                // res is of the form: (Coordinator, Trees)
                long coordinator_node = PyLong_AsLong(PyTuple_GetItem(res, 0));
                PyObject *trees = PyTuple_GetItem(res, 1);
                int num_trees = PyList_Size(trees);
                int k;
                btree_query_args *args[num_trees];
                for (k = 0; k < num_trees; k++) {
                    PyObject *query_tree = PyList_GetItem(trees, k);
                    args[k] = get_query_args(query_tree);
                }
                btree_query_args *coordinator_args = (btree_query_args *)
                    malloc(sizeof(btree_query_args));
                strcpy(coordinator_args->this_machine_address, SLAVE_ADDR[coordinator_node - 1]);
                coordinator_args->recur_query_list.recur_query_list_val = args;
                coordinator_args->recur_query_list.recur_query_list_len = num_trees;
                int num_coord_ops = num_trees + 1; // XXX may be off-by-1
                char coord_ops[num_coord_ops];
                coordinator_args->subquery_ops.subquery_ops_val = coord_ops;
                memset(coordinator_args->subquery_ops.subquery_ops_val, '&', num_coord_ops);
                coordinator_args->subquery_ops.subquery_ops_len = num_coord_ops;
                CLIENT *clnt = clnt_create(coordinator_args->this_machine_address,
                    BTREE_QUERY_PIPE, BTREE_QUERY_PIPE_V1, "tcp");
                btree_query_1(*coordinator_args, clnt);
            }
        }
    }
    else {

    }
    Py_Finalize();
    return 0;
}

btree_query_args *get_query_args(PyObject *vertex)
{
    PyObject *children = PyObject_GetAttrString(vertex, "children");
    btree_query_args *args = (btree_query_args *)
        malloc(sizeof(btree_query_args));
    PyObject *vectors = PyObject_GetAttrString(vertex, "vectors");
    unsigned int num_vecs = PyList_Size(vectors);
    unsigned int arr[num_vecs];
    char ops[num_vecs - 1];
    args->local_vectors.local_vectors_val = arr;
    args->local_vectors.local_vectors_len = num_vecs;
    args->local_ops.local_ops_val = ops;
    args->local_ops.local_ops_len = num_vecs - 1;
    memset(args->local_ops.local_ops_val, '|', num_vecs - 1);
    long machine_id = PyInt_AsLong(PyObject_GetAttrString(vertex, "key"));
    strcpy(args->this_machine_address, SLAVE_ADDR[machine_id - 1]);
    int j;
    for (j = 0; j < num_vecs; j++) {
        args->local_vectors.local_vectors_val[j] = PyLong_AsLong(PyList_GetItem(vectors, j));
    }
    int num_children = PyList_Size(children);
    if (num_children == 0) { // base case
        //args->recur_query_list = NULL; // XXX why did I do this...
        return args;
    }
    btree_query_args subqs[num_children];
    args->recur_query_list.recur_query_list_val = subqs;
    args->recur_query_list.recur_query_list_len = num_children;
    char subops[num_children - 1];
    args->subquery_ops.subquery_ops_val = subops;
    args->subquery_ops.subquery_ops_len = num_vecs - 1;
    memset(args->subquery_ops.subquery_ops_val, '|', num_vecs - 1);
    int i;
    for (i = 0; i < num_children; i++) {
        args->recur_query_list.recur_query_list_val[i] = *get_query_args(PyList_GetItem(children, i));
    }
    return args;
}

/**
 * Executes the given (formatted) range query, returning 0 on success,
 * or the machine that failed in the query process
 */
int
init_range_query(unsigned int *range_array, int num_ranges,
    char *ops, int array_len)
{
    rq_range_root_args *root = (rq_range_root_args *) malloc(sizeof(rq_range_root_args));
    root->range_array.range_array_val = range_array;
    root->range_array.range_array_len = array_len;
    root->num_ranges = num_ranges;
    root->ops.ops_val = ops;
    root->ops.ops_len = num_ranges - 1;
    char *coordinator = SLAVE_ADDR[0]; // arbitrary for now
    CLIENT *cl = clnt_create(coordinator, REMOTE_QUERY_ROOT,
        REMOTE_QUERY_ROOT_V1, "tcp");
    if (cl == NULL) {
        printf("Error: could not connect to coordinator %s.\n", coordinator);
        return 1;
    }
    printf("Calling RPC\n");
    query_result *res = rq_range_root_1(*root, cl);
    if (res == NULL) {
        printf("No response from coordinator.\n");
        return 1;
    }
    int i;
    if (res->exit_code == EXIT_SUCCESS) {
        for (i = 1; i < res->vector.vector_len - 1; i++) {
            printf("%llx\n",res->vector.vector_val[i]);
        }
        printf("%llx\n", res->vector.vector_val[i]);
    }
    else {
        printf("Query failed, reason: %s\n", res->error_message);
    }
    free(root);
    //if (res != NULL) free(res);
    return EXIT_SUCCESS;
}
