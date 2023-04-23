//Artur Sultanov
//xsulta01

#include <stdio.h>
#include <stdbool.h>
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

#define upsleep_for_random_time(time_max) { usleep((rand() % (time_max + 1)) * 1000); }


// Function prototypes
void semaphore_dest(void);
int semaphore_init(void);
void shared_memory_dest(void);
int shared_memory_init(void);
void customer_process(int idZ, int NZ, int TZ, int F);
void clerk_process(int idU, int NU, int TU, int F);
int random_number(int min, int max);

//global values
FILE *file;


// deklaracia zdielannych premennych
bool *post_is_closed = 0;
int *num_proc = NULL; 
int *oxy_cnt = NULL; // pocitadlo aktualnej kyslikovej fronty
int *post_servises_queues[3]; // 


// deklaracia semaforov
sem_t *sem_queue = NULL;


////////////////////////////    MAIN START  ////////////////////////////
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
    
    pid_t wpid;
    int status = 0;

    // Set output file and output buffer
    file = fopen("proj2.out", "w");
    setbuf(file, NULL);
    
    // Initialize shared memory and semaphores
    if(shared_memory_init()){
        fprintf(stderr, "Cannot alocate shared memory!\n");
        return 1;
    }
    if(semaphore_init()){
        fprintf(stderr, "Cannot open semaphores!\n");
        return 1;
    }
    // Fork customer processes
    for(int id  = 1; id <=NZ;id++){
        pid_t pid = fork();
        if(pid==0){
            customer_process(id, NZ, TZ, F);
            printf("CUSTOMER\n");
            srand((int)time(0) % getpid()); // for upsleep_for_random_time(time_max)
            exit(0);
        }
        else if (pid==-1){
            fprintf(stderr, "Fork customer processes error!\n");
            shared_memory_dest();
            semaphore_dest();
            fclose(file);
            exit(1);
        }
    }

    // Fork clerk processes
    for(int id  = 1; id <=NU;id++){
        pid_t pid = fork();
        if(pid==0){
            clerk_process(id, NU, TU, F);
            printf("CLERK\n");
            srand((int)time(0) % getpid()); // for upsleep_for_random_time(time_max)
            exit(0);
        }
        else if (pid==-1){
            fprintf(stderr, "Fork clerk processes error!\n");
            shared_memory_dest();
            semaphore_dest();
            fclose(file);
            exit(1);
        }
    }


















    // usleep((rand() % ((F / 2) + 1)) * 1000 + F / 2 * 1000);

    // // Close the post office
    // sem_wait(semaphore);
    // fprintf(output, "closing\n");
    // sem_post(semaphore);

    // Clean up shared memory and semaphores
    while ((wpid = wait(&status)) > 0);
    shared_memory_dest();
    semaphore_dest();
    fclose(file);

    return 0;
}
////////////////////////////    MAIN END    ////////////////////////////

//FUNCTIONS
// Semaphores destruction
void semaphore_dest(void){
    sem_close(sem_queue);         
    sem_unlink(SEMAPHORE_QUEUE);
}

// Semaphores initialization
int semaphore_init(void){
    semaphore_dest();
    sem_queue = sem_open(SEMAPHORE_QUEUE, O_CREAT | O_EXCL, 0666, 1) ;
    if (sem_queue == SEM_FAILED){
        return 1;
    }

    return 0;
}

// Shared memory destruction
void shared_memory_dest(void){
    munmap(num_proc, sizeof(int*));
    munmap(oxy_cnt, sizeof(int*));
    munmap(post_is_closed, sizeof(bool*));

    for (int i = 0; i < 3; i++) {
        munmap(post_servises_queues[i], sizeof(int));
    }
}

// Shared memory initialization
int shared_memory_init(void){
    void shared_memory_dest(void);

    num_proc = mmap(NULL, sizeof(*num_proc), PROT_READ|PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == num_proc) {
        return 1;
    }
    oxy_cnt = mmap(NULL, sizeof(*num_proc), PROT_READ|PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == oxy_cnt) {
        return 1;
    }

    post_is_closed = mmap(NULL, sizeof(*post_is_closed), PROT_READ|PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == post_is_closed) {
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        post_servises_queues[i] = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (MAP_FAILED == post_servises_queues[i]) {
            return 1;
        }
    }

    // inicializacia zdielanych premennych
    *num_proc=1;
    *oxy_cnt=0;
    *post_is_closed=0;

    for (int i = 0; i < 3; i++) {
        *post_servises_queues[i] = 0;
    }

    return 0;
}

// Customer process function
void customer_process(int idZ, int NZ, int TZ, int F) {
        // Print the initial message
    printf("A: Z %d: started\n", idZ);

    // Seed the random number generator with the current time
    srand(time(NULL) + idZ);

    // Wait for a random time between 0 and TZ
    usleep(rand() % (TZ + 1));

    // Check if the post office is closed
    if (post_is_closed) {
        printf("A: Z %d: going home\n", idZ);
        return;
    }
}

// Clerk process function
void clerk_process(int idU, int NU, int TU, int F) {
    // Clerk process logic
    return;
}

// Other functions
int random_number(int min, int max)
{
    int number = rand() % (max-min+1);
    return number+min;
}
