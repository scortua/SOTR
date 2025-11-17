/*
 * Portable.c
 *
 *  Created on: Sep 20, 2025
 *      Author: Scort
 */
#include "AppTypes.h"
#include "Portable.h"
#include "RTOS_Defines.h"

extern u32 Scheduller_GetNextTask(u32 ActualStack);

#define _enable_irq()	{asm("cpsie i"); }
#define _disable_irq()	{asm("cpsid i"); }

u32 PortNextedCounter = 0;
u32 PortActualStackPointer = 0; // una memoria para transportar asm a c
u32 PortTickCounter = 0;

__attribute__((always_inline))static inline void Port_SaveContext()
{
	__asm("PUSH {LR}");			// carga el Link Register para enlazar el retorno de funciones
	__asm("PUSH {R4-R11}");		//
	__asm("MRS R0,MSP");
	__asm("MOV %0, R0":"=r"(PortActualStackPointer):);
}

__attribute__((always_inline))static inline void Port_RestoreContext()
{
	__asm("MOV R0, %0": :"r"(PortActualStackPointer));
	__asm("MSR MSP, R0");
	__asm("POP {R4-R11}");
	__asm("POP {PC}");
}

void Port_EnableInterrupts(void)
{
	if(PortNextedCounter)
	{
		PortNextedCounter--;
	}
	if(!PortNextedCounter)
	{
		/*
		 * CPS es la forma de establecer las interrupciones
		 * condition flags Change Processor Status
		 * */
		_enable_irq();
	}
}

void Port_DisableInterrupts(void)
{
	_disable_irq();
	PortNextedCounter++;
}


void Port_InitTickTimer(void)
{
	pu32 pSTK_CTRL = (pu32)(0xE000E010);
	pu32 pSTK_LOAD = (pu32)(0xE000E014);

	u32 TickCounterValue = (RTOS_SYSTEM_CLOCK / RTOS_TICK_CLOCK) - 1;

	*pSTK_LOAD &= 0xFF000000;
	*pSTK_LOAD |= TickCounterValue;

	*pSTK_CTRL |= (1 << 2);
	*pSTK_CTRL |= (1 << 1);
	*pSTK_CTRL |= (1 << 0);
}

void Port_SoftwareInterrupt(void)
{
	pu32 pSCB_ICSR = (pu32)(0xE000ED04);
	*pSCB_ICSR |= (1 << 28);
}

u32 Port_GetSystemTick(void)
{
	return PortTickCounter;
}

u32 Port_InitStack(pu32 Stack, u32 StackSize, pv function)
{
	Stack += StackSize;
	Stack--;
	*Stack = 0x01000000UL;	// PSR
	Stack--;
	*Stack = (u32)((u32)function & 0xFFFFFFFEUL);	//PC
	*Stack |= 0x00000001UL;
	Stack--;
	*Stack = (u32)((u32)function & 0xFFFFFFFEUL);	//LR
	*Stack |= 0x00000001UL;
	Stack--;
	*Stack = 0;		//R12
	Stack--;
	*Stack = 0;		//R3
	Stack--;
	*Stack = 0;		//R2
	Stack--;
	*Stack = 0;		//R1
	Stack--;
	*Stack = 0;		//R0
	Stack--;
	*Stack = 0xFFFFFFF9UL;		//LR para EXEC_RETURN
	Stack--;
	*Stack = 0;		//R11
	Stack--;
	*Stack = 0;		//R10
	Stack--;
	*Stack = 0;		//R9
	Stack--;
	*Stack = 0;		//R8
	Stack--;
	*Stack = 0;		//R7
	Stack--;
	*Stack = 0;		//R6
	Stack--;
	*Stack = 0;		//R5
	Stack--;
	*Stack = 0;		//R4
	Stack--;
	return (u32)Stack;
}

void Port_RestoreFirstContext(u32 sp)
{
	PortNextedCounter = 0;
	PortActualStackPointer = sp;
	__asm("MOV R0, %0": :"r"(PortActualStackPointer));
	__asm("MSR MSP, R0");
	__asm("POP {R4-R11}");
	__asm("POP {LR}");
	__asm("POP {R0-R3}");
	__asm("POP {R12}");
	__asm("POP {LR}");
	__asm("POP {LR}");
	__asm("POP {R10}");
	__asm("MSR APSR_nzcvq, R10"); // PSR hay un error
	__asm("MOV R10, #0");
	__asm("BX  LR");
}

__attribute__((naked))void SysTick_Handler(void)
{
	Port_SaveContext();
	PortTickCounter++;
	PortActualStackPointer = Scheduller_GetNextTask(PortActualStackPointer);
	Port_RestoreContext();
}

__attribute__((naked))void PendSV_Handler(void)
{
	Port_SaveContext();
	PortActualStackPointer = Scheduller_GetNextTask(PortActualStackPointer);
	Port_RestoreContext();
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
