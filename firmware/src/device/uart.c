#include <iostm8s105k4.h>
#include "uart.h"
#include "../types.h"

static uint8 data = 0;

void UART_SendByte(uint8 word)
{
	UART2_DR = word;     //  Put the next character into the data transmission register.
	
	while (UART2_SR_TXE == 0);          //  Wait for transmission to complete.                              
}

#pragma vector = UART2_R_RXNE_vector
__interrupt void UART2_Receive_handler(void)
{
    data = UART2_DR;
}

void UART_Init(void)
{
    uint8 tmp = UART2_SR;
    tmp = UART2_DR;
    
    UART2_CR1 = 0;
    UART2_CR2 = 0;
    UART2_CR4 = 0;
    UART2_CR3 = 0;
    UART2_GTR = 0;
    UART2_PSCR = 0;
    
    //
    //  Now setup the port to 115200,n,8,1.
    //
    UART2_CR1_M = 0;        //  8 Data bits.
    UART2_CR1_PCEN = 0;     //  Disable parity.
    UART2_CR3_STOP = 0;     //  1 stop bit.
    UART2_BRR2 = 0x0b;      //  Set the baud rate registers to 115200 baud
    UART2_BRR1 = 0x08;      //  based upon a 16 MHz system clock.
    
    //
    //  Set the clock polarity, lock phase and last bit clock pulse.
    //
    UART2_CR3_CPOL = 0;
    UART2_CR3_CPHA = 0;
    UART2_CR3_LBCL = 0;
    
    //
    //  Turn on the UART transmit, receive and the UART clock.
    //
    UART2_CR2_TEN = 1;
    UART2_CR2_REN = 1;
    UART2_CR3_CKEN = 0;
    UART2_CR2_ILIEN = 0;
    
    UART2_CR2_RIEN = 1;
}

void UART_RS485Direction(RS485Dir dir)
{
    dir %= RS485Max;

    PD_ODR_ODR7 = dir;
}