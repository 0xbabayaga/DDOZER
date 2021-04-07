#ifndef _EEPROM_H
#define _EEPROM_H

#include "../base/types.h"
#include "stm8s_type.h"

#define STM8_EEPROM_ADDR                0x4000
#define DEV_EEPROM_CFG_ADDR             STM8_EEPROM_ADDR

#define EEPROM_RASS_KEY1                0x56
#define EEPROM_RASS_KEY2                0xAE 
#define EEPROM_DATA_BLOCKS_NUMBER       16
#define EEPROM_BLOCK_SIZE               128
#define EEPROM_START_PHYSICAL_ADDRESS   0x00004000
#define EEPROM_END_PHYSICAL_ADDRESS     (uint16)(EEPROM_START_PHYSICAL_ADDRESS + EEPROM_BLOCK_SIZE * EEPROM_DATA_BLOCKS_NUMBER) 
#define EEPROM_CLEAR_BYTE               0x00
#define OPERATION_TIMEOUT               0x1000
#define FAR                             __far
#define PointerAttr                     FAR

bool eeprom_read(uint16 addr, uint8 *buffer, uint16 size);
bool eeprom_write(uint16 addr, uint8 *buffer, uint16 size);

#endif
