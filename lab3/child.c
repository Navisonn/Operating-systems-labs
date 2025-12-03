#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <ctype.h>

#define SHM_SIZE 4096

struct shm_block {
    size_t len;
    char buffer[SHM_SIZE];
};

int is_vowel(char c) {
    c = tolower(c);
    return (c=='a'||c=='e'||c=='i'||c=='o'||c=='u');
}

char* remove_vowels(const char *str) {
    size_t len = strlen(str);
    char *res = malloc(len+1);
    char *p = res;
    while (*str) {
        if (!is_vowel(*str))
            *p++ = *str;
        str++;
    }
    *p = '\0';
    return res;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: child <filename> <shm_name> <sem_base>\n");
        return 1;
    }

    char *filename = argv[1];
    char *shm_name = argv[2];
    char *sem_base = argv[3];

    char sem_parent_name[256], sem_child_name[256];
    sprintf(sem_parent_name, "/%s_parent", sem_base);
    sprintf(sem_child_name,  "/%s_child",  sem_base);

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        return 1;
    }

    struct shm_block *shm = mmap(NULL, sizeof(struct shm_block),
                                 PROT_READ | PROT_WRITE,
                                 MAP_SHARED, shm_fd, 0);

    if (shm == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    sem_t *sem_parent = sem_open(sem_parent_name, 0);
    sem_t *sem_child  = sem_open(sem_child_name, 0);

    FILE *out = fopen(filename, "w");
    if (!out) {
        perror("fopen");
        return 1;
    }

    while (1) {
        sem_wait(sem_child);

        if (shm->len == 0) {
            break;
        }

        char *processed = remove_vowels(shm->buffer);
        char output[SHM_SIZE * 2];
        snprintf(output, sizeof(output), "Child processing: %s\n", processed);
        fputs(output, stdout);
        fflush(stdout);
        
        fprintf(out, "%s\n", processed);
        fflush(out);
        free(processed);

        sem_post(sem_parent);
    }

    fclose(out);
    munmap(shm, sizeof(struct shm_block));
    sem_close(sem_parent);
    sem_close(sem_child);
    
    return 0;
}
