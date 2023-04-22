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


#define SEMAPHORE_NAME "/xsulta01_iosproj2_"
#define FILE_NAME "proj2.out"
#define NUM_SERVICES 3

typedef struct {
    int action_counter;
    int customers[NUM_SERVICES];
    int closing;
} SharedData;

//global values
sem_t *semaphore = NULL;



void semaphore_create(){
    semaphore = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1) ;
}

void semaphore_kill(){
    sem_close(semaphore);     
    sem_unlink(SEMAPHORE_NAME);
}

int random_number(int min, int max)
{
    int number = rand() % (max-min+1);
    return number+min;
}

void customer_process(int idZ, int NZ, int TZ, int F) {
    // Customer process logic
}

void clerk_process(int idU, int TU, int F) {
    // Clerk process logic
}


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
      

    // Initialize shared memory and semaphores
    // ...


    // Clean up shared memory and semaphores
    // ...

    while (wait (NULL) != - 1 || errno != ECHILD)

    return 0;
}
