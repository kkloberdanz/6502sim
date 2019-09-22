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

_quit:  
    BRK

; data
_hello: .byt "HELLO WORLD"
        BRK ; NULL terminated C string

#include "subroutines.s"
