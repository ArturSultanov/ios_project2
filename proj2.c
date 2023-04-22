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


#define SEMAPHORE_NAME "/xsulta01_iosproj2_"
#define SHM_MEM = "/proj2_shm"
//#define FILE_NAME "proj2.out"
#define NUM_SERVICES 3

//global values
sem_t *semaphore = NULL;
FILE *file;

//

typedef struct {
    int customers[NUM_SERVICES];
    int closing;
    int action_counter;
    int activity_queues[3];
    int post_office_closed;
} SharedMemory;



// Function prototypes
void customer_process(int idZ, int NZ, int TZ, int F);
void clerk_process(int idU, int TU, int F);
void semaphore_create();
void semaphore_kill();
int random_number(int min, int max);
SharedMemory* init_shared_memory(const char *name);


// Funcrions

SharedMemory* init_shared_memory(const char *name) {
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate");
        return NULL;
    }

    SharedMemory *shared_memory = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    // Initialize the shared memory segment
    shared_memory->action_counter = 0;
    for (int i = 0; i < 3; i++) {
        shared_memory->activity_queues[i] = 0;
    }
    shared_memory->post_office_closed = 0;

    return shared_memory;
}





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
    file = fopen("proj2.out", "w");
    setbuf(file, NULL);

    const char *shm_name = "/proj2_shm";
    SharedMemory *shared_memory = init_shared_memory(shm_name);
    if (shared_memory == NULL) {
        return 1;
    }    

    // Initialize shared memory and semaphores
    // ...


    // Clean up shared memory and semaphores
    // ...

    if (munmap(shared_memory, sizeof(SharedMemory)) == -1) {
        perror("munmap");
    }

    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
    }

    while (wait (NULL) != - 1 || errno != ECHILD)

    return 0;
}
