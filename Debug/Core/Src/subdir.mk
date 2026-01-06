################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/bitmaps.c \
../Core/Src/logic.c \
../Core/Src/main.c \
../Core/Src/misc.c \
../Core/Src/startup_stm32f4xx.c \
../Core/Src/stm32_ub_vga_screen.c \
../Core/Src/stm32f4xx_dma.c \
../Core/Src/stm32f4xx_gpio.c \
../Core/Src/stm32f4xx_rcc.c \
../Core/Src/stm32f4xx_tim.c \
../Core/Src/syscalls.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/bitmaps.o \
./Core/Src/logic.o \
./Core/Src/main.o \
./Core/Src/misc.o \
./Core/Src/startup_stm32f4xx.o \
./Core/Src/stm32_ub_vga_screen.o \
./Core/Src/stm32f4xx_dma.o \
./Core/Src/stm32f4xx_gpio.o \
./Core/Src/stm32f4xx_rcc.o \
./Core/Src/stm32f4xx_tim.o \
./Core/Src/syscalls.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/bitmaps.d \
./Core/Src/logic.d \
./Core/Src/main.d \
./Core/Src/misc.d \
./Core/Src/startup_stm32f4xx.d \
./Core/Src/stm32_ub_vga_screen.d \
./Core/Src/stm32f4xx_dma.d \
./Core/Src/stm32f4xx_gpio.d \
./Core/Src/stm32f4xx_rcc.d \
./Core/Src/stm32f4xx_tim.d \
./Core/Src/syscalls.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/bitmaps.cyclo ./Core/Src/bitmaps.d ./Core/Src/bitmaps.o ./Core/Src/bitmaps.su ./Core/Src/logic.cyclo ./Core/Src/logic.d ./Core/Src/logic.o ./Core/Src/logic.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/misc.cyclo ./Core/Src/misc.d ./Core/Src/misc.o ./Core/Src/misc.su ./Core/Src/startup_stm32f4xx.cyclo ./Core/Src/startup_stm32f4xx.d ./Core/Src/startup_stm32f4xx.o ./Core/Src/startup_stm32f4xx.su ./Core/Src/stm32_ub_vga_screen.cyclo ./Core/Src/stm32_ub_vga_screen.d ./Core/Src/stm32_ub_vga_screen.o ./Core/Src/stm32_ub_vga_screen.su ./Core/Src/stm32f4xx_dma.cyclo ./Core/Src/stm32f4xx_dma.d ./Core/Src/stm32f4xx_dma.o ./Core/Src/stm32f4xx_dma.su ./Core/Src/stm32f4xx_gpio.cyclo ./Core/Src/stm32f4xx_gpio.d ./Core/Src/stm32f4xx_gpio.o ./Core/Src/stm32f4xx_gpio.su ./Core/Src/stm32f4xx_rcc.cyclo ./Core/Src/stm32f4xx_rcc.d ./Core/Src/stm32f4xx_rcc.o ./Core/Src/stm32f4xx_rcc.su ./Core/Src/stm32f4xx_tim.cyclo ./Core/Src/stm32f4xx_tim.d ./Core/Src/stm32f4xx_tim.o ./Core/Src/stm32f4xx_tim.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src

