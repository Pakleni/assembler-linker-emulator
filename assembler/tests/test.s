.extern a, b
.global kmecim, k

.section kme

    push r0
    push r1
    ldr r0, term_in
    str r0, term_out
    ldr r0, %myCounter # pcrel
    ldr r1, $1
    add r0, r1
    str r0, myCounter # abs
    pop r1
    pop r0
    iret
    
#comment
    .equ kmecim, 69
    .equ gnjecim, 0xFF00

.section gigachad
    .word   16
labelica:
    .skip   3
k:  
    .word   256
    .word   a,
            b,
            gnjecim,
            labelica
    
.section thundercock
grmi:
    .skip   10
    .word   a,
            b,
            kmecim,
            k
.end