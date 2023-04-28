#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#define SEMAPHORE1_NAME "/xvorob01_client_wait_1"
#define SEMAPHORE2_NAME "/xvorob01_client_wait_2"
#define SEMAPHORE3_NAME "/xvorob01_client_wait_3"
#define SEMAPHORE4_NAME "/xvorob01_print"
#define SEMAPHORE5_NAME "/ xvorob01_client_finish_sem"
#define SEMAPHORE6_NAME "/xvorob01_worker"
#define SEMAPHORE7_NAME "/xvorob01_process_start"


typedef struct {
    int NZ; //number of clients
    int NU; //number of workers
    int TZ; //Time in ms where client waiting to go for post
    int TU; //worker rest, in ms
    int F; //in ms, time after that post office will be closed  
}data_t;
typedef enum {OPEN, CLOSED} PostStatus;


sem_t *client_wait_1;
sem_t *client_wait_2;
sem_t *client_wait_3;
sem_t *worker;
sem_t *print;
sem_t *process_start;
sem_t *client_finish_sem;
PostStatus *post_status;
int *line;
int *clients_finished;
int *NZA;
int *NUR;

FILE *file;
int *arr1;
int *arr2;
int *arr3;

void semaphore_init(void){
    sem_unlink(SEMAPHORE1_NAME);
	sem_unlink(SEMAPHORE2_NAME);
	sem_unlink(SEMAPHORE3_NAME);
	sem_unlink(SEMAPHORE4_NAME);
	sem_unlink(SEMAPHORE5_NAME);
    sem_unlink(SEMAPHORE6_NAME);
    sem_unlink(SEMAPHORE7_NAME);
    //mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    line = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *line = 1;
    NZA = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *NZA = 0;
    NUR = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *NUR = 0;
    clients_finished = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *clients_finished = 0;
    post_status = mmap(NULL, sizeof(PostStatus), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *post_status = OPEN;
    //sem_init(mutex, 1, 1);
    client_wait_1 = sem_open("/xvorob01_client_wait_1", O_CREAT | O_EXCL, 0666, 0);
    client_wait_2 = sem_open("/xvorob01_client_wait_2", O_CREAT | O_EXCL, 0666, 0);
    client_wait_3 = sem_open("/xvorob01_client_wait_3", O_CREAT | O_EXCL, 0666, 0);
    client_finish_sem = sem_open("/xvorob01_client_finish_sem", O_CREAT | O_EXCL, 0666, 0);
    print = sem_open("/xvorob01_print", O_CREAT | O_EXCL, 0666, 1);
    worker = sem_open("/xvorob01_worker", O_CREAT | O_EXCL, 0666, 1);
    process_start = sem_open("/xvorob01_process_start", O_CREAT | O_EXCL, 0666, 1);
}

void cleanup(void){
    sem_close(client_wait_1);
    sem_close(client_wait_2);
    sem_close(client_wait_3);
    sem_unlink("/xvorob01_client_wait_1");
    sem_unlink("/xvorob01_client_wait_2");
    sem_unlink("/xvorob01_client_wait_3");
    sem_close(print);
    sem_unlink("/xvorob01_print");
    sem_close(client_finish_sem);
    sem_unlink("/xvorob01_client_finish_sem");
    sem_close(worker);
    sem_unlink("/xvorob01_worker");
    sem_close(process_start);
    sem_unlink(SEMAPHORE7_NAME);
    munmap(line, sizeof(int));
    if (file != NULL) fclose(file);
    
}

data_t check_arguments(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Wrong number of arguments.\n");
        exit(1);
    }
    char *endptr;
    data_t data;
    data.NZ = strtol(argv[1], &endptr, 10);
    if (errno == EINVAL || *endptr != '\0') {
        printf("Error. The client wait time is not a valid number.\n");
        exit(1);
    }
    data.NU = strtol(argv[2], &endptr, 10);
    if (errno == EINVAL || *endptr != '\0') {
        printf("Error. The client wait time is not a valid number.\n");
        exit(1);
    }
    data.TZ = strtol(argv[3], &endptr, 10);
    if (errno == EINVAL || *endptr != '\0') {
        printf("Error. The client wait time is not a valid number.\n");
        exit(1);
    }
    data.TU = strtol(argv[4], &endptr, 10);
    if (errno == EINVAL || *endptr != '\0') {
        printf("Error. The worker rest time is not a valid number.\n");
        exit(1);
    }
    data.F = strtol(argv[5], &endptr, 10);
    if (errno == EINVAL || *endptr != '\0') {
        printf("Error. The post closed time is not a valid number.\n");
        exit(1);
    }
    if (data.TZ < 0 || data.TZ > 10000) {
        printf("Error. Client waits in period only from 0 to 10000.\n");
        exit(1);
    }
    else if (data.TU < 0 || data.TU > 100) {
        printf("Error. Worker rest in period only from 0 to 100.\n");
        exit(1);
    }
    else if (data.F < 0 || data.F > 10000) {
        printf("Error. Post closed in period only from 0 to 10000.\n");
        exit(1);
    }
    return data;
}

int generate_random_number(int from, int to) {
    // Seed the random number generator with the current time
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand(ts.tv_nsec);
    
    // Generate a random number between `from` and `to`
    return (rand() % (to - from + 1)) + from;
}

void customer(int idZ, int TZ, int NZ, int NU) {
    (*NZA)++;
    sem_wait(print);
    printf("%d: Z %d: started\n", (*line)++, idZ);
    sem_post(print);

    if (*NZA == NZ && *NUR == NU) {
        //sem_post(process_start);
    }
    usleep(generate_random_number(0, TZ)*1000);
    
    int X = generate_random_number(1, 3);
    sem_wait(print);
    if (*post_status == CLOSED) {
        printf("%d: Z %d: going home\n", (*line)++, idZ);
        sem_post(print);
        exit(0);
        }
    printf("%d: Z %d: entering office for a service %d\n", (*line)++, idZ, X);
    sem_post(print);
    //sem_post(process_start);
    if (X == 1) {
        arr1[idZ-1] = idZ;
        (*clients_finished)++;
        sem_wait(client_wait_1);
    }
    else if (X == 2){
        arr2[idZ-1] = idZ;
        (*clients_finished)++;
        sem_wait(client_wait_2);
    }
    else if (X == 3) {
        arr3[idZ-1] = idZ;
        (*clients_finished)++;
        sem_wait(client_wait_3);
    }
    sem_wait(print);
    printf("%d: Z %d: called by office worker\n", (*line)++, idZ);
    sem_post(print);
    
    usleep(generate_random_number(0, 10)*1000);

    sem_wait(print);
    printf("%d: Z %d: going home\n", (*line)++, idZ);
    sem_post(print);
    
    exit(0);
    
}
bool is_array_empty(int *array, int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] != 0) {
            return false;
        }
    }
    return true;
}
int find_first_non_zero(int *array, int size) {
    for (int i = 0; i < size; i++) {
        if (array[i] != 0) {
            return i;
        }
    }
    return -1;
}
void worker_fun(int idU, int TU, int NZ, int NU) {
    (*NUR)++;

    sem_wait(print);
    printf("%d: U %d: started\n", (*line)++, idU);
    sem_post(print);

    if (*NZA == NZ && *NUR == NU) {
        //sem_post(process_start);
    }
    
    while (1) {
        int available_queues = 0;
        int queue_indices[3] = {0};

        if (!is_array_empty(arr1, NZ)) {
            queue_indices[available_queues++] = 1;
        }
        if (!is_array_empty(arr2, NZ)) {
            queue_indices[available_queues++] = 2;
        }
        if (!is_array_empty(arr3, NZ)) {
            queue_indices[available_queues++] = 3;
        }
        //sem_wait(worker);
        if (available_queues == 0) {
            *clients_finished = 0;
        }
        //printf("-----available queues: %d\n", available_queues);
        if (available_queues > 0) {
            int selected_queue = generate_random_number(0, available_queues-1);
            available_queues = 0;
            int task_number = 0;
            switch (queue_indices[selected_queue]) {
            case 1:
                task_number = 1;
                sem_wait(worker);
                int client1 = find_first_non_zero(arr1, NZ);
                if (client1 >= 0) {
                    arr1[client1] = 0;
                    (*clients_finished)--;
                    sem_post(client_wait_1);
                }
                sem_post(worker);
                break;
            case 2:
                
                task_number = 2;
                sem_wait(worker);
                int client2 = find_first_non_zero(arr2, NZ);
                if (client2 >= 0) {
                    arr2[client2] = 0;
                    (*clients_finished)--;
                    sem_post(client_wait_2);
                }
                sem_post(worker);
                break;
            case 3:
                
                task_number = 3;
                sem_wait(worker);
                int client3 = find_first_non_zero(arr3, NZ);
                if (client3 >= 0) {
                    arr3[client3] = 0;
                    (*clients_finished)--;
                    sem_post(client_wait_3);
                }
                sem_post(worker);
                break;
            }
            available_queues = 0;
    
            sem_wait(print);
            printf("%d: U %d: serving a service of type %d\n", (*line)++, idU, task_number);
            sem_post(print);
            usleep(generate_random_number(0, 10)*1000);
            sem_wait(print);
            printf("%d: U %d: service finished\n", (*line)++, idU);
            sem_post(print);
            //sem_post(worker);
            
        }
        sem_post(worker);
        sem_wait(print); // Acquire print semaphore

        // Check if all arrays are empty
        //printf("-----clients in wait: %d\n", *clients_finished);
        if (*clients_finished <= 0) {
            if (*post_status == CLOSED) {
                sem_wait(client_finish_sem);
                //sem_wait(print);
                printf("%d: U %d: going home\n", (*line)++, idU);
                sem_post(print); // Release print semaphore
                sem_post(client_finish_sem);
                exit(0);
            }
            //sem_wait(print);
            //printf("-----clients in wait2: %d\n", *clients_finished);
            if (*clients_finished == 0) {
                printf("%d: U %d: taking break\n", (*line)++, idU);
                sem_post(print); // Release print semaphore
                usleep(generate_random_number(0, TU)*1000);
                sem_wait(print); // Acquire print semaphore
                printf("%d: U %d: break finished\n", (*line)++, idU);
                sem_post(print);
            }
            else {
                sem_post(print);
                continue;  
                                }
        }
        sem_post(print); // Release print semaphore
    }
}

int main(int argc, char *argv[])
 {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    data_t data = check_arguments(argc, argv);
    if ((file = fopen("proj2.out", "w+")) == NULL) {
            fprintf(stderr, "ERROR: File could not be opened\n");
            return 1;
        }
    arr1 = mmap(NULL, data.NZ * sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (arr1 == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }
    arr2 = mmap(NULL, data.NZ * sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (arr2 == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }
    arr3 = mmap(NULL, data.NZ * sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (arr3 == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }
    memset(arr1, 0, data.NZ * sizeof(int));
    memset(arr2, 0, data.NZ * sizeof(int));
    memset(arr3, 0, data.NZ * sizeof(int));
    setbuf(file, NULL);
    semaphore_init();

    for (int i = 1; i <= data.NZ; i++) {
        pid_t client_pr = fork();
        //if(client_pr < 0){
         //   fprintf(stderr, "ERROR: Fork failure\n");
        //    cleanup();
         //   return 1;
        //}
        if (client_pr == 0) {
            customer(i, data.TZ, data.NZ, data.NU);
            exit(0);
        }
    }
    for (int i = 1; i <= data.NU; i++) {
        pid_t worker_pr = fork();
        //if(worker_pr < 0){
        //    fprintf(stderr, "ERROR: Fork failure2\n");
            
        //    cleanup();
        //    return 1;
       //}
       
        if (worker_pr == 0) {
            worker_fun(i, data.TU, data.NZ, data.NU);
            exit(0);
        }
    }
    //sem_wait(process_start);
    usleep(generate_random_number(data.F / 2, data.F)*1000);
    
    sem_wait(print);
    *post_status = CLOSED;
    printf("%d: closing\n", (*line)++);
    sem_post(print);
    //sem_post(process_start);
    sem_post(client_finish_sem);
    while(wait(NULL) > 0);
    
    cleanup();
    munmap(arr1, data.NZ * sizeof(int));
    munmap(arr2, data.NZ * sizeof(int));
    munmap(arr3, data.NZ * sizeof(int));
    
    return 0;

 }
