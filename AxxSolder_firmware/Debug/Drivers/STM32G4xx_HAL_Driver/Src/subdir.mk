################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.c \
../Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.c 

OBJS += \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.o \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.o 

C_DEPS += \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.d \
./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32G4xx_HAL_Driver/Src/%.o Drivers/STM32G4xx_HAL_Driver/Src/%.su Drivers/STM32G4xx_HAL_Driver/Src/%.cyclo: ../Drivers/STM32G4xx_HAL_Driver/Src/%.c Drivers/STM32G4xx_HAL_Driver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G431xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/LCD -I../Drivers/UGUI -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-STM32G4xx_HAL_Driver-2f-Src

clean-Drivers-2f-STM32G4xx_HAL_Driver-2f-Src:
	-$(RM) ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_crc_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_spi_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.o
	-$(RM) ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.su ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.cyclo ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.d ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.o ./Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.su

.PHONY: clean-Drivers-2f-STM32G4xx_HAL_Driver-2f-Src

