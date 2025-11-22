/*
 * Events.c
 *
 *  Created on: Nov 15, 2025
 *      Author: Scort
 */
#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"
#include "Events.h"
#include "Portable.h"
#include "Scheduller.h"

extern u16 Scheduller_SetTaskReady(TaskControlBlock_t_ptr Task);

u16 Events_Init(EventHandler_t_ptr Event)
{
	u16 Res = EVENT_ERR_NULL_PARAM;
	if(Event != NULL)
	{
		Event->EventBits = 0;
		Res = Queue_Init(&Event->WaitQueue);
	}
	return Res;
}

u16 Events_Get(EventHandler_t_ptr Event, EventType_t_ptr Bits)
{
	u16 Res = EVENT_ERR_NULL_PARAM;
	if((Event != NULL) && (Bits != NULL))
	{
		Port_DisableInterrupts();
		*Bits = Event->EventBits;
		Res = EVENT_OK;
		Port_EnableInterrupts();
	}
	return Res;
}

u16 Events_Set(EventHandler_t_ptr Event, EventType_t Bits)
{
	u16 Res = EVENT_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
		if(Event != NULL)
		{
			Port_DisableInterrupts();
			Event->EventBits |= (Bits & EVENT_WAIT_TYPE_MASK);
			if(Queue_GetCount(&Event->WaitQueue))
			{
				QueueHandler_t Temp;
				Res = Queue_Init(&Temp);
				if(Res == QUEUE_OK)
				{
					TaskControlBlock_t_ptr ActualTask = Scheduller_GetActualTask();
					while(Queue_GetCount(&Event->WaitQueue))
					{
						TaskControlBlock_t_ptr Task;
						Res = Queue_Dequeue(&Event->WaitQueue, (void **)&Task);
						if(Res == QUEUE_OK)
						{
							if(Task != NULL)
							{
								if(Task->WaitEvent & EVENT_WAIT_ALL)
								{
									EventType_t TaskWaitEvent = Task->WaitEvent & EVENT_WAIT_TYPE_MASK;
									if((TaskWaitEvent & Event->EventBits) == TaskWaitEvent)
									{
										Res = Scheduller_SetTaskReady(Task);
										if(ActualTask)
										{
											if((ActualTask->Priority) > (Task->Priority))
											{
												CallScheduller = TRUE;
											}
										}
									}
									else
									{
										Res = Queue_Enqueue(&Temp, &Task->QElement, Task);
									}
								}
								else
								{
									if(Task->WaitEvent & Event->EventBits)
									{
										Res = Scheduller_SetTaskReady(Task);
									}
									else
									{
										Res = Queue_Enqueue(&Temp, &Task->QElement, Task);
									}
								}
							}
						}
					}
					Res = Queue_Copy(&Temp, &Event->WaitQueue);
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

u16 Events_Clear(EventHandler_t_ptr Event, EventType_t Bits)
{
	u16 Res = EVENT_ERR_NULL_PARAM;
	if(Event != NULL)
	{
		Port_DisableInterrupts();
		Event->EventBits &= (~Bits);
		Port_EnableInterrupts();
	}
	return Res;
}
u16 Events_WaitAny(EventHandler_t_ptr Event, EventType_t Bits) // mascara en n y en cualquiera se crea el evento
{
	u16 Res = EVENT_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Event != NULL)
	{
		Port_DisableInterrupts();
		Bits &= EVENT_WAIT_TYPE_MASK;
		if((Event->EventBits) & Bits)
		{
			Res = EVENT_OK;
		}
		else
		{
			TaskControlBlock_t_ptr ActualTask = Scheduller_GetActualTask();
			if(ActualTask)
			{
				ActualTask->WaitEvent = Bits;
				ActualTask->Status = ST_BLOCKED;
				Res = Queue_Enqueue(&Event->WaitQueue, &ActualTask->QElement, ActualTask);
				if(Res == QUEUE_OK)
				{
					CallScheduller = TRUE;
				}
				else
				{
					ActualTask->Status = ST_RUNNING;
				}
			}
			else
			{
				Res = EVENT_ERR_WRONG_PARAM;
			}
		}

		Port_EnableInterrupts();
	}
	return Res;
}
u16 Events_WaitAll(EventHandler_t_ptr Event, EventType_t Bits) // hasta que toda la mascara este activa
{
	u16 Res = EVENT_ERR_NULL_PARAM;
	u8 CallScheduller = FALSE;
	if(Event != NULL)
	{
		Port_DisableInterrupts();
		Bits &= EVENT_WAIT_TYPE_MASK;
		if(((Event->EventBits) & Bits) == Bits)
		{
			Res = EVENT_OK;
		}
		else
		{
			TaskControlBlock_t_ptr ActualTask = Scheduller_GetActualTask();
			if(ActualTask)
			{
				ActualTask->WaitEvent = (Bits | EVENT_WAIT_ALL);
				ActualTask->Status = ST_BLOCKED;
				Res = Queue_Enqueue(&Event->WaitQueue, &ActualTask->QElement, ActualTask);
				if(Res == QUEUE_OK)
				{
					CallScheduller = TRUE;
				}
				else
				{
					ActualTask->Status = ST_RUNNING;
				}
			}
			else
			{
				Res = EVENT_ERR_WRONG_PARAM;
			}
		}

		Port_EnableInterrupts();
	}
	return Res;
}
