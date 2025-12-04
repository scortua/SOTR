/*
 * Events.c
 *
 *  Created on: Nov 15, 2025
 *      Author: jabel
 */

#include "Events.h"

extern u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);

u16 Events_Init(EventHandler_t_ptr Event){
	u16 Res = EVENT_ERR_NULL_PARAM;
	if(Event != NULL){
		Event -> EventBits = 0;
		Res = Queue_Init(&Event -> WaitQueue);
	}
	return Res;
}
u16 Events_Get(EventHandler_t_ptr Event, EventType_t_ptr Bits){
	u16 Res = EVENT_ERR_NULL_PARAM;
	if((Event != NULL)&&(Bits != NULL)){
		Portable_DisableInterrupts();
		*Bits = Event -> EventBits;
		Res = EVENT_OK;
		Portable_EnableInterrupts();
	}
	return Res;
}
u16 Events_Set(EventHandler_t_ptr Event, EventType_t Bits){
	u16 Res = EVENT_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Event != NULL){
		Portable_DisableInterrupts();
		Event -> EventBits |= (Bits & EVENT_WAIT_TYPE_MASK);
		if(Queue_GetCount(&Event->WaitQueue)){
			QueueHandler_t Temp;
			Res = Queue_Init(&Temp);
			TaskControlBlock_t_ptr ActualTask = Scheduller_GetActualTask();
			if(Res == QUEUE_OK){
				while(Queue_GetCount(&Event -> WaitQueue)){
					TaskControlBlock_t_ptr Task;
					Res = Queue_Dequeue(&Event->WaitQueue, (void **)&Task);
					if(Res == QUEUE_OK){
						if(Task != NULL){
							if(Task->WaitEvent & EVENT_WAIT_ALL){
								EventType_t TaskWaitEvent = (Task->WaitEvent &  EVENT_WAIT_TYPE_MASK);
								if((TaskWaitEvent & Event->EventBits) == TaskWaitEvent){
									Task -> WaitEvent = 0;
									Res = Scheduller_SetTaskReady(Task);
									if(ActualTask){
										if(ActualTask -> Priority > Task -> Priority){
											CallScheduller = TRUE;
										}
									}
								}else{
									Res = Queue_Enqueue(&Temp, &Task->QElement, Task);
								}
							}else{
								if(Task -> WaitEvent & Event -> EventBits){
									Task -> WaitEvent = 0;
									Res = Scheduller_SetTaskReady(Task);
									if(ActualTask -> Priority > Task -> Priority){
										CallScheduller = TRUE;
									}
								}else{
									Res = Queue_Enqueue(&Temp, &Task->QElement, Task);
								}
							}
						}
					}
				}
				Res = Queue_Copy(&Temp, &Event -> WaitQueue);
			}
		}
		Portable_EnableInterrupts();
		if(CallScheduller){
			Port_SoftwareInterrupt();
		}
	}
	return Res;
}
u16 Events_Clear(EventHandler_t_ptr Event, EventType_t Bits){
	u16 Res = EVENT_ERR_NULL_PARAM;
	if(Event != NULL){
		Portable_DisableInterrupts();
		Event->EventBits &= (~Bits);
		Portable_EnableInterrupts();
	}
	return Res;
}
u16 Events_WaitAny(EventHandler_t_ptr Event, EventType_t Bits){
	u16 Res = EVENT_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	Bits &= EVENT_WAIT_TYPE_MASK;
	if (Event != NULL){
			Portable_DisableInterrupts();
			if (Event->EventBits & Bits){
				Res = EVENT_OK;
			}
			else{
				TaskControlBlock_t_ptr ActualTask = Scheduller_GetActualTask();
				if (ActualTask){
					ActualTask->WaitEvent = Bits;
					ActualTask->Status = ST_BLOCKED;
					Res = Queue_Enqueue(&Event->WaitQueue, &ActualTask->QElement, (void*)ActualTask);
					if (Res == QUEUE_OK){
						CallScheduller = TRUE;
					}
					else {
						ActualTask->Status = ST_RUNNING;
					}
				}
				else {
					Res = EVENT_ERR_WRONG_PARAM;
				}
			}
			Portable_EnableInterrupts();
			if (CallScheduller){
				Port_SoftwareInterrupt();
			}
		}
		return Res;
}
u16 Events_WaitAll(EventHandler_t_ptr Event, EventType_t Bits){
	u16 res = EVENT_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if (Event != NULL){
		Portable_DisableInterrupts();
		Bits &= EVENT_WAIT_TYPE_MASK;
		if ((Event->EventBits & Bits) == Bits){
			res = EVENT_OK;
		}
		else{
			TaskControlBlock_t_ptr ActualTask = Scheduller_GetActualTask();
			if (ActualTask){
				ActualTask->WaitEvent = (Bits | EVENT_WAIT_ALL);
				ActualTask->Status = ST_BLOCKED;
				res = Queue_Enqueue(&Event->WaitQueue, &ActualTask->QElement, (void*)ActualTask);
				if (res == QUEUE_OK){
					CallScheduller = TRUE;
				}
				else {
					ActualTask->Status = ST_RUNNING;
				}
			}
			else {
				res = EVENT_ERR_WRONG_PARAM;
			}
		}
		Portable_EnableInterrupts();
		if (CallScheduller){
			Port_SoftwareInterrupt();
		}
	}
	return res;
}
