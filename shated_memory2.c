#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
typedef struct {
    int action_counter;
    int activity_queues[3];
    int post_office_closed;
} SharedMemory;
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
int main() {
    const char *shm_name = "/proj2_shm";
    SharedMemory *shared_memory = init_shared_memory(shm_name);
    if (shared_memory == NULL) {
        return 1;
    }

    // ... your code here ...

    // Cleanup
    if (munmap(shared_memory, sizeof(SharedMemory)) == -1) {
        perror("munmap");
    }

    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
    }

    return 0;
}
