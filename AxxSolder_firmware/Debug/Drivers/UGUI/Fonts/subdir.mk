################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/UGUI/Fonts/arial_10X13.c \
../Drivers/UGUI/Fonts/arial_12X15.c \
../Drivers/UGUI/Fonts/arial_16X18.c \
../Drivers/UGUI/Fonts/arial_19X22.c \
../Drivers/UGUI/Fonts/arial_20X23.c \
../Drivers/UGUI/Fonts/arial_25X28.c \
../Drivers/UGUI/Fonts/arial_29X35.c \
../Drivers/UGUI/Fonts/arial_35X40.c \
../Drivers/UGUI/Fonts/arial_36X44_C.c \
../Drivers/UGUI/Fonts/arial_36X44_NUMBERS.c \
../Drivers/UGUI/Fonts/arial_39X45.c \
../Drivers/UGUI/Fonts/arial_45X52.c \
../Drivers/UGUI/Fonts/arial_49X57.c \
../Drivers/UGUI/Fonts/arial_6X6.c \
../Drivers/UGUI/Fonts/arial_9X10.c \
../Drivers/UGUI/Fonts/font_10x16.c \
../Drivers/UGUI/Fonts/font_12x16.c \
../Drivers/UGUI/Fonts/font_12x20.c \
../Drivers/UGUI/Fonts/font_16x26.c \
../Drivers/UGUI/Fonts/font_22x36.c \
../Drivers/UGUI/Fonts/font_24x40.c \
../Drivers/UGUI/Fonts/font_32x53.c \
../Drivers/UGUI/Fonts/font_4x6.c \
../Drivers/UGUI/Fonts/font_5x12.c \
../Drivers/UGUI/Fonts/font_5x8.c \
../Drivers/UGUI/Fonts/font_6x10.c \
../Drivers/UGUI/Fonts/font_6x8.c \
../Drivers/UGUI/Fonts/font_7x12.c \
../Drivers/UGUI/Fonts/font_8x12.c \
../Drivers/UGUI/Fonts/font_8x14.c \
../Drivers/UGUI/Fonts/font_8x8.c 

OBJS += \
./Drivers/UGUI/Fonts/arial_10X13.o \
./Drivers/UGUI/Fonts/arial_12X15.o \
./Drivers/UGUI/Fonts/arial_16X18.o \
./Drivers/UGUI/Fonts/arial_19X22.o \
./Drivers/UGUI/Fonts/arial_20X23.o \
./Drivers/UGUI/Fonts/arial_25X28.o \
./Drivers/UGUI/Fonts/arial_29X35.o \
./Drivers/UGUI/Fonts/arial_35X40.o \
./Drivers/UGUI/Fonts/arial_36X44_C.o \
./Drivers/UGUI/Fonts/arial_36X44_NUMBERS.o \
./Drivers/UGUI/Fonts/arial_39X45.o \
./Drivers/UGUI/Fonts/arial_45X52.o \
./Drivers/UGUI/Fonts/arial_49X57.o \
./Drivers/UGUI/Fonts/arial_6X6.o \
./Drivers/UGUI/Fonts/arial_9X10.o \
./Drivers/UGUI/Fonts/font_10x16.o \
./Drivers/UGUI/Fonts/font_12x16.o \
./Drivers/UGUI/Fonts/font_12x20.o \
./Drivers/UGUI/Fonts/font_16x26.o \
./Drivers/UGUI/Fonts/font_22x36.o \
./Drivers/UGUI/Fonts/font_24x40.o \
./Drivers/UGUI/Fonts/font_32x53.o \
./Drivers/UGUI/Fonts/font_4x6.o \
./Drivers/UGUI/Fonts/font_5x12.o \
./Drivers/UGUI/Fonts/font_5x8.o \
./Drivers/UGUI/Fonts/font_6x10.o \
./Drivers/UGUI/Fonts/font_6x8.o \
./Drivers/UGUI/Fonts/font_7x12.o \
./Drivers/UGUI/Fonts/font_8x12.o \
./Drivers/UGUI/Fonts/font_8x14.o \
./Drivers/UGUI/Fonts/font_8x8.o 

C_DEPS += \
./Drivers/UGUI/Fonts/arial_10X13.d \
./Drivers/UGUI/Fonts/arial_12X15.d \
./Drivers/UGUI/Fonts/arial_16X18.d \
./Drivers/UGUI/Fonts/arial_19X22.d \
./Drivers/UGUI/Fonts/arial_20X23.d \
./Drivers/UGUI/Fonts/arial_25X28.d \
./Drivers/UGUI/Fonts/arial_29X35.d \
./Drivers/UGUI/Fonts/arial_35X40.d \
./Drivers/UGUI/Fonts/arial_36X44_C.d \
./Drivers/UGUI/Fonts/arial_36X44_NUMBERS.d \
./Drivers/UGUI/Fonts/arial_39X45.d \
./Drivers/UGUI/Fonts/arial_45X52.d \
./Drivers/UGUI/Fonts/arial_49X57.d \
./Drivers/UGUI/Fonts/arial_6X6.d \
./Drivers/UGUI/Fonts/arial_9X10.d \
./Drivers/UGUI/Fonts/font_10x16.d \
./Drivers/UGUI/Fonts/font_12x16.d \
./Drivers/UGUI/Fonts/font_12x20.d \
./Drivers/UGUI/Fonts/font_16x26.d \
./Drivers/UGUI/Fonts/font_22x36.d \
./Drivers/UGUI/Fonts/font_24x40.d \
./Drivers/UGUI/Fonts/font_32x53.d \
./Drivers/UGUI/Fonts/font_4x6.d \
./Drivers/UGUI/Fonts/font_5x12.d \
./Drivers/UGUI/Fonts/font_5x8.d \
./Drivers/UGUI/Fonts/font_6x10.d \
./Drivers/UGUI/Fonts/font_6x8.d \
./Drivers/UGUI/Fonts/font_7x12.d \
./Drivers/UGUI/Fonts/font_8x12.d \
./Drivers/UGUI/Fonts/font_8x14.d \
./Drivers/UGUI/Fonts/font_8x8.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/UGUI/Fonts/%.o Drivers/UGUI/Fonts/%.su Drivers/UGUI/Fonts/%.cyclo: ../Drivers/UGUI/Fonts/%.c Drivers/UGUI/Fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/LCD -I../Drivers/UGUI -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-UGUI-2f-Fonts

clean-Drivers-2f-UGUI-2f-Fonts:
	-$(RM) ./Drivers/UGUI/Fonts/arial_10X13.cyclo ./Drivers/UGUI/Fonts/arial_10X13.d ./Drivers/UGUI/Fonts/arial_10X13.o ./Drivers/UGUI/Fonts/arial_10X13.su ./Drivers/UGUI/Fonts/arial_12X15.cyclo ./Drivers/UGUI/Fonts/arial_12X15.d ./Drivers/UGUI/Fonts/arial_12X15.o ./Drivers/UGUI/Fonts/arial_12X15.su ./Drivers/UGUI/Fonts/arial_16X18.cyclo ./Drivers/UGUI/Fonts/arial_16X18.d ./Drivers/UGUI/Fonts/arial_16X18.o ./Drivers/UGUI/Fonts/arial_16X18.su ./Drivers/UGUI/Fonts/arial_19X22.cyclo ./Drivers/UGUI/Fonts/arial_19X22.d ./Drivers/UGUI/Fonts/arial_19X22.o ./Drivers/UGUI/Fonts/arial_19X22.su ./Drivers/UGUI/Fonts/arial_20X23.cyclo ./Drivers/UGUI/Fonts/arial_20X23.d ./Drivers/UGUI/Fonts/arial_20X23.o ./Drivers/UGUI/Fonts/arial_20X23.su ./Drivers/UGUI/Fonts/arial_25X28.cyclo ./Drivers/UGUI/Fonts/arial_25X28.d ./Drivers/UGUI/Fonts/arial_25X28.o ./Drivers/UGUI/Fonts/arial_25X28.su ./Drivers/UGUI/Fonts/arial_29X35.cyclo ./Drivers/UGUI/Fonts/arial_29X35.d ./Drivers/UGUI/Fonts/arial_29X35.o ./Drivers/UGUI/Fonts/arial_29X35.su ./Drivers/UGUI/Fonts/arial_35X40.cyclo ./Drivers/UGUI/Fonts/arial_35X40.d ./Drivers/UGUI/Fonts/arial_35X40.o ./Drivers/UGUI/Fonts/arial_35X40.su ./Drivers/UGUI/Fonts/arial_36X44_C.cyclo ./Drivers/UGUI/Fonts/arial_36X44_C.d ./Drivers/UGUI/Fonts/arial_36X44_C.o ./Drivers/UGUI/Fonts/arial_36X44_C.su ./Drivers/UGUI/Fonts/arial_36X44_NUMBERS.cyclo ./Drivers/UGUI/Fonts/arial_36X44_NUMBERS.d ./Drivers/UGUI/Fonts/arial_36X44_NUMBERS.o ./Drivers/UGUI/Fonts/arial_36X44_NUMBERS.su ./Drivers/UGUI/Fonts/arial_39X45.cyclo ./Drivers/UGUI/Fonts/arial_39X45.d ./Drivers/UGUI/Fonts/arial_39X45.o ./Drivers/UGUI/Fonts/arial_39X45.su ./Drivers/UGUI/Fonts/arial_45X52.cyclo ./Drivers/UGUI/Fonts/arial_45X52.d ./Drivers/UGUI/Fonts/arial_45X52.o ./Drivers/UGUI/Fonts/arial_45X52.su ./Drivers/UGUI/Fonts/arial_49X57.cyclo ./Drivers/UGUI/Fonts/arial_49X57.d ./Drivers/UGUI/Fonts/arial_49X57.o ./Drivers/UGUI/Fonts/arial_49X57.su ./Drivers/UGUI/Fonts/arial_6X6.cyclo ./Drivers/UGUI/Fonts/arial_6X6.d ./Drivers/UGUI/Fonts/arial_6X6.o ./Drivers/UGUI/Fonts/arial_6X6.su ./Drivers/UGUI/Fonts/arial_9X10.cyclo ./Drivers/UGUI/Fonts/arial_9X10.d ./Drivers/UGUI/Fonts/arial_9X10.o ./Drivers/UGUI/Fonts/arial_9X10.su ./Drivers/UGUI/Fonts/font_10x16.cyclo ./Drivers/UGUI/Fonts/font_10x16.d ./Drivers/UGUI/Fonts/font_10x16.o ./Drivers/UGUI/Fonts/font_10x16.su ./Drivers/UGUI/Fonts/font_12x16.cyclo ./Drivers/UGUI/Fonts/font_12x16.d ./Drivers/UGUI/Fonts/font_12x16.o ./Drivers/UGUI/Fonts/font_12x16.su ./Drivers/UGUI/Fonts/font_12x20.cyclo ./Drivers/UGUI/Fonts/font_12x20.d ./Drivers/UGUI/Fonts/font_12x20.o ./Drivers/UGUI/Fonts/font_12x20.su ./Drivers/UGUI/Fonts/font_16x26.cyclo ./Drivers/UGUI/Fonts/font_16x26.d ./Drivers/UGUI/Fonts/font_16x26.o ./Drivers/UGUI/Fonts/font_16x26.su ./Drivers/UGUI/Fonts/font_22x36.cyclo ./Drivers/UGUI/Fonts/font_22x36.d ./Drivers/UGUI/Fonts/font_22x36.o ./Drivers/UGUI/Fonts/font_22x36.su ./Drivers/UGUI/Fonts/font_24x40.cyclo ./Drivers/UGUI/Fonts/font_24x40.d ./Drivers/UGUI/Fonts/font_24x40.o ./Drivers/UGUI/Fonts/font_24x40.su ./Drivers/UGUI/Fonts/font_32x53.cyclo ./Drivers/UGUI/Fonts/font_32x53.d ./Drivers/UGUI/Fonts/font_32x53.o ./Drivers/UGUI/Fonts/font_32x53.su ./Drivers/UGUI/Fonts/font_4x6.cyclo ./Drivers/UGUI/Fonts/font_4x6.d ./Drivers/UGUI/Fonts/font_4x6.o ./Drivers/UGUI/Fonts/font_4x6.su ./Drivers/UGUI/Fonts/font_5x12.cyclo ./Drivers/UGUI/Fonts/font_5x12.d ./Drivers/UGUI/Fonts/font_5x12.o ./Drivers/UGUI/Fonts/font_5x12.su ./Drivers/UGUI/Fonts/font_5x8.cyclo ./Drivers/UGUI/Fonts/font_5x8.d ./Drivers/UGUI/Fonts/font_5x8.o ./Drivers/UGUI/Fonts/font_5x8.su ./Drivers/UGUI/Fonts/font_6x10.cyclo ./Drivers/UGUI/Fonts/font_6x10.d ./Drivers/UGUI/Fonts/font_6x10.o ./Drivers/UGUI/Fonts/font_6x10.su ./Drivers/UGUI/Fonts/font_6x8.cyclo ./Drivers/UGUI/Fonts/font_6x8.d ./Drivers/UGUI/Fonts/font_6x8.o ./Drivers/UGUI/Fonts/font_6x8.su ./Drivers/UGUI/Fonts/font_7x12.cyclo ./Drivers/UGUI/Fonts/font_7x12.d ./Drivers/UGUI/Fonts/font_7x12.o ./Drivers/UGUI/Fonts/font_7x12.su ./Drivers/UGUI/Fonts/font_8x12.cyclo ./Drivers/UGUI/Fonts/font_8x12.d ./Drivers/UGUI/Fonts/font_8x12.o ./Drivers/UGUI/Fonts/font_8x12.su ./Drivers/UGUI/Fonts/font_8x14.cyclo ./Drivers/UGUI/Fonts/font_8x14.d ./Drivers/UGUI/Fonts/font_8x14.o ./Drivers/UGUI/Fonts/font_8x14.su ./Drivers/UGUI/Fonts/font_8x8.cyclo ./Drivers/UGUI/Fonts/font_8x8.d ./Drivers/UGUI/Fonts/font_8x8.o ./Drivers/UGUI/Fonts/font_8x8.su

.PHONY: clean-Drivers-2f-UGUI-2f-Fonts

