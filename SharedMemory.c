#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdlib.h>

char shm_fn[] = "my_shm";
char sem_fn[] = "my_sem";

/**** WRITER ****/

int main() {
    caddr_t shmptr;
    unsigned int mode;
    int shmdes, index;
    sem_t* sem_des;
    int SHM_SIZE;
    int a;

    mode = S_IRWXU | S_IRWXG;

    /* Open the shared memory object */

    if ((shmdes = shm_open(shm_fn, O_CREAT | O_RDWR | O_TRUNC, mode)) == -1) {
        perror("shm_open failure");
        exit(0);
    }

    /* Preallocate a shared memory area */

    SHM_SIZE = sysconf(_SC_PAGE_SIZE);

    if (ftruncate(shmdes, SHM_SIZE) == -1) {
        perror("ftruncate failure");
        exit(0);
    }

    if ((shmptr = mmap(0, SHM_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED,
        shmdes, 0)) == (caddr_t)-1) {
        perror("mmap failure");
        exit(0);
    }

    /* Create a semaphore in locked state */

    sem_des = sem_open(sem_fn, O_CREAT, 0644, 0);

    if (sem_des == (void*)-1) {
        perror("sem_open failure");
        exit(0);
    }

    /* Access to the shared memory area */

    for (index = 1; index <= 1000; index++) {
        a = rand() % 2 + 1;

        if (a == 2) {
            printf("write %d into the shared memory shmptr[%d]\n", index, index);
            shmptr[index] = index;
        }
    }

    /* Release the semaphore lock */

    sem_post(sem_des);
    munmap(shmptr, SHM_SIZE);

    /* Close the shared memory object */

    close(shmdes);

    /* Close the Semaphore */

    sem_close(sem_des);

    /* Delete the shared memory object */

    shm_unlink(shm_fn);
}