/*
 * Task.h
 *
 *  Created on: Oct 4, 2025
 *      Author: Scort
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_

#include "AppTypes.h"
#include "Queue.h"

/**********************************************
*     DEFINICIÓN DE CONSTANTES
* ********************************************/

#ifndef NULL
#define NULL ((void*)0)
#endif

#define TASK_BASE_ERR              0x0100

#define TASK_OK                    0
#define TASK_ERR_NULL_PARAM        TASK_BASE_ERR | 0x00FF
#define TASK_ERR_EMPTY             TASK_BASE_ERR | 0x00FE
#define TASK_ERR_WRONG_PARAM	   TASK_BASE_ERR | 0x00FD

typedef void (*TaskFunction)(void);	// Callback

typedef enum
{
	// Estados para manejar tarea
	ST_READY = 0,			// tarea preparada para ejecutar
	ST_RUNNING,				// estado de ejecucion en cpu
	ST_BLOCKED,				// estado de bloqueo como en espera, suspencion o terminacion
	ST_STACK_OVERFLOW
}TaskStatus_t, * TaskStatus_t_ptr;

typedef struct
{
	char * Name;			// Nombre de la tarea
	u8 Id;					// id de la tarea
	TaskFunction Function;	// funcion que corre la tarea
	u8 Priority;			// prioridad de la tarea
	pu32 Stack;				// stack con la que inicializa la tarea
	u32 StackSize;			// tamaño del stack de esta tarea
	TaskStatus_t Status;
	// cada tarea tiene su stackpointer que guardar con la informacion que necesita guardar
	u32 ActualStackPointer;
	u32 DelayTime;
	EventType_t WaitEvent;
	QueueElement_t QElement;
}TaskControlBlock_t, * TaskControlBlock_t_ptr;

u16 Task_Init(void);
u16 Task_CreateTask(TaskControlBlock_t_ptr Tcb, char * Name, u8 Id, u8 Priority, pu32 Stack, u32 StackSize, TaskFunction Function);
u16 Task_CreateIdleTask(void);
u16 Task_Delay(u32 Ticks);
TaskControlBlock_t_ptr Task_GetIdleTask(void);

#endif /* INC_TASK_H_ */
