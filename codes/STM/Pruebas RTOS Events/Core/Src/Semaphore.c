/*
 * Semaphore.c
 *
 *  Created on: Nov 8, 2025
 *      Author: Scort
 */

#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"
#include "Semaphore.h"
#include "Scheduller.h"
#include "Portable.h"

u16 Semaphore_Init(SemaphoreHandler_t_ptr Semaphore, u16 Limit)
{
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	if(Semaphore != NULL)
	{
		Res = SEMAPHORE_ERR_WRONG_PARAM;
		if(Limit)
		{
			Semaphore->Count = 0;
			Semaphore->Limit = Limit;
			Res = Queue_Init(&Semaphore->WaitQueue);
		}
	}
	return Res;
}

u16 Semaphore_GetCount(SemaphoreHandler_t_ptr Semaphore)
{
	u16 Res = 0;
	if(Semaphore != NULL)
	{
		Port_DisableInterrupts();
		Res = Semaphore->Count;
		Port_EnableInterrupts();
	}
	return Res;
}

u16 Semaphore_GetLimit(SemaphoreHandler_t_ptr Semaphore)
{
	u16 Res = 0;
	if(Semaphore != NULL)
	{
		Port_DisableInterrupts();
		Res = Semaphore->Limit;
		Port_EnableInterrupts();
	}
	return Res;
}

u16 Semaphore_Take(SemaphoreHandler_t_ptr Semaphore)
{
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Semaphore != NULL)
	{
		Port_DisableInterrupts();
		if(Semaphore->Count < Semaphore->Limit)
		{
			Semaphore->Count++;
			Res = SEMAPHORE_OK;
		}
		else
		{
			TaskControlBlock_t_ptr Task = Scheduller_GetActualTask();
			if(Task != NULL)
			{
				Task->Status = ST_BLOCKED;
				Res = Queue_Enqueue(&Semaphore->WaitQueue, &Task->QElement, (void *) Task);
				if(Res == QUEUE_OK)
				{
					CallScheduller = TRUE;
				}
				else
				{
					Task->Status = ST_RUNNING;
				}
			}
		}
		Port_EnableInterrupts();
		if(CallScheduller)
		{
			Port_SoftwareInterrupt();
		}
	}
	return Res;
}

u16 Semaphore_Give(SemaphoreHandler_t_ptr Semaphore)
{
	u16 Res = SEMAPHORE_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Semaphore != NULL)
	{
		Port_DisableInterrupts();
		if(Queue_GetCount(&Semaphore->WaitQueue)>0)
		{
			TaskControlBlock_t_ptr Task = NULL;
			Res = Queue_Dequeue(&Semaphore->WaitQueue, (void **)Task);
			if(Res == QUEUE_OK)
			{
				if(Task != NULL)
				{
					Res = Scheduller_SetTaskReady(Task);
					if(Res == SCHEDULLER_OK)
					{
						TaskControlBlock_t_ptr ActTask = Scheduller_GetActualTask();
						if(ActTask != NULL)
						{
							if(ActTask->Priority > Task->Priority)
							{
								CallScheduller = TRUE;
							}
						}
					}
				}
			}
		}
		else
		{
			if(Semaphore->Count)
			{
				Semaphore->Count--;
			}
			Res = SEMAPHORE_OK;
		}
		Port_EnableInterrupts();
		if(CallScheduller)
		{
			Port_SoftwareInterrupt();
		}
	}
	return Res;
}
