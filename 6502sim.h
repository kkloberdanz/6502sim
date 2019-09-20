#ifndef _6502SIM_H
#define _6502SIM_H

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

int run_6502(struct MachineState *machine);
void init_6502(struct MachineState *machine, uint8_t *memory);

#endif /*_6502SIM_H */
