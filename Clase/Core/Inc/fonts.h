/*
 * fonts.h
 *
 *  Created on: Dec 5, 2025
 *      Author: jabel
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include "AppTypes.h"

typedef struct {
	u8 FontWidth;    /*!< Font width in pixels */
	u8 FontHeight;   /*!< Font height in pixels */
	const u16 *data; /*!< Pointer to data font data array */
} FontDef_t;

extern FontDef_t Font_7x10;
extern FontDef_t Font_6x8;

#endif /* INC_FONTS_H_ */
