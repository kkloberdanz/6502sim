#define VIDEO_BEGIN 8000
#define VIDEO_END A000

JMP _end_stdlib

print:
; precondition:
;     low and high bytes of the address of the NULL terminated string to
;     print are stored in the zero page with the address to them in 
;     register A with X containing the offset to begin printing
;
; postcondition:
;     all registers are invalidated
;     the string will be written to the screen

    _print_loop:
        CMP #$0
        BEQ _print_done
        STA $VIDEO_BEGIN,X
        SBC #$1
        INX
        JMP _print_loop

    _print_done:
        RTS

_end_stdlib:
