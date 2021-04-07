#include <stdio.h>
#include "menu.h"
#include "strings.h"
#include "../device/encoder.h"
#include "../device/delay_us.h"
#include "../drivers/drv8825.h"
#include "../drivers/ds1307.h"
#include "../drivers/ssd1306.h"
#include "../device/timers.h"

//-$CUR_LINE$:"$FILE_PATH$"

//#define MOTOR_FREQ_TEST

#define MENU_MENU_CNT               5
#define MENU_SUBMENU_PARAM_MAX      6

#define MENU_DOSING_ITEMS_CNT       6
#define MENU_DATETIME_ITEMS_CNT     8
#define MENU_CALIBRATION_ITEMS_CNT  6   
#define MENU_OPTIONS_ITEMS_CNT      5
#define MENU_STAT_ITEMS_CNT         7

#define MENU_MATERIALS_MAX          DEV_CFG_MATERIAL_OPTION_MAX    
#define MENU_SPEED_MAX              DEV_CFG_SPEED_OPTION_MAX     
#define MENU_DIR_MAX                DEV_CFG_DIR_OPTION_MAX + 1
#define MENU_CALIBRATION_VOLUME_ML  20  //ml

static eScreens currentScreen = eScreen_StandBy;
static eMainMenu currentMenu = eMainMenu_Back;
static eEncKey key = eKeyNone;
static eCalState calibState = eCalibration_NotStarted;
static uint8 subMenuIndex = 0;
static uint8 subMenuSelected = 0;
static uint16 subMenuParam[MENU_SUBMENU_PARAM_MAX] = {0};
static float volume = 0;
static uint8 menuBlink = 0;
static uint8 prevSubItem = 0;
static uint32 goToStandByTick = TIMER_10000MS_TMT;
static char str[16] = {0};
static eIcon ico = eIcon_None;

static uint16 lastTodayVolumeML = 0;
static uint8 lastTimeSeconds = 0;
//static uint8 soundNum = 0;

//#define MOTOR_FREQ_TEST

#ifdef MOTOR_FREQ_TEST
static uint16 fre = 1000;
#endif

static const char* materialOption[MENU_MATERIALS_MAX] = 
{
    STR_ALK,
    STR_CA,
    STR_MG,
    STR_NO3,
    STR_PO4,
    STR_FOOD
};

static const char* speedOption[MENU_SPEED_MAX] = 
{
    STR_LOW,
    STR_NORMAL,
    STR_HIGH
};

static const char* dirOption[MENU_DIR_MAX] = 
{
    STR_CW,
    STR_ACW
};

static const char* menuNames[eMainMenu_Max] = 
{
    STR_BACK,
    STR_DOSING,
    STR_DATETIME,
    STR_CALIBRATION,
    STR_OPTIONS,
    STR_STAT
};

static SubMenu subMenu_Dosing[MENU_DOSING_ITEMS_CNT] = 
{
	{	STR_CANCEL,			0,			           0,					0,		            0		                 },
    {	STR_DAILY_DOSAGE,	STR_FORMAT_VOLUME,     &subMenuParam[0],	0,		            DEV_CFG_VOLUME_MAX       },
    {	STR_PORTIONS,		STR_FORMAT_TIMES,	   &subMenuParam[1],	DEV_CFG_TRIES_MIN,	DEV_CFG_TRIES_MAX        },
    {	STR_START_TIME,		STR_FORMAT_HH_MM,	   &subMenuParam[2],	0,		            DEV_CFG_TASK_TIME_MAX	 },
    {	STR_END_TIME,		STR_FORMAT_HH_MM,      &subMenuParam[3],	0,		            DEV_CFG_TASK_TIME_MAX	 },
    {	STR_SAVE,			0,				       0,					0,		            0		                 }
};

static SubMenu subMenu_DateTime[MENU_DATETIME_ITEMS_CNT] = 
{
	{	STR_CANCEL,			0,					   0,					0,		0		}, 
    {	STR_SET_HOURS,		STR_FORMAT_HH_MM_SS,   &subMenuParam[0],	0,		23		},
    {	STR_SET_MINUTES,	STR_FORMAT_HH_MM_SS,   &subMenuParam[1],	0,		59		},
    {	STR_SET_SECONDS,	STR_FORMAT_HH_MM_SS,   &subMenuParam[2],	0,		59		},
    {   STR_SET_DAY,        STR_FORMAT_DD_MM_YY,   &subMenuParam[3],    1,      31      },
    {   STR_SET_MONTH,      STR_FORMAT_DD_MM_YY,   &subMenuParam[4],    1,      12      },
    {   STR_SET_YEAR,       STR_FORMAT_DD_MM_YY,   &subMenuParam[5],    0,      99      },
    {	STR_SAVE,			0,					   0,					0,		0		}
};

static SubMenu subMenu_Calib[MENU_CALIBRATION_ITEMS_CNT] = 
{
    {   STR_CANCEL,         0,                     0,                  0,      0                    },
    {   STR_START,          STR_FORMAT_STR,        0,                  0,      0                    },
    {   STR_CHECK,          STR_FORMAT_STR,        0,                  0,      0                    },
    {   STR_TEST_VOLUME,    STR_FORMAT_VOLUME,     &subMenuParam[0],   0,      0                    },
    {   STR_SET_VOLUME,     STR_FORMAT_VOLUME,     &subMenuParam[1],   0,      DEV_CFG_VOLUME_MAX   },
    {   STR_SAVE,           0,                     0,                  0,      0                    }
};

static SubMenu subMenu_Options[MENU_OPTIONS_ITEMS_CNT] = 
{
    {   STR_CANCEL,         0,                     0,                  0,      0                    },
    {   STR_MATERIAL,       STR_FORMAT_STR,        &subMenuParam[0],   0,      MENU_MATERIALS_MAX - 1},
    {   STR_SPEED,          STR_FORMAT_STR,        &subMenuParam[1],   0,      MENU_SPEED_MAX - 1   },
    {   STR_ROTATION,       STR_FORMAT_STR,        &subMenuParam[2],   0,      MENU_DIR_MAX - 1     },
    {   STR_SAVE,           0,                     0,                  0,      0                    }
};


static SubMenu subMenu_Stat[MENU_STAT_ITEMS_CNT] = 
{
    {   STR_CANCEL,         0,                     0,                  0,      0                    },
    {   STR_UPTIME,         STR_FORMAT_UPTIME,     0,                  0,      0                    },
    {   STR_SINCE_CAL,      STR_FORMAT_VOLUME_L,   0,                  0,      0                    },
    {   STR_SINCE_CAL,      STR_FORMAT_09LU,       0,                  0,      0                    },
    {   STR_TOTAL,          STR_FORMAT_VOLUME_L,   0,                  0,      0                    },
    {   STR_TOTAL,          STR_FORMAT_09LU,       0,                  0,      0                    },
    {   STR_START_DATE,     STR_FORMAT_DD_MM_YY,   0,                  0,      0                    }
};


static eMenuExit keyHandler_Menu_Base(eEncKey key, uint8 menuMin, uint8 menuMax);
static bool keyHandler_MenuParam_Base(SubMenu *menu, eEncKey key, uint8 index, uint8 inc);
static bool keyHandler_Menu_StandBy(eEncKey key);
static bool keyHandler_Menu_Menu(eEncKey key);
static bool keyHandler_Menu_Dosing(eEncKey key);
static bool keyHandler_Menu_DateTime(eEncKey key);
static bool keyHandler_Menu_Calibration(eEncKey key);
static bool keyHandler_Menu_Options(eEncKey key);
static bool keyHandler_Menu_Stat(eEncKey key);

static void drawer_Menu_StandBy(eEncKey key, bool isUpdate);
static void drawer_Menu_Menu(eEncKey key, bool isUpdate);
static void drawer_Menu_Dosing(eEncKey key, bool isUpdate);
static void drawer_Menu_DateTime(eEncKey key, bool isUpdate);
static void drawer_Menu_Calibration(eEncKey key, bool isUpdate);
static void drawer_Menu_Options(eEncKey key, bool isUpdate);
static void drawer_Menu_Stat(eEncKey key, bool isUpdate);
static void drawer_ClearMenu();
static void drawer_DrawMenuData(eEncKey key, eIcon ico, char *header, char *subHeader);
static void drawer_DrawBlinkedData(char *str, uint8 *menuBlink);
static void drawer_GetMenuIconAndString(uint8 idx, char *str, eIcon *icon);

//static void DRV_Sound(uint8 tries);


bool MENU_Activity(bool isUpdate)
{
    key = ENC_getPressedKey();

    if (key != eKeyNone)
        SSD1306_Brightness(SSD1306_BRIGHTNESS_MAX);
    
    if (currentScreen == eScreen_StandBy)
        isUpdate |= keyHandler_Menu_StandBy(key);
    else if (currentScreen == eScreen_Menu)
        isUpdate = keyHandler_Menu_Menu(key);
    else if (currentScreen == eScreen_MenuDosing)
        isUpdate |= keyHandler_Menu_Dosing(key);
    else if (currentScreen == eScreen_MenuDateTime)
        isUpdate |= keyHandler_Menu_DateTime(key);
    else if (currentScreen == eScreen_MenuCalibration)
        isUpdate |= keyHandler_Menu_Calibration(key);
    else if (currentScreen == eScreen_MenuOptions)
        isUpdate |= keyHandler_Menu_Options(key);
    else if (currentScreen == eScreen_MenuStat)
        isUpdate |= keyHandler_Menu_Stat(key);
    
    if (key != eKeyNone || (calibState == eCalibration_CalStarted || calibState == eCalibration_CheckStarted))
        goToStandByTick = TIM_getSystemTick();
    else
        if (TIM_isTimeoutExpired(goToStandByTick, TIMER_30000MS_TMT) == TRUE)
        {
            if (currentScreen != eScreen_StandBy)
            {
                currentScreen = eScreen_StandBy;
                calibState = eCalibration_NotStarted;
                SSD1306_Clear();
                isUpdate = TRUE;
                lastTimeSeconds = 100;
            }

            goToStandByTick = TIM_getSystemTick();
            SSD1306_Brightness(SSD1306_BRIGHTNESS_MIN);
        }

    if ((calibState == eCalibration_CalStarted || calibState == eCalibration_CheckStarted))
    {
        volume = DEV_TickToML(DRV_IsMotorRunning());
        *subMenu_Calib[3].value = (uint16) volume; 
    }

    if ((calibState == eCalibration_CalStarted || calibState == eCalibration_CheckStarted) && DRV_IsMotorRunning() == 0)
    {
        *subMenu_Calib[3].value = 0;

        if (calibState == eCalibration_CalStarted)
        {
            calibState = eCalibration_CalDone;
            subMenuIndex = 4;
            subMenuSelected = TRUE;
        }
        else if (calibState == eCalibration_CheckStarted)
        {
            calibState = eCalibration_CheckDone;
            subMenuIndex = 5;
            subMenuSelected = FALSE;
        }
        
        isUpdate = TRUE;
    }
    
    if (isUpdate == TRUE)
    {
        if (currentScreen == eScreen_StandBy)
            drawer_Menu_StandBy(key, isUpdate);
        else if (currentScreen == eScreen_Menu)
            drawer_Menu_Menu(key, isUpdate);
        else if (currentScreen == eScreen_MenuDosing)
            drawer_Menu_Dosing(key, isUpdate);
        else if (currentScreen == eScreen_MenuDateTime)
            drawer_Menu_DateTime(key, isUpdate);
        else if (currentScreen == eScreen_MenuCalibration)
            drawer_Menu_Calibration(key, isUpdate);
        else if (currentScreen == eScreen_MenuOptions)
            drawer_Menu_Options(key, isUpdate);
        else if (currentScreen == eScreen_MenuStat)
            drawer_Menu_Stat(key, isUpdate);
    }
    
    return FALSE;
}

eMenuExit keyHandler_Menu_Base(eEncKey key, uint8 menuMin, uint8 menuMax)
{
    if (key >= eKeyFwd && key <= eKeyFwd10)
    {
        if (subMenuIndex < menuMax)
        {
            subMenuIndex++;
            return eMenuExit_Changed;
        }
    }
    else if (key <= eKeyBwd && key >= eKeyBwd10)
    {
        if (subMenuIndex > menuMin)
        {
            subMenuIndex--;
            return eMenuExit_Changed;
        }
    }
    else if (key == eKeyPress)
    {
        if (subMenuIndex == menuMin || subMenuIndex == menuMax)
        {
            currentScreen = eScreen_Menu;

            if (subMenuIndex == menuMin)
                return eMenuExit_Cancel;
            else
                return eMenuExit_Ok;
        }
        else
            subMenuSelected = 1;
    }

    return eMenuExit_None;
}

bool keyHandler_MenuParam_Base(SubMenu *menu, eEncKey key, uint8 index, uint8 inc)
{
    if (key >= eKeyFwd && key <= eKeyFwd10)
    {
        if (inc == 1)
            inc *= key;

        if ((*menu[index].value + inc) <= menu[index].max)
        {
            (*menu[index].value) += inc;
            return TRUE;
        }
    }
    else if (key <= eKeyBwd && key >= eKeyBwd10)
    {
        key *= -1;

        if (inc == 1)
            inc *= key;

        if (*menu[index].value >= (menu[index].min + inc))
        {
            (*menu[index].value) -= inc;
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************
    Standby menu
****************************************************************/
bool keyHandler_Menu_StandBy(eEncKey key)
{
    if (key == eKeyPress)
    {
        currentScreen++;
        currentMenu = eMainMenu_Dosing;

        return TRUE;
    }
    
    return FALSE;
}

void drawer_Menu_StandBy(eEncKey key, bool isUpdate)
{
    uint16 todayVolumeML = DEV_GetStatistics()->todayDosage;

    if (key != eKeyNone)
        SSD1306_Clear();

    if (lastTodayVolumeML != DEV_GetStatistics()->todayDosage ||
        lastTimeSeconds != DS1307_getDateTime()->seconds ||
        key != eKeyNone)
    {
        SSD1306_ClearRect(48, 48, 80, 16);
        sprintf(str, STR_FORMAT_VOL, todayVolumeML/10, todayVolumeML%10);
        SSD1306_DrawText16(54, 48, str, NO_FB, eJustify_Right); 

        lastTodayVolumeML = DEV_GetStatistics()->todayDosage;

        SSD1306_DrawText8(1, 32, STR_DOSING, eJustify_None);     
        SSD1306_DrawText16(0, 48, (char*)materialOption[DEV_GetDevOptions()->materialIdx], NO_FB, eJustify_None);  

        sprintf(str, STR_FORMAT_VOLUME"/day", DEV_GetDevTask()->volumeML/10, DEV_GetDevTask()->volumeML%10);  
        SSD1306_DrawText8(60, 32, str, eJustify_None); 
    }

    if (lastTimeSeconds != DS1307_getDateTime()->seconds ||
        key != eKeyNone)
    {
        SSD1306_ClearRect(64, 8, 64, 16);
        lastTimeSeconds = DS1307_getDateTime()->seconds;

        sprintf(str, STR_FORMAT_02D, DS1307_getDateTime()->hours);
        SSD1306_DrawText16(68, 8, str, NO_FB, eJustify_None);
        sprintf(str, STR_FORMAT_02D, DS1307_getDateTime()->minutes);
        SSD1306_DrawText16(104, 8, str, NO_FB, eJustify_None);

        if ((lastTimeSeconds % 2) == 0)
            SSD1306_DrawText16(92, 8, ":", NO_FB, eJustify_None);        
    }
}


/****************************************************************
    Main menu
****************************************************************/
bool keyHandler_Menu_Menu(eEncKey key)
{
    if (key >= eKeyFwd && key <= eKeyFwd10)
    {
        if (currentMenu < (eMainMenu_Max - 1))
        {
            currentMenu++;

            return TRUE;
        }
    }
    else if (key <= eKeyBwd && key >= eKeyBwd10)
    {
        if (currentMenu > eMainMenu_Back)
        {
            currentMenu--;
            
            return TRUE;
        }
    }
    else if (key == eKeyPress)
    {
        subMenuIndex = 1;
        subMenuSelected = 0;

        if (currentMenu == eMainMenu_Back)
            currentScreen--;
        else if (currentMenu == eMainMenu_Dosing)
        {
            currentScreen = eScreen_MenuDosing;

            *subMenu_Dosing[1].value = DEV_GetDevTask()->volumeML;
            *subMenu_Dosing[2].value = DEV_GetDevTask()->tries;
            *subMenu_Dosing[3].value = DEV_GetDevTask()->startTime;
            *subMenu_Dosing[4].value = DEV_GetDevTask()->endTime;
        }
        else if (currentMenu == eMainMenu_DateTime)
        {
            currentScreen = eScreen_MenuDateTime;

            *subMenu_DateTime[1].value = DS1307_getDateTime()->hours;
            *subMenu_DateTime[2].value = DS1307_getDateTime()->minutes;
            *subMenu_DateTime[3].value = DS1307_getDateTime()->seconds;
            *subMenu_DateTime[4].value = DS1307_getDateTime()->day;
            *subMenu_DateTime[5].value = DS1307_getDateTime()->month;
            *subMenu_DateTime[6].value = DS1307_getDateTime()->year;
        }
        else if (currentMenu == eMainMenu_Calibration)
        {
            if (calibState != eCalibration_NotStarted)
                subMenuIndex = 0;

            currentScreen = eScreen_MenuCalibration;
        }
        else if (currentMenu == eMainMenu_Options)
        {
            currentScreen = eScreen_MenuOptions;
            *subMenu_Options[1].value = DEV_GetDevOptions()->materialIdx;
            *subMenu_Options[2].value = DEV_GetDevOptions()->speedPrecise;
            *subMenu_Options[3].value = DEV_GetDevOptions()->dir;
        }
        else if (currentMenu == eMainMenu_Stat)
        {
            currentScreen = eScreen_MenuStat;
        }
        
        return TRUE;
    }
    
    return FALSE;
}

void drawer_Menu_Menu(eEncKey key, bool isUpdate)
{
    SSD1306_Clear();
    SSD1306_ScrollMenuHorizontally(32, 24, (char*) menuNames[currentMenu], (currentMenu == 0) ? eIcon_Back : eIcon_None, (key == eKeyFwd) ? eScroll_Left : eScroll_Right);
}


/****************************************************************
    Dosing menu
****************************************************************/
bool keyHandler_Menu_Dosing(eEncKey key)
{
    uint8 inc = 1;
    eMenuExit menuRes = eMenuExit_None;

    if (subMenuSelected == FALSE)
    {
        menuRes = keyHandler_Menu_Base(key, 0, MENU_DOSING_ITEMS_CNT - 1);

#ifdef MOTOR_FREQ_TEST
        DRV_RunMotorForCount(DEV_MLToTicks(MENU_CALIBRATION_VOLUME_ML * 10));
#endif
        if (menuRes == eMenuExit_Ok)
        {
            DEV_GetDevTask()->volumeML = *subMenu_Dosing[1].value;
            DEV_GetDevTask()->tries = *subMenu_Dosing[2].value;
            DEV_GetDevTask()->startTime = *subMenu_Dosing[3].value;
            DEV_GetDevTask()->endTime = *subMenu_Dosing[4].value;

            DEV_WriteCfg();
        }

        return (menuRes == eMenuExit_None) ? FALSE : TRUE;
    }
    else
    {
        if (subMenu_Dosing[subMenuIndex].value != 0)
        {
            if ((key >= eKeyFwd && key <= eKeyFwd10) || (key <= eKeyBwd && key >= eKeyBwd10))
            {
                if (subMenuSelected == 2)
                    inc = 60;

#ifdef MOTOR_FREQ_TEST
                if (key == eKeyFwd)
                    fre += 10;
                else if (key == eKeyBwd)
                    fre -= 10;

                DRV_SetMotorFreq(fre);

                return TRUE;
#else
                return keyHandler_MenuParam_Base(subMenu_Dosing, key, subMenuIndex, inc);
#endif
            }
        }
        
        if (key == eKeyPress)
        {
            if (subMenuIndex == 1 || subMenuIndex == 2)
            {
                subMenuSelected = FALSE;  
            }
            else
            {
                subMenuSelected++;

                if (subMenuSelected > 2)
                    subMenuSelected = FALSE;
            }
            
            return TRUE;
        }
    }
    
    return FALSE;
}

void drawer_Menu_Dosing(eEncKey key, bool isUpdate)
{
    ico = eIcon_None;

    if (subMenuIndex == 1)
    {
#ifdef MOTOR_FREQ_TEST
        sprintf(str, "%04dHz", fre);
#else
        sprintf(str, subMenu_Dosing[subMenuIndex].format, (*subMenu_Dosing[subMenuIndex].value)/10, (*subMenu_Dosing[subMenuIndex].value)%10);
#endif
    }
    else if (subMenuIndex == 2)
        sprintf(str, subMenu_Dosing[subMenuIndex].format, *subMenu_Dosing[subMenuIndex].value);
    else if (subMenuIndex == 3 || subMenuIndex == 4)
        sprintf(str, subMenu_Dosing[subMenuIndex].format, *subMenu_Dosing[subMenuIndex].value/60, *subMenu_Dosing[subMenuIndex].value%60);
    else
        drawer_GetMenuIconAndString(subMenuIndex, str, &ico);
  
    if (subMenuSelected == FALSE)
        drawer_DrawMenuData(key, ico, STR_DOSING, (char*) subMenu_Dosing[subMenuIndex].header);
    else
        drawer_DrawBlinkedData(str, &menuBlink);
}


/****************************************************************
    Date/Time menu
****************************************************************/
bool keyHandler_Menu_DateTime(eEncKey key)
{
    Realtime dt;
    eMenuExit res = eMenuExit_None;
  
    if (subMenuSelected == 0)
    {
        res = keyHandler_Menu_Base(key, 0, MENU_DATETIME_ITEMS_CNT - 1);

        if (res == eMenuExit_Ok)
        {
            dt.date = 1;
            dt.day = *subMenu_DateTime[4].value;
            dt.year = *subMenu_DateTime[6].value;
            dt.month = *subMenu_DateTime[5].value;
            dt.hours = *subMenu_DateTime[1].value;
            dt.minutes = *subMenu_DateTime[2].value;
            dt.seconds = *subMenu_DateTime[3].value;
        
            DS1307_writeDateTime(&dt);
        }

        return (res == eMenuExit_None) ? FALSE : TRUE; 
    }
    else
    {
        if ((key >= eKeyFwd && key <= eKeyFwd10) || (key <= eKeyBwd && key >= eKeyBwd10))
        {
        	if (subMenu_DateTime[subMenuIndex].value != 0)
            {
                return keyHandler_MenuParam_Base(subMenu_DateTime, key, subMenuIndex, 1);
    	    }
        }
        
        if (key == eKeyPress)
        {
            subMenuIndex++;

            if (subMenuIndex > 6)
            {
            	subMenuSelected = 0;
            	subMenuIndex = 6;
                prevSubItem = 0;
            }

            return TRUE;
        }
    }
    
    return FALSE;
}

void drawer_Menu_DateTime(eEncKey key, bool isUpdate)
{
    ico = eIcon_None;

    if (subMenuIndex > 0  && subMenuIndex < 4)
        sprintf(str, subMenu_DateTime[subMenuIndex].format, *subMenu_DateTime[1].value, 
                                                            *subMenu_DateTime[2].value,
                                                            *subMenu_DateTime[3].value);
    else if (subMenuIndex > 3 && subMenuIndex < 7)
        sprintf(str, subMenu_DateTime[subMenuIndex].format, *subMenu_DateTime[4].value, 
                                                            *subMenu_DateTime[5].value,
                                                            *subMenu_DateTime[6].value);
    else
        drawer_GetMenuIconAndString(subMenuIndex, str, &ico);

    if (subMenuSelected == FALSE)
    {
        if (key == eKeyPress)
            SSD1306_Clear();         
        
        if ((key >= eKeyFwd && key <= eKeyFwd10) || (key <= eKeyBwd && key >= eKeyBwd10) || key == eKeyPress)
        {
            SSD1306_DrawText8(0, 0, STR_DATETIME, eJustify_Center);
            
            if ((prevSubItem != subMenuIndex && (subMenuIndex == 0 || subMenuIndex == 7)) ||
            	(prevSubItem == 0 || prevSubItem == 7) ||
                (prevSubItem == 3 && subMenuIndex == 4) ||
                (prevSubItem == 4 && subMenuIndex == 3))
            {
            	SSD1306_ClearRect(0, 24, 128, 16);
            	SSD1306_ScrollMenuHorizontally(32, 24, str, ico, (key == eKeyFwd) ? eScroll_Left : eScroll_Right);
        	}
        }
    }
    else
        drawer_DrawBlinkedData(str, &menuBlink);

    if ((key >= eKeyFwd && key <= eKeyFwd10) || (key <= eKeyBwd && key >= eKeyBwd10) || key == eKeyPress)
    {
        SSD1306_ClearRect(0, 56, 128, 8);
        SSD1306_DrawText8(0, 56, (char*) subMenu_DateTime[subMenuIndex].header, eJustify_Center);
    }
    
    prevSubItem = subMenuIndex;
}



/****************************************************************
    Calibration menu
****************************************************************/
bool keyHandler_Menu_Calibration(eEncKey key)
{
    if (subMenuSelected == FALSE)
    {
        if (calibState == eCalibration_NotStarted)
        {    
            if (key >= eKeyFwd && key <= eKeyFwd10)
            {
                if (subMenuIndex == 0 || subMenuIndex == 1)
                {
                    subMenuIndex++;
                    return TRUE;
                }
            }
            else if (key <= eKeyBwd && key >= eKeyBwd10)
            {
                if (subMenuIndex == 2 || subMenuIndex == 1)
                {
                    subMenuIndex--;
                    return TRUE;
                }
            }
            else if (key == eKeyPress)
            {
                if (subMenuIndex == 2)
                {
                    calibState = eCalibration_CheckStarted;
                    DRV_RunMotorForCount(DEV_MLToTicks(MENU_CALIBRATION_VOLUME_ML * 10));
                    subMenuSelected = TRUE;
                    subMenuIndex = 3;

                    return TRUE;
                }
                else  if (subMenuIndex == 1)
                {
                    calibState = eCalibration_CalStarted;
                    DRV_RunMotorForCount(DEV_MLToTicks(MENU_CALIBRATION_VOLUME_ML * 10));
                    subMenuSelected = TRUE;
                    subMenuIndex = 3;

                    return TRUE;
                }
                else if (subMenuIndex == 0)
                {
                    currentScreen = eScreen_Menu;
                    calibState = eCalibration_NotStarted;
                    return TRUE;
                }
            }
        }
        else if (calibState == eCalibration_CalDone)
        {
            if (key >= eKeyFwd && key <= eKeyFwd10)
            {
                if (subMenuIndex == 0)
                {
                    subMenuIndex = 4;
                    return TRUE;
                }
                else if (subMenuIndex == 4) 
                {
                    subMenuIndex = 5;
                    return TRUE;
                }
            }
            else if (key <= eKeyBwd && key >= eKeyBwd10)
            {
                if (subMenuIndex == 5)
                {
                    subMenuIndex = 4;
                    return TRUE;
                }
                else if (subMenuIndex == 4)
                {
                    subMenuIndex = 0;
                    return TRUE;
                }
            }
            else if (key == eKeyPress)
            {
                if (subMenuIndex == 4)
                {      
                    subMenuSelected = TRUE;
                    return TRUE;
                }
                else if (subMenuIndex == 0 || subMenuIndex == 5)
                {
                    currentScreen = eScreen_Menu;
                    calibState = eCalibration_NotStarted;

                    if (subMenuIndex == 5)
                        DEV_CalcAndSaveCalibration(*subMenu_Calib[4].value, MENU_CALIBRATION_VOLUME_ML * 10, DS1307_getDateTime());
                    
                    return TRUE;
                }
            }
        }
        else if (calibState == eCalibration_CheckDone)
        {
            if (key >= eKeyFwd && key <= eKeyFwd10)
            {
                if (subMenuIndex == 0)
                {
                    subMenuIndex = 3;
                    return TRUE;
                }
                else if (subMenuIndex == 3) 
                {
                    subMenuIndex = 5;
                    return TRUE;
                }
            }
            else if (key <= eKeyBwd && key >= eKeyBwd10)
            {
                if (subMenuIndex == 5)
                {
                    subMenuIndex = 3;
                    return TRUE;
                }
                else if (subMenuIndex == 3)
                {
                    subMenuIndex = 0;
                    return TRUE;
                }
            }
            else if (key == eKeyPress)
            {
                if (subMenuIndex == 0 || subMenuIndex == 5)
                {
                    currentScreen = eScreen_Menu;
                    calibState = eCalibration_NotStarted;
                    return TRUE;
                }
            }
        }
    }
    else
    {
        if (calibState == eCalibration_CalDone)
        {
            if ( (key >= eKeyFwd && key <= eKeyFwd10) || (key <= eKeyBwd && key >= eKeyBwd10))
                return keyHandler_MenuParam_Base(subMenu_Calib, key, subMenuIndex, 1);
            else if (key == eKeyPress)
            {
                subMenuSelected = FALSE;
                return TRUE;
            }
        }
        else if (calibState == eCalibration_CheckStarted || calibState == eCalibration_CalStarted)
        {
            if (key == eKeyPress)
            {
                DRV_StopMotor();
                calibState = eCalibration_NotStarted;
                subMenuIndex = 0;
                subMenuSelected = FALSE;

                return TRUE;
            }
        }
    }

    return FALSE;
}

void drawer_Menu_Calibration(eEncKey key, bool isUpdate)
{
    ico = eIcon_None;

    if (subMenuIndex == 1)
        sprintf(str, subMenu_Calib[subMenuIndex].format, STR_START);
    else if (subMenuIndex == 2)
        sprintf(str, subMenu_Calib[subMenuIndex].format, STR_CHECK);
    else if (subMenuIndex > 2  && subMenuIndex < 5)
    {
        sprintf(str, subMenu_Calib[subMenuIndex].format, *subMenu_Calib[subMenuIndex].value/10, *subMenu_Calib[subMenuIndex].value % 10);
    }
    else
        drawer_GetMenuIconAndString(subMenuIndex, str, &ico);
  
    if (subMenuSelected == FALSE)
        drawer_DrawMenuData(key, ico, STR_CALIBRATION, (char*) subMenu_Calib[subMenuIndex].header);
    else
    {
        if (calibState == eCalibration_CalStarted || calibState == eCalibration_CheckStarted || calibState == eCalibration_CalDone)
        {
            SSD1306_ClearRect(0, 24, 128, 16);
            SSD1306_ClearRect(0, 56, 128, 8);
        }

        if (calibState == eCalibration_CalStarted || calibState == eCalibration_CheckStarted)
        {
            SSD1306_DrawText16(34, 24, str, NO_FB, eJustify_Center);
            SSD1306_DrawText8(0, 56, (char*) subMenu_Calib[subMenuIndex].header, eJustify_Center);
        }
        else if (calibState == eCalibration_CalDone)
        {
            SSD1306_DrawText8(0, 56, (char*) subMenu_Calib[subMenuIndex].header, eJustify_Center);
            drawer_DrawBlinkedData(str, &menuBlink);
        }
    }
}



/****************************************************************
    Options menu
****************************************************************/
bool keyHandler_Menu_Options(eEncKey key)
{
    eMenuExit res = eMenuExit_None;

    if (subMenuSelected == 0)
    {
        res = keyHandler_Menu_Base(key, 0, MENU_OPTIONS_ITEMS_CNT - 1);

        if (res == eMenuExit_Ok)
        {
            DEV_GetDevOptions()->materialIdx = *subMenu_Options[1].value;
            DEV_GetDevOptions()->speedPrecise = *subMenu_Options[2].value;
            DEV_GetDevOptions()->dir = *subMenu_Options[3].value;
            DEV_WriteCfg();

            DRV_SetMotorMode((eMotorSpeedMode) DEV_GetDevOptions()->speedPrecise, (eDir) DEV_GetDevOptions()->dir);
        }

        return (res == eMenuExit_None) ? FALSE : TRUE; 
    }
    else
    {
        if ((key >= eKeyFwd && key <= eKeyFwd10) || (key <= eKeyBwd && key >= eKeyBwd10))
        {
            if (subMenu_Options[subMenuIndex].value != 0)
                return keyHandler_MenuParam_Base(subMenu_Options, key, subMenuIndex, 1);
        }
        else if (key == eKeyPress)
        {
            subMenuSelected = FALSE;

            return TRUE;
        }
    }

    return FALSE;
}

void drawer_Menu_Options(eEncKey key, bool isUpdate)
{
    ico = eIcon_None;

    if (subMenuIndex == 1)
        sprintf(str, subMenu_Options[subMenuIndex].format, materialOption[*subMenu_Options[subMenuIndex].value]);
    else if (subMenuIndex == 2)
        sprintf(str, subMenu_Options[subMenuIndex].format, speedOption[*subMenu_Options[subMenuIndex].value]);
    else if (subMenuIndex == 3)
        sprintf(str, subMenu_Options[subMenuIndex].format, dirOption[*subMenu_Options[subMenuIndex].value]);
    else
        drawer_GetMenuIconAndString(subMenuIndex, str, &ico);

    if (subMenuSelected == FALSE)
        drawer_DrawMenuData(key, ico, STR_OPTIONS, (char*) subMenu_Options[subMenuIndex].header);
    else
        drawer_DrawBlinkedData(str, &menuBlink);
}


/****************************************************************
    Statistics menu
****************************************************************/
bool keyHandler_Menu_Stat(eEncKey key)
{
    eMenuExit res;

    res = keyHandler_Menu_Base(key, 0, MENU_STAT_ITEMS_CNT - 1);
    
    return (res == eMenuExit_None) ? FALSE : TRUE; 
}

void drawer_Menu_Stat(eEncKey key, bool isUpdate)
{
    ico = eIcon_None;
    uint32 p1 = 0;
    uint32 p2 = 0;

    if (subMenuIndex == 2)
    {
        p1 = DEV_GetStatistics()->sinceCalDosage/10;
        p2 = DEV_GetStatistics()->sinceCalDosage%10;
    }
    else if (subMenuIndex == 4)
    {
        p1 = DEV_GetStatistics()->totalDosage/10;
        p2 = DEV_GetStatistics()->totalDosage%10;
    }
    else if (subMenuIndex == 3)
    {
        p1 = DEV_GetStatistics()->sinceCalTicks;
        p1 /= DEV_MOTOR_STEPS_PER_REV;
    }
    else if (subMenuIndex == 5)
    {
        p1 = DEV_GetStatistics()->totalTicks;
        p1 /= DEV_MOTOR_STEPS_PER_REV;
    }

    if (subMenuIndex == 1)
    {
        p1 = getUpTime();
        p1 /= 60;
        sprintf(str, subMenu_Stat[subMenuIndex].format, p1/1440, (p1%1440)/60, p1%60);
    }
    else if (subMenuIndex > 1 && subMenuIndex < 6)
        sprintf(str, subMenu_Stat[subMenuIndex].format, p1, p2);
    else if (subMenuIndex == 6)
        sprintf(str, subMenu_Stat[subMenuIndex].format, DEV_GetStatistics()->dateTime.day, DEV_GetStatistics()->dateTime.month, DEV_GetStatistics()->dateTime.year);
    else
        drawer_GetMenuIconAndString(subMenuIndex, str, &ico);

    drawer_DrawMenuData(key, ico, STR_STAT, (char*) subMenu_Stat[subMenuIndex].header);
}

void drawer_DrawMenuData(eEncKey key, eIcon ico, char *header, char *subHeader)
{
    if (key == eKeyPress)
        SSD1306_Clear();

    if ((key >= eKeyFwd && key <= eKeyFwd10) || (key <= eKeyBwd && key >= eKeyBwd10) || key == eKeyPress)
    {
        drawer_ClearMenu();  
        SSD1306_DrawText8(0, 0, header, eJustify_Center);
        SSD1306_DrawText8(0, 56, subHeader, eJustify_Center);
        SSD1306_ScrollMenuHorizontally(32, 24, str, ico, (key == eKeyFwd) ? eScroll_Left : eScroll_Right);
    }
}

void drawer_DrawBlinkedData(char *str, uint8 *menuBlink)
{
    (*menuBlink)++;
      
    SSD1306_ClearRect(0, 24, 128, 16);
        
    if (((*menuBlink) & 1) == 0)
        SSD1306_DrawText16(0, 24, str, NO_FB, eJustify_Center);
}

void drawer_GetMenuIconAndString(uint8 idx, char *str, eIcon *icon)
{
    sprintf(str, " ");

    if (idx == 0)
        *icon = eIcon_Cancel;
    else
        *icon = eIcon_Ok;
}

void drawer_ClearMenu()
{
    SSD1306_ClearRect(0, 24, 128, 16);
    SSD1306_ClearRect(0, 56, 128, 8);
}

/*
void DRV_Sound(uint8 tries)
{
    while (tries > 0)
    {  
        DRV_SetMotorMode((eMotorSpeedMode) DEV_GetDevOptions()->speedPrecise, (eDir) (soundNum%2));
        DRV_RunMotorForCount(150);

        while (DRV_IsMotorRunning() > 0);
        
        tries--;
        soundNum++;
        
        if (tries > 0)
            delay_ms(20);
    }

    DRV_SetMotorMode((eMotorSpeedMode) DEV_GetDevOptions()->speedPrecise, (eDir) DEV_GetDevOptions()->dir);
    delay_ms(5);
}
*/