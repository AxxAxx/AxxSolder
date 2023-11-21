#ifndef _SOFT_IIC_H_
#define _SOFT_IIC_H_

#include "DEV_Config.h"

#define IIC_SOFT_SCL_PIN         0//IIC_SCL_SOFT_Pin
#define IIC_SOFT_SDA_PIN         0//IIC_SDA_SOFT_Pin

#define IIC_SOFT_SCL_GPIO        GPIOC
#define IIC_SOFT_SDA_GPIO        GPIOC

#define __IIC_SCL_SET()     HAL_GPIO_WritePin(IIC_SOFT_SCL_GPIO, IIC_SOFT_SCL_PIN, GPIO_PIN_SET)
#define __IIC_SCL_CLR()     HAL_GPIO_WritePin(IIC_SOFT_SCL_GPIO, IIC_SOFT_SCL_PIN, GPIO_PIN_RESET)

#define __IIC_SDA_SET()		HAL_GPIO_WritePin(IIC_SOFT_SDA_GPIO, IIC_SOFT_SDA_PIN, GPIO_PIN_SET)
#define __IIC_SDA_CLR()     HAL_GPIO_WritePin(IIC_SOFT_SDA_GPIO, IIC_SOFT_SDA_PIN, GPIO_PIN_RESET)

#define __IIC_SDA_IN()     	do { \
								GPIO_InitTypeDef tGPIO; \
								tGPIO.Pin = IIC_SOFT_SDA_PIN; \
								tGPIO.Mode = GPIO_MODE_INPUT; \
								tGPIO.Pull = GPIO_PULLUP; \
								tGPIO.Speed = GPIO_SPEED_FREQ_MEDIUM; \
								HAL_GPIO_Init(IIC_SOFT_SDA_GPIO, &tGPIO); \
							}while(0)				

#define __IIC_SDA_OUT() 	do { \
								GPIO_InitTypeDef tGPIO; \
								tGPIO.Pin = IIC_SOFT_SDA_PIN; \
								tGPIO.Mode = GPIO_MODE_OUTPUT_PP; \
								tGPIO.Pull = GPIO_PULLUP; \
								tGPIO.Speed = GPIO_SPEED_FREQ_MEDIUM; \
								HAL_GPIO_Init(IIC_SOFT_SDA_GPIO, &tGPIO); \
							}while(0)   



#define __IIC_SDA_READ()    HAL_GPIO_ReadPin(IIC_SOFT_SDA_GPIO, IIC_SOFT_SDA_PIN)

extern void iic_init(void);
extern void iic_start(void);
extern void iic_stop(void);
extern int iic_wait_for_ack(void);
extern void iic_write_byte(UBYTE chData);

#endif

