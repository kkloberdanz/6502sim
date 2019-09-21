#include <stdio.h>
#include <stdlib.h>

#include "6502sim.h"

int main(int argc, char **argv) {
    char *filename; /* name of binary file to run */
    int ret_code = 0;
    if (argc > 1) {
        filename = argv[1];
        ret_code = run_6502_bin_file(filename);
        return ret_code;
    } else {
        fprintf(stderr,
            "%s: error: no input file, expecting 6502 binary\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }
}
