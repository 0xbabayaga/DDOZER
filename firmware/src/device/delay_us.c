#include "delay_us.h"

void delay_us(unsigned short delay)
{
    unsigned long ff = 0;
  
    while (delay > 0)
    {
        ff++;
        delay--;
    }
}

void delay_ms(unsigned short delay) 
{
    while (delay > 0)
    {
        delay_us(1000);
        delay--;
    }
}
