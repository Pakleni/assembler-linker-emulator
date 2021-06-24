.extern a, b
.global kmecim, k

.section kme

    #jmps
        #symbol
        call k
        call %k
        #call %gnjecim #relative jump to absolute location
        call *labelica

        #lits
        call *3000
        call 3000
    #regop
        #symbol
        ldr r0, k
        ldr r0, $k
        ldr r0, %k

        #lits
        ldr r0, $300
        ldr r0, 123


    
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