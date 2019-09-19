#include <stdio.h>
#include <stdlib.h>

#include "6502sim.h"

#define START_ADDR 0x8000

size_t sizeof_bin_file(FILE *fp) {
    size_t sz;
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fprintf(stderr, "failed to seek");
    }
    sz = (size_t)ftell(fp);
    rewind(fp);
    return sz;
}

void *malloc_or_exit(size_t sz) {
    void *buf = malloc(sz);
    if (buf == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    } else {
        return buf;
    }
}

int main(int argc, char **argv) {
    size_t prog_size; /* size in bytes of the program to run on emulator */
    char *memory;     /* virtual memory of 6502 */
    char *filename;   /* name of binary file to run */
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

    memory = malloc_or_exit(0xFFFF);

    if (fread(memory + START_ADDR, 1, prog_size, bin_file) != prog_size) {
        fprintf(stderr,
            "%s: error: failed to read file: %s\n",
            argv[0],
            filename);
        exit(EXIT_FAILURE);
    } else {
        fclose(bin_file);
    }

    for (i = START_ADDR; i < prog_size + START_ADDR; i++) {
        printf("%04lx:\t%02x\n", i, 0xFF & memory[i]);
    }

    free(memory);

    return 0;
}
