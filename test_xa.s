*=$8000    
    LDX #$FF
_loop:
    CPX #$0
    BEQ _exit
    LDA #$FA
    STA $00,X
    JMP _loop
_exit:
    BRK
