/*
 * Message.h
 *
 *  Created on: Nov 22, 2025
 *      Author: jabel
 */

#ifndef INC_MESSAGE_H_
#define INC_MESSAGE_H_

#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"
#include "portable.h"
#include "Scheduller.h"
#include "Semaphore.h"

#define MESSAGE_BASE_ERR              	0x0700

#define MESSAGE_OK                    	0
#define MESSAGE_ERR_NULL_PARAM        	MESSAGE_BASE_ERR | 0x00FF
#define MESSAGE_ERR_EMPTY        		MESSAGE_BASE_ERR | 0x00FE
#define MESSAGE_ERR_WRONG_PARAM       	MESSAGE_BASE_ERR | 0x00FD

#define MSG_GET_POOL_SIXE(X,Y)			((X+sizeof(QueueElement_t))*Y)		// X = Tamaño de mensaje, Y= Cantidad de mensajes

typedef struct{
	pu8 MsgPool;
	u16 MsgSize;
	u16 MsgCount;
	QueueHandler_t MsgFree;
	QueueHandler_t MsgSend;
	SemaphoreHandler_t TxSem;
	SemaphoreHandler_t RxSem;
}MessageHandler_t, * MessageHandler_t_ptr;

u16 Message_Init(MessageHandler_t_ptr Msg, pu8 MessagePool, u16 Size, u16 MsgCount);
u16 Message_GetFree(MessageHandler_t_ptr Msg);
u16 Message_GetSend(MessageHandler_t_ptr Msg);
u16 Message_Send(MessageHandler_t_ptr Msg, pu8 Data);
u16 Message_Recv(MessageHandler_t_ptr Msg, pu8 Data);

#endif /* INC_MESSAGE_H_ */
