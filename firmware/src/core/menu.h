#ifndef _MENU_H
#define _MENU_H

#include "../base/types.h"
#include "../device/device.h"
#include "../device/encoder.h"

typedef enum _eCalState
{
    eCalibration_NotStarted = 0,
    
    eCalibration_CalStarted = 1,
    eCalibration_CalDone = 2,
    eCalibration_CalError = 3,

    eCalibration_CheckStarted = 4,
    eCalibration_CheckDone = 5,
    eCalibration_CheckError = 6
}   eCalState;

typedef enum _Screens
{
    eScreen_StandBy = 0,
    eScreen_Menu = 1,
    eScreen_MenuDosing = 2,
    eScreen_MenuDateTime = 3,
    eScreen_MenuCalibration = 4,
    eScreen_MenuOptions = 5,
    eScreen_MenuStat = 6 
}   eScreens;

typedef enum _MainMenu
{
    eMainMenu_Back = 0,
    eMainMenu_Dosing = 1,
    eMainMenu_DateTime = 2,
    eMainMenu_Calibration = 3,
    eMainMenu_Options = 4,
    eMainMenu_Stat = 5,
    eMainMenu_Max = eMainMenu_Stat + 1
}   eMainMenu;

typedef enum _MenuExit
{
    eMenuExit_None = 0,
    eMenuExit_Ok = 1,
    eMenuExit_Cancel = 2,
    eMenuExit_Changed = 3
}   eMenuExit;

typedef struct _SubMenu
{
    const char* header;
    const char* format;
    uint16      *value;
    uint16      min;
    uint16      max;
}   SubMenu;

bool MENU_Activity(bool isUpdate);

#endif