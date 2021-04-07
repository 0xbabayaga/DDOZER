#include <iostm8s105k4.h>
#include "drv8825.h"
#include "../device/delay_us.h"

/* Motor modes
 * M3 - 1200 - 630mA
 * M2 - 1200 - 590ma
 * M1 - 1400 - 250mA
 * M0 - 1000 - 130mA
*/
static const MotorOpt motorOpt[eSpeedMode_Total] = 
{
    //  Mode                  Freq (Hz)
    {   eStepMode_1_32_Step,    1500    },
    {   eStepMode_1_16_Step,    1700    },
    {   eStepMode_1_4_Step,     1800    }
};

static uint8    tmrFreq = TIM4_500NS_CNT;
static uint32   runStepCount = 0;

static void drv_startTimer();
static void drv_stopTimer();

uint32 DRV_IsMotorRunning()
{
    return (DRV_PIN_EN == 0) ? runStepCount >> 1 : 0;
}

void DRV_SetMotorFreq(uint16 freq)
{
    uint32 val = F_CPU;

    val /= FPU_CLOCK_DIVIDER;
    val >>= 1;
    val /= freq;
    
    if (val < 256)
        tmrFreq = 256 - val;
    else
        tmrFreq = TIM4_500NS_CNT;
}

void DRV_SetMotorMode(eMotorSpeedMode mode, eDir dir)
{
    uint32 val = F_CPU;

    val /= FPU_CLOCK_DIVIDER;
    val >>= 1;
    val /= motorOpt[mode].freq;
    
    if (val < 256)
        tmrFreq = 256 - val;
    else
        tmrFreq = TIM4_500NS_CNT;

    /* Set mode */
    DRV_PIN_M0 = motorOpt[mode].mode;
    DRV_PIN_M1 = motorOpt[mode].mode >> 1;

    /* Set direction */
    DRV_PIN_DIR = dir;
}

void DRV_RunMotorForCount(uint32 steps)
{
    DRV_PIN_EN = 0;
  
    runStepCount = steps;
    //Since timer has to generate full period
    runStepCount <<= 1;
    
    delay_ms(1);
    
    drv_startTimer();
}

void DRV_StopMotor()
{
    drv_stopTimer();
  
    DRV_PIN_EN = 1;
}

void drv_startTimer()
{
    TIM4_SR_UIF = 0;
    TIM4_CNTR = TIM4_500NS_CNT;
    TIM4_CR1_CEN = 1;
}

void drv_stopTimer()
{
    TIM4_CR1_CEN = 0;
    TIM4_SR_UIF = 0;
}

void DRV_Init()
{
    PF_DDR_DDR4 = 0;    //Fault Input
    PF_CR1_C14 = 1;     //Fault pullup
    PF_CR2_C24 = 0;     //Interrupt disabled
    
    PB_DDR_DDR0 = 1;    //M0 Output
    PB_CR1_C10 = 1;     //Push-Pull
    PB_CR2_C20 = 0;     //2Mhz
    PB_ODR_ODR0 = 0;
    
    PB_DDR_DDR1 = 1;    //M1 Output
    PB_CR1_C11 = 1;     //Push-Pull
    PB_CR2_C21 = 0;     //2Mhz
    PB_ODR_ODR1 = 0;
    
    PB_DDR_DDR2 = 1;    //Step Output
    PB_CR1_C12 = 1;     //Push-Pull
    PB_CR2_C22 = 0;     //2Mhz
    PB_ODR_ODR2 = 0;
    
    PB_DDR_DDR3 = 1;    //DIR Output
    PB_CR1_C13 = 1;     //Push-Pull
    PB_CR2_C23 = 0;     //2Mhz
    PB_ODR_ODR3 = 0;
    
    PC_DDR_DDR1 = 1;    //EN Output
    PC_CR1_C11 = 1;     //Push-Pull
    PC_CR2_C21 = 0;     //2Mhz
    PC_ODR_ODR1 = 0;
    
    TIM4_CR1 = 0;
    TIM4_PSCR = TIM4_PRSC_1_128;
    TIM4_CNTR = tmrFreq;
    TIM4_IER = MASK_TIM1_IER_UIE;
    TIM4_CR1 = MASK_TIM2_CR1_URS;
}

#pragma vector = TIM4_OVR_UIF_vector
__interrupt void TIM4_OVR_UIF_handler(void)
{
    /* Configured to create interrupt every 500ns */
    if (TIM4_SR_UIF == 1)
    {        
        TIM4_CNTR = tmrFreq;
        TIM4_SR_UIF = 0;     
        
        runStepCount--;
        
        DRV_PIN_STEP = runStepCount;
        
        if (runStepCount == 0)
            DRV_StopMotor();
    }  
}