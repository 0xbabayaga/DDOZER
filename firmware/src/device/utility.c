#include <iostm8s105k4.h>
#include "utility.h"

void LED_Set(uint8 state)
{
    PA_ODR_ODR2 = !state%2;
}

void LED_Toggle()
{
    if (PA_ODR_ODR2 == 1)
        PA_ODR_ODR2 = 0;
    else
        PA_ODR_ODR2 = 1;
}

/*
void delay_us(unsigned short cycles)
{  
    while (cycles > 0)
    {
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        cycles--;
    }
}

void delay_ms(unsigned short ms)
{
    while (ms > 0)
    {
        delay_us(150);
        delay_us(150);
        ms--;
    }
}
*/