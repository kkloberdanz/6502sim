#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "6502sim.h"

void sleep_6502() {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 10000000;
    nanosleep(&ts, NULL);
}

int main(int argc, char **argv) {
    char *filename; /* name of binary file to run */
    int ret_code = 0;
    if (argc > 1) {
        filename = argv[1];
        ret_code = run_6502_bin_file(filename, sleep_6502);
        return ret_code;
    } else {
        fprintf(stderr,
            "%s: error: no input file, expecting 6502 binary\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }
}
