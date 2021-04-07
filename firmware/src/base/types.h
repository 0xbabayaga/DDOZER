#ifndef _TYPES_H
#define _TYPES_H

#define false   					0
#define true    					1

#define ON      					1
#define OFF     					0

#define NO_ERROR                    0
#define NO_WARN                     0

#define ERROR_WRONG_CFG_SIZE        0x01
#define ERROR_WRONG_CFG_DATA        0x02
#define ERROR_WRONG_TASKLIST_SIZE   0x03
#define ERROR_WRONG_TASKLIST_DATA   0x04
#define ERROR_WRONG_RELAY_NUM       0x05
#define ERROR_WRONG_RTC_DATA        0x06
#define ERROR_SET_RTC               0x07
#define ERROR_WRONG_BUS_ADDR        0x08


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

typedef char int8;
typedef short int16;
typedef long int32;

#endif