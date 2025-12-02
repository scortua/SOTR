/*
 * portable.c
 *
 *  Created on: Sep 20, 2025
 *      Author: jabel
 */

#include "RTOSDefines.h"
#include "AppTypes.h"
#include "portable.h"

#include "string.h"

extern u32 Scheduller_GetNextTask(u32 ActualTask);

#define STACK_CHECH_BYTE				0XAAAAAAAA
#define STACK_CHECK_SIZE				16

#define _enable_irq()	{ asm ("cpsie i"); }
#define _disable_irq()	{ asm ("cpsid i"); }

u32 PortNextedCounter = 0;
u32 PortActualStackPointer = 0;
u32 PortTickCount = 0;

__attribute__((always_inline))static inline void Port_SaveContext(void){
	__asm("PUSH {LR}");
	__asm("PUSH {R4-R11}");
	__asm("MRS 	R0,MSP");
	__asm("MOV  %0,	R0":"=r"(PortActualStackPointer));
}

__attribute__((always_inline))static inline void Port_RestoreContext(void){
	__asm("MOV  R0,	%0": :"r"(PortActualStackPointer));
	__asm("MSR  MSP,R0");
	__asm("POP {R4-R11}");
	__asm("POP {PC}");
}

void Portable_EnableInterrupts(void){
	if(PortNextedCounter){
		PortNextedCounter--;
	}
	if(!PortNextedCounter){
		_enable_irq();
	}
}
void Portable_DisableInterrupts(void){
	_disable_irq();
	PortNextedCounter++;
}
void Port_InitTickTimer(void){
	pu32 pSTK_CTRL = (pu32)(0XE000E010);
	pu32 pSTK_LOAD = (pu32)(0xE000E014);

	u32 TickCounterValue = (RTOS_SYSTEM_CLOCK / RTOS_TICK_CLOCK) - 1;

	*pSTK_LOAD &= 0XFF000000;
	*pSTK_LOAD |= TickCounterValue;

	*pSTK_CTRL |= (1 << 2);
	*pSTK_CTRL |= (1 << 1);
	*pSTK_CTRL |= (1 << 0);

}

void Port_SoftwareInterrupt(void){
	pu32 pICSR = (pu32)(0XE000ED04);
	*pICSR |= (1 << 28);
}

u32 Port_GetSystemTick(void){
	return PortTickCount;
}

u32 Port_InitStack(pu32 Stack, u32 StackSize, pv Fuction){
	memset(Stack,STACK_CHECH_BYTE,StackSize * sizeof(u32));
	Stack += StackSize;
	Stack--;
	*Stack = 0X01000000UL;								// PSR
	Stack--;
	*Stack = (u32)((u32)Fuction & 0XFFFFFFFFEUL);		// PC
	*Stack |= 0X00000001UL;
	Stack--;
	*Stack = (u32)((u32)Fuction & 0XFFFFFFFFEUL);		// LR
	*Stack |= 0X00000001UL;
	Stack--;
	*Stack = 0;											// R12
	Stack--;
	*Stack = 0;											// R3
	Stack--;
	*Stack = 0;											// R2
	Stack--;
	*Stack = 0;											// R1
	Stack--;
	*Stack = 0;											// R0
	Stack--;
	*Stack = 0XFFFFFFF9;								// LR para EXEC_RETURN
	Stack--;
	*Stack = 0;											// R11
	Stack--;
	*Stack = 0;											// R10
	Stack--;
	*Stack = 0;											// R9
	Stack--;
	*Stack = 0;											// R8
	Stack--;
	*Stack = 0;											// R7
	Stack--;
	*Stack = 0;											// R6
	Stack--;
	*Stack = 0;											// R5
	Stack--;
	*Stack = 0;											// R4
	return(u32) Stack;

}

void Port_RestoreFirstContext(u32 Sp){
	PortNextedCounter = 0;
	PortActualStackPointer = Sp;
	__asm("MOV R0, %0"::"r"(PortActualStackPointer));
	__asm("MSR MSP, R0");
	__asm("POP {R4-R11}");
	__asm("POP {LR}");
	__asm("POP {R0-R3}");
	__asm("POP {R12}");
	__asm("POP {LR}");
	__asm("POP {LR}");
	__asm("POP {R10}");
	__asm("MSR PSR, R10");
	__asm("MOV R10, #0");
	__asm("BX LR");
}

__attribute__((naked))void SysTick_Handler(void){
	Port_SaveContext();
	PortTickCount++;
	PortActualStackPointer = Scheduller_GetNextTask(PortActualStackPointer);
	Port_RestoreContext();
}

__attribute__((naked))void PendSV_Handler(void){
	Port_SaveContext();
	PortActualStackPointer = Scheduller_GetNextTask(PortActualStackPointer);
	Port_RestoreContext();
}

u8 Port_CheckStackOverflow(pu32 Stack){
	u8 Res = FALSE;
	for(int i = STACK_CHECK_SIZE - 1; i >= 0; i--){
		if(Stack[i] != STACK_CHECH_BYTE){
			Res = TRUE;
			break;
		}
	}
	return Res;
}
