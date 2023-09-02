################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/WaveShareOLED/Example/ImageData.c \
../Drivers/WaveShareOLED/Example/OLED_0in91_test.c \
../Drivers/WaveShareOLED/Example/OLED_0in95_rgb_test.c \
../Drivers/WaveShareOLED/Example/OLED_0in96_test.c \
../Drivers/WaveShareOLED/Example/OLED_1in3_c_test.c \
../Drivers/WaveShareOLED/Example/OLED_1in3_test.c \
../Drivers/WaveShareOLED/Example/OLED_1in5_rgb_test.c \
../Drivers/WaveShareOLED/Example/OLED_1in5_test.c 

OBJS += \
./Drivers/WaveShareOLED/Example/ImageData.o \
./Drivers/WaveShareOLED/Example/OLED_0in91_test.o \
./Drivers/WaveShareOLED/Example/OLED_0in95_rgb_test.o \
./Drivers/WaveShareOLED/Example/OLED_0in96_test.o \
./Drivers/WaveShareOLED/Example/OLED_1in3_c_test.o \
./Drivers/WaveShareOLED/Example/OLED_1in3_test.o \
./Drivers/WaveShareOLED/Example/OLED_1in5_rgb_test.o \
./Drivers/WaveShareOLED/Example/OLED_1in5_test.o 

C_DEPS += \
./Drivers/WaveShareOLED/Example/ImageData.d \
./Drivers/WaveShareOLED/Example/OLED_0in91_test.d \
./Drivers/WaveShareOLED/Example/OLED_0in95_rgb_test.d \
./Drivers/WaveShareOLED/Example/OLED_0in96_test.d \
./Drivers/WaveShareOLED/Example/OLED_1in3_c_test.d \
./Drivers/WaveShareOLED/Example/OLED_1in3_test.d \
./Drivers/WaveShareOLED/Example/OLED_1in5_rgb_test.d \
./Drivers/WaveShareOLED/Example/OLED_1in5_test.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/WaveShareOLED/Example/%.o Drivers/WaveShareOLED/Example/%.su: ../Drivers/WaveShareOLED/Example/%.c Drivers/WaveShareOLED/Example/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Config" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Example" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Fonts" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/GUI" -I"C:/owncloud/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/OLED" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-WaveShareOLED-2f-Example

clean-Drivers-2f-WaveShareOLED-2f-Example:
	-$(RM) ./Drivers/WaveShareOLED/Example/ImageData.d ./Drivers/WaveShareOLED/Example/ImageData.o ./Drivers/WaveShareOLED/Example/ImageData.su ./Drivers/WaveShareOLED/Example/OLED_0in91_test.d ./Drivers/WaveShareOLED/Example/OLED_0in91_test.o ./Drivers/WaveShareOLED/Example/OLED_0in91_test.su ./Drivers/WaveShareOLED/Example/OLED_0in95_rgb_test.d ./Drivers/WaveShareOLED/Example/OLED_0in95_rgb_test.o ./Drivers/WaveShareOLED/Example/OLED_0in95_rgb_test.su ./Drivers/WaveShareOLED/Example/OLED_0in96_test.d ./Drivers/WaveShareOLED/Example/OLED_0in96_test.o ./Drivers/WaveShareOLED/Example/OLED_0in96_test.su ./Drivers/WaveShareOLED/Example/OLED_1in3_c_test.d ./Drivers/WaveShareOLED/Example/OLED_1in3_c_test.o ./Drivers/WaveShareOLED/Example/OLED_1in3_c_test.su ./Drivers/WaveShareOLED/Example/OLED_1in3_test.d ./Drivers/WaveShareOLED/Example/OLED_1in3_test.o ./Drivers/WaveShareOLED/Example/OLED_1in3_test.su ./Drivers/WaveShareOLED/Example/OLED_1in5_rgb_test.d ./Drivers/WaveShareOLED/Example/OLED_1in5_rgb_test.o ./Drivers/WaveShareOLED/Example/OLED_1in5_rgb_test.su ./Drivers/WaveShareOLED/Example/OLED_1in5_test.d ./Drivers/WaveShareOLED/Example/OLED_1in5_test.o ./Drivers/WaveShareOLED/Example/OLED_1in5_test.su

.PHONY: clean-Drivers-2f-WaveShareOLED-2f-Example

