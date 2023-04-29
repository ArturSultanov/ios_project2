// Artur Sultanov
// xsulta01
// IOS project 2 2023
#include "proj2_header.h"

// Global values declaration
FILE *file;
pid_t *child_processes;
int child_count;

// Semaphores declaration
sem_t *sem_mutex;
sem_t *sem_first_service;
sem_t *sem_second_service;
sem_t *sem_third_service;
sem_t *sem_clerk;
sem_t *sem_customer;

// Shared memory declaration
bool *post_is_closed = NULL;
int *first_service_queue = NULL;
int *second_service_queue = NULL;
int *third_service_queue = NULL;
int *action_number = NULL;
int *clerks_number = NULL;
int *customers_numbers = NULL;

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
        printf("mutex_sem_faidled");
        return 1;
    }
    sem_first_service = sem_open(SEMAPHORE_SERVICEFRST, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_first_service == SEM_FAILED){
        printf("frst_sem_faidled");
        return 1;
    }

    sem_second_service = sem_open(SEMAPHORE_SERVICESCND, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_second_service == SEM_FAILED){
        printf("mutex_sem_faidled");
        return 1;
    }

    sem_third_service = sem_open(SEMAPHORE_SERVICETHRD, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_third_service == SEM_FAILED){
        printf("thrd_sem_faidled");
        return 1;
    }

    sem_clerk = sem_open(SEMAPHORE_CLERK, O_CREAT | O_EXCL, 0666, 1) ;
    if (sem_clerk == SEM_FAILED){
        printf("clekr_sem_faidled");
        return 1;
    }

    sem_customer = sem_open(SEMAPHORE_CUSTOMER, O_CREAT | O_EXCL, 0666, 0) ;
    if (sem_customer == SEM_FAILED){
        printf("customer_sem_faidled");
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

    sem_close(sem_customer);   
    sem_unlink(SEMAPHORE_CUSTOMER);
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


    clerks_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == clerks_number) {
        return 1;
    }

    customers_numbers = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( MAP_FAILED == customers_numbers) {
        return 1;
    }

    // Initialization of shared variables.
    *post_is_closed = 0;
    *first_service_queue = 0;
    *second_service_queue = 0;
    *third_service_queue = 0;
    *action_number = 0;
    *clerks_number = 0;
    *customers_numbers = 0;

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
    if(munmap(clerks_number, sizeof(int))){
        return 1;
    }
    if(munmap(customers_numbers, sizeof(int))){
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

// Customer-process logic.
void customer_process(int idZ, int TZ) {
    (*customers_numbers)++;

    sem_wait(sem_mutex);
    // Print the initial message
    fprintf(file, "%d: Z %d: started\n", ++(*action_number), idZ);
    sem_post(sem_mutex);

    // Wait for a random time between 0 and TZ
    upsleep_for_random_time(TZ);

    // Check if the post office is closed
    if ((*post_is_closed)>0) {
        sem_wait(sem_mutex);
        fprintf(file, "%d: Z %d: going home\n", ++(*action_number), idZ);
        (*customers_numbers)--;
        sem_post(sem_mutex);
        //sem_post(sem_customer);
        exit(0);
    }

    int service = (rand() % NUM_SERVICES) + 1;
    sem_wait(sem_mutex);
    fprintf(file, "%d: Z %d: entering office for a service %d\n", ++(*action_number), idZ, service);
    sem_post(sem_mutex);

    switch(service) {
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
        printf("CUSTOMER FUNCTION SERVICE ERROR\n");
        exit(1);
    }
    
    sem_wait(sem_mutex);
    fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_number), idZ);
    sem_post(sem_mutex);

    upsleep_for_random_time(10);

    sem_wait(sem_mutex);
    fprintf(file, "%d: Z %d: going home\n", ++(*action_number), idZ);
    (*customers_numbers)--;
    sem_post(sem_mutex);

    exit(0);
}

// Clerk-process logic.
void clerk_process(int idU, int TU) {
    (*clerks_number)++;

    sem_wait(sem_mutex);
    fprintf(file, "%d: U %d: started\n", ++(*action_number), idU);
    sem_post(sem_mutex);

    while (1) {
        sem_wait(sem_clerk); 

        int service_type[3] = {0};
        int  occupied = 0; //number of occupited services

        if ((*first_service_queue) > 0)
        {   
            service_type[occupied] = 1;
            occupied++;
        }
        if ((*second_service_queue) > 0)
        {
            service_type[occupied] = 2;
            occupied++;
        }    
        if ((*third_service_queue) > 0)
        {
            service_type[occupied] = 3;
            occupied++;
        }

        if ((occupied == 0) && (*post_is_closed)) {
            sem_post(sem_clerk);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: going home\n", ++(*action_number), idU);
            sem_post(sem_mutex);
            (*clerks_number)--;

            exit(0);
        } else if (occupied == 0) {
            sem_post(sem_clerk);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: taking break\n", ++(*action_number), idU);
            sem_post(sem_mutex);

            upsleep_for_random_time(TU);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: break finished\n", ++(*action_number), idU);
            sem_post(sem_mutex);
        } else if (occupied > 0) {

            int service = (rand() % occupied);

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
                printf("ERROR IN CLERK FUNCTION - SERVICE; %d\n", service_type[service]);
                exit(1);
                break;
            }            
            sem_post(sem_clerk);
        
            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: serving customer at service %d\n", ++(*action_number), idU, service_type[service]);
            sem_post(sem_mutex);
        
            usleep(rand() % 11);

            sem_wait(sem_mutex);
            fprintf(file, "%d: U %d: finished serving customer\n", ++(*action_number), idU);
            sem_post(sem_mutex);

        } else {

        }
    }
}

// Main function.
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
    if (file != NULL) {
        fclose(file);
    }
    if ((file = fopen("proj2.out", "w+")) == NULL) {
        fprintf(stderr, "ERROR: Output file could not be opened\n");
        return 1;
    }
    setbuf(file, NULL);

    // Initialize shared memory
    if(shared_memory_dest()){
        fprintf(stderr, "Error: Can't dealocate needed shared memory at the start of program.\n");
        return 1;
    }
    if(shared_memory_init()){
        fprintf(stderr, "Cannot alocate shared memory!\n");
        fclose(file);
        return 1;
    }
    // Initialize semaphores
    semaphore_dest();

    if(semaphore_init()){
        fprintf(stderr, "Cannot open semaphores!\n");
        fclose(file);
        semaphore_dest();
        return 1;
    }

    // kill_child_processes() needed values 
    child_processes = (pid_t *)malloc((NZ + NU) * sizeof(pid_t));
    child_count = 0;
 
    // Fork customer processes
    for(int idZ  = 1; idZ <=NZ;idZ++){
        pid_t customer_pid = fork();
        
        if(customer_pid == 0){
            srand((int)time(0) % getpid()); // To get random value using upsleep_for_random_time(time_max)
            customer_process(idZ, TZ);
            exit(0);
        }
        else if (customer_pid == -1){
            fprintf(stderr, "Fork customer processes error!\n");
            cleanup();

            kill_child_processes();
            free(child_processes);
            exit(1);
        } else {
            child_processes[child_count++] = customer_pid;
        }
    }




    // Fork clerk processes
    for(int idU  = 1; idU <=NU;idU++){
        pid_t clerk_pid = fork();

        if(clerk_pid == 0){
            srand((int)time(0) % getpid()); // To get random value using upsleep_for_random_time(time_max)
            clerk_process(idU, TU);
            exit(0);
        }
        else if (clerk_pid == -1){
            fprintf(stderr, "Fork clerk processes error!\n");            
            cleanup();
            
            kill_child_processes();
            free(child_processes);
            exit(1);
        } else {
            child_processes[child_count++] = clerk_pid;
        }
    }

    usleep((rand() % ((F / 2) + 1)) * 1000 + F / 2 * 1000);
    (*post_is_closed) = 1;

   //Close the post office
    sem_wait(sem_mutex);
    fprintf(file, "%d: closing\n", ++(*action_number));
    sem_post(sem_mutex);

    while ((wpid = wait(&status)) > 0); // Main-process waiting for all Child-processes to terminate.

    // Clean up shared memory and semaphores
    cleanup();
    free(child_processes);

    return 0;
}

