//Artur Sultanov
//xsulta01

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>


#define SEMAPHORE_QUEUE "/xsulta01_iosproj2_sem_queue"
#define SHM_MEM = "/proj2_shm"
#define NUM_SERVICES 3


// Function prototypes
void customer_process(int idZ, int NZ, int TZ, int F);
void clerk_process(int idU, int TU, int F);
void semaphore_queue_create();
void semaphore_queue_kill();
int random_number(int min, int max);

//global values
FILE *file;

// deklaracia zdielannych premennych
int *num_proc = NULL; 

// deklaracia semaforov
sem_t *sem_queue = NULL;



// Functions
void semaphore_kill(){
    sem_close(sem_queue);         sem_unlink(SEMAPHORE_QUEUE);
}

int semaphore_create(){
    semaphore_kill();
    sem_queue = sem_open(SEMAPHORE_QUEUE, O_CREAT | O_EXCL, 0666, 1) ;
    if (sem_queue == SEM_FAILED){
        return 1;
    }

    return 0;
}


//processes functions
void customer_process(int idZ, int NZ, int TZ, int F) {
    // Customer process logic
}

void clerk_process(int idU, int TU, int F) {
    // Clerk process logic
}

int random_number(int min, int max)
{
    int number = rand() % (max-min+1);
    return number+min;
}

// Main
int main(int argc, char *argv[]) {



    if (argc != 6) {
        fprintf(stderr, "Error: Invalid number of arguments.\n");
        return 1;
    }

    int NZ = atoi(argv[1]); //počet zákazníků
    int NU = atoi(argv[2]); //počet úředníků
    int TZ = atoi(argv[3]);
    int TU = atoi(argv[4]);
    int F = atoi(argv[5]);

    // Check if input values are within allowed range
    if (NZ < 0 || NU < 0 || TZ < 0 || TZ > 10000 || TU < 0 || TU > 100 || F < 0 || F > 10000) {
        fprintf(stderr, "Error: Invalid input values.\n");
        return 1;
    }

    //
    pid_t wpid;
    int status = 0;
    // 
    file = fopen("proj2.out", "w");
    setbuf(file, NULL);

    // SharedMemory *shared_memory = init_shared_memory(shm_name);
    // if (shared_memory == NULL) {
    //     return 1;
    // }    



    // Initialize shared memory and semaphores
    // ...


    // Clean up shared memory and semaphores
    // ...

    while ((wpid = wait(&status)) > 0);

    // if (munmap(shared_memory, sizeof(SharedMemory)) == -1) {
    //     perror("munmap");
    // }

   
    fclose(file);

    return 0;
}
