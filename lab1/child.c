#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

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
        fprintf(stderr, "Usage: child <filename> <fd>\n");
        return 1;
    }

    char *filename = argv[1];
    int fd = atoi(argv[2]);

    FILE *in = fdopen(fd, "r");
    if (!in) {
        perror("fdopen");
        return 1;
    }

    FILE *out = fopen(filename, "w");
    if (!out) {
        perror("fopen");
        fclose(in);
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, in) != -1) {
        char *processed = remove_vowels(line);
        if (!processed) {
            perror("malloc");
            break;
        }
        printf("Child processing: %s", processed);
        fflush(stdout);
        fprintf(out, "%s", processed);
        fflush(out); 
        free(processed);
    }
    free(line);
    fclose(in);
    fclose(out);
    return 0;
}
