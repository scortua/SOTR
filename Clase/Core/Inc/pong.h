/*
 * pong.h
 *
 *  Created on: Nov 30, 2025
 *      Author: jabel
 */

#ifndef INC_PONG_H_
#define INC_PONG_H_

#include "SH1106.h"
#include "AppTypes.h"

void DrawPlayers(void);

void UpdateHeadPlayer1(u32 head);

void UpdateHeadPlayer2(u32 head);

void DrawPong(void);

#endif /* INC_PONG_H_ */
