/*
 * Portable.h
 *
 *  Created on: Sep 20, 2025
 *      Author: Scort
 */

#ifndef INC_PORTABLE_H_
#define INC_PORTABLE_H_

#include "AppTypes.h"

void Port_EnableInterrupts(void);
void Port_DisableInterrupts(void);
u32 Port_InitStack(pu32 Stack, u32 StackSize, pv function);
void Port_InitTickTimer(void);
void Port_SoftwareInterrupt(void);
u32 Port_GetSystemTick(void);
void Port_RestoreFirstContext(u32 sp);
u8 Port_CheckStackOverFlow(pu32 Stack);

#endif /* INC_PORTABLE_H_ */
