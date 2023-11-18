/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct
{
  float numerical_flash_values[10];
}ConfigurationMsg;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DEBUG_SIGNAL_A_Pin GPIO_PIN_0
#define DEBUG_SIGNAL_A_GPIO_Port GPIOF
#define DEBUG_SIGNAL_B_Pin GPIO_PIN_1
#define DEBUG_SIGNAL_B_GPIO_Port GPIOF
#define THERMOCOUPLE_Pin GPIO_PIN_0
#define THERMOCOUPLE_GPIO_Port GPIOA
#define CURRENT_Pin GPIO_PIN_1
#define CURRENT_GPIO_Port GPIOA
#define AMBIENT_TEMP_Pin GPIO_PIN_3
#define AMBIENT_TEMP_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_5
#define BUZZER_GPIO_Port GPIOA
#define HEATER_Pin GPIO_PIN_7
#define HEATER_GPIO_Port GPIOA
#define BUSVOLTAGE_Pin GPIO_PIN_0
#define BUSVOLTAGE_GPIO_Port GPIOB
#define ENC_BUTTON_INP_Pin GPIO_PIN_9
#define ENC_BUTTON_INP_GPIO_Port GPIOA
#define HANDLE_INP_Pin GPIO_PIN_10
#define HANDLE_INP_GPIO_Port GPIOA
#define STAND_INP_Pin GPIO_PIN_11
#define STAND_INP_GPIO_Port GPIOA
#define SPI_DC_Pin GPIO_PIN_12
#define SPI_DC_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define SPI_RST_Pin GPIO_PIN_4
#define SPI_RST_GPIO_Port GPIOB
#define SPI_CS_Pin GPIO_PIN_6
#define SPI_CS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
