// Artur Sultanov
// xsulta01
// IOS project 2 2023

#ifndef PROJ2_HEADER_FILE
#define PROJ2_HEADER_FILE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
//#include <sys/shm.h>
//#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>

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
