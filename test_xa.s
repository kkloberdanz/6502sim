; WRITE 0xFF -> 0x00 to zero page

*=$0200
    LDX #$FF
    LDA #$FF

_loop:
    CPX #$0
    BEQ _done
    STA $00,X
    SBC #$1
    DEX
    JMP _loop

_done:
    BRK
