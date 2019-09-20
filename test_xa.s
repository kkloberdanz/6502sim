; WRITE 0XFA TO ZERO PAGE
*=$0200
    LDX #$FF
_LOOP:
    CPX #$0
    BEQ _EXIT
    LDA #$FA
    STA $00,X
    DEX
    JMP _LOOP
_EXIT:
    BRK
