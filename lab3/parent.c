#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>

#define SHM_SIZE 4096

struct shm_block {
    size_t len;
    char buffer[SHM_SIZE];
};

void random_name(char *buf, const char *prefix) {
    sprintf(buf, "/%s_%d_%ld", prefix, getpid(), random());
}

int main() {
    srandom(time(NULL));

    char *file1 = NULL;
    char *file2 = NULL;
    size_t len = 0;

    printf("Enter first output filename: ");
    fflush(stdout);
    if (getline(&file1, &len, stdin) == -1) {
        perror("getline file1");
        return 1;
    }
    file1[strcspn(file1, "\n")] = 0;

    printf("Enter second output filename: ");
    fflush(stdout);
    if (getline(&file2, &len, stdin) == -1) {
        perror("getline file2");
        free(file1);
        return 1;
    }
    file2[strcspn(file2, "\n")] = 0;

    char shm1_name[128], shm2_name[128];
    char sem1_base[128], sem2_base[128];

    random_name(shm1_name, "shm1");
    random_name(shm2_name, "shm2");
    random_name(sem1_base, "sem1");
    random_name(sem2_base, "sem2");

    int shm1_fd = shm_open(shm1_name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm1_fd, sizeof(struct shm_block));
    struct shm_block *shm1 = mmap(NULL, sizeof(struct shm_block),
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED, shm1_fd, 0);

    char sem1_parent_name[256], sem1_child_name[256];
    sprintf(sem1_parent_name, "/%s_parent", sem1_base);
    sprintf(sem1_child_name,  "/%s_child",  sem1_base);

    sem_t *sem1_parent = sem_open(sem1_parent_name, O_CREAT, 0666, 1);
    sem_t *sem1_child  = sem_open(sem1_child_name,  O_CREAT, 0666, 0);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        execl("./child", "child", file1, shm1_name, sem1_base, NULL);
        perror("execl child1");
        exit(1);
    }

    int shm2_fd = shm_open(shm2_name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm2_fd, sizeof(struct shm_block));
    struct shm_block *shm2 = mmap(NULL, sizeof(struct shm_block),
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED, shm2_fd, 0);

    char sem2_parent_name[256], sem2_child_name[256];
    sprintf(sem2_parent_name, "/%s_parent", sem2_base);
    sprintf(sem2_child_name,  "/%s_child",  sem2_base);

    sem_t *sem2_parent = sem_open(sem2_parent_name, O_CREAT, 0666, 1);
    sem_t *sem2_child  = sem_open(sem2_child_name,  O_CREAT, 0666, 0);

    pid_t pid2 = fork();
    if (pid2 == 0) {
        execl("./child", "child", file2, shm2_name, sem2_base, NULL);
        perror("execl child2");
        exit(1);
    }
    printf("Enter lines to process (Ctrl+D to finish):\n");
    fflush(stdout);
    char *line = NULL;
    size_t line_len = 0;
    int line_num = 1;

    while (getline(&line, &line_len, stdin) != -1) {
        line[strcspn(line, "\n")] = 0;

        if (line_num % 2 == 1) { 
            sem_wait(sem1_parent);
            shm1->len = strlen(line);
            strncpy(shm1->buffer, line, SHM_SIZE - 1);
            shm1->buffer[SHM_SIZE - 1] = '\0';
            sem_post(sem1_child);
        } else { 
            sem_wait(sem2_parent);
            shm2->len = strlen(line);
            strncpy(shm2->buffer, line, SHM_SIZE - 1);
            shm2->buffer[SHM_SIZE - 1] = '\0';
            sem_post(sem2_child);
        }

        line_num++;
    }

    free(line);
    free(file1);
    free(file2);
    sem_wait(sem1_parent);
    shm1->len = 0;
    sem_post(sem1_child);

    sem_wait(sem2_parent);
    shm2->len = 0;
    sem_post(sem2_child);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    munmap(shm1, sizeof(struct shm_block));
    munmap(shm2, sizeof(struct shm_block));
    close(shm1_fd);
    close(shm2_fd);
    shm_unlink(shm1_name);
    shm_unlink(shm2_name);
    
    sem_close(sem1_parent);
    sem_close(sem1_child);
    sem_close(sem2_parent);
    sem_close(sem2_child);
    sem_unlink(sem1_parent_name);
    sem_unlink(sem1_child_name);
    sem_unlink(sem2_parent_name);
    sem_unlink(sem2_child_name);

    return 0;
}
