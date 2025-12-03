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

#define EVENT_TASK				(1<<0)

extern EventHandler_t Event;

s16 head_player_1;
s16 head_player_2;

s16 Head_Pong_x = x_center;
s16 Head_Pong_y = y_center;

s8 Speed_Pong_x = 1;
s8 Speed_Pong_y = 1;

void DrawPlayers(void){
	for(int i = head_player_1;i < head_player_1 + tamanio_jugador;i++){
		SH1106_DrawPixel(2, i, SH1106_COLOR_WHITE);
	}
	for(int i = head_player_2;i < head_player_2 + tamanio_jugador;i++){
		SH1106_DrawPixel(129, i, SH1106_COLOR_WHITE);
	}
	SH1106_UpdateScreen();
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
	SH1106_UpdateScreen();
}

void PongMovement(void){
	// Cambio de velocidades debido a la colision con las paredes
	if(Head_Pong_y == 0){
		Speed_Pong_y = 1;
		Events_Set(&Event, EVENT_TASK);
	}else{
		if(Head_Pong_y == 59){
			Speed_Pong_y = -1;
			// Hacer sonido de colision
		}
	}
	// Cambio de velocidades debido a la colision con los jugadores
	if((Head_Pong_x == 3) && ((Head_Pong_y <= head_player_1 + tamanio_jugador) && (Head_Pong_y + tamanio_pong >= head_player_1))){
		//Colision con el jugador 1
		Speed_Pong_x = 1;
		// Hacer sonido de colision
	}else{
		if((Head_Pong_x == 125) && ((Head_Pong_y <= head_player_2 + tamanio_jugador) && (Head_Pong_y + tamanio_pong >= head_player_2))){
			//Colision con el jugador 2
			Speed_Pong_x = -1;
			// Hacer sonido de colision
		}
	}

	// Caso de goles

	if(Head_Pong_x == 2){
		// Hacer el evento para hacer el sonido y sumar en el marcaddor para jugador 2
	}else{
		if(Head_Pong_x ==126){
			// Hacer el evento para hacer el sonido y sumar en el marcaddor para jugador 2
		}
	}

	Head_Pong_x += Speed_Pong_x;
	Head_Pong_y += Speed_Pong_y;
}
