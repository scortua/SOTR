/*
 * Scheduler.c
 *
 *  Created on: Oct 4, 2025
 *      Author: jabel
 */


#include "RTOSDefines.h"
#include "AppTypes.h"
#include "Scheduller.h"
#include "Task.h"
#include "Queue.h"

u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);

QueueHandler_t SchedullerReadyTasks[RTOS_SYSTEM_PRIORITIES];
TaskControlBlock_t_ptr ActualTask = NULL;

u16 Scheduller_Init(void){
	u16 Res;
	ActualTask = NULL;
	for(int i=0; i < RTOS_SYSTEM_PRIORITIES;i++){
		Res = Queue_Init(&SchedullerReadyTasks[i]);
		if(Res){
			break;
		}
	}
	return Res;
}

u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task){
	u16 Res = SCHEDULLER_ERR_NULL_PARAM;
	if(Task != NULL){
		if(Task ->Status != ST_STACK_OVERFLOW){
			Task -> Status = ST_READY;
			if(Task->Priority < RTOS_SYSTEM_PRIORITIES){
				Res = Queue_Enqueue(&SchedullerReadyTasks[Task->Priority], &Task -> QElement, Task);
			}
		}
	}
	return Res;
}

TaskControlBlock_t_ptr Scheduller_GetActualTask(void){
	return ActualTask;
}

u32 Scheduller_GetNextTask(u32 ActualStack){
	if(ActualStack != (u32)NULL){
		if(ActualTask != NULL){
			ActualTask -> ActualStackPointer = ActualStack;
			if(ActualTask -> Id){
				if(Port_CheckStackOverflow(ActualTask -> Stack)){
					ActualTask -> Status = ST_STACK_OVERFLOW;
				}else{
					if(ActualTask -> Status == ST_RUNNING){
						Scheduller_SetTaskReady(ActualTask);
					}
				}
			}else{
				ActualTask -> Status = ST_READY;
			}
		}
	}
	ActualTask = NULL;
	for(int i = 0; i < RTOS_SYSTEM_PRIORITIES;i++){
		if(Queue_GetCount(&SchedullerReadyTasks[i])){
			if(Queue_Dequeue(&SchedullerReadyTasks[i], (pv*)&ActualTask) == QUEUE_OK){
				if(ActualTask != NULL){
					break;
				}
			}
		}
	}
	if(ActualTask == NULL){
		ActualTask = Task_GetIdleTask();
	}
	ActualTask->Status = ST_RUNNING;
	return ActualTask->ActualStackPointer;
}
