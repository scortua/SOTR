/*
 * Events.h
 *
 *  Created on: Nov 15, 2025
 *      Author: Scort
 */

#ifndef INC_EVENTS_H_
#define INC_EVENTS_H_

#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"
#include "RTOS_Defines.h"

#define EVENT_BASE_ERR              0x0600

#define EVENT_OK                    0
#define EVENT_ERR_NULL_PARAM        EVENT_BASE_ERR | 0x00FF
#define EVENT_ERR_EMPTY             EVENT_BASE_ERR | 0x00FE
#define EVENT_ERR_WRONG_PARAM	  	EVENT_BASE_ERR | 0x00FD

#define EVENT_WAIT_TYPE_BIT		(1 << ((sizeof(EventType_t) * 8) - 1))
#define EVENT_WAIT_ALL			EVENT_WAIT_TYPE_BIT
#define EVENT_WAIT_TYPE_MASK	(~EVENT_WAIT_TYPE_BIT)

typedef struct
{
	EventType_t EventBits;
	QueueHandler_t WaitQueue;
}EventHandler_t, * EventHandler_t_ptr;

u16 Events_Init(EventHandler_t_ptr Event);
u16 Events_Get(EventHandler_t_ptr Event, EventType_t_ptr Bits);
u16 Events_Set(EventHandler_t_ptr Event, EventType_t Bits);
u16 Events_Clear(EventHandler_t_ptr Event, EventType_t Bits);
u16 Events_WaitAny(EventHandler_t_ptr Event, EventType_t Bits); // mascara en n y en cualquiera se crea el evento
u16 Events_WaitAll(EventHandler_t_ptr Event, EventType_t Bits); // hasta que toda la mascara este activa

#endif /* INC_EVENTS_H_ */
