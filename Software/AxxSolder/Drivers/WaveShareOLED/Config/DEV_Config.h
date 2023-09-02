/******************************************************************************
**************************Hardware interface layer*****************************
* | file      		:	DEV_Config.c
* |	version			:	V1.0
* | date			:	2020-06-17
* | function		:	Provide the hardware underlying interface	
******************************************************************************/
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"
#include "main.h"
#include <stdint.h>
#include <stdlib.h>

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

#define USE_SPI_4W 		1
#define USE_IIC 		0
#define USE_IIC_SOFT	0

#define IIC_CMD		0X00
#define IIC_RAM		0X40

//OLED GPIO
#define OLED_CS_0		HAL_GPIO_WritePin(GPIOB, SPI_CS_Pin, GPIO_PIN_RESET)
#define OLED_CS_1		HAL_GPIO_WritePin(GPIOB, SPI_CS_Pin, GPIO_PIN_SET)

#define OLED_DC_0		HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin, GPIO_PIN_RESET)
#define OLED_DC_1		HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin, GPIO_PIN_SET)

#define OLED_RST_0		HAL_GPIO_WritePin(GPIOB, SPI_RST_Pin, GPIO_PIN_RESET)
#define OLED_RST_1		HAL_GPIO_WritePin(GPIOB, SPI_RST_Pin, GPIO_PIN_SET)

////SPI GPIO
//#define SPI1_SCK_0		HAL_GPIO_WritePin(SPI1_SCK_GPIO_Port, SPI1_SCK_Pin, GPIO_PIN_RESET)
//#define SPI1_SCK_1		HAL_GPIO_WritePin(SPI1_SCK_GPIO_Port, SPI1_SCK_Pin, GPIO_PIN_SET)

//#define SPI1_MOSI_0		HAL_GPIO_WritePin(SPI1_MOSI_GPIO_Port, SPI1_MOSI_Pin, GPIO_PIN_RESET)
//#define SPI1_MOSI_1		HAL_GPIO_WritePin(SPI1_MOSI_GPIO_Port, SPI1_MOSI_Pin, GPIO_PIN_SET)
/*------------------------------------------------------------------------------------------------------*/

UBYTE 	System_Init(void);
void    System_Exit(void);

UBYTE 	SPI4W_Write_Byte(UBYTE value);
void 	I2C_Write_Byte(UBYTE value, UBYTE Cmd);

void Driver_Delay_ms(uint32_t xms);
void Driver_Delay_us(uint32_t xus);

#endif
