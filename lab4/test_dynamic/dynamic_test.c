#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#define BUF 128

typedef float (*der_f)(float, float);
typedef float (*area_f)(float, float);

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
    if (ip == 0) tmp[tlen++] = '0';
    while (ip > 0) {
        tmp[tlen++] = '0' + (ip % 10);
        ip /= 10;
    }
    while (tlen > 0) buf[len++] = tmp[--tlen];

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
    void *lib = NULL;
    der_f der = NULL;
    area_f ar = NULL;
    int impl = 1;

    void load_lib(int num) {
        if (lib) dlclose(lib);

        const char *path = (num == 1) ? "impl1/libimpl1.so" : "impl2/libimpl2.so";
        lib = dlopen(path, RTLD_LAZY);
        if (!lib) {
            write(1, dlerror(), strlen(dlerror()));
            _exit(1);
        }

        der = (der_f)dlsym(lib, "cos_derivative");
        ar  = (area_f)dlsym(lib, "area");
        if (!der || !ar) {
            write(1, "Symbol not found\n", 17);
            _exit(1);
        }
    }

    load_lib(impl);

    char buf[BUF];
    while (1) {
        write(1, "> ", 2);
        int n = read(0, buf, BUF - 1);
        if (n <= 0) break;
        buf[n] = 0;

        char *cmd = strtok(buf, " \n");
        if (!cmd) continue;

        if (cmd[0] == '0') {
            impl = 3 - impl;  
            load_lib(impl);
            write(1, "switched\n", 9);
        } else if (cmd[0] == '1') {
            char *s_a = strtok(NULL, " \n");
            char *s_dx = strtok(NULL, " \n");
            if (!s_a || !s_dx) continue;
            float a = strtof(s_a, NULL);
            float dx = strtof(s_dx, NULL);
            print_float(der(a, dx));
        } else if (cmd[0] == '2') {
            char *s_a = strtok(NULL, " \n");
            char *s_b = strtok(NULL, " \n");
            if (!s_a || !s_b) continue;
            float a = strtof(s_a, NULL);
            float b = strtof(s_b, NULL);
            print_float(ar(a, b));
        }
    }

    if (lib) dlclose(lib);
    return 0;
}
