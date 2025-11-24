/*
 * Message.c
 *
 *  Created on: Nov 22, 2025
 *      Author: jabel
 */

#include "Message.h"
#include "string.h"

extern u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);

u16 Message_Init(MessageHandler_t_ptr Msg, pu8 MessagePool, u16 Size, u16 MsgCount){
	u16 Res = MESSAGE_ERR_NULL_PARAM;
	if((Msg != NULL)&&(MessagePool != NULL)){
		Res = MESSAGE_ERR_WRONG_PARAM;
		if((Size > 0)&&(MsgCount > 0)){
			Res = Queue_Init(&Msg->MsgFree);
			if(Res == QUEUE_OK){
				Res = Queue_Init(&Msg->MsgSend);
				if(Res == QUEUE_OK){
					Res = Semaphore_Init(&Msg->TxSem, MsgCount, MsgCount);
					if(Res == SEMAPHORE_OK){
						Res = Semaphore_Init(&Msg->RxSem, MsgCount, 0);
						if(Res == SEMAPHORE_OK){
							Msg->MsgPool = MessagePool;
							Msg->MsgSize = Size;
							Msg->MsgCount = MsgCount;
							QueueElement_t_ptr ElementPtr = (QueueElement_t_ptr)Msg ->MsgPool;
							for(int i = 0; i < MsgCount;i++){
								Res = Queue_Enqueue(&Msg->MsgFree, ElementPtr, (void *)((u32)ElementPtr + sizeof(QueueElement_t)));
								if(Res != QUEUE_OK){
									break;
								}
								ElementPtr = (QueueElement_t_ptr)(((u32)ElementPtr)+sizeof(QueueElement_t)+Size);
							}
						}
					}
				}
			}
		}
	}
	return Res;
}

u16 Message_GetFree(MessageHandler_t_ptr Msg){
	u16 Res = 0;
	if(Msg != NULL){
		Portable_DisableInterrupts();
		Res = Queue_GetCount(&Msg->MsgFree);
		Portable_EnableInterrupts();
	}
	return Res;
}
u16 Message_GetSend(MessageHandler_t_ptr Msg){
	u16 Res = 0;
	if(Msg != NULL){
		Portable_DisableInterrupts();
		Res = Queue_GetCount(&Msg->MsgSend);
		Portable_EnableInterrupts();
	}
	return Res;
}

u16 Message_Send(MessageHandler_t_ptr Msg, pu8 Data){
	u16 Res = MESSAGE_ERR_NULL_PARAM;
	if((Msg != NULL)&&(Data != NULL)){
		Res = Semaphore_Take(&Msg->TxSem);
		if(Res = SEMAPHORE_OK){
			QueueElement_t_ptr ElementPtr;
			Res = Queue_DequeueElement(&Msg->MsgFree, &ElementPtr);
			if(Res == QUEUE_OK){
				Res = MESSAGE_ERR_WRONG_PARAM;
				if(ElementPtr != NULL){
					memcpy(ElementPtr -> Data, Data, Msg->MsgSize);
					Res = Queue_Enqueue(&Msg->MsgSend, ElementPtr, ElementPtr -> Data);
					if(Res = QUEUE_OK){
						Res = Semaphore_Give(&Msg -> RxSem);
					}
				}
			}
		}
	}
	return Res;
}

u16 Message_Recv(MessageHandler_t_ptr Msg, pu8 Data){
	u16 Res = MESSAGE_ERR_NULL_PARAM;
	if((Msg != NULL)&&(Data != NULL)){
		Res = Semaphore_Take(&Msg->RxSem);
		if(Res = SEMAPHORE_OK){
			QueueElement_t_ptr ElementPtr;
			Res = Queue_DequeueElement(&Msg->MsgSend, &ElementPtr);
			if(Res == QUEUE_OK){
				Res = MESSAGE_ERR_WRONG_PARAM;
				if(ElementPtr != NULL){
					memcpy(Data, ElementPtr -> Data, Msg->MsgSize);
					Res = Queue_Enqueue(&Msg->MsgFree, ElementPtr, ElementPtr -> Data);
					if(Res = QUEUE_OK){
						Res = Semaphore_Give(&Msg -> TxSem);
					}
				}
			}
		}
	}
	return Res;
}
