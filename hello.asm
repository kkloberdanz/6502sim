# 1 "hello.s"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "hello.s"
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

# 1 "subroutines.s" 1



JMP _end_stdlib

print:
; precondition:
; low and high bytes of the address of the NULL terminated string to
; print are stored in the zero page with the address to them in
; register A with X containing the offset to begin printing
;
; postcondition:
; all registers are invalidated
; the string will be written to the screen

    _print_loop:
        CMP #$0
        BEQ _print_done
        STA $8000,X
        SBC #$1
        INX
        JMP _print_loop

    _print_done:
        RTS

_end_stdlib:
# 24 "hello.s" 2
