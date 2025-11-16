/*
 * mutex.c
 *
 *  Created on: Nov 8, 2025
 *      Author: jabel
 */
#include "mutex.h"
#include "AppTypes.h"

extern u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);

u16 Mutex_Init(MutexHandler_t_ptr Mutex){
	u16 Res =  MUTEX_ERR_NULL_PARAM;
	if(Mutex != NULL){
		Mutex -> Busy = FALSE;
		Res = Queue_Init(&Mutex->WaitQueue);
	}
	return Res;
}


u16 Mutex_Take(MutexHandler_t_ptr Mutex){
	u16 Res =  MUTEX_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Mutex != NULL){
		Portable_DisableInterrupts();
		if(!Mutex->Busy){
			Mutex->Busy = TRUE;
			Res = MUTEX_OK;
		}else{
			TaskControlBlock_t_ptr Task = Scheduller_GetActualTask();
			if(Task != NULL){
				Task ->Status = ST_BLOCKED;
				Res = Queue_Enqueue(&Mutex->WaitQueue, &Task->QElement, (void *)Task);
				if(Res == QUEUE_OK){
					CallScheduller = TRUE;
				}else{
					Task -> Status = ST_RUNNING;
				}
			}
		}
		Portable_EnableInterrupts();
		if(CallScheduller){
			Port_SoftwareInterrupt();
		}
	}
	return Res;
}

u16 Mutex_Give(MutexHandler_t_ptr Mutex){
	u16 Res = MUTEX_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Mutex != NULL){
		Portable_DisableInterrupts();
		if(Queue_GetCount(&Mutex->WaitQueue)>0){
			TaskControlBlock_t_ptr Task = NULL;
			Res = Queue_Dequeue(&Mutex->WaitQueue, (void **)&Task);
			if(QUEUE_OK){
				if(Task != NULL){
					Res = Scheduller_SetTaskReady(Task);
					if(SCHEDULLER_OK){
						TaskControlBlock_t_ptr ActTask = Scheduller_GetActualTask();
						if(ActTask != NULL){
							if(ActTask->Priority > Task -> Priority){
								CallScheduller = TRUE;

							}
						}
					}
				}
			}

		}else{
			Mutex->Busy = FALSE;
			Res = MUTEX_OK;
		}
		Portable_EnableInterrupts();
		if(CallScheduller){
			Port_SoftwareInterrupt();
		}
	}
	return Res;
}
