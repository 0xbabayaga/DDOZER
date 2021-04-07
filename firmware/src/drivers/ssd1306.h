#ifndef _SSD1306_DRIVER_H
#define _SSD1306_DRIVER_H

#include "stm8s_type.h"
#include "../base/types.h"
#include "ascii.h"

#define SSD1306_HEIGHT              64
#define SSD1306_WIDTH               128
#define SSD1306_FRAME_ROW_HEIGHT    2
#define SSD1306_BRIGHTNESS_MIN		4
#define SSD1306_BRIGHTNESS_MAX		100

#define CENTERED              		(bool)TRUE
#define FB              			(bool)TRUE
#define NO_FB              			(bool)FALSE

typedef enum
{
    eScroll_None = 0,
    eScroll_Left = 1,
    eScroll_Right = 2,
    eScroll_Up = 3,
    eScroll_Down = 4
}   eScrollDir;

typedef enum
{
	eJustify_None = 0,
	eJustify_Left = 1,
	eJustify_Right = 2,
	eJustify_Center = 3
}	eTextJustify;

void SSD1306_DrawIcon(uint8 x, uint8 y, uint8 icon, bool fb);
void SSD1306_ScrollMenuHorizontally(uint8 x, uint8 y, char *str, eIcon icon, eScrollDir dir);
void SSD1306_DrawText8(uint8 x, uint8 y, char *str, eTextJustify justify);
void SSD1306_DrawText16(uint8 x, uint8 y, char *str, bool fb, eTextJustify justify);
void SSD1306_ClearRect(uint8 x, uint8 y, uint8 w, uint8 h);
void SSD1306_Clear(void); 
void SSD1306_Brightness(uint8 level);

// Procedure definitions
void SSD1306_Init(void);

#endif