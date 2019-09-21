#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "6502sim.h"

#define START_ADDR 0x0200
#define RAM_SIZE 0xFFFF

#define FETCH_NEXT_BYTE (machine->memory[++(machine->pc)])

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

static void do_compare(
    struct MachineState *machine,
    const uint8_t reg,
    const uint8_t immediate,

    /* due to bithacking, this MUST be either 0 or 1
     * the only instructions that affect overflow are:
     * ADC, BIT, CLV, PLP, RTI, and SBC */
    const uint8_t overflow
) {
    const uint8_t carry_flag = (reg >= immediate) & 1;
    const uint8_t zero_flag = (reg == immediate) & 1;
    const uint8_t negative_flag = (reg >= 0x80) & 1;

    machine->status_reg =
          (negative_flag << 7)
        | (overflow << 6)
        | (zero_flag << 1)
        | (carry_flag);
}

static void push_stack(struct MachineState *machine, uint8_t val) {
    uint16_t addr;
    machine->stack_ptr--;
    addr = 0x100 + machine->stack_ptr;
    machine->memory[addr] = val;
}

static uint8_t pop_stack(struct MachineState *machine) {
    const uint16_t addr = 0x100 + machine->stack_ptr;
    const uint8_t val = machine->memory[addr];
    machine->stack_ptr++;
    return val;
}

static int execute(struct MachineState *machine) {
    const enum OpCode opcode = machine->memory[machine->pc];
    switch (opcode) {
        case BRK:
            return IRQ;

        case PHA:
            push_stack(machine, machine->accum);
            machine->pc++;
            break;

        case JMP_IMM: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t immediate = low | (high << 8);
            machine->pc = immediate;
            break;
        }

        case PLA:
            machine->accum = pop_stack(machine);
            machine->pc++;
            break;

        case STA_ZP_INDEX_X: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            const uint8_t addr = machine->x_reg + immediate;
            machine->memory[addr] = machine->accum;
            machine->pc++;
            break;
        }

        case TXS:
            machine->stack_ptr = machine->x_reg;
            machine->pc++;
            break;

        case LDY_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            machine->y_reg = immediate;
            machine->pc++;
            break;
        }

        case LDA_ABS_Y: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t addr = ((low) | (high << 8)) + machine->y_reg;
            machine->accum = machine->memory[addr];
            machine->pc++;
            break;
        }

        case TSX:
            machine->x_reg = machine->stack_ptr;
            machine->pc++;
            break;

        case LDA_ABS_X: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t addr = ((low) | (high << 8)) + machine->x_reg;
            machine->accum = machine->memory[addr];
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

        case LDA_ABS: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t addr = (low) | (high << 8);
            machine->accum = addr;
            machine->pc++;
            break;
        }

        case INY:
            machine->y_reg++;
            machine->pc++;
            break;

        case DEX:
            machine->x_reg--;
            machine->pc++;
            break;

        case CPX_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            do_compare(machine, machine->x_reg, immediate, 0);
            machine->pc++;
            break;
        }

        case INX: {
            machine->x_reg++;
            machine->pc++;
            break;
        }

        case CMP_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            do_compare(machine, machine->accum, immediate, 0);
            machine->pc++;
            break;
        }

        case SBC_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            const uint8_t overflow = (immediate > machine->accum) & 1;
            machine->accum -= immediate;
            do_compare(machine, machine->accum, immediate, overflow);
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

static size_t map_bin_file_to_memory(uint8_t *memory, FILE *bin_file) {
    const size_t prog_size = sizeof_bin_file(bin_file);

    if (prog_size > RAM_SIZE - START_ADDR) {
        fprintf(
            stderr,
            "%s\nprogram size: %ld, 6502 memory size: %d\n",
            "program does not fit in 6502 memory",
            prog_size,
            RAM_SIZE);
        exit(EXIT_FAILURE);
    }

    if (fread(memory + START_ADDR, 1, prog_size, bin_file) != prog_size) {
        fprintf(stderr, "error: failed to read bin file\n");
        exit(EXIT_FAILURE);
    } else {
        fclose(bin_file);
    }
    return prog_size;
}

/****************** public interface *****************************************/
void memory_dump(
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

int run_6502(struct MachineState *machine) {
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

void init_6502(struct MachineState *machine, uint8_t *memory) {
    machine->accum = 0;
    machine->x_reg = 0;
    machine->y_reg = 0;
    machine->pc = START_ADDR; /* TODO: start at reset vector, 0xFFFC */
    machine->memory = memory;
    machine->stack_ptr = 0xFF;
    machine->status_reg = 0;
}

int run_6502_bin_file(const char *filename) {
    size_t prog_size; /* size in bytes of the program to run on emulator */
    uint8_t memory[RAM_SIZE] = {0}; /* virtual memory of 6502 */
    FILE *bin_file;
    size_t i;
    struct MachineState machine;
    int ret_code;

    bin_file = fopen(filename, "rb");
    if (bin_file == NULL) {
        return -1;
    }

    prog_size = map_bin_file_to_memory(memory, bin_file);
    puts("PROGRAM MEMORY:");
    for (i = START_ADDR; i < prog_size + START_ADDR; i++) {
        printf("%04lX:\t%02X\n", i, 0xFF & memory[i]);
    }

    init_6502(&machine, memory);
    ret_code = run_6502(&machine);
    memory_dump(machine.memory, RAM_SIZE);
    return ret_code;
}
/*****************************************************************************/
