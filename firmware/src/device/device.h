#ifndef _DEVICE_H
#define _DEVICE_H

#include "../base/types.h"
#include "stm8s_type.h"
#include "device_types.h"
#include "eeprom.h"

#define DEVICE_FIRMWARE_VER     "ver1.7"
#define DEVICE_NAME             "DDOSER"

#define DEVICE_CFG_OFFSET		DEV_EEPROM_CFG_ADDR
#define DEVICE_STAT_OFFSET		DEV_EEPROM_CFG_ADDR + 0x100

#define DEVICE_STAT_MAGIC_KEY	0xEE8ECE01

DevTask* 		DEV_GetDevTask();
DevOptions*		DEV_GetDevOptions();
DevCalibration* DEV_GetCalibration();
DevStat* 		DEV_GetStatistics();

uint16 DEV_TickToML(uint32 ticks);
uint32 DEV_MLToTicks(uint16 ml);
uint32 DEV_MLToTicksNormilized(uint16 ml);
void   DEV_CalcAndSaveCalibration(uint16 ml, uint32 ticks, Realtime *tm);

bool DEV_ReadCfg(void);
bool DEV_WriteCfg(void);

bool DEV_ReadStat(Realtime *tm);
bool DEV_WriteStat(void);

#endif