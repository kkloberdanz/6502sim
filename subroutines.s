#ifndef SUBROUTINES_S
#define SUBROUTINES_S

#define VIDEO_BEGIN 8000
#define VIDEO_END A000

JMP _end_stdlib

print:
; precondition:
;     low and high bytes of the address of the NULL terminated string to
;     print are stored in the zero page at addresses $00 and $01 respectively
;
; postcondition:
;     all registers are invalidated
;     the string will be written to the screen

    _print_loop:
        LDA ($00),Y
        CMP #$0
        BEQ _print_done
        STA $8000,Y
        INY
        JMP _print_loop

    _print_done:
        RTS

_end_stdlib:

#endif /* SUBROUTINES_S */
