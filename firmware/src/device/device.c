#include <string.h>
#include "device.h"
#include "eeprom.h"
#include "../drivers/ds1307.h"

static DevCfg devCfg;
static DevStat devStat;

static uint16 calcCrc(uint8 *ptr, uint16 size);

DevTask*    DEV_GetDevTask()
{
    return &devCfg.taskCfg;
}

DevOptions* DEV_GetDevOptions()
{
    return &devCfg.optionsCfg;
}

DevCalibration* DEV_GetCalibration()
{
    return &devCfg.calibrationCfg;
}

DevStat* DEV_GetStatistics()
{
    return &devStat;
}

/* Return volume x 10 (to handle float without float) */
uint16 DEV_TickToML(uint32 ticks)
{
    ticks *= DEV_VOLUME_PRECISION_MULTI;
	ticks /= devCfg.calibrationCfg.ticksPerML;
    ticks <<= devCfg.optionsCfg.speedPrecise;
    
    if (devCfg.optionsCfg.speedPrecise == 2)
        ticks <<= 1;
    
    ticks >>= 4;
    
	return (uint16) ticks;
}

/* Return volume x 10 (to handle float without float) */
uint32 DEV_MLToTicks(uint16 ml)
{   
    uint32 ticks = devCfg.calibrationCfg.ticksPerML;

    ticks *= ml;
    ticks /= DEV_VOLUME_PRECISION_MULTI;
    ticks <<= 4;
    ticks >>= devCfg.optionsCfg.speedPrecise;
    
    if (devCfg.optionsCfg.speedPrecise == 2)
        ticks >>= 1;

    return ticks;
}

uint32 DEV_MLToTicksNormilized(uint16 ml)
{
    uint32 ticks = DEV_MLToTicks(ml);

    ticks <<= DEV_GetDevOptions()->speedPrecise;

    if (devCfg.optionsCfg.speedPrecise == 2)
        ticks <<= 1;

    ticks >>= 4;

    return ticks;
}

void DEV_CalcAndSaveCalibration(uint16 actualML, uint32 calML, Realtime *tm)
{
    uint32 tmp = DEV_MLToTicks(calML);  

    tmp *= 10;
    tmp >>= 4;
    tmp <<= devCfg.optionsCfg.speedPrecise;
    
    if (devCfg.optionsCfg.speedPrecise == 2)
        tmp <<= 1;

    tmp /= actualML;

    DEV_GetCalibration()->ticksPerML = tmp;
    DEV_GetCalibration()->crc = ~tmp;
    memcpy((void*)&DEV_GetCalibration()->dateTime, (void*)tm, sizeof(Realtime));

    DEV_GetStatistics()->sinceCalDosage = 0;
    DEV_GetStatistics()->sinceCalTicks = 0;

    DEV_WriteCfg(); 
    DEV_WriteStat();
}

bool DEV_ReadCfg(void)
{   
    uint16 crc = 0;

    eeprom_read(DEVICE_CFG_OFFSET, (uint8*)&devCfg, sizeof(DevCfg));
    
    if (devCfg.optionsCfg.materialIdx > DEV_CFG_MATERIAL_OPTION_MAX)
        devCfg.optionsCfg.materialIdx = DEV_CFG_MATERIAL_OPTION_MIN;

    if (devCfg.optionsCfg.speedPrecise > DEV_CFG_SPEED_OPTION_MAX)
        devCfg.optionsCfg.speedPrecise = DEV_CFG_SPEED_OPTION_MIN;

    if (devCfg.optionsCfg.dir > DEV_CFG_DIR_OPTION_MAX)
        devCfg.optionsCfg.dir = DEV_CFG_DIR_OPTION_MIN;

    crc = calcCrc((uint8*) &devCfg.taskCfg,  (sizeof(DevTask) - sizeof(uint16)));

    if (devCfg.taskCfg.volumeML > DEV_CFG_VOLUME_MAX || crc != devCfg.taskCfg.crc)
        devCfg.taskCfg.volumeML = DEV_CFG_VOLUME_MIN;

    if (devCfg.taskCfg.tries < DEV_CFG_TRIES_MIN || devCfg.taskCfg.tries > DEV_CFG_TRIES_MAX  || crc != devCfg.taskCfg.crc)
        devCfg.taskCfg.tries = DEV_CFG_TRIES_MIN;

    if (devCfg.taskCfg.startTime > DEV_CFG_TASK_TIME_MAX  || crc != devCfg.taskCfg.crc)
        devCfg.taskCfg.startTime = DEV_CFG_TASK_STARTTIME_DEFAULT;
    
    if (devCfg.taskCfg.endTime > DEV_CFG_TASK_TIME_MAX  || crc != devCfg.taskCfg.crc)
        devCfg.taskCfg.endTime = DEV_CFG_TASK_ENDTIME_DEFAULT;

    crc = calcCrc((uint8*) &devCfg.calibrationCfg,  (sizeof(DevCalibration) - sizeof(uint16)));

    if (crc != devCfg.calibrationCfg.crc)
        devCfg.calibrationCfg.ticksPerML = DEV_CAL_DEFAULT_TICKSPERML;

    return TRUE;
}

bool DEV_WriteCfg(void)
{
    devCfg.taskCfg.crc = calcCrc((uint8*) &devCfg.taskCfg,  (sizeof(DevTask) - sizeof(uint16)));
    devCfg.calibrationCfg.crc = calcCrc((uint8*) &devCfg.calibrationCfg,  (sizeof(DevCalibration) - sizeof(uint16)));

    return eeprom_write(DEV_EEPROM_CFG_ADDR, (uint8*)&devCfg, sizeof(DevCfg));
}

bool DEV_ReadStat(Realtime *tm)
{
    bool res = FALSE;

    res = eeprom_read(DEVICE_STAT_OFFSET, (uint8*)&devStat, sizeof(DevStat));

    if (devStat.magicKey != DEVICE_STAT_MAGIC_KEY)
    {
        devStat.magicKey = DEVICE_STAT_MAGIC_KEY;
        devStat.todayDosage = 0;
        devStat.sinceCalDosage = 0;
        devStat.sinceCalTicks = 0;
        devStat.totalDosage = 0;
        devStat.totalTicks = 0;
        memcpy((void*) &devStat.dateTime, (void*)tm, sizeof(Realtime));

        DEV_WriteStat();
    }

    return res;
}

bool DEV_WriteStat(void)
{
    return eeprom_write(DEVICE_STAT_OFFSET, (uint8*)&devStat, sizeof(DevStat));
}

uint16 calcCrc(uint8 *ptr, uint16 size)
{
    uint8 i = 0;
    uint16 crc = 0;
    
    for (i = 0; i < size; i++)
        crc += ptr[i];

    crc = ~crc;

    return crc;
}