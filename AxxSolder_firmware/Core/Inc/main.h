/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* Struct to hold flash_data values */
typedef struct{
	double startup_temperature;
	double temperature_offset;
	double standby_temp;
	double standby_time;
	double emergency_time;
	double buzzer_enable;
}Flash_values;
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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
#define BUZZER_Pin GPIO_PIN_13
#define BUZZER_GPIO_Port GPIOC
#define USR_1_Pin GPIO_PIN_0
#define USR_1_GPIO_Port GPIOA
#define CURRENT_Pin GPIO_PIN_1
#define CURRENT_GPIO_Port GPIOA
#define TC_Pin GPIO_PIN_2
#define TC_GPIO_Port GPIOA
#define SPI1_DC_Pin GPIO_PIN_3
#define SPI1_DC_GPIO_Port GPIOA
#define SPI1_RST_Pin GPIO_PIN_4
#define SPI1_RST_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_6
#define SPI1_CS_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_0
#define SD_CS_GPIO_Port GPIOB
#define VBIT_0_Pin GPIO_PIN_1
#define VBIT_0_GPIO_Port GPIOB
#define VBIT_1_Pin GPIO_PIN_2
#define VBIT_1_GPIO_Port GPIOB
#define VBIT_2_Pin GPIO_PIN_10
#define VBIT_2_GPIO_Port GPIOB
#define VBIT_3_Pin GPIO_PIN_11
#define VBIT_3_GPIO_Port GPIOB
#define STAND_INP_Pin GPIO_PIN_12
#define STAND_INP_GPIO_Port GPIOB
#define V_INP_Pin GPIO_PIN_13
#define V_INP_GPIO_Port GPIOB
#define HANDLE_INP_2_Pin GPIO_PIN_14
#define HANDLE_INP_2_GPIO_Port GPIOB
#define HANDLE_INP_1_Pin GPIO_PIN_15
#define HANDLE_INP_1_GPIO_Port GPIOB
#define USR_2_Pin GPIO_PIN_8
#define USR_2_GPIO_Port GPIOA
#define ENC_B_Pin GPIO_PIN_15
#define ENC_B_GPIO_Port GPIOA
#define ENC_A_Pin GPIO_PIN_3
#define ENC_A_GPIO_Port GPIOB
#define SW_1_Pin GPIO_PIN_4
#define SW_1_GPIO_Port GPIOB
#define SW_2_Pin GPIO_PIN_5
#define SW_2_GPIO_Port GPIOB
#define SW_3_Pin GPIO_PIN_6
#define SW_3_GPIO_Port GPIOB
#define HEATER_Pin GPIO_PIN_7
#define HEATER_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
