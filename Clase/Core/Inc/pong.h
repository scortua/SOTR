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
#include "Events.h"


typedef enum{
	Playing = 0,
	GOL
}Status_Pong;

void DrawPlayers(void);

void UpdateHeadPlayer1(u32 head);

void UpdateHeadPlayer2(u32 head);

void DrawPong(void);

void PongReset(void);

Status_Pong PongMovement(void);

void DrawText(void);

void DrawScore(void);

void ResetScore(void);

#endif /* INC_PONG_H_ */
