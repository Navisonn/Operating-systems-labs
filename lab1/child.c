#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

int is_vowel(char c) {
    c = tolower(c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

char *remove_vowels(const char *str) {
    size_t len = strlen(str);
    char *result = malloc(len + 1);
    if (!result) {
        return NULL;
    }
    char *p = result;
    while (*str) {
        if (!is_vowel(*str)) {
            *p++ = *str;
        }
        str++;
    }
    *p = '\0';
    return result;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        const char *error_msg = "Usage: child <filename> <fd>\n";
        write(STDERR_FILENO, error_msg, strlen(error_msg));
        return 1;
    }

    char *filename = argv[1];
    int fd = atoi(argv[2]);

    int out_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("open");
        return 1;
    }

    char buffer[4096];
    ssize_t bytes_read;
    char line[4096];
    int line_pos = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || line_pos >= sizeof(line) - 1) {
                line[line_pos] = '\0';
                
                if (strlen(line) > 0) {
                    char *processed = remove_vowels(line);
                    if (!processed) {
                        const char *error_msg = "malloc failed\n";
                        write(STDERR_FILENO, error_msg, strlen(error_msg));
                        close(out_fd);
                        return 1;
                    }
                    
                    const char *prefix = "Child processing: ";
                    write(STDOUT_FILENO, prefix, strlen(prefix));
                    write(STDOUT_FILENO, processed, strlen(processed));
                    write(STDOUT_FILENO, "\n", 1);
                    
                    write(out_fd, processed, strlen(processed));
                    write(out_fd, "\n", 1);
                    
                    free(processed);
                }
                line_pos = 0;
            } else {
                line[line_pos++] = buffer[i];
            }
        }
    }

    if (line_pos > 0) {
        line[line_pos] = '\0';
        char *processed = remove_vowels(line);
        if (processed) {
            const char *prefix = "Child processing: ";
            write(STDOUT_FILENO, prefix, strlen(prefix));
            write(STDOUT_FILENO, processed, strlen(processed));
            write(STDOUT_FILENO, "\n", 1);
            
            write(out_fd, processed, strlen(processed));
            write(out_fd, "\n", 1);
            
            free(processed);
        }
    }

    if (bytes_read < 0) {
        perror("read");
    }

    close(fd);
    close(out_fd);
    return 0;
}