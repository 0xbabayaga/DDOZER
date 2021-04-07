#ifndef _DRV8825_H_
#define _DRV8825_H_

#include "../base/types.h"

#define DRV_PIN_FAULT       PF_IDR_IDR4
#define DRV_PIN_DIR         PB_ODR_ODR3
#define DRV_PIN_STEP        PB_ODR_ODR2
#define DRV_PIN_EN          PC_ODR_ODR1
#define DRV_PIN_M0          PB_ODR_ODR0
#define DRV_PIN_M1          PB_ODR_ODR1

/* 1 : 128 Prescaler */
#define TIM4_PRSC_1_128     0x07
#define FPU_CLOCK_DIVIDER	128
/* 500ns period timer  */
#define TIM4_500NS_CNT      0xA0

/*
 * Speed caclculation
 * Timer tick = 16000000 / 128 = 125000Hz = 8us
 * Freq = 125000 / (255 - x) 
 * x = 255 - 250000/freq
*/

typedef enum _MotorSpeedMode
{
    eSpeedMode_Low = 0,
    eSpeedMode_Normal = 1,
    eSpeedMode_High = 2,
    eSpeedMode_Total = 3
}   eMotorSpeedMode;

typedef enum _StepMode
{
    eStepMode_FullStep = 0,
    //eStepMode_1_2_Step = 1,
    eStepMode_1_4_Step = 1,
    //eStepMode_1_8_Step = 3,
    eStepMode_1_16_Step = 2,
    eStepMode_1_32_Step = 3
}   eStepMode;

typedef enum _Direction
{
	eDir_CW = 0,
	eDir_ACW = 1
}	eDir;

typedef struct _MotorOpt
{
	eStepMode 	mode;
	uint16		freq;
}	MotorOpt;

void DRV_Init();
void DRV_SetMotorFreq(uint16 freq);
void DRV_SetMotorMode(eMotorSpeedMode mode, eDir dir);
void DRV_RunMotorForCount(uint32 steps);
void DRV_StopMotor();
uint32 DRV_IsMotorRunning();

#endif