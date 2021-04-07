#include <iostm8s105k4.h>
#include "cpuinit.h"
#include "utility.h"

void CPU_Init()
{
    CLK_ICKR_HSIEN = 1;            
    CLK_SWCR_SWEN = 1;               
    while(CLK_ICKR_HSIRDY != 1) {} 
    CLK_CKDIVR = 0;                    
    
    //Enabling clock for TIM1 & UART's
    CLK_PCKENR1 = 0x92; //0b10010010
    CLK_PCKENR2 = 0x00;
  
    PA_DDR = 0x04;      //0b00000100;
    PA_ODR = 0x00;
    PA_CR1 = 0x04;
    
    PB_DDR = 0x0F;      //0b00001111;
    PB_CR1 = 0x0F;      //Push-Pull
    PB_CR2 = 0x00;      //Port speed 2MHz
    PB_ODR = 0x00;      //Default state
    
    PC_DDR = 0x7A;      //0b01111010;
    PC_CR1 = 0xFF;      //Push-Pull
    PC_CR2 = 0x64;      //Enabling interrupt on pin
    PC_ODR = 0x04;      //0b10000010;
    
    PD_DDR = 0x00;      //0b00000000
    PD_CR1 = 0xF8;      //0b11111000
    PD_CR2 = 0x05;      //0b00000101 Enabling interrupt on pin      
    PD_ODR = 0x21;      //0b00100001
    
    PE_DDR = 0x20;      //0b00100000;
    PE_CR1 = 0x20;
    PE_ODR = 0x20;
    
    PF_DDR = 0x00;
    
    //Interrupt on falling edge
    EXTI_CR1_PDIS = 0x02;
    EXTI_CR1_PCIS = 0x02;
    
    //Configuring SPI-bus
    //Set OFF
    SPI_CR1 = 0;
    SPI_CR2 = 0;
}