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
    const uint16_t branch_addr = will_branch * (pc + immediate + 1);
    const uint16_t wont_branch_addr = (!will_branch) * (pc + 1);
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

uint16_t fetch_addr(struct MachineState *machine) {
    const uint8_t low = FETCH_NEXT_BYTE;
    const uint8_t high = FETCH_NEXT_BYTE;
    const uint16_t addr = low | (high << 8);
    return addr;
}

/* Executes a single instruction and modifies machine state */
static int execute(struct MachineState *machine) {
    const enum OpCode opcode = machine->memory[machine->pc];
    switch (opcode) {
        case BRK:
            return IRQ;

        case JSR: {
            const uint16_t new_addr = fetch_addr(machine);
            const uint16_t old_addr = machine->pc;
            const uint8_t low = old_addr & 0x00ff;
            const uint8_t high = (old_addr & 0xff00) >> 8;
            push_stack(machine, high);
            push_stack(machine, low);
            machine->pc = new_addr;
            goto skip_pc_increment;
            break;
        }

        case PHA:
            push_stack(machine, machine->accum);
            break;

        case JMP_IMM: {
            const uint16_t addr = fetch_addr(machine);
            machine->pc = addr;
            goto skip_pc_increment;
            break;
        }

        case RTS: {
            const uint8_t low = pop_stack(machine);
            const uint8_t high = pop_stack(machine);
            const uint16_t addr = low | (high << 8);
            machine->pc = addr;
            break;
        }

        case PLA:
            machine->accum = pop_stack(machine);
            break;

        case STA_ZP: {
            const uint8_t addr = FETCH_NEXT_BYTE;
            machine->memory[addr] = machine->accum;
            break;
        }

        case STA_ZP_INDEX_X: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            const uint8_t addr = machine->x_reg + immediate;
            machine->memory[addr] = machine->accum;
            break;
        }

        case STA_ABS_Y: {
            const uint16_t addr = fetch_addr(machine) + machine->y_reg;
            machine->memory[addr] = machine->accum;
            break;
        }

        case TXS:
            machine->stack_ptr = machine->x_reg;
            break;

        case LDY_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            machine->y_reg = immediate;
            break;
        }

        case LDA_IND_X: {
            const uint8_t zp_addr = FETCH_NEXT_BYTE;
            const uint8_t with_offset = zp_addr + machine->x_reg;
            const uint16_t addr = machine->memory[with_offset];
            machine->accum = machine->memory[addr];
            break;
        }

        case LDA_IND_Y: {
            const uint8_t zp_addr = FETCH_NEXT_BYTE;
            const uint8_t low = machine->memory[zp_addr];
            const uint8_t high = machine->memory[zp_addr + 1];
            const uint16_t ptr = low | (high << 8);
            const uint16_t addr = ptr + machine->y_reg;
            machine->accum = machine->memory[addr];
            break;
        }

        case LDA_ZP_X: {
            const uint8_t orig_addr = FETCH_NEXT_BYTE;
            const uint8_t addr = orig_addr + machine->x_reg;
            machine->accum = machine->memory[addr];
            break;
        }

        case LDA_ABS_Y: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t addr = ((low) | (high << 8)) + machine->y_reg;
            machine->accum = machine->memory[addr];
            break;
        }

        case TSX:
            machine->x_reg = machine->stack_ptr;
            break;

        case LDA_ABS_X: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t addr = ((low) | (high << 8)) + machine->x_reg;
            machine->accum = machine->memory[addr];
            break;
        }

        case LDX_IMM:
            machine->x_reg = FETCH_NEXT_BYTE;
            break;

        case LDA_IMM:
            machine->pc++;
            machine->accum = machine->memory[machine->pc];
            break;

        case LDA_ABS: {
            const uint8_t low = FETCH_NEXT_BYTE;
            const uint8_t high = FETCH_NEXT_BYTE;
            const uint16_t addr = (low) | (high << 8);
            machine->accum = addr;
            break;
        }

        case INY:
            machine->y_reg++;
            break;

        case DEX:
            machine->x_reg--;
            break;

        case CPX_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            do_compare(machine, machine->x_reg, immediate, 0);
            break;
        }

        case INX: {
            machine->x_reg++;
            break;
        }

        case NOP: {
            break;
        }

        case CMP_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            do_compare(machine, machine->accum, immediate, 0);
            break;
        }

        case SBC_IMM: {
            const uint8_t immediate = FETCH_NEXT_BYTE;
            const uint8_t overflow = (immediate > machine->accum) & 1;
            machine->accum -= immediate;
            do_compare(machine, machine->accum, immediate, overflow);
            break;
        }

        case BEQ_PCR: {
            const uint8_t will_branch = (machine->status_reg & ZERO_FLAG) >> 1;
            machine->pc = calculate_branch_addr(machine, will_branch);
            break;
        }

        default:
            fprintf(stderr, "unknown opcode: $%02X\n", opcode);
            return ERR;
    }
    machine->pc++;
skip_pc_increment:
    return RUNNING;
}

static int32_t map_bin_file_to_memory(uint8_t *memory, FILE *bin_file) {
    const size_t prog_size = sizeof_bin_file(bin_file);

    if (prog_size > RAM_SIZE - START_ADDR) {
        fprintf(
            stderr,
            "%s\nprogram size: %ld, 6502 memory size: %d\n",
            "program does not fit in 6502 memory",
            prog_size,
            RAM_SIZE);
        return -1;
    }

    if (fread(memory + START_ADDR, 1, prog_size, bin_file) != prog_size) {
        fprintf(stderr, "error: failed to read bin file\n");
        return -2;
    } else {
        fclose(bin_file);
        return prog_size;
    }
}

/****************** public interface *****************************************/
int memory_dump(
    const uint8_t *memory,
    const size_t size
) {
    FILE *memory_file = fopen("memory.dump", "wb");
    if (memory_file == NULL) {
        fprintf(stderr, "failed to open memory.dump file\n");
        return -1;
    } else {
        fwrite(memory, 1, size, memory_file);
        fclose(memory_file);
        return 0;
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
    int32_t prog_size; /* size in bytes of the program to run on emulator */
    uint8_t memory[RAM_SIZE] = {0}; /* virtual memory of 6502 */
    FILE *bin_file;
    struct MachineState machine;
    int ret_code;

    bin_file = fopen(filename, "rb");
    if (bin_file == NULL) {
        return -1;
    }

    prog_size = map_bin_file_to_memory(memory, bin_file);
    if (prog_size < 0) {
        return prog_size; /* < 0 indicates error */
    }

    init_6502(&machine, memory);
    ret_code = run_6502(&machine);
    memory_dump(machine.memory, RAM_SIZE);
    return ret_code;
}
/*****************************************************************************/
