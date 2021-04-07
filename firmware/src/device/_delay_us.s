
        MODULE ?testm

        PUBLIC _delay_us

        SECTION `.near_func.text`:CODE:REORDER:NOROOT(0)
        CODE
_delay_us:
        
loop:        
        DECW X
        JREQ fin
        JRNE loop
fin:
        RET
        
        