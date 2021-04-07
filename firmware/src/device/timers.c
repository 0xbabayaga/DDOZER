#include <iostm8s105k4.h>
#include "stm8s_type.h"
#include "timers.h"

/* 1 : 16 000 Prescaler */
#define TIM1_PREH       0x3E
#define TIM1_PREL       0x80

/* 100msec period timer */
#define TIM1_100MS_CNTH  0xFF
#define TIM1_100MS_CNTL  0x9C

/* 10msec period timer */
#define TIM1_10MS_CNTH   0xFF
#define TIM1_10MS_CNTL   0xF6

static uint8    systemTimerTick = 0;
static uint32   tick10msCnt = 0;


uint32 TIM_getSystemTick(void)
{
    return tick10msCnt;
}

bool TIM_isTimeoutExpired(uint32 tick, uint32 timeout)
{
    return ((tick + timeout) >= tick10msCnt) ? FALSE : TRUE;
}

bool TIM_isSystemTimerTick(void)
{
    if (systemTimerTick > 0)
    {
        systemTimerTick = 0;
        return TRUE;
    }
    
    return FALSE;
}

void TIM_Init(void)
{
    TIM1_CR1 = 0;
    TIM1_CR2 = 0;
    TIM1_SMCR = 0;
    TIM1_ETR = 0;
    TIM1_IER = MASK_TIM1_IER_UIE;
    TIM1_PSCRH = TIM1_PREH;
    TIM1_PSCRL = TIM1_PREL;
    TIM1_CNTRH = TIM1_10MS_CNTH;
    TIM1_CNTRL = TIM1_10MS_CNTL;
    TIM1_CR1 = (MASK_TIM1_CR1_URS | MASK_TIM1_CR1_CEN);
}

#pragma vector = TIM1_OVR_UIF_vector
__interrupt void TIM1_OVR_UIF_handler(void)
{
    if (TIM1_SR1_UIF == 1)
    {        
        TIM1_CNTRH = TIM1_10MS_CNTH;
        TIM1_CNTRL = TIM1_10MS_CNTL;
    
        TIM1_SR1_UIF = 0;
        
        systemTimerTick = 1;
        
        tick10msCnt++;
    }  
}