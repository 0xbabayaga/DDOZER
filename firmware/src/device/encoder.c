#include <iostm8s105k4.h>
#include <string.h>
#include "stm8s_type.h"
#include "timers.h"
#include "../base/types.h"
#include "encoder.h"
#include "utility.h"

static uint32   keyPressedTick = 0;
static bool     isExpired_Quick = FALSE;
static bool     isExpired_Long = FALSE;
static eEncKey keyPressedQueue[ENC_QUEUE_SIZE];
static void addKey(eEncKey key);

#pragma vector = 7
__interrupt void EXTI_PORTC_IRQHandler(void)
{
    if (PC_IDR_IDR2 == 0)
        addKey(eKeyPress);
}

#pragma vector = 0x08
__interrupt void EXTI_PORTD_IRQHandler(void)
{
    isExpired_Quick = TIM_isTimeoutExpired(keyPressedTick, TIMER_20MS_TMT);
    isExpired_Long = TIM_isTimeoutExpired(keyPressedTick, TIMER_100MS_TMT);

    if (PD_IDR_IDR2 == 1)
    {
        if (isExpired_Quick == FALSE)
            addKey(eKeyFwd10);
        else if (isExpired_Long == FALSE)
            addKey(eKeyFwd2);
        else
            addKey(eKeyFwd);
    }
    else
    {
        if (isExpired_Quick == FALSE)
            addKey(eKeyBwd10);
        else if (isExpired_Long == FALSE)
            addKey(eKeyBwd2);
        else
            addKey(eKeyBwd);
    }

    //addKey(keyDetected);
    
    keyPressedTick = TIM_getSystemTick();
}

void addKey(eEncKey key)
{
    uint8 i = 0;
  
    for (i = 0; i < ENC_QUEUE_SIZE; i++)
    {
        if (keyPressedQueue[i] == eKeyNone)
        {
            keyPressedQueue[i] = key;
            break;
        }
    }
}

eEncKey ENC_getPressedKey(void)
{
    uint8 i = 0;
    eEncKey pressed = keyPressedQueue[0];
    
    for (i = 0; i < (ENC_QUEUE_SIZE - 1); i++)
        keyPressedQueue[i] = keyPressedQueue[i + 1];
    
    keyPressedQueue[ENC_QUEUE_SIZE - 1] = eKeyNone;
    
    return pressed;
}

void ENC_Init(void)
{  
    PD_DDR_DDR0 = 0;
    PD_DDR_DDR2 = 0;
    PC_DDR_DDR2 = 0;
    
    PD_CR1_C10 = 0;
    PD_CR1_C12 = 0;
    PC_CR1_C12 = 0;
    
    PD_CR2_C20 = 1;
    PD_CR2_C22 = 0;
    PC_CR2_C22 = 1;
      
    EXTI_CR1_PDIS = 2; //Interrupts on falling edge
    EXTI_CR1_PCIS = 2; //Interrupts on falling edge

    memset(keyPressedQueue, eKeyNone, ENC_QUEUE_SIZE);
}