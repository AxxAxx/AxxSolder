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
#include <stdbool.h>

/* states for runtime switch */
typedef enum {
    RUN,
	STANDBY,
	SLEEP,
	EMERGENCY_SLEEP,
	HALTED
} mainstates;

/* Handles */
enum handles {
	NT115,
	T210,
	T245,
	//////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	No_name
	//////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
};
// extern declaration (only a declaration)
extern enum handles attached_handle;
// Definition of enum for power sources
typedef enum {
	POWER_DC,
	POWER_USB,
	POWER_BAT // Future feature
} power_source_t;
// Declaration of external variable
extern power_source_t power_source;

/* Struct to hold flash_data values */
typedef struct{
	float startup_temperature;
	float temperature_offset;
	float standby_temp;
	float standby_time;
	float emergency_time;
	float buzzer_enabled;
	float preset_temp_1;
	float preset_temp_2;
	float GPIO4_ON_at_run;
	float screen_rotation;
	float power_limit;
	float current_measurement;
	float startup_beep;
	float deg_celsius;
	float temp_cal_100;
	float temp_cal_200;
	float temp_cal_300;
	float temp_cal_350;
	float temp_cal_400;
	float temp_cal_450;
	float serial_debug_print;
	float displayed_temp_filter;
	float startup_temp_is_previous_temp;
	float three_button_mode;
	float beep_at_set_temp;
	float beep_tone;
	float momentary_stand;
	float language;
}Flash_values;
/* Global variables defined in main.c */
extern Flash_values flash_values;
extern Flash_values default_flash_values;

// Sensor values structure
typedef struct {
    float set_temperature;
    float thermocouple_temperature;
    float thermocouple_temperature_previous;
    float thermocouple_temperature_filtered;
    float requested_power;
    float requested_power_filtered;
    float bus_voltage;
    float heater_current;
    uint16_t leak_current;
    float mcu_temperature;
    float in_stand;
    float handle1_sense;
    float handle2_sense;
    mainstates current_state;
    mainstates previous_state;
    float max_power_watt;
    float USB_PD_power_limit;
} sensor_values_struct;
/* Global variable, defined in main.c */
extern sensor_values_struct sensor_values;

// Enum definition
typedef enum {
	ATTACHED,
	DETACHED
} cartridge_state_t;
// Declaration of external variables
extern cartridge_state_t cartridge_state;
extern cartridge_state_t previous_cartridge_state;

extern uint8_t sleep_state_written_to_LCD;
extern uint8_t standby_state_written_to_LCD;

extern uint8_t fw_version_major;
extern uint8_t fw_version_minor;
extern uint8_t fw_version_patch;

/* Flag to indicate that settings menu is active */
extern uint8_t settings_menu_active;

#define MAX_POWER 		150

/* Min and Max selectable values */
#define MIN_SELECTABLE_TEMPERATURE 20
#define MAX_SELECTABLE_TEMPERATURE 450

/* General PID parameters */
#define PID_MAX_OUTPUT 500
#define FIXED_MEASURE_DUTY (PID_MAX_OUTPUT / 2)  // i.e. 250 / 2 = 250
#define PID_UPDATE_INTERVAL 25
#define PID_ADD_I_MIN_ERROR 75
extern float PID_NEG_ERROR_I_MULT;
extern float PID_NEG_ERROR_I_BIAS;

extern char DISPLAY_buffer[40];

/* Flag to indicate if a popup is shown */
extern uint8_t popup_shown;

extern  bool flag_show_popup;

/* Timing constants */
extern uint32_t previous_millis_display;
extern uint32_t interval_display;

extern uint32_t previous_millis_debug;
extern uint32_t interval_debug;

extern uint32_t previous_millis_heating_halted_update;
extern uint32_t interval_heating_halted_update;

extern uint32_t previous_millis_left_stand;

extern uint32_t previous_millis_standby;

extern uint32_t previous_measure_current_update;
extern uint32_t interval_measure_current;

extern uint32_t previous_sensor_update_high_update;
extern uint32_t interval_sensor_update_high_update;

extern uint32_t previous_sensor_update_low_update;
extern uint32_t interval_sensor_update_low_update;

extern uint32_t previous_millis_popup;
extern uint32_t interval_popup;

/* Thermocouple temperature */
extern float TC_temp;

/* RAW ADC from current measurement */
extern uint16_t current_raw;

extern uint16_t current_leak;

extern bool Flag_ALARM;

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

/* Return the temperature in the correct unit */
float convert_temperature(float temperature);

void handle_button_status(void);
/* Disable the duty cycle of timer controlling the heater PWM */
void heater_off();

/* Function to change the main device state */
void change_state(mainstates new_state);


/**
 * @brief Gets the hardware version of the device from the state of 3 input pins.
 * The states of VERSION_BIT_1..3 form a 3-bit number:
 *     VERSION_BIT_3 - most significant bit (MSB)
 *     VERSION_BIT_2 - middle bit
 *     VERSION_BIT_1 - least significant bit (LSB)
 *
 * Version numbering starts from 3, i.e. version = 3 + [bit state]
 *
 * @return uint8_t Hardware version number (3..10)
 */
uint8_t get_hw_version(void);

/* Function to clamp d between the limits min and max */
float clamp(float d, float min, float max);

/* Function to convert RGB to BRG */
uint16_t RGB_to_BRG(uint16_t color);

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
