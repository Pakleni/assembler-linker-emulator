.extern a, b
.global kmecim, k

.section kme

    #jmps
    call k
    call %gnjecim
    call *labelica
    #regop
    ldr r0, k
    ldr r0, $k
    ldr r0, %k

    
#comment
    .equ kmecim, 69
    .equ gnjecim, 0xFF00

.section gigachad
    .word   16
labelica:
    .skip   3
k:  
    .word   k
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