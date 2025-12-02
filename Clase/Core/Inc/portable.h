/*
 * portable.h
 *
 *  Created on: Sep 20, 2025
 *      Author: jabel
 */

#ifndef INC_PORTABLE_H_
#define INC_PORTABLE_H_

#include "AppTypes.h"

void Portable_EnableInterrupts(void);
void Portable_DisableInterrupts(void);
void Port_InitTickTimer(void);
void Port_SoftwareInterrupt(void);
u32 Port_GetSystemTick(void);
u32 Port_InitStack(pu32 Stack, u32 StackSize, pv Fuction);
void Port_RestoreFirstContext(u32 Sp);
u8 Port_CheckStackOverflow(pu32 Stack);

#endif /* INC_PORTABLE_H_ */
