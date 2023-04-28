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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>


#define SEMAPHORE_MUTEX "/xsulta01_sem_mutex"
#define SEMAPHORE_SERVICEFRST "/xsulta01_servicefrst" 
#define SEMAPHORE_SERVICESCND "/xsulta01_servicescnd" 
#define SEMAPHORE_SERVICETHRD "/xsulta01_servicethrd" 
#define SEMAPHORE_CLERK "/xsulta01_clerk"
#define SEMAPHORE_CUSTOMER "/xsulta01_customer"

typedef struct {
    int NZ; //number of clients
    int NU; //number of workers
    int TZ; //Time in ms where client waiting to go for post
    int TU; //worker rest, in ms
    int F; //in ms, time after that post office will be closed  
}args_t;

#define NUM_SERVICES 3
#define upsleep_for_random_time(time_max) usleep((rand() % (time_max + 1)) * 1000)


// Function prototypes
void semaphore_dest(void);
int semaphore_init(void);
void shared_memory_dest(void);
int shared_memory_init(void);
void customer_process(int idZ, int TZ);
void clerk_process(int idU, int TU);
int random_number(int min, int max);

// Global values
FILE *file;

// Shared memory declaration
bool *post_is_closed;
int *first_service_queue;
int *second_service_queue;
int *third_service_queue;
int *action_number;
int *clerks_number;
int *customers_numbers;


// deklaracia semaforov
sem_t *sem_mutex;
sem_t *sem_first_service;
sem_t *sem_second_service;
sem_t *sem_third_service;
sem_t *sem_clerk;
sem_t *sem_customer;

////////////////////////////    MAIN START  ////////////////////////////
int main(int argc, char *argv[]) {
    // if (argc != 6) {
    //     fprintf(stderr, "Error: Invalid number of arguments.\n");
    //     return 1;
    // }

    // int NZ = atoi(argv[1]); //počet zákazníků
    // int NU = atoi(argv[2]); //počet úředníků
    // int TZ = atoi(argv[3]); 
    // int TU = atoi(argv[4]);
    // int F = atoi(argv[5]);


    int NZ = 30; //počet zákazníků
    int NU = 20; //počet úředníků
    int TZ = 20; 
    int TU = 20;
    int F = 20;

    // Check if input values are within allowed range
    if (NZ < 0 || NU < 0 || TZ < 0 || TZ > 10000 || TU < 0 || TU > 100 || F < 0 || F > 10000) {
        fprintf(stderr, "Error: Invalid input values.\n");
        return 1;
    }
    

    //pid_t wpid;
    //int status = 0;



    // Initialize shared memory and semaphores
    if(shared_memory_init()){
        fprintf(stderr, "Cannot alocate shared memory!\n");
        return 1;
    }

    if(semaphore_init()){
        fprintf(stderr, "Cannot open semaphores!\n");
        return 1;
    }

    // Set output file and output buffer
    if ((file = fopen("proj2.out", "w+")) == NULL) {
        fprintf(stderr, "ERROR: Output file could not be opened\n");
        return 1;
    }
    setbuf(file, NULL);
    setbuf(stdout, NULL);
    //setbuf(stderr, NULL);

    // Fork customer processes
    for(int idZ  = 1; idZ <=NZ;idZ++){
        pid_t pid = fork();
        if(pid==0){
            //printf("CUSTOMER\n");
            srand((int)time(0) % getpid()); // for upsleep_for_random_time(time_max)
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
            //printf("CLERK\n");
            srand((int)time(0) % getpid()); // for upsleep_for_random_time(time_max)
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
    (*post_is_closed) = true;
    fprintf(file, "%d: closing\n", ++(*action_number));
    sem_post(sem_mutex);

    // Clean up shared memory and semaphores
    //while ((wpid = wait(&status)) > 0);
    while(wait(NULL)>0);
    shared_memory_dest();
    semaphore_dest();
    fclose(file);

    return 0;
}
////////////////////////////    MAIN END    ////////////////////////////

// Semaphores destruction
void semaphore_dest(void){
    sem_close(sem_mutex);   
    sem_unlink(SEMAPHORE_MUTEX);

    sem_close(sem_first_service);   
    sem_unlink(SEMAPHORE_SERVICEFRST);

    sem_close(sem_second_service);   
    sem_unlink(SEMAPHORE_SERVICESCND);

    sem_close(sem_third_service);   
    sem_unlink(SEMAPHORE_SERVICEFRST);

    sem_close(sem_clerk);   
    sem_unlink(SEMAPHORE_CLERK);


    sem_close(sem_customer);   
    sem_unlink(SEMAPHORE_CUSTOMER);

    if (file != NULL) fclose(file);

    return;
}

// Semaphores initialization
int semaphore_init(void){
    semaphore_dest();

    sem_mutex = sem_open(SEMAPHORE_MUTEX, O_CREAT | O_EXCL, 0666, 1) ;
    if (sem_mutex == SEM_FAILED){
        return 1;
    }
    sem_first_service = sem_open(SEMAPHORE_SERVICEFRST, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_first_service == SEM_FAILED){
        return 1;
    }

    sem_second_service = sem_open(SEMAPHORE_SERVICESCND, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_second_service == SEM_FAILED){
        return 1;
    }

    sem_third_service = sem_open(SEMAPHORE_SERVICETHRD, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_third_service == SEM_FAILED){
        return 1;
    }

    sem_clerk = sem_open(SEMAPHORE_CLERK, O_CREAT | O_EXCL, 0666, 1) ;
    if (sem_clerk == SEM_FAILED){
        return 1;
    }

    sem_customer = sem_open(SEMAPHORE_CUSTOMER, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_customer == SEM_FAILED){
        return 1;
    }

    return 0;
}

// Shared memory destruction
void shared_memory_dest(void){

    munmap(post_is_closed, sizeof(bool));
    munmap(first_service_queue, sizeof(int));
    munmap(second_service_queue, sizeof(int));
    munmap(third_service_queue, sizeof(int));
    munmap(action_number, sizeof(int));
    munmap(clerks_number, sizeof(int));
    munmap(customers_numbers, sizeof(int));

    return;
}

// Shared memory initialization
int shared_memory_init(void){
    void shared_memory_dest(void);

    post_is_closed = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == post_is_closed) {
        return 1;
    }

    first_service_queue = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == first_service_queue) {
        return 1;
    }
    second_service_queue = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == second_service_queue) {
        return 1;
    }
    third_service_queue = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == third_service_queue) {
        return 1;
    }

    action_number = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == action_number) {
        return 1;
    }


    clerks_number = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == clerks_number) {
        return 1;
    }

    customers_numbers = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == customers_numbers) {
        return 1;
    }

    // inicializacia zdielanych premennych
    *post_is_closed = false;
    *first_service_queue = 0;
    *second_service_queue = 0;
    *third_service_queue = 0;
    *action_number = 0;
    *clerks_number = 0;
    *customers_numbers = 0;

    return 0;
}

// Customer process function
void customer_process(int idZ, int TZ) {
    (*customers_numbers)++;

    sem_wait(sem_mutex);
    // Print the initial message
    fprintf(file, "%d: Z %d: started\n", ++(*action_number), idZ);
    sem_post(sem_mutex);

    // Wait for a random time between 0 and TZ
    upsleep_for_random_time(TZ);

    sem_wait(sem_mutex);
    // Check if the post office is closed
    if (*post_is_closed) {
        fprintf(file, "%d: Z %d: going home\n", ++(*action_number), idZ);
        sem_post(sem_mutex);
        exit(0);
    }

    int service = rand() % NUM_SERVICES;
    fprintf(file, "%d: Z %d: entering office for a service %d\n", ++(*action_number), idZ, service + 1);
    sem_post(sem_mutex);

    switch(service) {
    case 0:
        (*first_service_queue)++;
        sem_wait(sem_first_service);
        break;
    case 1:
        (*second_service_queue)++;
        sem_wait(sem_second_service);
        break;
    case 2:
        (*third_service_queue)++;
        sem_wait(sem_third_service);
        break;
    default:
        printf("SERVICE ERROR\n");
        exit(1);
    }
    
    sem_wait(sem_mutex);
    fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_number), idZ);
    sem_post(sem_mutex);

    upsleep_for_random_time(10);

    sem_wait(sem_mutex);
    fprintf(file, "%d: Z %d: going home\n", ++(*action_number), idZ);
    sem_post(sem_mutex);

    exit(0);
}

// Clerk process function
void clerk_process(int idU, int TU) {
    (*clerks_number)++;

    sem_wait(sem_mutex);
    fprintf(file, "%d: U %d: started\n", ++(*action_number), idU);
    sem_post(sem_mutex);

    while (1) {
        int services_queues[3] = {0, 0, 0};
        int service = -1;

        if (*first_service_queue)
        {   
            ++service;
            services_queues[service] = 1;
            
        }
        if (*second_service_queue)
        {
            ++service;
            services_queues[service] = 2;

        }    
        if (*third_service_queue)
        {
            ++service;
            services_queues[service] = 3;
        }

        if ((service == -1) && (*post_is_closed)) {
            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: going home\n", ++(*action_number), idU);
            sem_post(sem_mutex);
            (*clerks_number)--;
            exit(0);
        } else if (service == -1) {
            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: taking break\n", ++(*action_number), idU);
            sem_post(sem_mutex);

            upsleep_for_random_time(TU);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: break finished\n", ++(*action_number), idU);
            sem_post(sem_mutex);
        } else {
            

        










        }
        ////////////////////////////////

        
        


        //sem_wait(sem_mutex);
        int service = -1;
        for (int i = 0; i < NUM_SERVICES; i++) {
            if ((*customer_services_queue[i]) > 0) {
                service = i;
                //sem_post(sem_mutex);
                break;
            }
        }
        sem_wait(sem_mutex);

        sem_post(sem_mutex);
        if ((service == -1) && ((*post_is_closed) > 0)) {
            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: going home\n", ++(*action_number), idU);
            (*clerks_amount)--;
            sem_post(sem_mutex);
            exit(0);
        } else if (service == -1) {
            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: taking break\n", ++(*action_number), idU);
            sem_post(sem_mutex);

            upsleep_for_random_time(TU);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: break finished\n", ++(*action_number), idU);
            sem_post(sem_mutex);
            //continue;
        } else {

        //sem_post(sem_mutex);
        
        sem_post(sem_customer_services[service]);
        sem_wait(sem_customer_waiting); //waiting for customer to come.


        sem_wait(sem_mutex);
        fprintf(file, "%d: U %d: serving customer at service %d\n", ++(*action_number), idU, service + 1);
        (*customer_services_queue[service])--;
        sem_post(sem_mutex);

        usleep(rand() % 11);


        sem_wait(sem_mutex);
        fprintf(file, "%d: U %d: finished serving customer\n", ++(*action_number), idU);
        sem_post(sem_mutex);
        }
    }
    //return;
}

////////////////////////////    OTHER FUNCTIONS ////////////////////////////
// int random_number(int min, int max)
// {
//     int number = rand() % (max-min+1);
//     return number+min;
// }
