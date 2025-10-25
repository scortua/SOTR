/*
 * Task.h
 *
 *  Created on: Oct 4, 2025
 *      Author: jabel
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_

#include "AppTypes.h"
#include "Queue.h"

#define TASK_BASE_ERR              0x0200

#define TASK_OK                    0
#define TASK_ERR_EMPTY             TASK_BASE_ERR | 0x00FE
#define TASK_ERR_NULL_PARAM        TASK_BASE_ERR | 0x00FF
#define TASK_ERR_WRONG_PARAM       TASK_BASE_ERR | 0x00FD

typedef void (*TaskFunction)(void);

typedef enum{
	ST_READY = 0,
	ST_RUNNING,
	ST_BLOCKED
}TaskStatus_t, * TaskStatus_t_ptr;

typedef struct{
	char * Name;
	u8 Id;
	TaskFunction Function;
	u8 Priority;
	pu32 Stack;
	u32 StackSize;
	u32 ActualStackPointer;
	u32 DelayTime;
	TaskStatus_t Status;
	QueueElement_t QElement;
}TaskControlBlock_t, * TaskControlBlock_t_ptr;

u16 Task_Init(void);
u16 Task_CreateTask(TaskControlBlock_t_ptr Tcb, char * Name, u8 Id, u8 Priority, pu32 Stack, u32 StackSize, TaskFunction Function);
u16 Task_CreateIdleTask(void);
u16 Task_Delay(u32 Ticks);
TaskControlBlock_t_ptr Task_GetIdleTask(void);

#endif /* INC_TASK_H_ */
