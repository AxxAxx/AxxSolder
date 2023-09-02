################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/WaveShareOLED/OLED/OLED_0in91.c \
../Drivers/WaveShareOLED/OLED/OLED_0in95_rgb.c \
../Drivers/WaveShareOLED/OLED/OLED_0in96.c \
../Drivers/WaveShareOLED/OLED/OLED_1in3.c \
../Drivers/WaveShareOLED/OLED/OLED_1in3_c.c \
../Drivers/WaveShareOLED/OLED/OLED_1in5.c \
../Drivers/WaveShareOLED/OLED/OLED_1in5_rgb.c 

OBJS += \
./Drivers/WaveShareOLED/OLED/OLED_0in91.o \
./Drivers/WaveShareOLED/OLED/OLED_0in95_rgb.o \
./Drivers/WaveShareOLED/OLED/OLED_0in96.o \
./Drivers/WaveShareOLED/OLED/OLED_1in3.o \
./Drivers/WaveShareOLED/OLED/OLED_1in3_c.o \
./Drivers/WaveShareOLED/OLED/OLED_1in5.o \
./Drivers/WaveShareOLED/OLED/OLED_1in5_rgb.o 

C_DEPS += \
./Drivers/WaveShareOLED/OLED/OLED_0in91.d \
./Drivers/WaveShareOLED/OLED/OLED_0in95_rgb.d \
./Drivers/WaveShareOLED/OLED/OLED_0in96.d \
./Drivers/WaveShareOLED/OLED/OLED_1in3.d \
./Drivers/WaveShareOLED/OLED/OLED_1in3_c.d \
./Drivers/WaveShareOLED/OLED/OLED_1in5.d \
./Drivers/WaveShareOLED/OLED/OLED_1in5_rgb.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/WaveShareOLED/OLED/%.o Drivers/WaveShareOLED/OLED/%.su: ../Drivers/WaveShareOLED/OLED/%.c Drivers/WaveShareOLED/OLED/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Config" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Example" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Fonts" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/GUI" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/OLED" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-WaveShareOLED-2f-OLED

clean-Drivers-2f-WaveShareOLED-2f-OLED:
	-$(RM) ./Drivers/WaveShareOLED/OLED/OLED_0in91.d ./Drivers/WaveShareOLED/OLED/OLED_0in91.o ./Drivers/WaveShareOLED/OLED/OLED_0in91.su ./Drivers/WaveShareOLED/OLED/OLED_0in95_rgb.d ./Drivers/WaveShareOLED/OLED/OLED_0in95_rgb.o ./Drivers/WaveShareOLED/OLED/OLED_0in95_rgb.su ./Drivers/WaveShareOLED/OLED/OLED_0in96.d ./Drivers/WaveShareOLED/OLED/OLED_0in96.o ./Drivers/WaveShareOLED/OLED/OLED_0in96.su ./Drivers/WaveShareOLED/OLED/OLED_1in3.d ./Drivers/WaveShareOLED/OLED/OLED_1in3.o ./Drivers/WaveShareOLED/OLED/OLED_1in3.su ./Drivers/WaveShareOLED/OLED/OLED_1in3_c.d ./Drivers/WaveShareOLED/OLED/OLED_1in3_c.o ./Drivers/WaveShareOLED/OLED/OLED_1in3_c.su ./Drivers/WaveShareOLED/OLED/OLED_1in5.d ./Drivers/WaveShareOLED/OLED/OLED_1in5.o ./Drivers/WaveShareOLED/OLED/OLED_1in5.su ./Drivers/WaveShareOLED/OLED/OLED_1in5_rgb.d ./Drivers/WaveShareOLED/OLED/OLED_1in5_rgb.o ./Drivers/WaveShareOLED/OLED/OLED_1in5_rgb.su

.PHONY: clean-Drivers-2f-WaveShareOLED-2f-OLED

