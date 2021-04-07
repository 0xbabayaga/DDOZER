#ifndef _DS1307_DRV_H
#define _DS1307_DRV_H

#include "../base/types.h"
#include "../device/device_types.h"
#include "stm8s_type.h"

#define TIME_ONE_DAY_SECONDS	86400
#define TIME_YEAR_SECONDS		365 * TIME_ONE_DAY_SECONDS

#define DS1307_CLOCKWRITE       0xD0
#define DS1307_CLOCKREAD        0xD1
#define DS1307_SECONDS_ADDR     0x00
#define DS1307_DAY_ADDR         0x04
#define DS1307_CONTROL_ADDR     0x07

#define DS1307_START_CLOCK      0x7F
#define DS1307_ACK              0

#define SDA_HIGH()              PB_ODR_ODR4 = 1;
#define SDA_LOW()               PB_ODR_ODR4 = 0;
#define SDA_SET_AS_OUT()        PB_DDR_DDR4 = 1;
#define SDA_SET_AS_IN()         PB_DDR_DDR4 = 0;
#define SDA_GET_IN              PB_IDR_IDR4

#define SCL_HIGH()              PB_ODR_ODR5 = 1;
#define SCL_LOW()               PB_ODR_ODR5 = 0;

bool DS1307_init(void);
bool DS1307_writeDateTime(Realtime * dt);
bool DS1307_readDateTime(void);
Realtime* DS1307_getDateTime(void);
uint8 maxDayInMonth(uint8 yy, uint8 mo);
uint32 time(Realtime* rt);
uint32 getUpTime();
void resetUpTime();

#endif