#ifndef SH1106_H
#define SH1106_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include "main.h"


#include "stdlib.h"
#include "string.h"

#include "fonts.h"


/* I2C address */
#ifndef SH1106_I2C_ADDR
#define SH1106_I2C_ADDR         0x3C<<1
#endif

/* SH1106 settings */
/* SH1106 width in pixels */
#ifndef SH1106_WIDTH
#define SH1106_WIDTH            132
#endif
/* SH1106 LCD height in pixels */
#ifndef SH1106_HEIGHT
#define SH1106_HEIGHT           64
#endif

/**
 * @brief  SH1106 color enumeration
 */
typedef enum {
	SH1106_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
	SH1106_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} SH1106_COLOR_t;


uint8_t SH1106_Init(void);

void SH1106_UpdateScreen(void);

void SH1106_Fill(SH1106_COLOR_t Color);

void SH1106_DrawPixel(uint16_t x, uint16_t y, SH1106_COLOR_t color);

void SH1106_GotoXY(uint16_t x, uint16_t y);

char SH1106_Putc(char ch, FontDef_t* Font, SH1106_COLOR_t color) ;

char SH1106_Puts(char* str, FontDef_t* Font, SH1106_COLOR_t color);

void SH1106_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count);

void SH1106_I2C_Write(uint8_t address, uint8_t reg, uint8_t data);

// clear the display

void SH1106_Clear (void);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
