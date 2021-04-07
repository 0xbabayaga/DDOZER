#include "ascii.h"

uint8* ASCII_FontPtr(char c)
{
    if (c < 0xC0 && c >= 0x20)
        return (uint8*) &ASCII_EN[(c - 0x20) * SMALL_FONT_WIDTH];
    else if (c >= 0xC0)
        return (uint8*) &ASCII_RU[(c - 0xC0) * SMALL_FONT_WIDTH];
    else
        return (uint8*) ASCII_EN;
}