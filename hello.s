; Write HELLO WORLD to zero page

; text
*=$0200

_print_screen:
    LDA _hello
    STA $00
    LDA _hello >> 8
    STA $01
    LDY #$00
    JSR print

_switch_case:
    LDX #$00

    _case_loop:
        LDA _hello,X
        CMP #$00
        BEQ _loop_back
        LDA #$20
        EOR _hello,X
        INX
        JMP _case_loop

_loop_back:
    JMP _print_screen

_delay:
    LDA #$FF
_delay_loop:
    DEC
    CMP #$00
    BEQ _delay_done
    JMP _delay_loop
_delay_done:
    RTS

_quit:  
    BRK

; data
_hello: .byt "HELLO WORLD, "
        .byt "This is comming from a "
        .byt "6502 microprocessor!"
        BRK ; NULL terminated C string

#include "subroutines.s"
