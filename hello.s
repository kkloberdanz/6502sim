; Write HELLO WORLD to zero page

; text
*=$0200
        LDX #$02
        LDY #$00

_loop:
        LDA _hello,Y
        CMP #$0
        BEQ _quit
        STA $00,X
        INX
        INY
        JMP _loop

_quit:  
        BRK

; data
_hello: .byt "HELLO WORLD"
        BRK ; NULL terminated C string

#include "subroutines.s"
