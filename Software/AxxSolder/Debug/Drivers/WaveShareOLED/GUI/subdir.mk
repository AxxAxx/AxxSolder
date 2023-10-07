################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/WaveShareOLED/GUI/GUI_Paint.c 

OBJS += \
./Drivers/WaveShareOLED/GUI/GUI_Paint.o 

C_DEPS += \
./Drivers/WaveShareOLED/GUI/GUI_Paint.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/WaveShareOLED/GUI/%.o Drivers/WaveShareOLED/GUI/%.su Drivers/WaveShareOLED/GUI/%.cyclo: ../Drivers/WaveShareOLED/GUI/%.c Drivers/WaveShareOLED/GUI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I"C:/Users/axel_/ownCloud - axel@192.168.1.36/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Config" -I"C:/Users/axel_/ownCloud - axel@192.168.1.36/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Example" -I"C:/Users/axel_/ownCloud - axel@192.168.1.36/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/Fonts" -I"C:/Users/axel_/ownCloud - axel@192.168.1.36/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/GUI" -I"C:/Users/axel_/ownCloud - axel@192.168.1.36/2_Project/00_AxxProjects/AxxSolder/Software/AxxSolder/Drivers/WaveShareOLED/OLED" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-WaveShareOLED-2f-GUI

clean-Drivers-2f-WaveShareOLED-2f-GUI:
	-$(RM) ./Drivers/WaveShareOLED/GUI/GUI_Paint.cyclo ./Drivers/WaveShareOLED/GUI/GUI_Paint.d ./Drivers/WaveShareOLED/GUI/GUI_Paint.o ./Drivers/WaveShareOLED/GUI/GUI_Paint.su

.PHONY: clean-Drivers-2f-WaveShareOLED-2f-GUI

