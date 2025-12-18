#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "contract.h"

#define BUF 128

void print_float(float x) {
    char buf[64];
    int len = 0;

    if (x < 0) {
        buf[len++] = '-';
        x = -x;
    }

    int ip = (int)x;
    float fp = x - ip;

    char tmp[16];
    int tlen = 0;

    if (ip == 0) {
        tmp[tlen++] = '0';
    } else {
        while (ip > 0) {
            tmp[tlen++] = '0' + (ip % 10);
            ip /= 10;
        }
    }

    while (tlen > 0)
        buf[len++] = tmp[--tlen];

    buf[len++] = '.';

    for (int i = 0; i < 3; i++) {
        fp *= 10;
        int d = (int)fp;
        buf[len++] = '0' + d;
        fp -= d;
    }

    buf[len++] = '\n';
    write(1, buf, len);
}


int main() {
    char buf[BUF];
    write(1, "1 a dx | 2 a b\n", 14);

    int n = read(0, buf, BUF - 1);
    buf[n] = 0;

    char *cmd = strtok(buf, " ");
    if (!cmd) return 0;

    if (cmd[0] == '1') {
        float a = strtof(strtok(NULL, " "), NULL);
        float dx = strtof(strtok(NULL, " "), NULL);
        float r = cos_derivative(a, dx);
        print_float(r);
    } else if (cmd[0] == '2') {
        float a = strtof(strtok(NULL, " "), NULL);
        float b = strtof(strtok(NULL, " "), NULL);
        float r = area(a, b);
        print_float(r);
    }
    return 0;
}
