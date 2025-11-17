/*
 * RTOS.h
 *
 *  Created on: Oct 18, 2025
 *      Author: Scort
 */

#ifndef INC_RTOS_H_
#define INC_RTOS_H_

#include "AppTypes.h"
#include "Queue.h"
#include "Task.h"
#include "RTOS_Defines.h"
#include "Mutex.h"

#define RTOS_CreateTask		Task_CreateTask
#define RTOS_Delay			Task_Delay

u16 RTOS_Init(void);
void RTOS_Start(void);


#endif /* INC_RTOS_H_ */
