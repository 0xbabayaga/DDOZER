#ifndef _DEVICE_TYPES_H
#define _DEVICE_TYPES_H

#ifndef WIN32
#include "../base/types.h"
#else
#include "types.h"
#endif

#define DEV_CFG_MATERIAL_OPTION_MIN     0
#define DEV_CFG_MATERIAL_OPTION_MAX     6
#define DEV_CFG_SPEED_OPTION_MIN        0
#define DEV_CFG_SPEED_OPTION_MAX        3

#define DEV_CFG_DIR_OPTION_MIN          0
#define DEV_CFG_DIR_OPTION_MAX          1

#define DEV_CFG_TASK_TIME_MIN           0
#define DEV_CFG_TASK_TIME_MAX           1439
#define DEV_CFG_TASK_STARTTIME_DEFAULT  480
#define DEV_CFG_TASK_ENDTIME_DEFAULT    1410

#define DEV_CFG_VOLUME_MIN              0
#define DEV_CFG_VOLUME_MAX              2500

#define DEV_CFG_TRIES_MIN               1
#define DEV_CFG_TRIES_MAX               24

#define DEV_CAL_DEFAULT_TICKSPERML      1446
#define DEV_MOTOR_STEPS_PER_REV         200

#define DEV_VOLUME_PRECISION_MULTI      10

typedef struct
{
    uint8 seconds;
    uint8 minutes;
    uint8 hours;
    uint8 day;
    uint8 date;
    uint8 month;
    uint8 year;
}   Realtime;

typedef struct _Calibr
{
    uint16      ticksPerML;
    Realtime    dateTime;
    uint16      crc;
}   DevCalibration;

typedef struct _Options
{
    uint8       materialIdx;  
    uint8       speedPrecise;    
    uint8       dir;
}   DevOptions;

typedef struct _Task
{
    uint16      volumeML;
    uint16      tries;
    uint16      startTime;
    uint16      endTime;  
    uint16      crc;     
}   DevTask;

typedef struct _DeviceCfg
{
    DevOptions      optionsCfg;
    DevTask         taskCfg;
    DevCalibration  calibrationCfg;
}   DevCfg;

typedef struct _DeviceStat
{
    uint32      magicKey;
    Realtime    lastDosingTime;
    Realtime    dateTime;
    uint32      todayDosage;
    uint32      sinceCalDosage;
    uint32      sinceCalTicks;
    uint32      totalDosage;
    uint32      totalTicks;
    uint32      crc;
}   DevStat;


#endif
