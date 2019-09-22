; Write HELLO WORLD to zero page

; text
*=$0200
        LDX #$02
        LDY #$00

_loop:
        LDA _hello,Y
        CMP #$0
        BEQ _loop_done
        STA $00,X
        INX
        INY
        JMP _loop

_loop_done:

_print_screen:
        LDY #$00
        LDX #$00
        LDA #$02
        STA $00
        LDA #$00
        STA $01
        JSR print

_quit:  
        BRK

; data
_hello: .byt "HELLO WORLD"
        BRK ; NULL terminated C string

#include "subroutines.s"
