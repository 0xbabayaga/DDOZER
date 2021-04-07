#ifndef _TASK_H
#define _TASK_H

#include "../base/types.h"
#include "../drivers/ds1307.h"
#include "../device/device.h"

void task_initActivity(void);
void task_doserActivity(void);

#endif