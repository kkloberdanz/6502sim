#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define START_ADDR 0x0200
#define RAM_SIZE 0xFFFF

enum StatusCode {
    HALT = 0,
    ERR,
    RUNNING,
    IRQ
};

enum OpCode {
    BRK = 0x00,
    ORA_ZP_X = 0x01,
    TSB_ZP = 0x04,
    /* ... */
    LDX_IMM = 0xA2
};

struct MachineState {
    uint8_t accum;
    uint8_t x_reg;
    uint8_t y_reg;
    uint8_t status_reg;
    uint8_t stack_ptr;
    uint16_t pc;
    uint8_t *memory;
};

static size_t sizeof_bin_file(FILE *fp) {
    size_t sz;
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fprintf(stderr, "failed to seek");
    }
    sz = (size_t)ftell(fp);
    rewind(fp);
    return sz;
}

static int execute(struct MachineState *machine) {
    enum OpCode opcode = machine->memory[machine->pc];
    switch (opcode) {
        case BRK:
            return IRQ;

        case LDX_IMM:
            machine->pc++;
            machine->x_reg = machine->memory[machine->pc];
            machine->pc++;
            break;

        default:
            fprintf(stderr, "unknown opcode: %02X\n", opcode);
            return ERR;
    }
    return RUNNING;
}

int run(struct MachineState *machine) {
    for (;;) {
        enum StatusCode status = execute(machine);
        switch (status) {
            case HALT:
            case ERR:
                return status;

            case RUNNING:
                break;

            /* TODO: Handle interupts */
            default:
                return 255;
        }
    }
}

int setup(int argc, char **argv) {
    size_t prog_size;         /* size in bytes of the program to run on emulator */
    uint8_t memory[RAM_SIZE]; /* virtual memory of 6502 */
    char *filename;           /* name of binary file to run */
    FILE *bin_file;
    size_t i;
    struct MachineState machine;

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

    if (prog_size > RAM_SIZE) {
        fprintf(
            stderr,
            "%s\nprogram size: %ld, 6502 memory size: %d\n",
            "program does not fit in 6502 memory",
            prog_size,
            RAM_SIZE);
        exit(EXIT_FAILURE);
    }

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
        printf("%04lX:\t%02X\n", i, 0xFF & memory[i]);
    }

    machine.accum = 0;
    machine.x_reg = 0;
    machine.y_reg = 0;
    machine.pc = START_ADDR; /* TODO: start at reset vector, 0xFFFC */
    machine.memory = memory;
    machine.stack_ptr = 0xFF;
    machine.status_reg = 0;

    return run(&machine);
}

int main(int argc, char **argv) {
    return setup(argc, argv);
}
