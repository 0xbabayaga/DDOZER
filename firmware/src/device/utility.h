#ifndef _UTILITY_H
#define _UTILITY_H

#include "../base/types.h"

#define LED_ON  1
#define LED_OFF 0

//void delay_us(unsigned short cycles);
//void delay_ms(unsigned short ms);
void LED_Set(uint8 state);
void LED_Toggle();

#endif