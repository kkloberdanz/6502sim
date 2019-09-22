#ifndef _6502SIM_H
#define _6502SIM_H

#include <stdio.h>
#include <stdint.h>

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
    JSR = 0x20,
    PHA = 0x48,
    JMP_IMM = 0x4C,
    PLA = 0x68,
    STA_ZP = 0x85,
    STA_ZP_INDEX_X = 0x95,
    TXS = 0x9A,
    LDY_IMM = 0xA0,
    LDA_ZP_X = 0xB5,
    LDA_ABS_Y = 0xB9,
    TSX = 0xBA,
    LDA_ABS_X = 0xBD,
    LDX_IMM = 0xA2,
    LDA_IMM = 0xA9,
    LDA_ABS = 0xAD,
    DEX = 0xCA,
    INY = 0xC8,
    CPX_IMM = 0xE0,
    INX = 0xE8,
    CMP_IMM = 0xC9,
    SBC_IMM = 0xE9,
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

int run_6502(struct MachineState *machine);
void init_6502(struct MachineState *machine, uint8_t *memory);
void memory_dump(const uint8_t *memory, const size_t size);
int run_6502_bin_file(const char *filename);

#endif /*_6502SIM_H */
