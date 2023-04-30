// Author:  Artur Sultanov
// xlogin:  xsulta01
// Date:    Aprill 2023
// Task:    IOS project 2

#include "proj2_header.h"

// Global values declaration
FILE *file;
pid_t *child_processes;
int child_count;

int NZ; // Number of customers
int NU; // Number of clerks
int TZ; // Maximum time (in milliseconds) of customer waiting before entering post office for a service. 
int TU; // Maximum time of clerk's break (in milliseconds).
int F;  // Maximum time (in milliseconds) before post office would be closed for new customers.

// Semaphores declaration
sem_t *sem_mutex;
sem_t *sem_first_service;
sem_t *sem_second_service;
sem_t *sem_third_service;
sem_t *sem_clerk;

// Shared memory declaration
bool *post_is_closed = NULL;
int *first_service_queue = NULL;
int *second_service_queue = NULL;
int *third_service_queue = NULL;
int *action_number = NULL;

// Kill all Child-processes were created by Main-process.
void kill_child_processes(void) {
    for (int i = 0; i < child_count; i++) {
        kill(child_processes[i], SIGTERM);
    }
}

// Semaphores initialization(opening) function.
int semaphore_init(void){
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

    return 0;
}

// Semaphores destruction(closing, unlinking) function.
void semaphore_dest(void){
    sem_close(sem_mutex);   
    sem_unlink(SEMAPHORE_MUTEX);

    sem_close(sem_first_service);   
    sem_unlink(SEMAPHORE_SERVICEFRST);

    sem_close(sem_second_service);   
    sem_unlink(SEMAPHORE_SERVICESCND);

    sem_close(sem_third_service);   
    sem_unlink(SEMAPHORE_SERVICETHRD);

    sem_close(sem_clerk);   
    sem_unlink(SEMAPHORE_CLERK);
}

// Shared memory initialization(mapping) function.
int shared_memory_init(void){
    post_is_closed = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == post_is_closed) {
        return 1;
    }

    first_service_queue = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == first_service_queue) {
        return 1;
    }
    second_service_queue = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == second_service_queue) {
        return 1;
    }
    third_service_queue = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == third_service_queue) {
        return 1;
    }

    action_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == action_number) {
        return 1;
    }

    // Initialization of shared variables.
    *post_is_closed = 0;
    *first_service_queue = 0;
    *second_service_queue = 0;
    *third_service_queue = 0;
    *action_number = 0;

    return 0;
}

// Shared memory destruction(unmapping) function.
int shared_memory_dest(void){
    if(munmap(post_is_closed, sizeof(bool))){
        return 1;
    }
    if(munmap(first_service_queue, sizeof(int))){
        return 1;
    }
    if (munmap(second_service_queue, sizeof(int))){
        return 1;
    }
    if(munmap(third_service_queue, sizeof(int))){
        return 1;
    }
    if(munmap(action_number, sizeof(int))){
        return 1;
    }

    return 0;
}

// Semaphores and shared variables destruction, closing output *file, if it wasn't.
void cleanup(void){
    if (file != NULL) fclose(file);
    shared_memory_dest();
    semaphore_dest();
}

int check_input_arguments(int argc, char *argv[]) {

    if (argc != 6) { // Check if the number og arguments is correct
        fprintf(stderr, "Error: Invalid number of arguments. Please use 5 argumetns: ./proj2 NZ NU TZ TU F. \n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        for (int j = 0; argv[i][j] != '\0'; j++) {
            if (!isdigit(argv[i][j])) {
                fprintf(stderr, "Error: Invalid type of arguments. Please use numerical-type argunemts.\n");
                return 1;
            }
        }
    }

    NZ = atoi(argv[1]); // Number of customers
    NU = atoi(argv[2]); // Number of clerks
    TZ = atoi(argv[3]); // Maximum time (in milliseconds) of customer waiting before entering post office for a service. 
    TU = atoi(argv[4]); // Maximum time of clerk's break (in milliseconds).
    F = atoi(argv[5]);  // Maximum time (in milliseconds) before post office would be closed for new customers.

    // Check if input values are within allowed range.
    if (NZ < 0 || NU <= 0 || TZ < 0 || TZ > 10000 || TU < 0 || TU > 100 || F < 0 || F > 10000) {
        fprintf(stderr, "Error: Invalid input values.\n");
        return 1;
    } 
    return 0;
}

// Customer-process logic.
void customer_process(int idZ, int TZ) {
    sem_wait(sem_mutex);
    fprintf(file, "%d: Z %d: started\n", ++(*action_number), idZ); // Print the initial message
    sem_post(sem_mutex);

    upsleep_for_random_time(TZ); // Wait for a random time between 0 and TZ

    if (*post_is_closed) { // Check if the post office is closed.
        sem_wait(sem_mutex);
        fprintf(file, "%d: Z %d: going home\n", ++(*action_number), idZ); // If it's closed, customer will go home.
        sem_post(sem_mutex);
        exit(0);
    }

    int service = (rand() % NUM_SERVICES) + 1; // Customer chooses a random service in range from 1 to 3 (inclided).
    sem_wait(sem_mutex);
    fprintf(file, "%d: Z %d: entering office for a service %d\n", ++(*action_number), idZ, service);
    sem_post(sem_mutex);

    switch(service) { //Customer si waiting for service in appropriate service queue.
    case 1:
        (*first_service_queue)++;
        sem_wait(sem_first_service);
        break;
    case 2:
        (*second_service_queue)++;
        sem_wait(sem_second_service);
        break;
    case 3:
        (*third_service_queue)++;
        sem_wait(sem_third_service);
        break;
    default:
        fprintf(stderr, "Error: customer function service error.\n");
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

// Clerk-process logic.
void clerk_process(int idU, int TU) {
    sem_wait(sem_mutex);
    fprintf(file, "%d: U %d: started\n", ++(*action_number), idU);  // Print the initial message
    sem_post(sem_mutex);

    while (1) {
        sem_wait(sem_clerk); // Semaphore to prevent changing of variables value caused by another clerk process.

        int service_type[3] = {0};  // Array is needed to store info about how many and what kind of services are occupied by customers at the moment.
        int  occupied = 0;          // Number of occupited services.

        if (*first_service_queue)   // If there is any customer in service_1 queue 
        {   
            service_type[occupied] = 1;
            occupied++;
        }
        if (*second_service_queue)  // If there is any customer in service_2 queue 
        {
            service_type[occupied] = 2;
            occupied++;
        }    
        if (*third_service_queue)   // If there is any customer in service_2 queue 
        {
            service_type[occupied] = 3;
            occupied++;
        }

        if ((occupied == 0) && (*post_is_closed)) { // If there are no customers in the queues and post office is closed,
            sem_post(sem_clerk);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: going home\n", ++(*action_number), idU); // clerk will home.
            sem_post(sem_mutex);

            exit(0);
        } else if (occupied == 0) { // If there are no customers in the queues, but post office is still open,

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: taking break\n", ++(*action_number), idU);
            sem_post(sem_mutex);

            upsleep_for_random_time(TU); // clerk will take a break for random time in range from 0 to TU(inclusive).
            sem_post(sem_clerk);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: break finished\n", ++(*action_number), idU);
            sem_post(sem_mutex);
        } else if (occupied > 0) { // If there is any customer in any queue,

            int service = (rand() % occupied); // clerk will choose random service from number of occupied services.

            switch (service_type[service])
            {
            case 1:
                (*first_service_queue)--;
                sem_post(sem_first_service);
                break;
            case 2:
                (*second_service_queue)--;
                sem_post(sem_second_service);
                break;
            case 3:
                (*third_service_queue)--;
                sem_post(sem_third_service);
                break;
            default:
                fprintf(stderr,"Error; error in clerk function - service switch. Unavailable service type: %d\n", service_type[service]);
                sem_post(sem_clerk);
                exit(1);
                break;
            }            
            sem_post(sem_clerk);
        
            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: serving a service of type %d\n", ++(*action_number), idU, service_type[service]);
            sem_post(sem_mutex);
        
            usleep(rand() % 11); 

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: service finished\n", ++(*action_number), idU);
            sem_post(sem_mutex);

        } else if (occupied < 0){
            fprintf(stderr, "Error: clerk function service error");
            sem_post(sem_clerk);
            exit(1);
        }
    }
}

// Main function.
int main(int argc, char *argv[]) {
    if (check_input_arguments(argc, argv)) {
        return 1;
    }

    pid_t wpid;    
    int status = 0;

    // Set output file and output buffer.
    if (file != NULL) {
        fclose(file);
    }
    if ((file = fopen("proj2.out", "w+")) == NULL) {
        fprintf(stderr, "Error: Output file could not be opened\n");
        return 1;
    }
    setbuf(file, NULL);

    // Initialize shared memory
    if(shared_memory_dest()){
        fprintf(stderr, "Error: Can't dealocate needed shared memory at the start of program.\n");
        return 1;
    }
    if(shared_memory_init()){
        fprintf(stderr, "Error: Cannot alocate shared memory!\n");
        fclose(file);
        return 1;
    }
    // Initialize semaphores
    semaphore_dest();

    if(semaphore_init()){
        fprintf(stderr, "Error: Cannot open semaphores.\n");
        fclose(file);
        semaphore_dest();
        return 1;
    }

    // kill_child_processes() function needed values.
    child_processes = (pid_t *)malloc((NZ + NU) * sizeof(pid_t));
    child_count = 0;
 
    // Fork customer processes
    for(int idZ  = 1; idZ <=NZ;idZ++){  // Creation of NZ customer processes.
        pid_t customer_pid = fork();
        
        if(customer_pid == 0){
            srand((int)time(0) % getpid()); // To get random value for each process when using upsleep_for_random_time(time_max).
            customer_process(idZ, TZ);
            exit(0);
        }
        else if (customer_pid == -1){
            fprintf(stderr, "Error: Fork customer processes error.\n");
            cleanup();

            kill_child_processes();
            free(child_processes);
            exit(1);
        } else {
            child_processes[child_count++] = customer_pid;
        }
    }

    // Fork clerk processes
    for(int idU  = 1; idU <=NU;idU++){  /// Creation of NU clerks processes.
        pid_t clerk_pid = fork();

        if(clerk_pid == 0){
            srand((int)time(0) % getpid()); // To get random value for each process when using upsleep_for_random_time(time_max).
            clerk_process(idU, TU);
            exit(0);
        }
        else if (clerk_pid == -1){
            fprintf(stderr, "Error: Fork clerk processes error.\n");            
            cleanup();
            
            kill_child_processes();
            free(child_processes);
            exit(1);
        } else {
            child_processes[child_count++] = clerk_pid;
        }
    }

    usleep((rand() % ((F / 2) + 1)) * 1000 + F / 2 * 1000); // Waiting for post office to close.
    (*post_is_closed) = 1;

    //Closing of the post office.
    sem_wait(sem_mutex);
    fprintf(file, "%d: closing\n", ++(*action_number));
    sem_post(sem_mutex);

    while ((wpid = wait(&status)) > 0); // Main-process waiting for all Child-processes to terminate.

    // Clean up memory and semaphores
    cleanup();
    free(child_processes);

    return 0;
}
