/*
 * Semaphore.c
 *
 *  Created on: Nov 8, 2025
 *      Author: jabel
 */

#include "Semaphore.h"

extern u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);

u16 Semaphore_Init(SemaphoreHandler_t_ptr Semaphore, u16 Limit, u16 Init){
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	if(Semaphore != NULL){
		Res = SEMAPHORE_ERR_WRONG_PARAM;
		if(Limit){
			if(Init > Limit){
				Init = Limit;
			}
			Semaphore -> Count = Init;
			Semaphore -> Limit = Limit;
			Res = Queue_Init(&Semaphore -> WaitQueue);
		}
	}
	return Res;
}
u16 Semaphore_GetCount(SemaphoreHandler_t_ptr Semaphore){
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	if(Semaphore != NULL){
		Portable_DisableInterrupts();
		Res = Semaphore -> Count;
		Portable_EnableInterrupts();
	}
	return Res;

}
u16 Semaphore_GetLimit(SemaphoreHandler_t_ptr Semaphore){
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	if(Semaphore != NULL){
		Portable_DisableInterrupts();
		Res = Semaphore -> Limit;
		Portable_EnableInterrupts();
	}
	return Res;
}
u16 Semaphore_Take(SemaphoreHandler_t_ptr Semaphore){
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Semaphore !=NULL){
		Portable_DisableInterrupts();
		if(Semaphore->Count < Semaphore->Limit){
			Semaphore -> Count++;
			Res = SEMAPHORE_OK;
		}else{
			TaskControlBlock_t_ptr Task = Scheduller_GetActualTask();
			if(Task != NULL){
				Task ->Status = ST_BLOCKED;
				Res = Queue_Enqueue(&Semaphore->WaitQueue, &Task->QElement, (void *)Task);
				if(Res == QUEUE_OK){
					CallScheduller = TRUE;
				}else{
					Task -> Status = ST_RUNNING;
				}
			}
		}
		Portable_EnableInterrupts();
	}
	if(CallScheduller){
		Port_SoftwareInterrupt();
	}
	return Res;
}

u16 Semaphore_Give(SemaphoreHandler_t_ptr Semaphore){
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Semaphore != NULL){
		Portable_DisableInterrupts();
		if(Queue_GetCount(&Semaphore->WaitQueue)>0){
			TaskControlBlock_t_ptr Task = NULL;
			Res = Queue_Dequeue(&Semaphore->WaitQueue, (void **)&Task);
			if(QUEUE_OK){
				if(Task != NULL){
					Res = Scheduller_SetTaskReady(Task);
					if(SCHEDULLER_OK){
						TaskControlBlock_t_ptr ActualTask = Scheduller_GetActualTask();
						if(ActualTask != NULL){
							if(ActualTask->Priority > Task -> Priority){
								CallScheduller = TRUE;

							}
						}
					}
				}
			}

		}else{
			if(Semaphore->Count){
				Semaphore -> Count--;
			}
			Res = SEMAPHORE_OK;
		}
		Portable_EnableInterrupts();
		if(CallScheduller){
			Port_SoftwareInterrupt();
		}
	}
	return Res;
}
