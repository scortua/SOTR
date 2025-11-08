/*
 * Mutex.h
 *
 *  Created on: Nov 8, 2025
 *      Author: Scort
 */

#ifndef INC_MUTEX_H_
#define INC_MUTEX_H_

#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"

#define MUTEX_BASE_ERR              0x0400
#define MUTEX_OK                    0
#define MUTEX_ERR_NULL_PARAM        MUTEX_BASE_ERR | 0x00FF
#define MUTEX_ERR_EMPTY             MUTEX_BASE_ERR | 0x00FE

typedef struct
{
	u8 Busy;
	QueueHandler_t WaitQueue;
}MutexHandler_t, * MutexHandler_t_ptr;

u16 Mutex_Init(MutexHandler_t_ptr Mutex);
u16 Mutex_Take(MutexHandler_t_ptr Mutex);
u16 Mutex_Give(MutexHandler_t_ptr Mutex);

#endif /* INC_MUTEX_H_ */
