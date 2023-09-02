################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/WaveShareOLED/Fonts/font12.c \
../Drivers/WaveShareOLED/Fonts/font12CN.c \
../Drivers/WaveShareOLED/Fonts/font16.c \
../Drivers/WaveShareOLED/Fonts/font20.c \
../Drivers/WaveShareOLED/Fonts/font24.c \
../Drivers/WaveShareOLED/Fonts/font24CN.c \
../Drivers/WaveShareOLED/Fonts/font8.c 

OBJS += \
./Drivers/WaveShareOLED/Fonts/font12.o \
./Drivers/WaveShareOLED/Fonts/font12CN.o \
./Drivers/WaveShareOLED/Fonts/font16.o \
./Drivers/WaveShareOLED/Fonts/font20.o \
./Drivers/WaveShareOLED/Fonts/font24.o \
./Drivers/WaveShareOLED/Fonts/font24CN.o \
./Drivers/WaveShareOLED/Fonts/font8.o 

C_DEPS += \
./Drivers/WaveShareOLED/Fonts/font12.d \
./Drivers/WaveShareOLED/Fonts/font12CN.d \
./Drivers/WaveShareOLED/Fonts/font16.d \
./Drivers/WaveShareOLED/Fonts/font20.d \
./Drivers/WaveShareOLED/Fonts/font24.d \
./Drivers/WaveShareOLED/Fonts/font24CN.d \
./Drivers/WaveShareOLED/Fonts/font8.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/WaveShareOLED/Fonts/%.o Drivers/WaveShareOLED/Fonts/%.su: ../Drivers/WaveShareOLED/Fonts/%.c Drivers/WaveShareOLED/Fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Config" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Example" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Fonts" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/GUI" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/OLED" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-WaveShareOLED-2f-Fonts

clean-Drivers-2f-WaveShareOLED-2f-Fonts:
	-$(RM) ./Drivers/WaveShareOLED/Fonts/font12.d ./Drivers/WaveShareOLED/Fonts/font12.o ./Drivers/WaveShareOLED/Fonts/font12.su ./Drivers/WaveShareOLED/Fonts/font12CN.d ./Drivers/WaveShareOLED/Fonts/font12CN.o ./Drivers/WaveShareOLED/Fonts/font12CN.su ./Drivers/WaveShareOLED/Fonts/font16.d ./Drivers/WaveShareOLED/Fonts/font16.o ./Drivers/WaveShareOLED/Fonts/font16.su ./Drivers/WaveShareOLED/Fonts/font20.d ./Drivers/WaveShareOLED/Fonts/font20.o ./Drivers/WaveShareOLED/Fonts/font20.su ./Drivers/WaveShareOLED/Fonts/font24.d ./Drivers/WaveShareOLED/Fonts/font24.o ./Drivers/WaveShareOLED/Fonts/font24.su ./Drivers/WaveShareOLED/Fonts/font24CN.d ./Drivers/WaveShareOLED/Fonts/font24CN.o ./Drivers/WaveShareOLED/Fonts/font24CN.su ./Drivers/WaveShareOLED/Fonts/font8.d ./Drivers/WaveShareOLED/Fonts/font8.o ./Drivers/WaveShareOLED/Fonts/font8.su

.PHONY: clean-Drivers-2f-WaveShareOLED-2f-Fonts

