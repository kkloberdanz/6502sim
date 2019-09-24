#ifndef _6502SIM_H
#define _6502SIM_H

#include <stdio.h>
#include <stdint.h>

enum FlagPosition {
    CARRY_POS = 0,
    ZERO_POS = 1,
    IRQ_DISBLE_POS = 2,
    DECIMAL_MODE_POS = 3,
    BRK_CMD_POS = 4,
    ONE_POS = 5,
    OVERFLOW_POS = 6,
    NEGATIVE_POS = 7
};

enum FlagMask {
    CARRY_MASK = (1 << CARRY_POS),
    ZERO_MASK = (1 << ZERO_POS),
    IRQ_DISBLE_MASK = (1 << IRQ_DISBLE_POS),
    DECIMAL_MODE_MASK = (1 << DECIMAL_MODE_POS),
    BRK_CMD_MASK = (1 << BRK_CMD_POS),
    ONE_MASK = (1 << ONE_POS),
    OVERFLOW_MASK = (1 << OVERFLOW_POS),
    NEGATIVE_MASK = (1 << NEGATIVE_POS)
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
    BPL = 0x10,
    CLC = 0x18,
    JSR = 0x20,
    AND_IMM = 0x29,
    PHA = 0x48,
    JMP_IMM = 0x4C,
    EOR_ABS_X = 0x5D,
    RTS = 0x60,
    ADC_IMM = 0x69,
    PLA = 0x68,
    STA_ZP = 0x85,
    TXA = 0x8A,
    STA_ZP_INDEX_X = 0x95,
    STA_ABS_Y = 0x99,
    TXS = 0x9A,
    LDY_IMM = 0xA0,
    LDA_IND_X = 0xA1,
    LDX_ZP = 0xA6,
    LDA_IND_Y = 0xB1,
    LDA_ZP_X = 0xB5,
    LDA_ABS_Y = 0xB9,
    TSX = 0xBA,
    LDA_ABS_X = 0xBD,
    LDX_IMM = 0xA2,
    LDA_ZP = 0xA5,
    LDA_IMM = 0xA9,
    LDA_ABS = 0xAD,
    DEX = 0xCA,
    INY = 0xC8,
    CPX_IMM = 0xE0,
    CPX_ZP = 0xE4,
    INX = 0xE8,
    NOP = 0xEA,
    CMP_ZP = 0xC5,
    CMP_IMM = 0xC9,
    SBC_IMM = 0xE9,
    CMP_ABS = 0xEC,
    BNE = 0xD0,
    BEQ = 0xF0
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

int run_6502(struct MachineState *machine, void (*sleep_function)());
void init_6502(struct MachineState *machine, uint8_t *memory);
int memory_dump(const uint8_t *memory, const size_t size);
int run_6502_bin_file(const char *filename, void (*sleep_function)());

#endif /*_6502SIM_H */
