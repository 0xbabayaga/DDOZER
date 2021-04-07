#include <string.h>
#include "task.h"
#include "../device/device_types.h"
#include "../device/eeprom.h"
#include "../device/utility.h"
#include "../device/device.h"
#include "../drivers/ds1307.h"
#include "../drivers/drv8825.h"

static uint16 dosagePeriod = 0;
static uint16 dosageVolume = 0;
static uint16 dosageStartedTime = 0xFFFF;
static bool   dosageStarted = FALSE;

void task_initActivity(void)
{
    Realtime tmRtc;
    uint32 tmTaskStart;
    uint32 tmDayStart;
    uint32 tmNow;
    uint32 tmLast; 
    uint16 tmp = 0;

    memcpy(&tmRtc, DS1307_getDateTime(), sizeof(Realtime));
    tmNow = time(&tmRtc);    

    tmp = DEV_GetDevTask()->startTime;
    tmp /= 60;
    tmRtc.hours = tmp;
    tmp = DEV_GetDevTask()->startTime;
    tmp %= 60;
    tmRtc.minutes = tmp;
    tmRtc.seconds = 0;
    tmTaskStart = time(&tmRtc);

    tmRtc.hours = 0;
    tmRtc.minutes = 0;
    tmDayStart = time(&tmRtc);
    tmLast = time(&DEV_GetStatistics()->lastDosingTime);

    if (DEV_GetStatistics()->todayDosage != 0)
    {
        if ((tmNow > tmDayStart && tmNow < tmTaskStart) ||
            ((tmLast + TIME_ONE_DAY_SECONDS) < tmNow))
            DEV_GetStatistics()->todayDosage = 0;    
    }
}

void task_doserActivity(void)
{
    uint16 doTime = 0;    
    uint32 tmp = 0;
    uint8 i = 0;

    /* Reset today dosage counter at the midnight */
    if (DEV_GetStatistics()->todayDosage != 0 &&
        DS1307_getDateTime()->hours == 0 &&
        DS1307_getDateTime()->minutes == 0 && 
        DS1307_getDateTime()->seconds == 0)
    {
        DEV_GetStatistics()->todayDosage = 0;
    }

    dosagePeriod = 0;
    dosageVolume = DEV_GetDevTask()->volumeML;

    if (DEV_GetDevTask()->tries > 1)
    {
        dosagePeriod = DEV_GetDevTask()->endTime;
        dosagePeriod -= DEV_GetDevTask()->startTime;
        dosagePeriod /= (DEV_GetDevTask()->tries - 1);
        dosageVolume /= DEV_GetDevTask()->tries;
    }

    doTime = DS1307_getDateTime()->hours;
    doTime *= 60;
    doTime += DS1307_getDateTime()->minutes;

    /* Check if dosing is not started */
    if (dosageStartedTime != doTime)
    {
        if (DEV_GetStatistics()->todayDosage < DEV_GetDevTask()->volumeML)
        {    
            for (i = 0; i < DEV_GetDevTask()->tries; i++)
            {
                if (doTime == (DEV_GetDevTask()->startTime + i * dosagePeriod))
                {
                    /* Start dosing */
                    DRV_RunMotorForCount(DEV_MLToTicks(dosageVolume));
                    
                    /* Mark dosing started */
                    dosageStartedTime = doTime;
                    dosageStarted = TRUE;
                }
            }
        }
    }

    if (dosageStarted == TRUE)
    {
        /* Since dosage is in progress 
        check if it is completed */
        if (DRV_IsMotorRunning() == FALSE)
        {
            dosageStarted = FALSE;

            memcpy((void*) &DEV_GetStatistics()->lastDosingTime, (void*)DS1307_getDateTime(), sizeof(Realtime));

            DEV_GetStatistics()->todayDosage += dosageVolume;
            DEV_GetStatistics()->sinceCalDosage += dosageVolume;
            DEV_GetStatistics()->totalDosage += dosageVolume;

            tmp = DEV_MLToTicksNormilized(dosageVolume);
            
            DEV_GetStatistics()->sinceCalTicks += tmp;
            DEV_GetStatistics()->totalTicks += tmp;

            DEV_WriteStat();
        }
    }
}