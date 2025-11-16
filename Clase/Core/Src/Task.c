/*
 * Task.c
 *
 *  Created on: Oct 4, 2025
 *      Author: jabel
 */

#include "RTOSDefines.h"
#include "AppTypes.h"
#include "Task.h"
#include "portable.h"
#include "Queue.h"

extern u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);
extern TaskControlBlock_t_ptr Scheduller_GetActualTask(void);
void Task_CheckDelayTimes(u32 Ticks);
void IdleTaskFunction(void);

TaskControlBlock_t IdleTask;
u32 IdleTaskStack[RTOS_MIN_STACK_SIZE];
QueueHandler_t TaskDelayWaitQueue;

u16 Task_Init(void){
	return Queue_Init(&TaskDelayWaitQueue);
}

u16 Task_CreateTask(TaskControlBlock_t_ptr Tcb, char * Name, u8 Id, u8 Priority, pu32 Stack, u32 StackSize, TaskFunction Function){
	u16 Res = TASK_ERR_NULL_PARAM;
	if((Tcb != NULL)&&(Stack != NULL)&&(Function != NULL) && (Id > 0)){
		Res = TASK_ERR_WRONG_PARAM;
		if((StackSize >= RTOS_MIN_STACK_SIZE)){
			if(Priority > RTOS_MIN_PRIORITY){
				Priority = RTOS_MIN_PRIORITY;
			}
			Tcb -> Name = Name;
			Tcb -> Id = Id;
			Tcb -> Stack = Stack;
			Tcb -> StackSize = StackSize;
			Tcb -> Function = Function;
			Tcb -> Priority = Priority;
			Tcb -> Status = ST_READY;
			Tcb -> QElement.Data = Tcb;
			Tcb -> QElement.Next = NULL;
			Tcb -> WaitEvent = 0;
			Tcb -> ActualStackPointer = Port_InitStack(Tcb -> Stack, Tcb -> StackSize, (pv)Tcb -> Function);
			Res = Scheduller_SetTaskReady(Tcb);
		}
	}
	return Res;
}


u16 Task_CreateIdleTask(void){
	IdleTask.Name = "IDLE TASK";
	IdleTask.Id = 0;
	IdleTask.Stack = IdleTaskStack;
	IdleTask.StackSize = RTOS_MIN_STACK_SIZE;
	IdleTask.Function = IdleTaskFunction;
	IdleTask.Priority = RTOS_SYSTEM_PRIORITIES;
	IdleTask.Status = ST_READY;
	IdleTask.QElement.Data = (pv)&IdleTask;	IdleTask.QElement.Next = NULL;
	IdleTask.WaitEvent = 0;	IdleTask.ActualStackPointer = Port_InitStack(IdleTask.Stack, IdleTask.StackSize, (pv)IdleTask.Function);
	return TASK_OK;
}

u16 Task_Delay(u32 Ticks){
	u16 Res = TASK_OK;
	if(Ticks){
		u8 CallSheduller = 0;
		Res = TASK_ERR_NULL_PARAM;
		Portable_DisableInterrupts();
		TaskControlBlock_t_ptr Tcb = Scheduller_GetActualTask();
		if(Tcb != NULL){
			Tcb -> DelayTime = Ticks;
			Tcb -> Status = ST_BLOCKED;
			Res = Queue_Enqueue(&TaskDelayWaitQueue, &Tcb -> QElement, (void *) Tcb);
			if(Res == QUEUE_OK){
				CallSheduller = 1;
			}else{
				Tcb -> DelayTime = 0;
				Tcb -> Status = ST_RUNNING;
			}
		}
		Portable_EnableInterrupts();
		if(CallSheduller){
			Port_SoftwareInterrupt();
		}
	}
	return Res;
}

TaskControlBlock_t_ptr Task_GetIdleTask(void){
	return &IdleTask;
}

void Task_CheckDelayTimes(u32 Ticks){
	u8 CallScheduller = 0;
	Portable_DisableInterrupts();
	if(Queue_GetCount(&TaskDelayWaitQueue)){
		u16 Res;
		QueueHandler_t TempQueue;
		TaskControlBlock_t_ptr Tcb = NULL;
		Res = Queue_Init(&TempQueue);
		if(Res == QUEUE_OK){
			while(Queue_GetCount(&TaskDelayWaitQueue)){
				Res = Queue_Dequeue(&TaskDelayWaitQueue, (void **) &Tcb);
				if(Res == QUEUE_OK){
					if(Tcb != NULL){
						if(Tcb -> DelayTime > Ticks){
							Tcb -> DelayTime -= Ticks;
							Res = Queue_Enqueue(&TempQueue, &Tcb-> QElement, (void*) Tcb);
						}else{
							CallScheduller = 1;
							Tcb -> DelayTime = 0;
							Scheduller_SetTaskReady(Tcb);
						}
					}
				}
			}
		}
		Res = Queue_Copy(&TempQueue, &TaskDelayWaitQueue);
	}
	Portable_EnableInterrupts();
	if(CallScheduller){
		Port_SoftwareInterrupt();
	}
}

void IdleTaskFunction(void){
	u32 LastTickCount = 0;
	u32 ActualTickCount = 0;
	while(1){
		ActualTickCount = Port_GetSystemTick();
		if(ActualTickCount != LastTickCount){
			Task_CheckDelayTimes(ActualTickCount-LastTickCount);
			LastTickCount = ActualTickCount;
		}
	}
}
