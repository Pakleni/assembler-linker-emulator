.extern a, b
.global kmecim, k

.section kme

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