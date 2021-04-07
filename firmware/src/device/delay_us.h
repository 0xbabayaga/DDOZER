#ifndef _DELAY_US_H
#define _DELAY_US_H

#ifndef F_CPU
#define F_CPU 16000000
#endif

#define CYCLES_PER_US (F_CPU/1000000)	
#define C4PUS (CYCLES_PER_US/4)		
#define DVUS(x) (C4PUS*x)

void delay_us(unsigned short delay);
void delay_ms(unsigned short delay);

#endif