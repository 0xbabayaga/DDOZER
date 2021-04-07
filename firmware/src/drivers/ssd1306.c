#include <iostm8s105k4.h>
#include "ssd1306.h"
#include "../device/delay_us.h"
#include  "ascii.h"
#include <string.h>

#define READ_TIMEOUT            200
#define INIT_SEQUENCE_SIZE      28

#define CS_HIGH                 (PE_ODR_ODR5 = 1)   //(GPIOA->BSRR = GPIO_Pin_15)
#define CS_LOW                  (PE_ODR_ODR5 = 0)   //(GPIOA->BRR = GPIO_Pin_15)

#define RES_HIGH                (PC_ODR_ODR4 = 1)   //(GPIOB->BSRR = GPIO_Pin_4)
#define RES_LOW                 (PC_ODR_ODR4 = 0)   //(GPIOB->BRR = GPIO_Pin_4)

#define DC_HIGH                 (PC_ODR_ODR3 = 1)   //(GPIOB->BSRR = GPIO_Pin_12)
#define DC_LOW                  (PC_ODR_ODR3 = 0)   //(GPIOB->BRR = GPIO_Pin_12)

static const uint8 initSequence[INIT_SEQUENCE_SIZE] = {   
                                0xAE, 0x20, 0x10, 0xB0, 0xC8, 0x00, 0x10, 0x40,
                                0x81, 0xFF, 0xA1, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3,
                                0x00, 0xD5, 0xF0, 0xD9, 0x22, 0xDA, 0x12, 0xDB,
                                0x20, 0x8D, 0x14, 0xAF };

static uint8 frameBuffer[SSD1306_FRAME_ROW_HEIGHT][SSD1306_WIDTH] = {0};

/* Local software functions */
static void clearFB();
static void drawFB(uint8 x, uint8 y, uint8 shift, uint8 width, uint8 height);
static uint16 getText16PixelWidth(char *str);

/* Local hardware functions */
static void ssd1306_HardwareInit(void);
static void ssd1306_Write(char data);
static void ssd1306_Reset(void);
static void ssd1306_WriteCommand(uint8 byte);
static void ssd1306_WriteData(uint8* buffer, uint16 buff_size);


void SSD1306_DrawIcon(uint8 x, uint8 y, uint8 icon, bool fb)
{
    uint8 page = y / 8;
    uint8 i = 0;
    uint8 *ptr = (uint8*) &ICONS[icon][0]; 
    uint8 fbX = x;
    
    if ((y%8) == 0)
    {
        for (i = 0; i < 2; i++)
        {
            fbX = x;
          
            if (fb == FALSE)
            {
                ssd1306_WriteCommand(0xB0 + page); 
                ssd1306_WriteCommand(x & 0x0F);
                ssd1306_WriteCommand(0x10|((x & 0xF0)>>4));
                    
                CS_LOW;
                DC_HIGH;
            }
            
            if (fb == FALSE)
                ssd1306_WriteData(ptr, ICON_WIDTH); 
            else
            {
                memcpy(&frameBuffer[i][fbX], ptr, ICON_WIDTH);
                ptr += ICON_WIDTH;
            }
            
            page++;
        }
    }
}

void SSD1306_ScrollMenuHorizontally(uint8 x, uint8 y, char *str, eIcon icon, eScrollDir dir)
{
    uint8 cycles = SSD1306_WIDTH - 1;
    uint8 xx = 0;
    uint16 txtWidth = 0;
    uint16 iconX = 0;
  
    txtWidth = getText16PixelWidth(str);
    
    clearFB();
    SSD1306_DrawText16(0, y, str, FB, eJustify_Center);
    
    if (icon != eIcon_None && icon < eIcon_Total)
    {
        if (txtWidth == 0)
            iconX = SSD1306_WIDTH - ICON_WIDTH / 2;
        else
            iconX = ((SSD1306_WIDTH - txtWidth) >> 1) - ICON_MARGIN;
          
        SSD1306_DrawIcon(iconX, y, icon, TRUE);
    }
    
    if (dir == eScroll_Right)
        xx = 0;
    else
        xx = SSD1306_WIDTH - 1;
    
    while(1)
    {
        if (dir == eScroll_Right)
        {
            drawFB(0, y, SSD1306_WIDTH - xx - 1, xx + 1, 16);
            xx++;
        }
        else
        {
            drawFB(xx, y, 0, SSD1306_WIDTH - xx, 16);
            xx--;
        }
            
        cycles--;
        
        if (cycles == 0)
            break;
              
        delay_us(1000);
    }
}

uint16 getText16PixelWidth(char *str)
{
    uint16 width = 0;
    uint8 i = 0;
    uint8 len = strlen(str);
    uint16 idx = 0;
    
    for (i = 0; i < len; i++)
    {
        if (*str >= 0x20 && *str < 0x80)
        {
            idx = *str;
            idx -= 0x20;
            idx *= FONT16_SYMBOL_SIZE;
          
            width += Segoe_UI16[idx];
        }
        
        str++;
    }
    
    return width;
}


void SSD1306_DrawText8(uint8 x, uint8 y, char *str, eTextJustify justify)
{
    uint8 page = y / 8;
    uint8 i = 0;
    uint8 len = strlen(str);
    uint8 add_x = 0;
    
    if (justify == eJustify_Center)
    {
        x = SSD1306_WIDTH;
        x -= (len * (SMALL_FONT_WIDTH + 1));
        x /= 2;
    }
    
    if ((y%8) == 0)
    {
        for (i = 0; i < len; i++)
        {
            add_x = x + i * (SMALL_FONT_WIDTH + 1);
            ssd1306_WriteCommand(0xB0 + page);
            ssd1306_WriteCommand(add_x & 0x0F);
            ssd1306_WriteCommand(0x10|((add_x & 0xF0)>>4));
            ssd1306_WriteData(ASCII_FontPtr(*str), SMALL_FONT_WIDTH);
            str++;
        }
    }
}


void SSD1306_DrawText16(uint8 x, uint8 y, char *str, bool fb, eTextJustify justify)
{
    uint8 page = y / 8;
    uint8 i = 0;
    uint8 r = 0;
    uint8 len = strlen(str);
    uint8 fntWidth = 0;
    uint8 toWrite = 0;
    uint8 *fntPtr = 0; 
    uint16 idx = 0;
    uint8 fntX = x;

    if (justify == eJustify_Center)
    {
        x = SSD1306_WIDTH - getText16PixelWidth(str);
        x >>= 1;
        fntX = x;
    }
    else if (justify == eJustify_Right)
    {
        x = SSD1306_WIDTH - getText16PixelWidth(str);
        fntX = x;
    }
    
    if ((y%8) == 0)
    {
        for (r = 0; r < 2; r++)
        {
            for (i = 0; i < len; i++)
            {
                if (*str >= 0x20 && *str < 0x80)
                {
                    idx = *str;
                    idx -= 0x20;
                    idx *= FONT16_SYMBOL_SIZE;
                  
                    fntPtr = (uint8*) &Segoe_UI16[idx];
                    fntWidth = *fntPtr;
                    fntPtr++;
                    fntPtr += r;
                 
                    if (fb == FALSE)
                    {
                        ssd1306_WriteCommand(0xB0 + page); 
                        ssd1306_WriteCommand(fntX&0x0F);
                        ssd1306_WriteCommand(0x10|((fntX&0xF0)>>4));
                    
                        CS_LOW;
                        DC_HIGH;
                    }
                    
                    toWrite = fntWidth;
                    
                    while (toWrite > 0)
                    {
                        if (fb == 0)
                            ssd1306_Write(*fntPtr);
                        else
                        {
                            frameBuffer[r][fntX] = *fntPtr;
                            fntX++;
                        }
                        
                        fntPtr += 2;
                        toWrite--;
                    }
                    
                    if (fb == FALSE)
                    {
                        CS_HIGH;
                        fntX += fntWidth;
                    }
                }
                
                str++;
            }

            str -= len;
            page++;
            fntX = x;
        }
    }
}

void SSD1306_Brightness(uint8 level)
{
    ssd1306_WriteCommand(0x81); 
    ssd1306_WriteCommand(level); 
}

void SSD1306_ClearRect(uint8 x, uint8 y, uint8 w, uint8 h)
{
    uint8 page = y / 8;
    uint8 i = 0;
    uint8 n = 0;
    
    for (n = 0; n < (h/8); n++)
    {
        ssd1306_WriteCommand(0xB0 + page + n); 
        ssd1306_WriteCommand(x&0x0F);
        ssd1306_WriteCommand(0x10|((x & 0xF0)>>4));
                    
        CS_LOW;
        DC_HIGH;
                    
        i = w;
        
        while (i > 0)
        {
            ssd1306_Write(0);
            i--;
        }
                    
        CS_HIGH;
    }
}

void clearFB()
{
    memset(&frameBuffer[0][0], 0, SSD1306_FRAME_ROW_HEIGHT * SSD1306_WIDTH);
}

void drawFB(uint8 x, uint8 y, uint8 shift, uint8 width, uint8 height)
{
    uint8 page = y / 8;
    uint8 i = 0;
    
    for (i = 0; i < height / 8; i++)
    {
        ssd1306_WriteCommand(0xB0 + page + i);
        ssd1306_WriteCommand(x & 0x0F);
        ssd1306_WriteCommand(0x10|((x & 0xF0)>>4));
        ssd1306_WriteData(&frameBuffer[i][shift], width);
    }
}

// Initialize the oled screen
void SSD1306_Init(void) 
{
    uint8 i = 0;

    ssd1306_HardwareInit();
  
    // Reset OLED
    ssd1306_Reset();

    // Wait for the screen to boot
    delay_ms(100);
    
    while (i < INIT_SEQUENCE_SIZE)
    {
        ssd1306_WriteCommand(initSequence[i]);
        i++;
    }

    SSD1306_Clear();
}

void ssd1306_Reset(void)
{
    //cs = high
    CS_HIGH;
  
    //res = low
    RES_LOW;
    delay_ms(50);
    
    //res = high
    RES_HIGH;
    delay_ms(50);
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8 byte) 
{
    CS_LOW;
    DC_LOW;
    ssd1306_Write(byte);
    CS_HIGH;
}

// Send data
void ssd1306_WriteData(uint8* buffer, uint16 buff_size) 
{
    CS_LOW;
    DC_HIGH;
    
    while (buff_size > 0)
    {
        ssd1306_Write(*buffer);
        buffer++;
        buff_size--;
    }
    
    CS_HIGH;
}

// Send data
void SSD1306_Clear(void) 
{
    uint8 i = 0;
    uint8 n = SSD1306_WIDTH;
  
    for (i = 0; i < SSD1306_HEIGHT / 8; i++)
    {
        ssd1306_WriteCommand(0xB0 + i);
        ssd1306_WriteCommand(0);
        ssd1306_WriteCommand(0x10);
  
        CS_LOW;
        DC_HIGH;
        
        n = SSD1306_WIDTH;
        
        while (n > 0)
        {
            ssd1306_Write(0);
            n--;
        }

        CS_HIGH;
    }
}

void ssd1306_HardwareInit(void)
{
    //Configuring SPI-bus
    SPI_CR1 = 0;
    SPI_CR2 = 0;
  
    SPI_CR1 = 0x07; // 0b00000111;
    SPI_CR2 = 0x43; // 0b01000011;
    SPI_ICR = 0x00; // 0b10000000;
    SPI_SR = 0x02;  // 0b00000010;
    
    SPI_CR1 |= 0x40; //Enabling SPI
}

void ssd1306_Write(char data)
{
    SPI_DR = data;
    
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
}


#pragma vector = SPI_TXE_vector
__interrupt void SPI_interrupt_handler(void)
{
    if (TIM4_SR_UIF == 1)
    { 

    }
}