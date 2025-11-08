/*
 * Semaphore.h
 *
 *  Created on: Nov 8, 2025
 *      Author: jabel
 */

#ifndef INC_SEMAPHORE_H_
#define INC_SEMAPHORE_H_

#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"

#define SEMAPHORE_BASE_ERR              0x0500

#define SEMAPHORE_OK                    0
#define SEMAPHORE_ERR_NULL_PARAM        SEMAPHORE_BASE_ERR | 0x00FF
#define SEMAPHORE_ERR_EMPTY             SEMAPHORE_BASE_ERR | 0x00FE
#define SEMAPHORE_ERR_WRONG_PARAM		SEMAPHORE_BASE_ERR | 0X00FD

typedef struct{
	u16 Count;
	u16 Limit;
	QueueHandler_t WaitQueue;
}SemaphoreHandler_t, *SemaphoreHandler_t_ptr;

u16 Semaphore_Init(SemaphoreHandler_t_ptr Semaphore, u16 Limit);
u16 Semaphore_GetCount(SemaphoreHandler_t_ptr Semaphore);
u16 Semaphore_GetLimit(SemaphoreHandler_t_ptr Semaphore);
u16 Semaphore_Take(SemaphoreHandler_t_ptr Semaphore);
u16 Semaphore_Give(SemaphoreHandler_t_ptr Semaphore);

#endif /* INC_SEMAPHORE_H_ */
