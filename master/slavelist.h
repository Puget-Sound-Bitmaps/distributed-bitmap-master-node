#ifndef SLAVELIST
#define SLAVELIST

/* XXX: keep these in a private file, pipe them as command line arguments
 into the live version
*/
#define DB_CAPSTONE_1_ADDR "10.250.94.63"
#define DB_CAPSTONE_2_ADDR "10.250.94.56"
#define DB_CAPSTONE_3_ADDR "10.250.94.72"
#define HOME_ADDR "127.0.1.1"

/* #define LOCALTEST */
/* #define FULL_TEST */
#define DBCAP_1_TEST

#ifdef DBCAP_1_TEST
    #define NUM_SLAVES 1
    static char SLAVE_ADDR[NUM_SLAVES][32] = {
        DB_CAPSTONE_1_ADDR
    };
#endif /* DBCAP_1_TEST */

#ifdef LOCALTEST
    #define NUM_SLAVES 1
    static char SLAVE_ADDR[NUM_SLAVES][32] = {
        HOME_ADDR
    };
#endif /* LOCALTEST */

#ifdef FULL_TEST /* Full test with 3 slaves */
    #define NUM_SLAVES 3
    static char SLAVE_ADDR[NUM_SLAVES][32] = {
        DB_CAPSTONE_1_ADDR,
        DB_CAPSTONE_2_ADDR,
        DB_CAPSTONE_3_ADDR
    };
#endif /* FULL_TEST */

#define NUM_MACHINES NUM_SLAVES + 1

#endif /* SLAVELIST */
