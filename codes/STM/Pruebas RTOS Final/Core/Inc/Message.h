/*
 * message.h
 *
 *  Created on: Nov 22, 2025
 *      Author: Scort
 */

#ifndef INC_MESSAGE_H_
#define INC_MESSAGE_H_

#include "AppTypes.h"
#include "Queue.h"
#include "Semaphore.h"

#define MSG_BASE_ERR              0x0700

#define MSG_OK                    0
#define MSG_ERR_NULL_PARAM        MSG_BASE_ERR | 0x00FF
#define MSG_ERR_EMPTY             MSG_BASE_ERR | 0x00FE
#define MSG_ERR_WRONG_PARAM	  	  MSG_BASE_ERR | 0x00FD

#define MSG_GET_POOL_SIZE(X,Y)	  ((x + sizeof(QueueElement_t)) * Y)

typedef struct
{
	pu8 MsgPoll;
	u16 MsgSize;
	u16 MsgCount;
	QueueHandler_t MsgFree;
	QueueElement_t MsgSend;
	SemaphoreHandler_t TxSem;
	SemaphoreHandler_t RxSem;
}MessageHandler_t, * MessageHandler_t_ptr;

u16 Message_Init(MessageHandler_t_ptr Msg, pu8 MessagePool, u16 MessageSize, u16 MessageCount);
u16 Message_GetFree(MessageHandler_t_ptr Msg);
u16 Message_GetSend(MessageHandler_t_ptr Msg);
u16 Message_Send(MessageHandler_t_ptr Msg, pu8 Data);
u16 Message_Recv(MessageHandler_t_ptr Msg, pu8 Data);

#endif /* INC_MESSAGE_H_ */
