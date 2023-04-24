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

#define SEMAPHORE_MUTEX "/xsulta01_iosproj2_mutex"
#define SEMAPHORE_SERVICE1 "/xsulta01_iosproj2_service1"
#define SEMAPHORE_SERVICE2 "/xsulta01_iosproj2_service2"
#define SEMAPHORE_SERVICE3 "/xsulta01_iosproj2_service3"

#define NUM_SERVICES 3
//#define upsleep_for_random_time(time_max) { usleep((rand() % (time_max + 1)) * 1000); }
#define upsleep_for_random_time(time_max) usleep((rand() % (time_max + 1)) * 1000)


// Function prototypes
void semaphore_dest(void);
int semaphore_init(void);
void shared_memory_dest(void);
int shared_memory_init(void);
void customer_process(int idZ, int TZ);
void clerk_process(int idU, int TU);
//int random_number(int min, int max);

//global values
FILE *file;


// deklaracia zdielannych premennych
bool *post_is_closed;
int *action_number;
int *customer_services_queue[3]; // service


// deklaracia semaforov
sem_t *sem_mutex;
sem_t *sem_customer_service1;
sem_t *sem_customer_service2;
sem_t *sem_customer_service3;



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
    
    semaphore_dest();

    pid_t wpid;
    int status = 0;

    // Set output file and output buffer
    file = fopen("proj2.out", "w");
    setbuf(file, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    // Initialize shared memory and semaphores
    if(shared_memory_init()){
        fprintf(stderr, "Cannot alocate shared memory!\n");
        return 1;
    }
    if(semaphore_init()){
        fprintf(stderr, "Cannot open semaphores!\n");
        return 1;
    }

    srand(time(NULL));

    // Fork customer processes
    for(int idZ  = 1; idZ <=NZ;idZ++){
        pid_t pid = fork();
        if(pid==0){
            //srand((int)time(0) % getpid()); // for upsleep_for_random_time(time_max)
            customer_process(idZ, TZ);
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
    for(int idU  = 1; idU <=NU;idU++){
        pid_t pid = fork();
        if(pid==0){
            //srand((int)time(0) % getpid()); // for upsleep_for_random_time(time_max)
            clerk_process(idU, TU);
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



    usleep((rand() % ((F / 2) + 1)) * 1000 + F / 2 * 1000);

   //Close the post office
    sem_wait(sem_mutex);
    *post_is_closed = 1;
    ++(*action_number);
    fprintf(file, "%d: closing\n", *action_number);
    sem_post(sem_mutex);

    // Clean up shared memory and semaphores
    while ((wpid = wait(&status)) > 0);
    shared_memory_dest();
    semaphore_dest();
    fclose(file);

    return 0;
}
////////////////////////////    MAIN END    ////////////////////////////

////////////////////////////    SEMAPHORES  ////////////////////////////
// Semaphores destruction
void semaphore_dest(void){
    sem_close(sem_mutex);                   
    sem_unlink(SEMAPHORE_MUTEX);

    sem_close(sem_customer_service1);
    sem_unlink(SEMAPHORE_SERVICE1);

    sem_close(sem_customer_service2);
    sem_unlink(SEMAPHORE_SERVICE2);

    sem_close(sem_customer_service3);
    sem_unlink(SEMAPHORE_SERVICE3);

    
    return;
}

// Semaphores initialization
int semaphore_init(void){
    
    semaphore_dest();
    
    sem_mutex = sem_open(SEMAPHORE_MUTEX, O_CREAT | O_EXCL, 0644, 1) ;
    if (sem_mutex == SEM_FAILED){
        return 1;
    }

    sem_customer_service1 = sem_open(SEMAPHORE_SERVICE1, O_CREAT | O_EXCL, 0644, 0) ;
    if (sem_customer_service1 == SEM_FAILED){
         return 1;
    }

        sem_customer_service2 = sem_open(SEMAPHORE_SERVICE2, O_CREAT | O_EXCL, 0644, 0) ;
    if (sem_customer_service2 == SEM_FAILED){
         return 1;
    }

        sem_customer_service3 = sem_open(SEMAPHORE_SERVICE3, O_CREAT | O_EXCL, 0644, 0) ;
    if (sem_customer_service3 == SEM_FAILED){
         return 1;
    }

    return 0;
}

////////////////////////////    SHARED MEMORY  ////////////////////////////
// Shared memory destruction
void shared_memory_dest(void){

    munmap(post_is_closed, sizeof(bool*));
    munmap(action_number, sizeof(int*));

    for (int i = 0; i < 3; i++) {
        munmap(customer_services_queue[i], sizeof(int));
    }
}

// Shared memory initialization
int shared_memory_init(void){
    void shared_memory_dest(void);

    post_is_closed = mmap(NULL, sizeof(*post_is_closed), PROT_READ|PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == post_is_closed) {
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        customer_services_queue[i] = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (MAP_FAILED == customer_services_queue[i]) {
            return 1;
        }
    }

    action_number = mmap(NULL, sizeof(*action_number), PROT_READ|PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == action_number) {
        return 1;
    }

    // inicializacia zdielanych premennych
    *post_is_closed=0;
    *action_number=0;

    for (int i = 0; i < 3; i++) {
        *customer_services_queue[i] = 0;
    }

    return 0;
}

////////////////////////////    PROCESSES   ////////////////////////////
// Customer process function
void customer_process(int idZ, int TZ) {
    sem_wait(sem_mutex);
    int action = ++(*action_number);
    // Print the initial message
    fprintf(file, "%d: Z %d: started\n", action, idZ);
    sem_post(sem_mutex);

    // Wait for a random time between 0 and TZ
    upsleep_for_random_time(TZ);

    sem_wait(sem_mutex);
    // Check if the post office is closed
    if (*post_is_closed) {
        action = ++(*action_number);
        fprintf(file, "%d: Z %d: going home\n", action, idZ);
        sem_post(sem_mutex);
        exit(0);
    }

    int service = rand() % NUM_SERVICES;

    *customer_services_queue[service]=(*customer_services_queue[service])++;
    action = ++(*action_number);
    fprintf(file, "%d: Z %d: entering office for a service %d\n", action, idZ, service + 1);
    sem_post(sem_mutex);

    if (service == 0)
    {
        sem_wait(sem_customer_service1);
    } else if (service == 1)
    {
        sem_wait(sem_customer_service2);

    } else if (service == 2)
    {
        sem_wait(sem_customer_service3);

    }
    

    sem_wait(sem_mutex);
    action = ++(*action_number);
    fprintf(file, "%d: Z %d: called by office worker\n", action, idZ);
    sem_post(sem_mutex);

    upsleep_for_random_time(10);

    sem_wait(sem_mutex);
    action = ++(*action_number);
    fprintf(file, "%d: Z %d: going home\n", action, idZ);
    sem_post(sem_mutex);

    exit(0);
    return;
}

// Clerk process function
void clerk_process(int idU, int TU) {
    
    sem_wait(sem_mutex);
    int action = ++(*action_number);
    fprintf(file, "%d: U %d: started\n", action, idU);
    sem_post(sem_mutex);

    while (1) {
        int service = -1;
        sem_wait(sem_mutex);
        for (int i = 0; i < NUM_SERVICES; i++) {
            if (*customer_services_queue[i] > 0) {
                service = i;
                break;
            }
        }

        if (service == -1 && *post_is_closed) {
            action = ++(*action_number);
            fprintf(file, "%d: U %d: going home\n", action, idU);
            sem_post(sem_mutex);
            exit(0);
        } else if (service == -1) {
            action = ++(*action_number);
            fprintf(file, "%d: U %d: taking break\n", action, idU);
            sem_post(sem_mutex);

            upsleep_for_random_time(TU);

            sem_wait(sem_mutex);
            action = ++(*action_number);
            fprintf(file, "%d: U %d: break finished\n", action, idU);
            sem_post(sem_mutex);
        } else {
        *customer_services_queue[service]=(*customer_services_queue[service])--;
        action = ++(*action_number);
        fprintf(file, "%d: U %d: serving customer at service %d\n", action, idU, service + 1);
        sem_post(sem_mutex);

        usleep(rand() % 11);

    if (service == 0)
    {
        sem_post(sem_customer_service1);
    } else if (service == 1)
    {
        sem_post(sem_customer_service2);

    } else if (service == 2)
    {
        sem_post(sem_customer_service3);

    }

        sem_wait(sem_mutex);
        action = ++(*action_number);
        fprintf(file, "%d: U %d: finished serving customer\n", action, idU);
        sem_post(sem_mutex);
        }
    }
    return;
}

////////////////////////////    OTHER FUNCTIONS ////////////////////////////
// int random_number(int min, int max)
// {
//     int number = rand() % (max-min+1);
//     return number+min;
// }