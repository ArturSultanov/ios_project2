// Artur Sultanov
// xsulta01
// IOS project 2 2023

#ifndef PROJ2_HEADER_FILE
#define PROJ2_HEADER_FILE

#include <stdio.h>      // standard input/output functions
#include <stdbool.h>    // the bool type
#include <stdlib.h>     // functions like exit, malloc, and free

#include <unistd.h>     // the fork, usleep 
#include <sys/types.h>  // data type 'pid_t'
#include <time.h>       // time-related functions
#include <fcntl.h>      // flags like O_CREAT, O_EXCL
#include <semaphore.h>  // POSIX semaphore functions
#include <sys/wait.h>   // wait and waitpid functions
#include <sys/mman.h>   // shared memory functions
#include <signal.h>     // signal handling functions


#define SEMAPHORE_MUTEX "/xsulta01_sem_mutex"
#define SEMAPHORE_SERVICEFRST "/xsulta01_servicefrst" 
#define SEMAPHORE_SERVICESCND "/xsulta01_servicescnd" 
#define SEMAPHORE_SERVICETHRD "/xsulta01_servicethrd" 
#define SEMAPHORE_CLERK "/xsulta01_clerk"
#define SEMAPHORE_CUSTOMER "/xsulta01_customer"


#define NUM_SERVICES 3
#define upsleep_for_random_time(time_max) usleep((rand() % (time_max + 1)) * 1000)

// Function prototypes
void semaphore_dest(void);
int semaphore_init(void);
int shared_memory_dest(void);
int shared_memory_init(void);
void customer_process(int idZ, int TZ);
void clerk_process(int idU, int TU);
void cleanup(void);
void kill_child_processes(void);

#endif
