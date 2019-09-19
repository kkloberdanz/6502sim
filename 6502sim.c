#include <stdio.h>
#include <stdlib.h>

#include "6502sim.h"

size_t sizeof_bin_file(FILE *fp) {
    size_t sz;
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    rewind(fp);
    return sz;
}

int main(int argc, char **argv) {
    size_t prog_size;
    char *program;
    char *filename;
    FILE *bin_file;
    size_t i;

    if (argc > 1) {
        filename = argv[1];
    } else {
        fprintf(stderr,
            "%s: error: no input file, expecting 6502 binary\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }

    bin_file = fopen(filename, "rb");
    prog_size = sizeof_bin_file(bin_file);
    program = malloc(prog_size * sizeof(char) + 1);

    if (fread(program, 1, prog_size, bin_file) != prog_size) {
        fprintf(stderr,
            "%s: error: failed to read file: %s\n",
            argv[0],
            filename);
        exit(EXIT_FAILURE);
    } else {
        fclose(bin_file);
    }

    for (i = 0; i < prog_size; i++) {
        printf("%04lx:\t%02x\n", 0x8000 + i, 0xFF & program[i]);
    }

    free(program);

    return 0;
}
