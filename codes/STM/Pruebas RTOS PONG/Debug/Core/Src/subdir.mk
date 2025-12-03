################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Events.c \
../Core/Src/Message.c \
../Core/Src/Mutex.c \
../Core/Src/Portable.c \
../Core/Src/RTOS.c \
../Core/Src/Scheduller.c \
../Core/Src/Semaphore.c \
../Core/Src/Task.c \
../Core/Src/main.c \
../Core/Src/queue.c \
../Core/Src/ssd1306.c \
../Core/Src/ssd1306_fonts.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/Events.o \
./Core/Src/Message.o \
./Core/Src/Mutex.o \
./Core/Src/Portable.o \
./Core/Src/RTOS.o \
./Core/Src/Scheduller.o \
./Core/Src/Semaphore.o \
./Core/Src/Task.o \
./Core/Src/main.o \
./Core/Src/queue.o \
./Core/Src/ssd1306.o \
./Core/Src/ssd1306_fonts.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/Events.d \
./Core/Src/Message.d \
./Core/Src/Mutex.d \
./Core/Src/Portable.d \
./Core/Src/RTOS.d \
./Core/Src/Scheduller.d \
./Core/Src/Semaphore.d \
./Core/Src/Task.d \
./Core/Src/main.d \
./Core/Src/queue.d \
./Core/Src/ssd1306.d \
./Core/Src/ssd1306_fonts.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/Events.cyclo ./Core/Src/Events.d ./Core/Src/Events.o ./Core/Src/Events.su ./Core/Src/Message.cyclo ./Core/Src/Message.d ./Core/Src/Message.o ./Core/Src/Message.su ./Core/Src/Mutex.cyclo ./Core/Src/Mutex.d ./Core/Src/Mutex.o ./Core/Src/Mutex.su ./Core/Src/Portable.cyclo ./Core/Src/Portable.d ./Core/Src/Portable.o ./Core/Src/Portable.su ./Core/Src/RTOS.cyclo ./Core/Src/RTOS.d ./Core/Src/RTOS.o ./Core/Src/RTOS.su ./Core/Src/Scheduller.cyclo ./Core/Src/Scheduller.d ./Core/Src/Scheduller.o ./Core/Src/Scheduller.su ./Core/Src/Semaphore.cyclo ./Core/Src/Semaphore.d ./Core/Src/Semaphore.o ./Core/Src/Semaphore.su ./Core/Src/Task.cyclo ./Core/Src/Task.d ./Core/Src/Task.o ./Core/Src/Task.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/queue.cyclo ./Core/Src/queue.d ./Core/Src/queue.o ./Core/Src/queue.su ./Core/Src/ssd1306.cyclo ./Core/Src/ssd1306.d ./Core/Src/ssd1306.o ./Core/Src/ssd1306.su ./Core/Src/ssd1306_fonts.cyclo ./Core/Src/ssd1306_fonts.d ./Core/Src/ssd1306_fonts.o ./Core/Src/ssd1306_fonts.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src

