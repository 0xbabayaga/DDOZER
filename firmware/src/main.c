#include "stm8s.h"
#include <stdio.h>
#include "../device/cpuinit.h"
#include "../device/utility.h"
#include "../drivers/ds1307.h"
#include "../drivers/drv8825.h"
#include "../drivers/ssd1306.h"
#include "../device/timers.h"
#include "../device/device.h"
#include "../device/encoder.h"
#include "../device/delay_us.h"
#include "core/task.h"
#include "core/menu.h"

static uint32   rtcReadTick = TIMER_500MS_TMT;
static uint32   menuTick = TIMER_100MS_TMT;
static bool     menuUpdate = TRUE;
static bool     timeChanged = FALSE;
static uint8    lastSecond = 0;

int main( void )
{    
    asm("sim");

    CPU_Init();
    TIM_Init();  
    DRV_Init();
    ENC_Init();
    SSD1306_Init();
    
    DRV_StopMotor();
    
    if (DS1307_init() == FALSE)
        DS1307_init();

    DS1307_readDateTime();

    DEV_ReadCfg();
    DEV_ReadStat(DS1307_getDateTime());
    DRV_SetMotorMode((eMotorSpeedMode) DEV_GetDevOptions()->speedPrecise, (eDir) DEV_GetDevOptions()->dir);

    SSD1306_Clear();
    SSD1306_DrawText16(0, 16, DEVICE_NAME, FALSE, eJustify_Center);
    SSD1306_DrawText8(0, 40, DEVICE_FIRMWARE_VER, eJustify_Center);
    delay_ms(1777);
    SSD1306_Clear();

    task_initActivity();
    resetUpTime();

    asm("rim");
    
    for(;;)
    {      
        /* Reading the system time */
        if (TIM_isTimeoutExpired(rtcReadTick, TIMER_500MS_TMT) == TRUE)
        {
            rtcReadTick = TIM_getSystemTick();
            
            DS1307_readDateTime();

            if (lastSecond != DS1307_getDateTime()->seconds)
            {
                lastSecond = DS1307_getDateTime()->seconds;
                timeChanged = TRUE;
            }
            
            //menuUpdate = TRUE;
        }      
        
        /* Display update */
        if (TIM_isTimeoutExpired(menuTick, TIMER_100MS_TMT) == TRUE)
        {
            menuTick = TIM_getSystemTick();
            menuUpdate = TRUE;
        }
        
        /* Menu handler */
        MENU_Activity(menuUpdate);
        menuUpdate = FALSE;

        if (timeChanged == TRUE)
        {
            task_doserActivity();

            if (DRV_IsMotorRunning() > 0)
                LED_Set(LED_ON);
            else
                LED_Set(LED_OFF);

            timeChanged = FALSE;
        }
        
        delay_ms(1); 
    }
}
