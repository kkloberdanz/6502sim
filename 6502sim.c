#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define START_ADDR 0x0200
#define RAM_SIZE 0xFFFF

#define FETCH_NEXT_BYTE (machine->memory[++(machine->pc)])

enum FlagMask {
    CARRY_FLAG = 1,
    ZERO_FLAG = (1 << 1),
    IRQ_DISBLE_FLAG = (1 << 2),
    DECIMAL_MODE_FLAG = (1 << 3),
    BRK_CMD_FLAG = (1 << 4),
    ONE_FLAG = (1 << 5),
    OVERFLOW_FLAG = (1 << 6),
    NEGATIVE_FLAG = (1 << 7)
};

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
    JMP_IMM = 0x4C,
    STA_ZP_INDEX_X = 0x95,
    LDX_IMM = 0xA2,
    LDA_IMM = 0xA9,
    DEX = 0xCA,
    CPX_IMM = 0xE0,
    BEQ_PCR = 0xF0
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

static uint16_t calculate_branch_addr(
    const struct MachineState *machine,

    /* will_branch MUST be equal to either 1 or 0 as bitwise math
     * hacks are used in this function! */
    const uint8_t will_branch
) {
    const uint8_t *mem = machine->memory;
    const uint16_t pc = machine->pc;

    /* immediate is a 2 byte vector encoded as little endian */
    const uint16_t immediate = mem[pc + 1];

    /* this bitwise hack saves us from doing a branch on the
     * host machine. If an 'if/else' statement were used here, then
     * we may have a branch misprediction. doing bitwise math
     * is fast for the branch predictor */
    const uint16_t branch_addr = will_branch * (pc + immediate + 2);
    const uint16_t wont_branch_addr = (!will_branch) * (pc + 2);
    const uint16_t branch_target = branch_addr | wont_branch_addr;
    return branch_target;
}

static int execute(struct MachineState *machine) {
    const enum OpCode opcode = machine->memory[machine->pc];
    switch (opcode) {
        case BRK:
            return IRQ;

        case JMP_IMM: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t immediate = low | (high << 8);
            machine->pc = immediate;
            break;
        }

        case STA_ZP_INDEX_X: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            const uint8_t addr = machine->x_reg + immediate;
            machine->memory[addr] = machine->accum;
            machine->pc++;
            break;
        }

        case LDX_IMM:
            machine->x_reg = FETCH_NEXT_BYTE;
            machine->pc++;
            break;

        case LDA_IMM:
            machine->pc++;
            machine->accum = machine->memory[machine->pc];
            machine->pc++;
            break;

        case DEX:
            machine->x_reg--;
            machine->pc++;
            break;

        case CPX_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            const uint8_t carry_flag = (machine->x_reg >= immediate) & 1;
            const uint8_t zero_flag = (machine->x_reg == immediate) & 1;
            const uint8_t negative_flag = (machine->x_reg >= 0x80) & 1;

            machine->status_reg =
                  (negative_flag << 7)
                | (zero_flag << 1)
                | (carry_flag);

            machine->pc++;
            break;
        }

        case BEQ_PCR: {
            const uint8_t will_branch = (machine->status_reg & ZERO_FLAG) >> 1;
            machine->pc = calculate_branch_addr(machine, will_branch);
            break;
        }

        default:
            fprintf(stderr, "unknown opcode: %02X\n", opcode);
            return ERR;
    }
    return RUNNING;
}

static int run(struct MachineState *machine) {
    for (;;) {
        enum StatusCode status = execute(machine);
        switch (status) {
            case RUNNING:
                break;

            /* TODO: Handle interupts */
            case HALT:
            case IRQ:
                return 0;

            case ERR:
                return status;

            default:
                return 255;
        }
    }
}

static void memory_dump(
    const uint8_t *memory,
    const size_t size
) {
    FILE *memory_file = fopen("memory.dump", "wb");
    if (memory_file == NULL) {
        fprintf(stderr, "failed to open memory.dump file\n");
        exit(EXIT_FAILURE);
    } else {
        fwrite(memory, 1, size, memory_file);
        fclose(memory_file);
    }
}

static int setup(int argc, char **argv) {
    size_t prog_size; /* size in bytes of the program to run on emulator */
    uint8_t memory[RAM_SIZE] = {0}; /* virtual memory of 6502 */
    char *filename; /* name of binary file to run */
    FILE *bin_file;
    size_t i;
    struct MachineState machine;
    int ret_code;

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

    puts("PROGRAM MEMORY:");
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

    ret_code = run(&machine);
    memory_dump(machine.memory, RAM_SIZE);
    return ret_code;
}

int main(int argc, char **argv) {
    return setup(argc, argv);
}
