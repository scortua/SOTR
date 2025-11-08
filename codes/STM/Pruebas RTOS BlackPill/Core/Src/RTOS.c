/*
 * RTOS.c
 *
 *  Created on: Oct 18, 2025
 *      Author: Scort
 */
#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"
#include "Portable.h"
#include "Scheduller.h"

u16 RTOS_Init(void)
{
	u16 Res = Scheduller_Init();
	if(Res == SCHEDULLER_OK)
	{
		Res = Task_Init();
		if(Res == TASK_OK)
		{
			Res = Task_CreateIdleTask();
		}
	}
	return Res;
}

void RTOS_Start(void)
{
	u32 FirstTask = Scheduller_GetNextTask(0);
	Port_InitTickTimer();
	Port_EnableInterrupts();
	Port_RestoreFirstContext(FirstTask);
}
