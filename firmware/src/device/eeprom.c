#include <iostm8s105k4.h>
#include "eeprom.h"

bool eeprom_read(uint16 addr, uint8 *buffer, uint16 size)
{
    if (addr >= EEPROM_START_PHYSICAL_ADDRESS &&
        (addr + size) < EEPROM_END_PHYSICAL_ADDRESS)
    {
        FLASH_DUKR = EEPROM_RASS_KEY2;
        FLASH_DUKR = EEPROM_RASS_KEY1;
        
        while(size > 0)
        {
            *buffer = (*((PointerAttr u8*) addr));
            buffer++;
            addr++;
            size--;
        }
        
        FLASH_IAPSR_DUL = 0;
        
        return TRUE;
    }
    return FALSE;
}

bool eeprom_write(uint16 addr, uint8 *buffer, uint16 size)
{
    if (addr >= EEPROM_START_PHYSICAL_ADDRESS &&
        (addr + size) < EEPROM_END_PHYSICAL_ADDRESS)
    {
        FLASH_DUKR = EEPROM_RASS_KEY2;
        FLASH_DUKR = EEPROM_RASS_KEY1;
        
        while (size > 0)
        {
            *((PointerAttr u8*) addr) = *buffer;
            addr++;
            buffer++;
            size--;
        }
        
        FLASH_IAPSR_DUL = 0;
        
        return TRUE;
    }
  
    return FALSE;
}