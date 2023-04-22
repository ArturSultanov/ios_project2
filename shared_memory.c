#include <fcntl.h>    // For O_* constants
#include <sys/mman.h> // For mmap and munmap
#include <unistd.h>   // For ftruncate and close
#include <stdbool.h>  // For bool type

typedef struct shared
{
    int proc_count;
    int elves;
    int reindeers;
    bool workshop_closed;
} shared_t;

typedef struct shared_mem
{
    shared_t *ptr;
    int fd;
} shared_mem_t;

shared_mem_t initialize_shared()
{
    shared_mem_t local_sh_mem;
    
    local_sh_mem.fd = shm_open("/shared_vars", O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(local_sh_mem.fd, sizeof(shared_t));
    local_sh_mem.ptr = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE, MAP_SHARED, local_sh_mem.fd, 0);
    
    local_sh_mem.ptr->proc_count = 0;
    local_sh_mem.ptr->elves = 0;
    local_sh_mem.ptr->reindeers = 0;
    local_sh_mem.ptr->workshop_closed = false;

    return local_sh_mem;
}

void cleanup_shared(shared_mem_t local_sh_mem)
{
    // Unmap the shared memory
    munmap(local_sh_mem.ptr, sizeof(shared_t));

    // Close the file descriptor
    close(local_sh_mem.fd);

    // Unlink the shared memory object
    shm_unlink("/shared_vars");
}

int main()
{
    // Initialize the shared memory
    shared_mem_t sh_mem = initialize_shared();

    // Perform operations using the shared memory (sh_mem.ptr)
    // ...

    // Clean up the shared memory when no longer needed
    cleanup_shared(sh_mem);

    return 0;
}
