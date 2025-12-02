/**
 * original author:  Tilen Majerle<tilen@majerle.eu>
 * modification for SH1106: ControllersTech (www.controllerstech.com)

   ----------------------------------------------------------------------
   	Copyright (C) Alexander Lutsai, 2016
    Copyright (C) Tilen Majerle, 2015

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
 */
#include "SH1106.h"

extern I2C_HandleTypeDef hi2c1;
#define SH1106_I2C &hi2c1

static uint8_t SH1106_Buffer[SH1106_WIDTH * SH1106_HEIGHT / 8];

/* Write command */
#define SH1106_WRITECOMMAND(command)      SH1106_I2C_Write(SH1106_I2C_ADDR, 0x00, (command))
/* Write data */
#define SH1106_WRITEDATA(data)            SH1106_I2C_Write(SH1106_I2C_ADDR, 0x40, (data))
/* Absolute value */

/* Private SH1106 structure */
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SH1106_t;

/* Private variable */
static SH1106_t SH1106;

#define SH1106_NORMALDISPLAY       0xA6
#define SH1106_INVERTDISPLAY       0xA7

uint8_t SH1106_Init(void) {
	  // Initialize the display
	SH1106_WRITECOMMAND(0xAE); //display off
	SH1106_WRITECOMMAND(0xB0|0x00); //Set Page Start Address for Page Addressing Mode,0-7
	SH1106_WRITECOMMAND(0x81); //--set contrast control register
	SH1106_WRITECOMMAND(0xFF); // contrast value
	SH1106_WRITECOMMAND(0xA1); //--set segment re-map 0 to 127
	SH1106_WRITECOMMAND(0xA6); //--set normal display
	SH1106_WRITECOMMAND(0xA8); //--set multiplex ratio(1 to 64)
	SH1106_WRITECOMMAND(0x3F); // multiplex value
	SH1106_WRITECOMMAND(0xAD); // Set Pump Mode
	SH1106_WRITECOMMAND(0x8B); // Pump ON
	SH1106_WRITECOMMAND(0x30|0x02); // Set Pump Voltage 8.0
	SH1106_WRITECOMMAND(0xC8); //Set COM Output Scan Direction
	SH1106_WRITECOMMAND(0xD3); //-set display offset
	SH1106_WRITECOMMAND(0x00); //-not offset
	SH1106_WRITECOMMAND(0xD5); //--set display clock divide ratio/oscillator frequency
	SH1106_WRITECOMMAND(0x80); //--set divide ratio
	SH1106_WRITECOMMAND(0xD9); //--set pre-charge period
	SH1106_WRITECOMMAND(0x1F); //
	SH1106_WRITECOMMAND(0xDA); //--set com pins hardware configuration
	SH1106_WRITECOMMAND(0x12);
	SH1106_WRITECOMMAND(0xDB); //--set vcomh
	SH1106_WRITECOMMAND(0x40); //
	SH1106_WRITECOMMAND(0xAF); //--turn on SH1106 panel


	/* Clear screen */
	SH1106_Fill(SH1106_COLOR_BLACK);

	/* Update screen */
	SH1106_UpdateScreen();

	/* Set default values */
	SH1106.CurrentX = 0;
	SH1106.CurrentY = 0;

	/* Initialized OK */
	SH1106.Initialized = 1;

	/* Return OK */
	return 1;
}

void SH1106_UpdateScreen(void) {
	uint8_t m;

	for (m = 0; m < 8; m++) {
		SH1106_WRITECOMMAND(0xB0 + m);
		SH1106_WRITECOMMAND(0x00);
		SH1106_WRITECOMMAND(0x10);

		/* Write multi data */
		SH1106_I2C_WriteMulti(SH1106_I2C_ADDR, 0x40, &SH1106_Buffer[SH1106_WIDTH * m], SH1106_WIDTH);
	}
}

void SH1106_Fill(SH1106_COLOR_t color) {
	/* Set memory */
	memset(SH1106_Buffer, (color == SH1106_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SH1106_Buffer));
}

void SH1106_DrawPixel(uint16_t x, uint16_t y, SH1106_COLOR_t color) {
	if (
		x >= SH1106_WIDTH ||
		y >= SH1106_HEIGHT
	) {
		/* Error */
		return;
	}

	/* Check if pixels are inverted */
	if (SH1106.Inverted) {
		color = (SH1106_COLOR_t)!color;
	}

	/* Set color */
	if (color == SH1106_COLOR_WHITE) {
		SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] |= 1 << (y % 8);
	} else {
		SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
	}
}

void SH1106_GotoXY(uint16_t x, uint16_t y) {
	/* Set write pointers */
	SH1106.CurrentX = x;
	SH1106.CurrentY = y;
}


void SH1106_Clear (void)
{
	SH1106_Fill (0);
    SH1106_UpdateScreen();
}

void SH1106_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
uint8_t dt[256];
dt[0] = reg;
uint8_t i;
for(i = 0; i < count; i++)
dt[i+1] = data[i];
HAL_I2C_Master_Transmit(SH1106_I2C, address, dt, count+1, 10);
}

void SH1106_I2C_Write(uint8_t address, uint8_t reg, uint8_t data) {
	uint8_t dt[2];
	dt[0] = reg;
	dt[1] = data;
	HAL_I2C_Master_Transmit(SH1106_I2C, address, dt, 2, 10);
}
