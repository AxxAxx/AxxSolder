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
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* Struct to hold flash_data values */
typedef struct{
	double startup_temperature;
	double temperature_offset;
	double standby_temp;
	double standby_time;
	double emergency_time;
	double buzzer_enabled;
	double preset_temp_1;
	double preset_temp_2;
	double GPIO4_ON_at_run;
	double screen_rotation;
	double power_limit;
	double current_measurement;
	double startup_beep;
	double deg_celsius;
	double temp_cal_100;
	double temp_cal_200;
	double temp_cal_300;
	double temp_cal_350;
	double temp_cal_400;
	double temp_cal_450;
	double serial_debug_print;
	double displayed_temp_filter;
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
#define VERSION_BIT_1_Pin GPIO_PIN_13
#define VERSION_BIT_1_GPIO_Port GPIOC
#define VERSION_BIT_2_Pin GPIO_PIN_14
#define VERSION_BIT_2_GPIO_Port GPIOC
#define VERSION_BIT_3_Pin GPIO_PIN_15
#define VERSION_BIT_3_GPIO_Port GPIOC
#define HEATER_Pin GPIO_PIN_0
#define HEATER_GPIO_Port GPIOF
#define CURRENT_Pin GPIO_PIN_1
#define CURRENT_GPIO_Port GPIOF
#define ENC_A_Pin GPIO_PIN_0
#define ENC_A_GPIO_Port GPIOA
#define I_LEAK_Pin GPIO_PIN_1
#define I_LEAK_GPIO_Port GPIOA
#define VBUS_Pin GPIO_PIN_2
#define VBUS_GPIO_Port GPIOA
#define THERMOCOUPLE_Pin GPIO_PIN_3
#define THERMOCOUPLE_GPIO_Port GPIOA
#define HANDLE_INP_1_Pin GPIO_PIN_4
#define HANDLE_INP_1_GPIO_Port GPIOA
#define HANDLE_INP_2_Pin GPIO_PIN_5
#define HANDLE_INP_2_GPIO_Port GPIOA
#define STAND_INP_Pin GPIO_PIN_6
#define STAND_INP_GPIO_Port GPIOA
#define SW_2_Pin GPIO_PIN_7
#define SW_2_GPIO_Port GPIOA
#define SW_2_EXTI_IRQn EXTI9_5_IRQn
#define USR_2_Pin GPIO_PIN_0
#define USR_2_GPIO_Port GPIOB
#define USR_3_Pin GPIO_PIN_1
#define USR_3_GPIO_Port GPIOB
#define USR_4_Pin GPIO_PIN_2
#define USR_4_GPIO_Port GPIOB
#define SPI2_SD_CS_Pin GPIO_PIN_10
#define SPI2_SD_CS_GPIO_Port GPIOB
#define SPI2_DC_Pin GPIO_PIN_11
#define SPI2_DC_GPIO_Port GPIOB
#define SPI2_RST_Pin GPIO_PIN_12
#define SPI2_RST_GPIO_Port GPIOB
#define SPI2_CS_Pin GPIO_PIN_14
#define SPI2_CS_GPIO_Port GPIOB
#define USR_1_Pin GPIO_PIN_8
#define USR_1_GPIO_Port GPIOA
#define ENC_B_Pin GPIO_PIN_3
#define ENC_B_GPIO_Port GPIOB
#define SW_1_Pin GPIO_PIN_5
#define SW_1_GPIO_Port GPIOB
#define SW_1_EXTI_IRQn EXTI9_5_IRQn
#define BUZZER_Pin GPIO_PIN_7
#define BUZZER_GPIO_Port GPIOB
#define SW_3_Pin GPIO_PIN_8
#define SW_3_GPIO_Port GPIOB
#define SW_3_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
