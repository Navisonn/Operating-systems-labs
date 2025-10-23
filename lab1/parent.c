#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main() {
    char *file1 = NULL;
    char *file2 = NULL;
    size_t len = 0;
    if (getline(&file1, &len, stdin) == -1) {
        if (errno != 0) perror("getline file1");
        exit(1);
    }
    file1[strcspn(file1, "\n")] = '\0';
    len = 0;
    if (getline(&file2, &len, stdin) == -1) {
        if (errno != 0) perror("getline file2");
        free(file1);
        exit(1);
    }
    file2[strcspn(file2, "\n")] = '\0';
    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) == -1) {
        perror("pipe1");
        free(file1);
        free(file2);
        exit(1);
    }
    if (pipe(pipe2) == -1) {
        perror("pipe2");
        free(file1);
        free(file2);
        exit(1);
    }
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork1");
        free(file1);
        free(file2);
        exit(1);
    }
    if (pid1 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        char fd_str[10];
        sprintf(fd_str, "%d", pipe1[0]);
        execl("./child", "child", file1, fd_str, (char *)NULL);
        perror("execl child1");
        exit(1);
    }

    close(pipe1[0]);
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork2");
        free(file1);
        free(file2);
        exit(1);
    }
    if (pid2 == 0) {
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);
        char fd_str[10];
        sprintf(fd_str, "%d", pipe2[0]);
        execl("./child", "child", file2, fd_str, (char *)NULL);
        perror("execl child2");
        exit(1);
    }

    close(pipe2[0]);
    char *line = NULL;
    size_t line_len = 0;
    int line_num = 1;
    ssize_t read_bytes;
    while ((read_bytes = getline(&line, &line_len, stdin)) != -1) {
        int pipe_fd = (line_num % 2 == 1) ? pipe1[1] : pipe2[1];
        ssize_t written = write(pipe_fd, line, read_bytes);
        if (written != read_bytes) {
            perror("write to pipe incomplete");
            free(line);
            free(file1);
            free(file2);
            exit(1);
        }
        line_num++;
    }
    if (errno != 0) {
        perror("getline input");
    }
    free(line);
    close(pipe1[1]);
    close(pipe2[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    free(file1);
    free(file2);
    return 0;
}
