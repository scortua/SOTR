/*
 * message.c
 *
 *  Created on: Nov 22, 2025
 *      Author: Scort
 */

#include <Message.h>
#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"
#include "Scheduller.h"
#include "Portable.h"
#include "stdlib.h"
#include "string.h"

extern u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);

u16 Message_Init(MessageHandler_t_ptr Msg, pu8 MessagePool, u16 MessageSize, u16 MessageCount)
{
	u16 Res = MSG_ERR_NULL_PARAM;
	if((Msg != NULL) && (MessagePool != NULL))
	{
		Res = MSG_ERR_WRONG_PARAM;
		if((MessageSize > 0) && (MessageCount > 0))
		{
			Res = Queue_Init(&Msg->MsgFree);
			if(Res == QUEUE_OK)
			{
				Res = Semaphore_Init(&Msg->TxSem, MessageCount, MessageCount);
				if(Res == SEMAPHORE_OK)
				{
					Res = Semaphore_Init(&Msg->RxSem, MessageCount, 0);
					if(Res == SEMAPHORE_OK)
					{
						Msg->MsgPoll = MessagePool;
						Msg->MsgSize = MessageSize;
						Msg->MsgCount = MessageCount;
						QueueElement_t_ptr ElementPtr = (QueueElement_t_ptr)(Msg->MsgPoll);
						for(int i=0; i < MessageCount; i++)
						{
							Res = Queue_Enqueue(&Msg->MsgFree, ElementPtr, (void *)(ElementPtr + sizeof(QueueElement_t)));
							if(Res != QUEUE_OK)
							{
								break;
							}
							ElementPtr = (QueueElement_t_ptr)(((u32)ElementPtr)+sizeof(QueueElement_t)+MessageSize);
						}
					}
				}
			}
		}
	}
}

u16 Message_GetFree(MessageHandler_t_ptr Msg)
{
	u16 Res = 0;
	if(Msg != NULL)
	{
		Port_DisableInterrupts();
		Res = Queue_GetCount(&Msg->MsgFree);
		Port_EnableInterrupts();
	}
	return Res;
}

u16 Message_GetSend(MessageHandler_t_ptr Msg)
{
	u16 Res = 0;
	if(Msg != NULL)
	{
		Port_DisableInterrupts();
		Res = Queue_GetCount(&Msg->MsgSend);
		Port_EnableInterrupts();
	}
}

u16 Message_Send(MessageHandler_t_ptr Msg, pu8 Data)
{
	u16 Res = MSG_ERR_NULL_PARAM;
	if((Msg != NULL) && (Data != NULL))
	{
		Res = Semaphore_Take(&Msg->TxSem);
		if(Res == SEMAPHORE_OK)
		{
			QueueElement_t_ptr ElementPtr;
			Res = Queue_DequeueElement(&Msg->MsgFree, &ElementPtr);
			if(Res == QUEUE_OK)
			{
				Res = MSG_ERR_WRONG_PARAM;
				if(ElementPtr != NULL)
				{
					memcpy(ElementPtr->Data, Data, Msg->MsgSize);
					Res = Queue_Enqueue(&Msg->MsgSend, ElementPtr, ElementPtr->Data);
					if(Res == QUEUE_OK)
					{
						Res = Semaphore_Give(&Msg->RxSem);
					}
				}
			}
		}
	}
	return Res;
}

u16 Message_Recv(MessageHandler_t_ptr Msg, pu8 Data)
{
	u16 Res = MSG_ERR_NULL_PARAM;
	if((Msg != NULL) && (Data != NULL))
	{
		Res = Semaphore_Take(&Msg->RxSem);
		if(Res == SEMAPHORE_OK)
		{
			QueueElement_t_ptr ElementPtr;
			Res = Queue_DequeueElement(&Msg->MsgSend, &ElementPtr);
			if(Res == QUEUE_OK)
			{
				Res = MSG_ERR_WRONG_PARAM;
				if(ElementPtr != NULL)
				{
					memcpy(Data, ElementPtr->Data, Msg->MsgSize);
					Res = Queue_Enqueue(&Msg->MsgFree, ElementPtr, ElementPtr->Data);
					if(Res == QUEUE_OK)
					{
						Res = Semaphore_Give(&Msg->TxSem);
					}
				}
			}
		}
	}
	return Res;
}
