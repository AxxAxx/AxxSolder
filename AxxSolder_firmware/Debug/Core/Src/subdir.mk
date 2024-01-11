################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/flash.c \
../Core/Src/fonts.c \
../Core/Src/main.c \
../Core/Src/moving_average.c \
../Core/Src/pid.c \
../Core/Src/st7789.c \
../Core/Src/stm32f3xx_hal_msp.c \
../Core/Src/stm32f3xx_it.c \
../Core/Src/stusb4500.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f3xx.c 

OBJS += \
./Core/Src/flash.o \
./Core/Src/fonts.o \
./Core/Src/main.o \
./Core/Src/moving_average.o \
./Core/Src/pid.o \
./Core/Src/st7789.o \
./Core/Src/stm32f3xx_hal_msp.o \
./Core/Src/stm32f3xx_it.o \
./Core/Src/stusb4500.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f3xx.o 

C_DEPS += \
./Core/Src/flash.d \
./Core/Src/fonts.d \
./Core/Src/main.d \
./Core/Src/moving_average.d \
./Core/Src/pid.d \
./Core/Src/st7789.d \
./Core/Src/stm32f3xx_hal_msp.d \
./Core/Src/stm32f3xx_it.d \
./Core/Src/stusb4500.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f3xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303xC -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/flash.cyclo ./Core/Src/flash.d ./Core/Src/flash.o ./Core/Src/flash.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/moving_average.cyclo ./Core/Src/moving_average.d ./Core/Src/moving_average.o ./Core/Src/moving_average.su ./Core/Src/pid.cyclo ./Core/Src/pid.d ./Core/Src/pid.o ./Core/Src/pid.su ./Core/Src/st7789.cyclo ./Core/Src/st7789.d ./Core/Src/st7789.o ./Core/Src/st7789.su ./Core/Src/stm32f3xx_hal_msp.cyclo ./Core/Src/stm32f3xx_hal_msp.d ./Core/Src/stm32f3xx_hal_msp.o ./Core/Src/stm32f3xx_hal_msp.su ./Core/Src/stm32f3xx_it.cyclo ./Core/Src/stm32f3xx_it.d ./Core/Src/stm32f3xx_it.o ./Core/Src/stm32f3xx_it.su ./Core/Src/stusb4500.cyclo ./Core/Src/stusb4500.d ./Core/Src/stusb4500.o ./Core/Src/stusb4500.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f3xx.cyclo ./Core/Src/system_stm32f3xx.d ./Core/Src/system_stm32f3xx.o ./Core/Src/system_stm32f3xx.su

.PHONY: clean-Core-2f-Src

