/*
 * Scheduler.h
 *
 *  Created on: Oct 4, 2025
 *      Author: jabel
 */

#ifndef INC_SCHEDULLER_H_
#define INC_SCHEDULLER_H_

#include "Task.h"


#define SCHEDULLER_BASE_ERR              0x0300

#define SCHEDULLER_OK                    0
#define SCHEDULLER_ERR_EMPTY             SCHEDULLER_BASE_ERR | 0x00FE
#define SCHEDULLER_ERR_NULL_PARAM        SCHEDULLER_BASE_ERR | 0x00FF
#define SCHEDULLER_ERR_WRONG_PARAM       SCHEDULLER_BASE_ERR | 0x00FD

u16 Scheduller_Init(void);
u32 Scheduller_GetNextTask(u32 ActualTask);
TaskControlBlock_t_ptr Scheduller_GetActualTask(void);

#endif /* INC_SCHEDULLER_H_ */
