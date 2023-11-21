/******************************************************************************
**************************Hardware interface layer*****************************
* | file      		:	DEV_Config.c
* |	version			:	V1.0
* | date			:	2020-06-17
* | function		:	Provide the hardware underlying interface	
******************************************************************************/
#include "DEV_Config.h"
#include "stm32g4xx_hal_i2c.h"
#include "main.h"

//include "stm32g4xx_hal_spi.h"
//#include "spi.h"

//#include "usart.h"
#include <stdio.h>		//printf()
#include <string.h>
#include <stdlib.h>

/********************************************************************************
function:	System Init
note:
	Initialize the communication method
********************************************************************************/
uint8_t System_Init(void)
{
#if USE_SPI_4W
	printf("USE_SPI_4W\r\n");
#elif USE_IIC
	printf("USE_IIC\r\n");
	OLED_CS_0;
	OLED_DC_0;//DC = 0,1 >> Address = 0x3c,0x3d
#elif USE_IIC_SOFT
	printf("USEI_IIC_SOFT\r\n");
	OLED_CS_0;
	OLED_DC_0;
#endif
  return 0;
}

void System_Exit(void)
{

}
/********************************************************************************
function:	Hardware interface
note:
	SPI4W_Write_Byte(value) : 
		HAL library hardware SPI
		Register hardware SPI
		Gpio analog SPI
	I2C_Write_Byte(value, cmd):
		HAL library hardware I2C
********************************************************************************/
uint8_t SPI4W_Write_Byte(uint8_t value)
{
#if 0
    HAL_SPI_Transmit(&hspi1, &value, 1, 500);
#elif 0
    char i;
    for(i = 0; i < 8; i++) {
        SPI_SCK_0;
        if(value & 0X80)
            SPI_MOSI_1;
        else
            SPI_MOSI_0;
        Driver_Delay_us(10);
        SPI_SCK_1;
        Driver_Delay_us(10);
        value = (value << 1);
    }
#else
    __HAL_SPI_ENABLE(&hspi1);
    SPI1->CR2 |= (1) << 12;

    while((SPI1->SR & (1 << 1)) == 0)
        ;

    *((__IO uint8_t *)(&SPI1->DR)) = value;

    while(SPI1->SR & (1 << 7)) ; //Wait for not busy

    while((SPI1->SR & (1 << 0)) == 0) ; // Wait for the receiving area to be empty

    return *((__IO uint8_t *)(&SPI1->DR));
#endif
}

void I2C_Write_Byte(uint8_t value, uint8_t Cmd)
{
    int Err;
    uint8_t W_Buf[2] ;
    W_Buf[0] = Cmd;
    W_Buf[1] = value;
    if(HAL_I2C_Master_Transmit(&hi2c1, (0X3C << 1) | 0X00, W_Buf, 2, 0x10) != HAL_OK) {
        Err++;
        if(Err == 1000) {
            printf("send error\r\n");
            return ;
        }
    }
}

/********************************************************************************
function:	Delay function
note:
	Driver_Delay_ms(xms) : Delay x ms
	Driver_Delay_us(xus) : Delay x us
********************************************************************************/
void Driver_Delay_ms(uint32_t xms)
{
    HAL_Delay(xms);
}

void Driver_Delay_us(uint32_t xus)
{
    int j;
    for(j=xus; j > 0; j--);
}
