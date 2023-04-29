// Artur Sultanov
// xsulta01
// IOS project 2 2023

#ifndef PROJ2_HEADER_FILE
#define PROJ2_HEADER_FILE

// Headers

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

#define NUM_SERVICES 3  // Number of Post services: dopisy:1, balíky:2, peněžní služby:3.
#define upsleep_for_random_time(time_max) usleep((rand() % (time_max + 1)) * 1000) // Macros for wait random time in range from 0 to time_max(inclusive).

// Function prototypes

int semaphore_init(void);               // Semaphores initialization(opening) function.
void semaphore_dest(void);              // Semaphores destruction(closing, unlinking) function.
int shared_memory_init(void);           // Shared memory initialization(mapping) function.
int shared_memory_dest(void);           // Shared memory destruction(unmapping) function.
void customer_process(int idZ, int TZ); // Customer-process logic.
void clerk_process(int idU, int TU);    // Clerk-process logic.
void cleanup(void);                     // Semaphores and shared variables destruction, closing output *file, if it wasn't.
void kill_child_processes(void);        // Kill all Child-processes were created by Main-process.
 
#endif
