#include "ds1307.h"
#include <iostm8s105k4.h>
#include "../device/device_types.h"
#include "../device/utility.h"
#include "../device/delay_us.h"

/******************************************************************************/
/* Static methods definition */
/******************************************************************************/
static Realtime rtc;
static uint32 upTime = 0;

static void i2c_init_bus(void);
static void i2c_start(void);
static void i2c_stop(void);
static uint8 i2c_write_byte(uint8 byte);
static uint8 i2c_read_byte(bool isLast);

static uint8 bcdToDec(uint8 data);
static uint8 decToBcd(uint8 data);

/******************************************************************************/
/* Global methods */
/******************************************************************************/

void resetUpTime()
{
    upTime = time(DS1307_getDateTime());
}

uint32 getUpTime()
{
    return time(DS1307_getDateTime()) - upTime;
}

Realtime* DS1307_getDateTime(void)  
{
    return &rtc;
}

uint8 maxDayInMonth(uint8 yy, uint8 mo)
{
    switch(mo)
    {
        case 2:  if ((yy%4) == 0)  return 29; else return 28;
        case 4:  return 30;
        case 6:  return 30;
        case 9:  return 30;
        case 11: return 30;
        default: return 31;
    }
}

uint32 time(Realtime* rt)
{
    uint32 tm = 0;
    uint8 i = 0;
    uint32 tmp = 0;

    while (i < (30 + rt->year))
    {
        tm += TIME_YEAR_SECONDS;

        if ((i%4) == 0)
            tm += TIME_ONE_DAY_SECONDS;

        i++;
    }    

    i = 0;
    while (i < (rt->month - 1))
    {
        tm += TIME_ONE_DAY_SECONDS * maxDayInMonth(rt->year, i + 1);
        i++;
    }

    i = 0;
    while (i < (rt->day - 1))
    {
        tm += TIME_ONE_DAY_SECONDS;
        i++;
    }
    
    tmp = rt->hours;
    tmp *= 3600;
    tm += tmp;
    
    tmp = rt->minutes;
    tmp *= 60;
    tm += tmp;
  
    tm += rt->seconds;

    return tm;
}


bool DS1307_init(void)
{
    uint8 ack = 0;
    uint8 read = 0;
    
    i2c_init_bus();
    
    delay_us(200);
    
    i2c_start();
    ack += i2c_write_byte(DS1307_CLOCKWRITE);
    ack += i2c_write_byte(DS1307_SECONDS_ADDR);
    i2c_stop();
    
    i2c_start();
    ack += i2c_write_byte(DS1307_CLOCKREAD);
    read = i2c_read_byte(TRUE);
    i2c_stop();
    
    read &= DS1307_START_CLOCK;
    
    i2c_start();
    ack += i2c_write_byte(DS1307_CLOCKWRITE);
    ack += i2c_write_byte(DS1307_SECONDS_ADDR);
    ack += i2c_write_byte(read);
    i2c_stop();
    
    return (ack == 0) ? TRUE : FALSE;
}

bool DS1307_writeDateTime(Realtime * dt)
{
    uint8 ack = 0;
    Realtime data;
  
    if (dt != 0)
    {
        data.seconds = decToBcd(dt->seconds);
        data.minutes = decToBcd(dt->minutes);
        data.hours = decToBcd(dt->hours);
        data.day = decToBcd(dt->day);
        data.month = decToBcd(dt->month);
        data.year = decToBcd(dt->year);
         
        i2c_start();
        ack += i2c_write_byte(DS1307_CLOCKWRITE);
        ack += i2c_write_byte(DS1307_SECONDS_ADDR);
        ack += i2c_write_byte(data.seconds);
        ack += i2c_write_byte(data.minutes);
        ack += i2c_write_byte(data.hours);
        i2c_stop();
        
        delay_us(5);
        
        i2c_start();
        ack += i2c_write_byte(DS1307_CLOCKWRITE);
        ack += i2c_write_byte(DS1307_DAY_ADDR);
        ack += i2c_write_byte(data.day);
        ack += i2c_write_byte(data.month);
        ack += i2c_write_byte(data.year);
        i2c_stop();
        
        return (ack == 0) ? TRUE : FALSE;
    }
    else
        return FALSE;
}

bool DS1307_readDateTime(void)
{
    uint8 ack = 0;
    Realtime data;
    
    i2c_start();
    ack += i2c_write_byte(DS1307_CLOCKWRITE);
    ack += i2c_write_byte(DS1307_SECONDS_ADDR);
    i2c_stop();
    
    delay_us(5);
    
    i2c_start();
    ack += i2c_write_byte(DS1307_CLOCKREAD);
    data.seconds = i2c_read_byte(FALSE);
    data.minutes = i2c_read_byte(FALSE);
    data.hours = i2c_read_byte(TRUE);
    i2c_stop();
    
    delay_us(5);
    
    i2c_start();
    ack += i2c_write_byte(DS1307_CLOCKWRITE);
    ack += i2c_write_byte(DS1307_DAY_ADDR);
    i2c_stop();
    
    delay_us(5);
    
    i2c_start();
    ack += i2c_write_byte(DS1307_CLOCKREAD);
    data.day = i2c_read_byte(FALSE);
    data.month = i2c_read_byte(FALSE);
    data.year = i2c_read_byte(TRUE);
    i2c_stop();
    
    if (ack == 0)
    {
        rtc.seconds = bcdToDec(data.seconds);
        rtc.minutes = bcdToDec(data.minutes);
        rtc.hours = bcdToDec(data.hours);
        rtc.day = bcdToDec(data.day);
        rtc.month = bcdToDec(data.month);
        rtc.year = bcdToDec(data.year);
    }
    
    return (ack == 0) ? TRUE : FALSE;
}


/******************************************************************************/
/* Static methods definition */
/******************************************************************************/

uint8 bcdToDec(uint8 data)
{
    uint8 val = 0;
    
    val = ((data>>4)&0x0F) * 10 + (data&0x0F);
    
    return val;
}

uint8 decToBcd(uint8 data)
{
    uint8 val = 0;
    
    val = (data/10);      
    val <<= 4;  
    val |= (data%10);
        
    return val;
}

void  i2c_init_bus(void)
{
    PB_DDR_DDR4 = 1;        //  Port D, bit 4 is output.
    PB_CR1_C14 = 0;         //  Pin is set to Open drain mode.
    PB_CR2_C24 = 0;         //  Pin can run up to 10 MHz.
    
    PB_DDR_DDR5 = 1;        //  Port D, bit 5 is output.
    PB_CR1_C15 = 0;         //  Pin is set to Open drain mode.
    PB_CR2_C25 = 0;         //  Pin can run up to 10 MHz.
    
    SDA_HIGH();
    SCL_HIGH();
}

void i2c_start(void)
{
    SDA_LOW()
    delay_us(5);
    SCL_LOW()
    delay_us(5);
}

void i2c_stop(void)
{
    SCL_HIGH()
    delay_us(5);
    SDA_HIGH()
    delay_us(10);
}

uint8 i2c_write_byte(uint8 byte)
{
    uint8 i = 0;
    
    for (i = 0; i < 8; i++)
    {
        if((byte & 0x80) == 0x80)
            SDA_HIGH()
        else
            SDA_LOW()
        
        delay_us(2);
        
        SCL_HIGH()
        delay_us(5);
        SCL_LOW()
        delay_us(2);
        
        byte <<= 1;
    }
    
    SDA_SET_AS_IN()
    delay_us(2);
    
    SCL_HIGH()
    delay_us(4);
    i = SDA_GET_IN;
    SCL_LOW()
    delay_us(4);
    
    SDA_SET_AS_OUT()
    SDA_LOW()
    delay_us(3);
    
    return i;
}

uint8 i2c_read_byte(bool isLast)
{
    uint8 i = 0;
    uint8 byte = 0;
    
    SDA_SET_AS_IN()
    
    for (i = 0; i < 8; i++)
    {
        delay_us(2);
        
        SCL_HIGH()
        delay_us(5);
        
        byte |= SDA_GET_IN;
        
        SCL_LOW()
        delay_us(2);  
        
        if (i < 7)
            byte <<= 1;
    }
    
    delay_us(2);
    
    if (isLast == FALSE)
        SDA_LOW()
    else
        SDA_HIGH()
    
    SDA_SET_AS_OUT()
    
    
    delay_us(5);
    SCL_HIGH()
    delay_us(5);
    SCL_LOW()
    delay_us(2);
    SDA_LOW();
    delay_us(4);
    
    return byte;
}