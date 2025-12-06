/*
 * pong.c
 *
 *  Created on: Nov 30, 2025
 *      Author: jabel
 */


#include "pong.h"

#define ratio_conversion		63/4095

#define tamanio_jugador			10

#define tamanio_pong			4

#define x_center				64
#define y_center				30

#define Speed_y					2
#define Speed_x					1

extern EventHandler_t Event;

s16 head_player_1;
s16 head_player_2;

s16 Head_Pong_x = x_center;
s16 Head_Pong_y = y_center;

s8 Speed_Pong_x = Speed_x;
s8 Speed_Pong_y = Speed_y;

u8 Score_player_1 = 0;
u8 Score_player_2 = 0;

char Score_player_1_c[2];
char Score_player_2_c[2];

void DrawPlayers(void){
	for(int i = head_player_1;i < head_player_1 + tamanio_jugador;i++){
		SH1106_DrawPixel(2, i, SH1106_COLOR_WHITE);
	}
	for(int i = head_player_2;i < head_player_2 + tamanio_jugador;i++){
		SH1106_DrawPixel(129, i, SH1106_COLOR_WHITE);
	}
}

void UpdateHeadPlayer1(u32 head){
	head_player_1 = (head & 0X00000FFF) * ratio_conversion;
}

void UpdateHeadPlayer2(u32 head){
	head_player_2 = ((head >> 16) & 0X00000FFF) * ratio_conversion;
}

/*
void DrawPong(u8 x, u8 y){
	for(int i = y; i < y + tamanio_pong; i++){
		for(int j = x; j < x + tamanio_pong; j++){
			SH1106_DrawPixel(j, i, SH1106_COLOR_WHITE);
		}
	}
	SH1106_UpdateScreen();
}
*/

void DrawPong(void){
	for(int i = Head_Pong_y; i < Head_Pong_y + tamanio_pong; i++){
		for(int j = Head_Pong_x; j < Head_Pong_x + tamanio_pong; j++){
			SH1106_DrawPixel(j, i, SH1106_COLOR_WHITE);
		}
	}
}

void PongReset(void){
	Head_Pong_x = x_center;
	Head_Pong_y = y_center;
	Speed_Pong_x = Speed_x;
	Speed_Pong_y = Speed_y;
}

Status_Pong PongMovement(void){

	Status_Pong Status = Playing;

	// Cambio de velocidades debido a colision con paredes

	if((Speed_Pong_y == -Speed_y) && ((Head_Pong_y + Speed_Pong_y) < 0)){
		Head_Pong_y += -Speed_Pong_y - (2 * Head_Pong_y);
		Speed_Pong_y = Speed_y;
		Events_Set(&Event, EVENT_WALL_COLLISION);
	}else{
		if((Speed_Pong_y == Speed_y) && ((Head_Pong_y + Speed_Pong_y) > 63 - tamanio_pong - 1)){
			Head_Pong_y += 126 - (2*(Head_Pong_y + tamanio_pong - 1)) - Speed_Pong_y;;
			Speed_Pong_y = - Speed_y;
			Events_Set(&Event, EVENT_WALL_COLLISION);
		}else{
			// Cambio de velocidades debido a la colision con las paredes (exacto)
			if(Head_Pong_y == 0){
				Speed_Pong_y = Speed_y;
				Head_Pong_y += Speed_Pong_y;
				Events_Set(&Event, EVENT_WALL_COLLISION);
			}else{
				if(Head_Pong_y == 59){
					Speed_Pong_y = -Speed_y;
					Head_Pong_y += Speed_Pong_y;
					Events_Set(&Event, EVENT_WALL_COLLISION);
				}else{
					Head_Pong_y += Speed_Pong_y;
				}
			}
		}
	}

	// Cambio de velocidades debido a la colision con los jugadores
	if((Head_Pong_x == 3) && ((Head_Pong_y <= head_player_1 + tamanio_jugador) && (Head_Pong_y + tamanio_pong >= head_player_1))){
		//Colision con el jugador 1
		Speed_Pong_x = Speed_x;
		Events_Set(&Event, EVENT_PLAYER_COLLISION);
	}else{
		if((Head_Pong_x == 125) && ((Head_Pong_y <= head_player_2 + tamanio_jugador) && (Head_Pong_y + tamanio_pong >= head_player_2))){
			//Colision con el jugador 2
			Speed_Pong_x = -Speed_x;
			Events_Set(&Event, EVENT_PLAYER_COLLISION);
		}
	}

	Head_Pong_x += Speed_Pong_x;

	// Caso de goles

	if(Head_Pong_x == 2){
		Speed_Pong_x = -Speed_x;
		Status = GOL;
		Score_player_2++;
		PongReset();
		Events_Set(&Event, EVENT_GOL);
	}else{
		if(Head_Pong_x ==126){
			Speed_Pong_x = Speed_x;
			Status = GOL;
			Score_player_1++;
			PongReset();
			Events_Set(&Event, EVENT_GOL);
		}
	}
	return Status;
}

void DrawText(void){
	SH1106_GotoXY(2, 50);
	SH1106_Puts("Presione para jugar", &Font_7x10, SH1106_COLOR_WHITE);
}

void DrawScore(void){
	sprintf(&Score_player_1_c,"%i",Score_player_1);
	sprintf(&Score_player_2_c,"%i",Score_player_2);
	SH1106_GotoXY(20, 2);
	SH1106_Puts(Score_player_1_c, &Font_7x10, SH1106_COLOR_WHITE);
	SH1106_GotoXY(107, 2);
	SH1106_Puts(Score_player_2_c, &Font_7x10, SH1106_COLOR_WHITE);
}

void ResetScore(void){
	Score_player_1 = 0;
	Score_player_2 = 0;
}
