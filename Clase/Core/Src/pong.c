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

u16 head_player_1;
u16 head_player_2;

u16 Head_Pong_x = x_center;
u16 Head_Pong_y = y_center;

u8 Speed_Pong_x = 1;
u8 Speed_Pong_y = 1;

void DrawPlayer(u32 head){
	UpdateHeadPlayer1(head);
	UpdateHeadPlayer2(head);
	for(int i = head_player_1;i < head_player_1 + tamanio_jugador;i++){
		SH1106_DrawPixel(2, i, SH1106_COLOR_WHITE);
	}
	for(int i = head_player_2;i < head_player_2 + tamanio_jugador;i++){
		SH1106_DrawPixel(128, i, SH1106_COLOR_WHITE);
	}
	SH1106_UpdateScreen();
}

void UpdateHeadPlayer1(u32 head){
	head_player_1 = (head & 0X00000FFF) * ratio_conversion;
}

void UpdateHeadPlayer2(u32 head){
	head_player_2 = ((head >> 16) & 0X00000FFF) * ratio_conversion;
}

void DrawPong(u8 x, u8 y){
	for(int i = y; i < y + tamanio_pong; i++){
		for(int j = x; j < x + tamanio_pong; j++){
			SH1106_DrawPixel(j, i, SH1106_COLOR_WHITE);
		}
	}
	SH1106_UpdateScreen();
}

void PongMovement(void){
	// Cambio de velocidades debido a la colision con las paredes
	if(Head_Pong_y == 0){
		Speed_Pong_y = -1;
	}else{
		if(Head_Pong_y == 59){
			Speed_Pong_y = 1;
		}
	}
	// Cambio de velocidades debido a la colision con los jugadores
	if((Head_Pong_x == 3) && ((Head_Pong_y <= head_player_1 + tamanio_jugador) || (Head_Pong_y + tamanio_pong >= head_player_1))){
		//Colision con el jugador 1
		Speed_Pong_x = 1;
	}else{
		if((Head_Pong_x == 126) && ((Head_Pong_y <= head_player_2 + tamanio_jugador) || (Head_Pong_y + tamanio_pong >= head_player_2))){
			//Colision con el jugador 2
			Speed_Pong_x = -1;
		}
	}

	Head_Pong_x += Speed_Pong_x;
	Head_Pong_y += Speed_Pong_y;
}
