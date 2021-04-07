#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "../base/types.h"

#define TIMER_CONV_VALUE(x)     (x/10)

#define TIMER_20MS_TMT          TIMER_CONV_VALUE(20)
#define TIMER_50MS_TMT          TIMER_CONV_VALUE(50)    
#define TIMER_100MS_TMT         TIMER_CONV_VALUE(100)
#define TIMER_250MS_TMT         TIMER_CONV_VALUE(250)
#define TIMER_500MS_TMT         TIMER_CONV_VALUE(500)
#define TIMER_1000MS_TMT        TIMER_CONV_VALUE(1000)
#define TIMER_10000MS_TMT       TIMER_CONV_VALUE(10000)
#define TIMER_30000MS_TMT       TIMER_CONV_VALUE(30000)


void    TIM_Init(void);
bool    TIM_isSystemTimerTick(void);
uint32  TIM_getSystemTick(void);
bool    TIM_isTimeoutExpired(uint32 tick, uint32 timeout);

#endif