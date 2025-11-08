/*
 * Scheduller.h
 *
 *  Created on: Oct 4, 2025
 *      Author: Scort
 */

#ifndef INC_SCHEDULLER_H_
#define INC_SCHEDULLER_H_

#include "Task.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

#define TASK_BASE_ERR              0x0100

#define SCHEDULLER_OK                    0
#define SCHEDULLER_ERR_NULL_PARAM        TASK_BASE_ERR | 0x00FF
#define SCHEDULLER_ERR_EMPTY             TASK_BASE_ERR | 0x00FE
#define SCHEDULLER_ERR_WRONG_PARAM	  	 TASK_BASE_ERR | 0x00FD

u16 Scheduller_Init(void);
TaskControlBlock_t_ptr Scheduller_GetActualTask(void);
u32 Scheduller_GetNextTask(u32 ActualStack);

#endif /* INC_SCHEDULLER_H_ */
