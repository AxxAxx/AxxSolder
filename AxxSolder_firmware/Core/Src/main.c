/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @author  		: Axel Johansson, Stockholm, Sweden
  * @brief          : Main program body of AxxSodler
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "string.h"
#include "pid.h"
#include "moving_average.h"
#include "hysteresis.h"
#include "type_packers.h"
#include "flash.h"
#include "stusb4500.h"
#include "buzzer.h"
#include "debug.h"
#include <math.h>
#include <stdio.h>
#include <float.h>
//#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t fw_version_major =  3;
uint8_t fw_version_minor =  2;
uint8_t fw_version_patch =  5;

//#define PID_TUNING
DEBUG_VERBOSITY_t debugLevel = DEBUG_INFO;

/* Cartridge type specific PID parameters */
#define KP_NT115 					5
#define KI_NT115 					2
#define KD_NT115 					0.3
#define MAX_I_NT115 				300

#define KP_T210 					7
#define KI_T210 					4
#define KD_T210 					0.3
#define MAX_I_T210 					300

#define KP_T245 					8
#define KI_T245 					2
#define KD_T245 					0.5
#define MAX_I_T245 					300

/* General PID parameters */
#define PID_MAX_OUTPUT 500
#define PID_UPDATE_INTERVAL 25
#define PID_ADD_I_MIN_ERROR 75
float PID_NEG_ERROR_I_MULT = 7;
float PID_NEG_ERROR_I_BIAS = 1;

/* Timing constants */
uint32_t previous_millis_display = 0;
uint32_t interval_display = 40;

uint32_t previous_millis_debug = 0;
uint32_t interval_debug = 50;

uint32_t previous_millis_heating_halted_update = 0;
uint32_t interval_heating_halted_update = 500;

uint32_t previous_millis_left_stand = 0;

uint32_t previous_millis_standby = 0;

uint32_t previous_measure_current_update = 0;
uint32_t interval_measure_current = 250;

uint32_t previous_sensor_update_high_update = 0;
uint32_t interval_sensor_update_high_update = 10;

uint32_t previous_sensor_update_low_update = 0;
uint32_t interval_sensor_update_low_update = 100;

/* Handles */
enum handles {
	NT115,
	T210,
	T245
} attached_handle;

/* power sources */
typedef enum {
	POWER_DC,
	POWER_USB,
	POWER_BAT //Future feature
}power_source_t;
power_source_t power_source = POWER_DC;

/* states for runtime switch */
typedef enum {
    RUN,
	STANDBY,
	SLEEP,
	EMERGENCY_SLEEP,
	HALTED
} mainstates;

/* states for cartridge */
typedef enum {
	ATTACHED,
	DETACHED
} cartridge_state_t;
cartridge_state_t cartridge_state = ATTACHED;
cartridge_state_t previous_cartridge_state = ATTACHED;

uint8_t sleep_state_written_to_LCD = 0;
uint8_t standby_state_written_to_LCD = 0;

/* Custom tuning parameters */
float Kp_tuning = 0;
float Ki_tuning = 0;
float Kd_tuning = 0;
float temperature_tuning = 100;
float PID_MAX_I_LIMIT_tuning = 0;

/* Allow use of custom temperatue, used for tuning */
uint8_t custom_temperature_on = 0;

/* Buffer for UART print */
char UART_buffer[40];

/* Buffer for UART print */
char DISPLAY_buffer[40];

/* ADC Buffer */
#define ADC1_BUF_LEN 57 //3*19
uint16_t ADC1_BUF[ADC1_BUF_LEN];

/* RAW ADC from current measurement */
uint16_t current_raw = 0;

/* Thermocouple temperature */
float TC_temp = 0;

/* Flag to indicate that startup sequence is done */
uint8_t startup_done = 0;

/* Variables for thermocouple outlier detection */
float TC_temp_from_ADC = 0;
float TC_temp_from_ADC_previous = 0;
float TC_temp_from_ADC_diff = 0;
uint16_t TC_outliers_detected = 0;
#define TC_OUTLIERS_THRESHOLD 300

/* Max allowed tip temperature */
#define EMERGENCY_SHUTDOWN_TEMPERATURE 490

/* Constants for scaling input voltage ADC value*/
#define VOLTAGE_COMPENSATION 0.00840442388
#define CURRENT_COMPENSATION 0.002864

/* Constants for scaling power in W to correct duty cycle */
#define POWER_CONVERSION_FACTOR 0.123

/* Min allowed bus voltage and power */
#define MIN_BUSVOLTAGE 8.0
#define MIN_BUSPOWER 8.0
#define USB_PD_POWER_REDUCTION_FACTOR 1.0

/* Max allowed power per handle type */
#define NT115_MAX_POWER 22
#define T210_MAX_POWER 	65
#define T245_MAX_POWER 	130
#define MAX_POWER 		150

/* Min and Max selectable values */
#define MIN_SELECTABLE_TEMPERATURE 20
#define MAX_SELECTABLE_TEMPERATURE 450

/* TC Compensation constants */
#define TC_COMPENSATION_X2_NT115 (5.1026665462522864e-05)
#define TC_COMPENSATION_X1_NT115 0.42050803230712813
#define TC_COMPENSATION_X0_NT115 20.14538589052425

#define TC_COMPENSATION_X2_T210 (4.223931712905644e-06)
#define TC_COMPENSATION_X1_T210 0.31863796444354214
#define TC_COMPENSATION_X0_T210 20.968033870812942

#define TC_COMPENSATION_X2_T245 (-4.735112838956741e-07)
#define TC_COMPENSATION_X1_T245 0.11936452029674384
#define TC_COMPENSATION_X0_T245 23.777399955382318

/* Constants for internal MCU temperture */
#define V30 		0.76 			// from datasheet
#define VSENSE 		(3.3/4096.0) 	// VSENSE value
#define Avg_Slope 	0.0025 			// 2.5mV from datasheet

/* Largest delta temperature before detecting a faulty or missing cartridge */
#define MAX_TC_DELTA_FAULTDETECTION 25

/* Define time for long button press */
#define BTN_LONG_PRESS 15 //*50ms (htim16 interval) --> 15 = 750ms

/* Button flags */
volatile uint8_t SW_ready = 1;
volatile uint8_t SW_1_pressed = 0;
volatile uint8_t SW_2_pressed = 0;
volatile uint8_t SW_3_pressed = 0;
volatile uint8_t SW_1_pressed_long = 0;
volatile uint8_t SW_2_pressed_long = 0;
volatile uint8_t SW_3_pressed_long = 0;

/* UART send buffers */
#define MAX_BUFFER_LEN 250
uint8_t UART_transmit_buffer[MAX_BUFFER_LEN];
uint8_t UART_packet_index = 0;
uint8_t UART_packet_length = 0;

/* Struct to hold sensor values */
struct sensor_values_struct {
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
};

/* Struct to hold sensor values */
struct sensor_values_struct sensor_values  = {.set_temperature = 0.0,
        									.thermocouple_temperature = 0.0,
        									.thermocouple_temperature_previous = 0.0,
											.thermocouple_temperature_filtered = 0,
											.requested_power = 0.0,
											.requested_power_filtered = 0.0,
											.bus_voltage = 0.0,
											.heater_current = 0,
											.leak_current = 0,
											.mcu_temperature = 0.0,
											.in_stand = 0.0,
											.handle1_sense  = 0.0,
											.handle2_sense  = 0.0,
											.current_state = SLEEP,
											.previous_state = SLEEP,
											.max_power_watt = 0,
											.USB_PD_power_limit = DBL_MAX};

/* Struct to hold flash values */
Flash_values flash_values;
Flash_values default_flash_values = {.startup_temperature = 330,
											.temperature_offset = 0,
											.standby_temp = 150,
											.standby_time = 10,
											.emergency_time = 30,
											.buzzer_enabled = 1,
											.preset_temp_1 = 330,
											.preset_temp_2 = 430,
											.GPIO4_ON_at_run = 0,
											.screen_rotation = 2,
											.power_limit = 0,
											.current_measurement = 1,
											.startup_beep = 1,
											.deg_celsius = 1,
											.temp_cal_100 = 100,
											.temp_cal_200 = 200,
											.temp_cal_300 = 300,
											.temp_cal_350 = 350,
											.temp_cal_400 = 400,
											.temp_cal_450 = 450,
											.serial_debug_print = 0,
											.displayed_temp_filter = 5,
											.startup_temp_is_previous_temp = 0,
											.three_button_mode = 0};

/* List of names for settings menu */
#define menu_length 27
char menu_names[menu_length][30] = { "Startup Temp °C    ",
							"Temp Offset °C      ",
							"Standby Temp °C   ",
							"Standby Time [min]  ",
							"Sleep Time [min]    ",
							"Buzzer Enabled      ",
							"Preset Temp 1 °C   ",
							"Preset Temp 2 °C   ",
							"GPIO4 ON at run    ",
							"Screen Rotation     ",
							"Limit Power [W]     ",
							"I Measurement       ",
							"Startup Beep         ",
							"Temp in Celsius     ",
							"Temp cal 100         ",
							"Temp cal 200         ",
							"Temp cal 300         ",
							"Temp cal 350         ",
							"Temp cal 400         ",
							"Temp cal 450         ",
							"Serial DEBUG        ",
							"Disp Temp. filter    ",
							"Start at prev. temp ",
							"3-button mode        ",
							"-Load Default-       ",
							"-Save and Reboot- ",
							"-Exit no Save-        "};

/* PID data */
float PID_setpoint = 0.0;

/* Flags for temp and current measurements */
uint8_t current_measurement_requested = 0;
uint8_t current_measurement_done = 1;
uint8_t thermocouple_measurement_requested = 0;
uint8_t thermocouple_measurement_done = 1;

/* Moving average filters for sensor data */
FilterTypeDef thermocouple_temperature_filter_struct;
FilterTypeDef thermocouple_temperature_filtered_filter_struct;
FilterTypeDef requested_power_filtered_filter_struct;
FilterTypeDef mcu_temperature_filter_struct;
FilterTypeDef input_voltage_filterStruct;
FilterTypeDef current_filterStruct;
FilterTypeDef stand_sense_filterStruct;
FilterTypeDef handle1_sense_filterStruct;
FilterTypeDef handle2_sense_filterStruct;

/* Hysteresis filters for sensor data */
Hysteresis_FilterTypeDef thermocouple_temperature_filtered_hysteresis;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_SPI2_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM17_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
PID_TypeDef TPID;

/* Function to clamp d between the limits min and max */
float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}

/* Function to get min value of a and b */
float min(float a, float b) {
  return a < b ? a : b;
}

/* Returns the average of 100 readings of the index+3*n value in the ADC_buffer vector */
float get_mean_ADC_reading_indexed(uint8_t index){
	float ADC_filter_mean = 0;
	for(int n=index;n<ADC1_BUF_LEN;n=n+3){
		ADC_filter_mean += ADC1_BUF[n];
	}
	return ADC_filter_mean/(ADC1_BUF_LEN/3.0);
}

/* Function to get the hardware version based on version bit pins */
uint8_t get_hw_version(){
	return 3 + (HAL_GPIO_ReadPin(GPIOC, VERSION_BIT_3_Pin) << 2) + (HAL_GPIO_ReadPin(GPIOC, VERSION_BIT_2_Pin) << 1) + (HAL_GPIO_ReadPin(GPIOC, VERSION_BIT_1_Pin)); //version counting starts at version 3
}

/* Convert RGB colour to BRG color */
uint16_t RGB_to_BRG(uint16_t color){
	return ((((color & 0b0000000000011111)  << 11) & 0b1111100000000000) | ((color & 0b1111111111100000) >> 5));
}

/* Function to change the main state */
void change_state(mainstates new_state){
	sensor_values.previous_state = sensor_values.current_state;
	sensor_values.current_state = new_state;
	// If transitioning to RUN and temperature should be saved for use as start-up temp
	if((sensor_values.previous_state != RUN) && (sensor_values.current_state == RUN) && (flash_values.startup_temp_is_previous_temp == 1)){
		flash_values.startup_temperature = sensor_values.set_temperature;
		FlashWrite(&flash_values);
	}
	if((sensor_values.current_state == RUN) && (flash_values.GPIO4_ON_at_run == 1)){
		HAL_GPIO_WritePin(GPIOB, USR_4_Pin, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIOB, USR_4_Pin, GPIO_PIN_RESET);
	}
}

/* Function to get the filtered MCU temperature */
void get_mcu_temp(){
	sensor_values.mcu_temperature =	Moving_Average_Compute((((get_mean_ADC_reading_indexed(2) * VSENSE) - V30) / Avg_Slope + 25), &mcu_temperature_filter_struct); //Index 2 is MCU temp
}

/* Function to get the filtered bus voltage */
void get_bus_voltage(){
	sensor_values.bus_voltage = Moving_Average_Compute(get_mean_ADC_reading_indexed(0), &input_voltage_filterStruct)*VOLTAGE_COMPENSATION; //Index 0 is bus voltage
}

/* Function to get the filtered heater current */
void get_heater_current(){
	sensor_values.heater_current = Moving_Average_Compute(current_raw, &current_filterStruct)*CURRENT_COMPENSATION;
}

void get_thermocouple_temperature(){
	/* Thermocouple outlier detection and filter */
	TC_temp_from_ADC_previous = TC_temp_from_ADC;
	TC_temp_from_ADC = get_mean_ADC_reading_indexed(1);
	TC_temp_from_ADC_diff = fabs(TC_temp_from_ADC_previous - TC_temp_from_ADC);
	if((TC_temp_from_ADC_diff > TC_OUTLIERS_THRESHOLD) && (TC_outliers_detected < 2)){
		TC_outliers_detected++;
		if(TC_outliers_detected < 2){
			TC_temp_from_ADC = TC_temp_from_ADC_previous;
		}
	}
	else{
		TC_outliers_detected = 0;
	}

	/* Compute moving average of Thermocouple measurements */
	TC_temp = Moving_Average_Compute(TC_temp_from_ADC, &thermocouple_temperature_filter_struct); /* Moving average */

	if(attached_handle == T210){
		sensor_values.thermocouple_temperature = TC_temp*TC_temp*TC_COMPENSATION_X2_T210 + TC_temp*TC_COMPENSATION_X1_T210 + TC_COMPENSATION_X0_T210;
	}
	else if(attached_handle == T245){
		sensor_values.thermocouple_temperature = TC_temp*TC_temp*TC_COMPENSATION_X2_T245 + TC_temp*TC_COMPENSATION_X1_T245 + TC_COMPENSATION_X0_T245;
	}
	else if(attached_handle == NT115){
		sensor_values.thermocouple_temperature = TC_temp*TC_temp*TC_COMPENSATION_X2_NT115 + TC_temp*TC_COMPENSATION_X1_NT115 + TC_COMPENSATION_X0_NT115;
	}

	/* Adjust measured temperature to fit calibrated values */
	if(sensor_values.thermocouple_temperature < 100){
		sensor_values.thermocouple_temperature = sensor_values.thermocouple_temperature*(flash_values.temp_cal_100)/100.0f;
		}
	else if(sensor_values.thermocouple_temperature < 200){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 100.0f)*(flash_values.temp_cal_200-flash_values.temp_cal_100)/100.0f + flash_values.temp_cal_100;
		}
	else if(sensor_values.thermocouple_temperature < 300){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 200.0f)*(flash_values.temp_cal_300-flash_values.temp_cal_200)/100.0f + flash_values.temp_cal_200;
	}
	else if(sensor_values.thermocouple_temperature < 350){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 300.0f)*(flash_values.temp_cal_350-flash_values.temp_cal_300)/50.0f + flash_values.temp_cal_300;
		}
	else if(sensor_values.thermocouple_temperature < 400){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 350.0f)*(flash_values.temp_cal_400-flash_values.temp_cal_350)/50.0f + flash_values.temp_cal_350;
		}
	else{
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 400.0f)*(flash_values.temp_cal_450-flash_values.temp_cal_400)/50.0f + flash_values.temp_cal_400;
		}

	sensor_values.thermocouple_temperature += flash_values.temperature_offset; // Add temperature offset value
	sensor_values.thermocouple_temperature = clamp(sensor_values.thermocouple_temperature ,0 ,500); // Clamp

	sensor_values.thermocouple_temperature_filtered = Moving_Average_Compute(sensor_values.thermocouple_temperature, &thermocouple_temperature_filtered_filter_struct); 	/* Moving average filter */
	sensor_values.thermocouple_temperature_filtered = Hysteresis_Add(sensor_values.thermocouple_temperature_filtered, &thermocouple_temperature_filtered_hysteresis);		/* Hysteresis filter */
}

/* Sets the duty cycle of timer controlling the heater */
void set_heater_duty(uint16_t dutycycle){
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, dutycycle);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dutycycle*0.3);
}

/* Update the duty cycle of timer controlling the heater PWM */
void update_heater_PWM(){
	float duty_cycle = sensor_values.requested_power*(sensor_values.max_power_watt*POWER_CONVERSION_FACTOR/sensor_values.bus_voltage);
	set_heater_duty(clamp(duty_cycle, 0.0, PID_MAX_OUTPUT));
}

/* Disable the duty cycle of timer controlling the heater PWM*/
void heater_off(){
	set_heater_duty(0);
}

/* Return the temperature in the correct unit */
float convert_temperature(float temperature){
	if (flash_values.deg_celsius == 1){
		return temperature;
	}
	else{
		return ((temperature * 9) + 3) / 5 + 32;
	}
}

/* Function to left align a string from float */
void left_align_float(char* str, float number, int8_t len)
	{
		char tempstring[len];
		memset(&tempstring, '\0', len);
		sprintf(tempstring, "%.0f", number);

		if(number < -99.5){
			;
		}
		else if(number < -9.5){
			tempstring[3] = 32;
			tempstring[4] = 32;
		}
		else if(number < 0){
			tempstring[2] = 32;
			tempstring[3] = 32;
			tempstring[4] = 32;
			tempstring[5] = 32;
		}
		else if(number < 9.5){
			tempstring[1] = 32;
			tempstring[2] = 32;
			tempstring[3] = 32;
			tempstring[4] = 32;
			tempstring[5] = 32;
			tempstring[6] = 32;
		}
		else if(number < 99.5){
			tempstring[2] = 32;
			tempstring[3] = 32;
			tempstring[4] = 32;
			tempstring[5] = 32;
		}
		else if(number < 999.5){
			tempstring[3] = 32;
			tempstring[4] = 32;
		}
		strcpy(str, tempstring);
	}

void settings_menu(){
	/* If SW_1 is pressed during startup - Show SETTINGS and allow to release button. */
	if (HAL_GPIO_ReadPin (GPIOB, SW_1_Pin) == 1){

		char str[32];
		memset(&str, '\0', strlen(str));
		if((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)){
			sprintf(str, "fw: %d.%d.%d     hw: %d", fw_version_major,fw_version_minor, fw_version_patch, get_hw_version());
			LCD_PutStr(6, 300, str, FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_BLACK));
		}
		else{
			sprintf(str, "fw: %d.%d.%d     hw: %d", fw_version_major,fw_version_minor, fw_version_patch, get_hw_version());
			LCD_PutStr(6, 215, str, FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_BLACK));
		}

		TIM2->CNT = 1000;
		uint16_t menu_cursor_position = 0;
		uint16_t old_menu_cursor_position = 0;
		uint16_t menu_start = 0;
		uint16_t menu_level = 0;
		uint16_t menu_active = 1;
		float old_value = 0;

		LCD_PutStr(70, 15, "SETTINGS", FONT_arial_20X23, RGB_to_BRG(C_YELLOW), RGB_to_BRG(C_BLACK));
		LCD_DrawLine(0,40,240,40,RGB_to_BRG(C_YELLOW));
		LCD_DrawLine(0,41,240,41,RGB_to_BRG(C_YELLOW));
		LCD_DrawLine(0,42,240,42,RGB_to_BRG(C_YELLOW));

		while(HAL_GPIO_ReadPin (GPIOB, SW_1_Pin) == 1){} // Wait until user releases button

		HAL_Delay(500);
		while(menu_active == 1){
			if(menu_level == 0){
				TIM2->CNT = clamp(TIM2->CNT, 1000, 1000000);
				menu_cursor_position = (TIM2->CNT - 1000) / 2;
			}
			if (menu_level == 1){
				if (menu_cursor_position == 10){
					((float*)&flash_values)[menu_cursor_position] = (float)old_value + round(((float)(TIM2->CNT - 1000.0) / 2.0 - (float)menu_cursor_position)) * 5;
				}
				else{
					((float*)&flash_values)[menu_cursor_position] = (float)old_value + (float)(TIM2->CNT - 1000.0) / 2.0 - (float)menu_cursor_position;
				}

				if ((menu_cursor_position == 5) || (menu_cursor_position == 8) || (menu_cursor_position == 11) || (menu_cursor_position == 12) || (menu_cursor_position == 13) || (menu_cursor_position == 20) || (menu_cursor_position == 22) || (menu_cursor_position == 23)){
					((float*)&flash_values)[menu_cursor_position] = fmod(round(fmod(fabs(((float*)&flash_values)[menu_cursor_position]), 2)), 2);
				}
				else if (menu_cursor_position == 9){
					((float*)&flash_values)[menu_cursor_position] = fmod(round(fmod(fabs(((float*)&flash_values)[menu_cursor_position]), 4)), 4);
				}
				else if (menu_cursor_position == 21){
					((float*)&flash_values)[menu_cursor_position] = 1 + fmod(round(fmod(fabs(((float*)&flash_values)[menu_cursor_position]), 10)), 10);
				}
				else if (menu_cursor_position == 1){
					((float*)&flash_values)[menu_cursor_position] = round(((float*)&flash_values)[menu_cursor_position]);
				}
				else if ((menu_cursor_position == 0)  || (menu_cursor_position == 1) || (menu_cursor_position == 2) || (menu_cursor_position == 6) || (menu_cursor_position == 7)){
					((float*)&flash_values)[menu_cursor_position] = fmod(round(fmod(fabs(((float*)&flash_values)[menu_cursor_position]), MAX_SELECTABLE_TEMPERATURE + 1)), MAX_SELECTABLE_TEMPERATURE + 1);
				}
				else if (menu_cursor_position == 10){
					((float*)&flash_values)[menu_cursor_position] = fmod(round(fmod(fabs(((float*)&flash_values)[menu_cursor_position]), MAX_POWER + 5)), MAX_POWER + 5);
				}
				else {
					((float*)&flash_values)[menu_cursor_position] = fabs(((float*)&flash_values)[menu_cursor_position]);
				}
			}

			if(menu_cursor_position > menu_length-1){
							menu_cursor_position = menu_length-1;
							TIM2->CNT = 1000 + (menu_length-1)*2;
			}

			if(menu_cursor_position >= 6){
				menu_start = menu_cursor_position-6;
			}
			else{
				menu_start = 0;
			}

			if((HAL_GPIO_ReadPin (GPIOB, SW_1_Pin) == 1) && (menu_cursor_position < menu_length-3)){
				if(menu_level == 0){
					old_value = ((float*)&flash_values)[menu_cursor_position];
					old_menu_cursor_position = menu_cursor_position;
				}
				if(menu_level == 1){
					TIM2->CNT = old_menu_cursor_position*2 + 1000;
				}

				menu_level = abs(menu_level-1);
				HAL_Delay(200);
			}
			else if((HAL_GPIO_ReadPin (GPIOB, SW_1_Pin) == 1) && (menu_cursor_position == menu_length-1)){
				menu_active = 0;
			}
			else if((HAL_GPIO_ReadPin (GPIOB, SW_1_Pin) == 1) && (menu_cursor_position == menu_length-2)){
				menu_active = 0;
				FlashWrite(&flash_values);
				HAL_NVIC_SystemReset();
			}
			else if((HAL_GPIO_ReadPin (GPIOB, SW_1_Pin) == 1) && (menu_cursor_position == menu_length-3)){
				flash_values = default_flash_values;
			}

			for(int i = menu_start;i<=menu_start+6;i++){

				if((i == menu_cursor_position) && (menu_level == 0)){
					LCD_PutStr(5, 45+(i-menu_start)*25, menu_names[i], FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_WHITE));
				}
				else{
					LCD_PutStr(5, 45+(i-menu_start)*25, menu_names[i], FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
				}

				char string[10];
				memset(&string, '\0', 10);
				if(i < menu_length-3){
					if((i == menu_cursor_position) && (menu_level == 1)){
						left_align_float(string, (((float*)&flash_values)[i]), strlen(string));
						LCD_PutStr(190, 45+(i-menu_start)*25, string, FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_WHITE));
					}
					else{
						left_align_float(string, (((float*)&flash_values)[i]), strlen(string));
						LCD_PutStr(190, 45+(i-menu_start)*25, string, FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
					}
				}
				if(i >= menu_length-3){
					LCD_PutStr(190, 45+(i-menu_start)*25, "        ", FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
				}
			}
		}
	}
}

void update_display(){
	if((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)){
		memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
		sprintf(DISPLAY_buffer, "%.f", convert_temperature(sensor_values.set_temperature));
		if(convert_temperature(sensor_values.set_temperature) < 99.5){
			DISPLAY_buffer[2] = 32;
			DISPLAY_buffer[3] = 32;
		}
		LCD_PutStr(19, 70, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		if(cartridge_state == DETACHED) {
			LCD_PutStr(15, 160, " ---  ", FONT_arial_36X44_NUMBERS, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
			sprintf(DISPLAY_buffer, "%.f", convert_temperature(sensor_values.thermocouple_temperature_filtered));
			if(convert_temperature(sensor_values.thermocouple_temperature_filtered) < 99.5){
				DISPLAY_buffer[2] = 32;
				DISPLAY_buffer[3] = 32;
			}
			LCD_PutStr(19, 160, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}

		memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
		sprintf(DISPLAY_buffer, "%.1f", sensor_values.bus_voltage);
		LCD_PutStr(125, 255, DISPLAY_buffer, FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
		if(convert_temperature(sensor_values.mcu_temperature) < 99.5){
			sprintf(DISPLAY_buffer, "  %.0f", convert_temperature(sensor_values.mcu_temperature));
		}
		else{
			sprintf(DISPLAY_buffer, "%.0f", convert_temperature(sensor_values.mcu_temperature));
		}
		LCD_PutStr(56, 275, DISPLAY_buffer, FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		if(attached_handle == T210){
			LCD_PutStr(125, 235, "T210   ", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else if(attached_handle == T245){
			LCD_PutStr(125, 235, "T245   ", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else if(attached_handle == NT115){
			LCD_PutStr(125, 235, "NT115", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}

		if(sensor_values.max_power_watt < 100){
			sprintf(DISPLAY_buffer, "  %.0f W", sensor_values.max_power_watt);
		}
		else{
			sprintf(DISPLAY_buffer, "%.0f W", sensor_values.max_power_watt);
		}
		LCD_PutStr(185, 45, DISPLAY_buffer, FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		if((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD){
			UG_FillFrame(210,66,230,268, RGB_to_BRG(C_ORANGE));
			LCD_PutStr(214, 73,  "Z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(216, 99, "z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(214, 129, "Z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(216, 158, "z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(214, 191, "Z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(216, 217, "z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(214, 247, "Z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			sleep_state_written_to_LCD = 1;
			standby_state_written_to_LCD = 0;
		}
		else if((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD){
			UG_FillFrame(210,66,230,268, RGB_to_BRG(C_ORANGE));
			LCD_PutStr(213, 73,  "S", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(213, 102,  "T", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(213, 131, "A", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(213, 160, "N", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(213, 189, "D", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(213, 218, "B", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(213, 247, "Y", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			standby_state_written_to_LCD = 1;
			sleep_state_written_to_LCD = 0;
		}
		else if(sensor_values.current_state == RUN){
			UG_FillFrame(210, 268-(sensor_values.requested_power_filtered/PID_MAX_OUTPUT)*202, 	230, 	268,									RGB_to_BRG(C_LIGHT_SKY_BLUE));
			UG_FillFrame(210, 66, 									230, 	268-(sensor_values.requested_power_filtered/PID_MAX_OUTPUT)*202, 	RGB_to_BRG(C_BLACK));
			standby_state_written_to_LCD = 0;
			sleep_state_written_to_LCD = 0;
		}
	}
	else{
		memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
		sprintf(DISPLAY_buffer, "%.f", convert_temperature(sensor_values.set_temperature));
		if(convert_temperature(sensor_values.set_temperature) < 99.5){
			DISPLAY_buffer[2] = 32;
			DISPLAY_buffer[3] = 32;
		}
		LCD_PutStr(64, 35, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		if(cartridge_state == DETACHED) {
			LCD_PutStr(60, 115, " ---  ", FONT_arial_36X44_NUMBERS, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
			sprintf(DISPLAY_buffer, "%.f", convert_temperature(sensor_values.thermocouple_temperature_filtered));
			if(convert_temperature(sensor_values.thermocouple_temperature_filtered) < 99.5){
				DISPLAY_buffer[2] = 32;
				DISPLAY_buffer[3] = 32;
			}
			LCD_PutStr(64, 115, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}

		memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
		sprintf(DISPLAY_buffer, "%.1f", sensor_values.bus_voltage);
		LCD_PutStr(170, 195, DISPLAY_buffer, FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
		if(convert_temperature(sensor_values.mcu_temperature) < 99.5){
			sprintf(DISPLAY_buffer, "  %.0f", convert_temperature(sensor_values.mcu_temperature));
		}
		else{
			sprintf(DISPLAY_buffer, "%.0f", convert_temperature(sensor_values.mcu_temperature));
		}
		LCD_PutStr(102, 215, DISPLAY_buffer, FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		if(attached_handle == T210){
			LCD_PutStr(170, 175, "T210   ", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else if(attached_handle == T245){
			LCD_PutStr(170, 175, "T245   ", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else if(attached_handle == NT115){
			LCD_PutStr(170, 175, "NT115", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}

		if(sensor_values.max_power_watt < 100){
			sprintf(DISPLAY_buffer, "%.0f W  ", sensor_values.max_power_watt);
		}
		else{
			sprintf(DISPLAY_buffer, "%.0f W", sensor_values.max_power_watt);
		}
		LCD_PutStr(2, 10, DISPLAY_buffer, FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

		if((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD){
			UG_FillFrame(10,32,30,209, RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 41,  "Z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(16, 68, "z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 95, "Z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(16, 122, "z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 149, "Z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(16, 176, "z", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			sleep_state_written_to_LCD = 1;
			standby_state_written_to_LCD = 0;
		}
		else if((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD){
			UG_FillFrame(10,32,30,209, RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 41,  "S", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 63,  "T", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 85, "A", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 107, "N", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 129, "D", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 151, "B", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			LCD_PutStr(14, 173, "Y", FONT_arial_20X23, RGB_to_BRG(C_BLACK), RGB_to_BRG(C_ORANGE));
			standby_state_written_to_LCD = 1;
			sleep_state_written_to_LCD = 0;
		}
		else if(sensor_values.current_state == RUN){
			UG_FillFrame(10, 209-(sensor_values.requested_power_filtered/PID_MAX_OUTPUT)*177, 	30, 	209, 									RGB_to_BRG(C_LIGHT_SKY_BLUE));
			UG_FillFrame(10, 32, 									30, 	209-(sensor_values.requested_power_filtered/PID_MAX_OUTPUT)*177, 	RGB_to_BRG(C_BLACK));
			standby_state_written_to_LCD = 0;
			sleep_state_written_to_LCD = 0;
		}
	}
}

void LCD_draw_main_screen(){
	if((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)){
		UG_FillScreen(RGB_to_BRG(C_BLACK));

		LCD_PutStr(53, 8, "AxxSolder", FONT_arial_19X22, RGB_to_BRG(C_YELLOW), RGB_to_BRG(C_BLACK));
		LCD_DrawLine(0,36,240,36,RGB_to_BRG(C_YELLOW));
		LCD_DrawLine(0,37,240,37,RGB_to_BRG(C_YELLOW));
		LCD_DrawLine(0,38,240,38,RGB_to_BRG(C_YELLOW));


		LCD_PutStr(19, 45, "Set temp", FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		UG_DrawCircle(128, 76, 5, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(128, 76, 4, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(128, 76, 3, RGB_to_BRG(C_WHITE));
		if(flash_values.deg_celsius == 1){
			LCD_PutStr(135, 70, "C", FONT_arial_36X44_C, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(135, 70, "F", FONT_arial_36X44_F, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		LCD_PutStr(19, 135, "Actual temp", FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		UG_DrawCircle(128, 166, 5, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(128, 166, 4, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(128, 166, 3, RGB_to_BRG(C_WHITE));
		if(flash_values.deg_celsius == 1){
			LCD_PutStr(135, 160, "C", FONT_arial_36X44_C, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(135, 160, "F", FONT_arial_36X44_F, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		UG_DrawFrame(11, 129, 187, 215, RGB_to_BRG(C_WHITE));
		UG_DrawFrame(10, 128, 188, 216, RGB_to_BRG(C_WHITE));

		LCD_PutStr(11, 235, "Handle type:", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		LCD_PutStr(11, 255, "Input voltage:         V", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		if(flash_values.deg_celsius == 1){
			LCD_PutStr(11, 275, "MCU:      °C", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(11, 275, "MCU:      °F", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		LCD_PutStr(113, 275, "SRC:", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		switch(power_source){
		case POWER_DC:
			LCD_PutStr(160, 275, "DC", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
			break;
		case POWER_USB:
			LCD_PutStr(160, 275, "USB", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
			break;
		case POWER_BAT:
			LCD_PutStr(160, 275, "BAT", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
			break;
		}

		UG_DrawLine(0, 296, 240, 296, RGB_to_BRG(C_DARK_SEA_GREEN));
		UG_DrawLine(0, 297, 240, 297, RGB_to_BRG(C_DARK_SEA_GREEN));

		if(flash_values.three_button_mode == 1){
			LCD_PutStr(11, 301, "TEMP         DOWN   UP", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(11, 301, "PRESETS", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			memset(DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
			sprintf(DISPLAY_buffer, "%.0f", convert_temperature(flash_values.preset_temp_1));
			LCD_PutStr(130, 301, DISPLAY_buffer, FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			memset(DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
			sprintf(DISPLAY_buffer, "%.0f", convert_temperature(flash_values.preset_temp_2));
			LCD_PutStr(190, 301, DISPLAY_buffer, FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
		}

		UG_DrawFrame(208, 64, 232, 270, RGB_to_BRG(C_WHITE));
		UG_DrawFrame(209, 65, 231, 269, RGB_to_BRG(C_WHITE));

		LCD_PutStr(205, 275, "0 W", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
	}
	else{
		UG_FillScreen(RGB_to_BRG(C_BLACK));
		LCD_PutStr(165, 10, "AxxSolder", FONT_arial_20X23, RGB_to_BRG(C_YELLOW), RGB_to_BRG(C_BLACK));

		LCD_PutStr(64, 10, "Set temp", FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		UG_DrawCircle(173, 41, 5, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(173, 41, 4, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(173, 41, 3, RGB_to_BRG(C_WHITE));
		if(flash_values.deg_celsius == 1){
			LCD_PutStr(180, 35, "C", FONT_arial_36X44_C, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(180, 35, "F", FONT_arial_36X44_F, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		LCD_PutStr(64, 90, "Actual temp", FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		UG_DrawCircle(173, 121, 5, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(173, 121, 4, RGB_to_BRG(C_WHITE));
		UG_DrawCircle(173, 121, 3, RGB_to_BRG(C_WHITE));
		if(flash_values.deg_celsius == 1){
			LCD_PutStr(180, 115, "C", FONT_arial_36X44_C, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(180, 115, "F", FONT_arial_36X44_F, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}

		UG_DrawFrame(56, 84, 232,170, RGB_to_BRG(C_WHITE));
		UG_DrawFrame(56, 83, 233, 171, RGB_to_BRG(C_WHITE));

		LCD_PutStr(56, 175, "Handle type:", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		LCD_PutStr(56, 195, "Input voltage:          V", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		if(flash_values.deg_celsius == 1){
			LCD_PutStr(56, 215, "MCU:      °C", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(56, 215, "MCU:       °F", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		}
		LCD_PutStr(160, 215, "SRC:", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
		switch(power_source){
		case POWER_DC:
			LCD_PutStr(210, 215, "DC", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
			break;
		case POWER_USB:
			LCD_PutStr(210, 215, "USB", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
			break;
		case POWER_BAT:
			LCD_PutStr(210, 215, "BAT", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
			break;
		}


		UG_DrawLine(265, 0, 265, 240, RGB_to_BRG(C_DARK_SEA_GREEN));
		UG_DrawLine(266, 0, 266, 240, RGB_to_BRG(C_DARK_SEA_GREEN));
		UG_DrawLine(315, 0, 315, 240, RGB_to_BRG(C_DARK_SEA_GREEN));
		UG_DrawLine(316, 0, 316, 240, RGB_to_BRG(C_DARK_SEA_GREEN));

		if(flash_values.three_button_mode == 1){
			LCD_PutStr(269, 90, "DEC", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			LCD_PutStr(269, 10, "INC", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
		}
		else{
			LCD_PutStr(285, 75, "P", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			LCD_PutStr(285, 97, "R", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			LCD_PutStr(285, 119, "E", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			LCD_PutStr(285, 141, "S", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			LCD_PutStr(285, 163, "E", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			LCD_PutStr(285, 185, "T", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			LCD_PutStr(285, 207, "S", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
			sprintf(DISPLAY_buffer, "%.0f", flash_values.preset_temp_1);
			LCD_PutStr(272, 40, DISPLAY_buffer, FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
			memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
			sprintf(DISPLAY_buffer, "%.0f", flash_values.preset_temp_2);
			LCD_PutStr(272, 10, DISPLAY_buffer, FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
		}



		UG_DrawFrame(8, 30, 32, 212, RGB_to_BRG(C_WHITE));
		UG_DrawFrame(9, 31, 31, 211, RGB_to_BRG(C_WHITE));

		LCD_PutStr(5, 215, "0 W", FONT_arial_17X18, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

	}
}

void show_popup(char *text){
	UG_FillFrame(10, 50, 235, 105, RGB_to_BRG(C_ORANGE));
	UG_FillFrame(15, 55, 230, 100, RGB_to_BRG(C_WHITE));
	LCD_PutStr(20, 70, text, FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_WHITE));
	HAL_Delay(2000);
	LCD_draw_main_screen();
	standby_state_written_to_LCD = 0;
	sleep_state_written_to_LCD = 0;
}

void LCD_draw_earth_fault_popup(){
	heater_off();

	UG_FillFrame(10, 50, 205, 205, RGB_to_BRG(C_ORANGE));
	UG_FillFrame(15, 55, 200, 200, RGB_to_BRG(C_WHITE));
	LCD_PutStr(20, 60, "GROUNDING", FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_WHITE));
	LCD_PutStr(20, 80, "ERROR", FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_WHITE));

	LCD_PutStr(20, 120, "CHECK", FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_WHITE));
	LCD_PutStr(20, 140, "CONNECTIONS", FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_WHITE));
	LCD_PutStr(20, 160, "AND REBOOT", FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_WHITE));

	Error_Handler();
}

/* Get encoder value (Set temp.) and limit */
void get_set_temperature(){
	if(custom_temperature_on == 0){
		TIM2->CNT = clamp(TIM2->CNT, MIN_SELECTABLE_TEMPERATURE, MAX_SELECTABLE_TEMPERATURE);
		if(flash_values.three_button_mode == 1){
			sensor_values.set_temperature = (uint16_t)(TIM2->CNT);
		}
		else{
			sensor_values.set_temperature = (uint16_t)(TIM2->CNT/2) * 2;
		}
	}
}

/* Function to check if the delta temperature is larger than expected */
void handle_delta_temperature(){
	if((startup_done == 1) && ((sensor_values.thermocouple_temperature - sensor_values.thermocouple_temperature_previous) > MAX_TC_DELTA_FAULTDETECTION)){
		heater_off();
		sensor_values.heater_current = 0;
		update_display();
		show_popup("No or Faulty tip!");
		change_state(EMERGENCY_SLEEP);
	}
	sensor_values.thermocouple_temperature_previous = sensor_values.thermocouple_temperature;
}

/* Function to set state to EMERGENCY_SLEEP */
void handle_emergency_shutdown(){
	/* Get time when iron turns on */
	if(sensor_values.previous_state != RUN && sensor_values.current_state == RUN){
		previous_millis_left_stand = HAL_GetTick();
	}
	/* Set state to EMERGENCY_SLEEP if iron ON for longer time than emergency_time */
	if ((sensor_values.in_stand == 0) && (HAL_GetTick() - previous_millis_left_stand >= flash_values.emergency_time*60000) && sensor_values.current_state == RUN){
		change_state(EMERGENCY_SLEEP);
		show_popup("Standby timeout");
	}
	/* Set state to EMERGENCY_SLEEP if input voltage is too low */
	if((sensor_values.bus_voltage <= MIN_BUSVOLTAGE) && (sensor_values.current_state == RUN)){
		change_state(EMERGENCY_SLEEP);
		show_popup("Inp. Voltage too low");
	}
	/* Set state to EMERGENCY_SLEEP if input power is too low */
	if((sensor_values.max_power_watt <= MIN_BUSPOWER) && (sensor_values.current_state == RUN)){
		change_state(EMERGENCY_SLEEP);
		show_popup("Inp. Power too low");
	}
	/* Set state to EMERGENCY_SLEEP if no tip detected (no current draw) */
	else if((sensor_values.heater_current < 1) && (sensor_values.current_state == RUN)){ //NT115 at 9V draws 2.3
		change_state(EMERGENCY_SLEEP);
		show_popup("No tip detected");
	}
	/* Set state to EMERGENCY_SLEEP if iron is over max allowed temp */
	else if((sensor_values.thermocouple_temperature_filtered > EMERGENCY_SHUTDOWN_TEMPERATURE) && (sensor_values.current_state == RUN)){
		change_state(EMERGENCY_SLEEP);
		show_popup("OVERTEMP");
	}
}

/* Function to handle the cartridge presence */
void handle_cartridge_presence(){
	if((sensor_values.heater_current < 1) || (TC_temp > 4096-10)) { //NT115 at 9V draws 2.3
		cartridge_state = DETACHED;
		change_state(EMERGENCY_SLEEP);
	}
	else{
		cartridge_state = ATTACHED;
	}
	/* When a inserted cartridge is detected - fill the moving average filter with TC measurements */
	if ((previous_cartridge_state == DETACHED) && (cartridge_state == ATTACHED)){
		HAL_Delay(100);
		Moving_Average_Set_Value(sensor_values.thermocouple_temperature, &thermocouple_temperature_filtered_filter_struct);
	}
	previous_cartridge_state = cartridge_state;
}

/* Function to toggle between RUN and HALTED at each press of the encoder button */
void handle_button_status(){
	if(SW_1_pressed == 1){
		SW_1_pressed = 0;
		// toggle between RUN and HALTED
		if ((sensor_values.current_state == RUN) || (sensor_values.current_state == STANDBY)){
			change_state(HALTED);
		}
		else if ((sensor_values.current_state == HALTED) || (sensor_values.current_state == EMERGENCY_SLEEP)){
			change_state(RUN);
		}
		previous_millis_heating_halted_update = HAL_GetTick();

	}
	/* Set "set temp" to preset temp 1 */
	if(SW_2_pressed == 1){
		SW_2_pressed = 0;
		if(flash_values.three_button_mode == 1){
			TIM2->CNT -= 5;
		}
		else{
			TIM2->CNT = flash_values.preset_temp_1;
		}
	}
	/* Set "set temp" to preset temp 2 */
	if(SW_3_pressed == 1){
		SW_3_pressed = 0;
		if(flash_values.three_button_mode == 1){
			TIM2->CNT += 5;
		}
		else{
			TIM2->CNT = flash_values.preset_temp_2;
		}
	}
}

/* Get the status of handle in/on stand to trigger SLEEP */
void get_stand_status(){
	uint8_t stand_status;
	if(HAL_GPIO_ReadPin (GPIOA, STAND_INP_Pin) == 0){
		stand_status = 1;
	}
	else{
		stand_status = 0;
	}
	sensor_values.in_stand = Moving_Average_Compute(stand_status, &stand_sense_filterStruct); /* Moving average filter */

	/* If handle is in stand set state to STANDBY */
	if(sensor_values.in_stand >= 0.2){
		if (sensor_values.current_state == RUN){
			change_state(STANDBY);
			previous_millis_standby = HAL_GetTick();
		}
		if((HAL_GetTick()-previous_millis_standby >= flash_values.standby_time*60000.0) && (sensor_values.current_state == STANDBY)){
			change_state(SLEEP);
		}
		if ((sensor_values.current_state == EMERGENCY_SLEEP) || (sensor_values.current_state == HALTED)){
			change_state(SLEEP);
		}
	}

	/* If handle is NOT in stand and state is SLEEP, change state to RUN */
	if(sensor_values.in_stand < 0.2){
		if ((sensor_values.current_state == SLEEP) || (sensor_values.current_state == STANDBY) || (sensor_values.current_state == RUN)){
			change_state(RUN);
		}
	}
}

/* Automatically detect handle type, NT115, T210 or T245 based on HANDLE_INP_1_Pin and HANDLE_INP_2_Pin.*/
void get_handle_type(){
	uint8_t handle_status;
	if(HAL_GPIO_ReadPin (GPIOA, HANDLE_INP_1_Pin) == 0){
		handle_status = 0;
	}
	else{
		handle_status = 1;
	}
	sensor_values.handle1_sense = Moving_Average_Compute(handle_status, &handle1_sense_filterStruct); /* Moving average filter */

	if(HAL_GPIO_ReadPin (GPIOA, HANDLE_INP_2_Pin) == 0){
		handle_status = 0;
	}
	else{
		handle_status = 1;
	}
	sensor_values.handle2_sense = Moving_Average_Compute(handle_status, &handle2_sense_filterStruct); /* Moving average filter */


	/* Determine if NT115 handle is detected */
	if((sensor_values.handle1_sense >= 0.5) && (sensor_values.handle2_sense < 0.5)){
		attached_handle = NT115;
	}
	/* Determine if T210 handle is detected */
	else if((sensor_values.handle1_sense < 0.5) && (sensor_values.handle2_sense >= 0.5)){
		attached_handle = T210;
	}
	/* Determine if T245 handle is detected */
	else{
		attached_handle = T245;
	}
}

/* Function to set heating values depending on detected handle */
void set_handle_values(){
	if(attached_handle == NT115){
		/* Set the max power to the lowest value from USB_PD and HANDLE_MAX_POWER */
		sensor_values.max_power_watt = min(sensor_values.USB_PD_power_limit, NT115_MAX_POWER);
		PID_SetTunings(&TPID, KP_NT115, KI_NT115, KD_NT115); // Update PID parameters based on handle type
		PID_SetILimits(&TPID, -MAX_I_NT115, MAX_I_NT115); 	// Set max and min I limit
	}
	else if(attached_handle == T210){
		/* Set the max power to the lowest value from USB_PD and HANDLE_MAX_POWER */
		sensor_values.max_power_watt = min(sensor_values.USB_PD_power_limit, T210_MAX_POWER);
		PID_SetTunings(&TPID, KP_T210, KI_T210, KD_T210); // Update PID parameters based on handle type
		PID_SetILimits(&TPID, -MAX_I_T210, MAX_I_T210); 	// Set max and min I limit
	}
	else if(attached_handle == T245){
		/* Set the max power to the lowest value from USB_PD and HANDLE_MAX_POWER */
		sensor_values.max_power_watt = min(sensor_values.USB_PD_power_limit, T245_MAX_POWER);
		PID_SetTunings(&TPID, KP_T245, KI_T245, KD_T245); // Update PID parameters based on handle type
		PID_SetILimits(&TPID, -MAX_I_T245, MAX_I_T245); 	// Set max and min I limit
	}

	/* If a custom power limit is specified in user flash, use this limit */
	if(flash_values.power_limit != 0){
		sensor_values.max_power_watt = flash_values.power_limit;
	}
}

/* Interrupts at button press */
volatile static uint16_t btnPressed = 0;
volatile static uint16_t debounceDone = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(((GPIO_Pin == SW_1_Pin) || (GPIO_Pin == SW_2_Pin) || (GPIO_Pin == SW_3_Pin)) && (SW_ready == 1)){ //A button is pressed
    	btnPressed = GPIO_Pin;
		HAL_TIM_Base_Start_IT(&htim16);
		SW_ready = 0;
		debounceDone = 0;
    }
}

/* Interrupts at every encoder increment */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	if ((htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) || (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) ) {
		beep(flash_values.buzzer_enabled);
	}
}

/* Timer Callbacks */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	if (((htim == &htim1) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)) && (current_measurement_requested == 1)){
		current_measurement_requested = 0;
		current_measurement_done = 0;
		HAL_ADC_Start_IT(&hadc2);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	/* Take thermocouple measurement every 25 ms */
	if (htim == &htim6){
		thermocouple_measurement_done = 0;
		heater_off();
		__HAL_TIM_ENABLE(&htim7);
	}

	if (htim == &htim7){
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_BUF, (uint32_t)ADC1_BUF_LEN);	//Start ADC DMA mode
		}

	/* Beep length timer */
	if (htim == &htim17){
		HAL_TIM_Base_Stop_IT(&htim17);
		HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_2);
	}

	/* Button De-bounce timer (50 ms) */
	static uint8_t timerCycles = 0;
	if(SW_ready == 0 && debounceDone == 0){
		timerCycles = 0;
		if((btnPressed == SW_1_Pin && HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET) ||
				(btnPressed == SW_2_Pin && HAL_GPIO_ReadPin(SW_2_GPIO_Port, SW_2_Pin) == GPIO_PIN_SET) ||
				(btnPressed == SW_3_Pin && HAL_GPIO_ReadPin(SW_3_GPIO_Port, SW_3_Pin) == GPIO_PIN_SET)){
			debounceDone = 1;
		}else{
			HAL_TIM_Base_Stop_IT(&htim16);
			SW_ready = 1;
		}
	}

	if (debounceDone == 1 && htim == &htim16 && SW_ready == 0){
		HAL_TIM_Base_Stop_IT(&htim16);
		if(btnPressed == SW_1_Pin && HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_RESET){
			SW_ready = 1;
			SW_1_pressed = 1;
			beep(flash_values.buzzer_enabled);
			timerCycles = 0;
		}else if(timerCycles > BTN_LONG_PRESS && btnPressed == SW_1_Pin && HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET){
			SW_ready = 1;
			beep(flash_values.buzzer_enabled);
			SW_1_pressed_long = 1;
			timerCycles = 0;
		}
		else if(btnPressed == SW_2_Pin && HAL_GPIO_ReadPin(SW_2_GPIO_Port, SW_2_Pin) == GPIO_PIN_RESET){
			SW_ready = 1;
			SW_2_pressed = 1;
			beep(flash_values.buzzer_enabled);
			timerCycles = 0;
		}else if(timerCycles > BTN_LONG_PRESS && btnPressed == SW_2_Pin && HAL_GPIO_ReadPin(SW_2_GPIO_Port, SW_2_Pin) == GPIO_PIN_SET){
			SW_ready = 1;
			beep(flash_values.buzzer_enabled);
			SW_2_pressed_long = 1;
			timerCycles = 0;
		}
		else if(btnPressed == SW_3_Pin && HAL_GPIO_ReadPin(SW_3_GPIO_Port, SW_3_Pin) == GPIO_PIN_RESET){
			SW_ready = 1;
			SW_3_pressed = 1;
			beep(flash_values.buzzer_enabled);
			timerCycles = 0;
		}
		else if(timerCycles > BTN_LONG_PRESS && btnPressed == SW_3_Pin && HAL_GPIO_ReadPin(SW_3_GPIO_Port, SW_3_Pin) == GPIO_PIN_SET){
			SW_ready = 1;
			beep(flash_values.buzzer_enabled);
			SW_3_pressed_long = 1;
			timerCycles = 0;
		}else{
			HAL_TIM_Base_Start_IT(&htim16);
			timerCycles++;
		}
	}
}
/* ADC conversion completed Callbacks */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if ((hadc->Instance == ADC1) && (thermocouple_measurement_done == 0)){
		HAL_ADC_Stop_DMA(&hadc1);
		get_thermocouple_temperature();
		update_heater_PWM();
		thermocouple_measurement_done = 1;
	}
	if ((hadc->Instance == ADC2) && (current_measurement_done == 0)){
		sensor_values.leak_current = HAL_ADC_GetValue(&hadc2);
		current_raw = HAL_ADC_GetValue(&hadc2);
		update_heater_PWM();
		current_measurement_done = 1;
	}
}

/* ADC watchdog Callback */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc __unused){
		LCD_draw_earth_fault_popup();
}

// For DEBUG
//HAL_GPIO_WritePin(USR_1_GPIO_Port, USR_1_Pin, GPIO_PIN_SET);
//HAL_GPIO_WritePin(GPIOB, USR_2_Pin, GPIO_PIN_RESET);
//HAL_GPIO_WritePin(GPIOB, USR_3_Pin, GPIO_PIN_RESET);
//HAL_GPIO_WritePin(GPIOB, USR_4_Pin, GPIO_PIN_RESET);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_CRC_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
  MX_TIM7_Init();
  MX_TIM8_Init();
  MX_TIM6_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

	set_heater_duty(0);		//Set heater duty to zero to ensure zero startup current
	HAL_TIMEx_PWMN_Start_IT(&htim1, TIM_CHANNEL_3);

	HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
	HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 5); //Set BUZZER duty to 50%
	HAL_TIM_Base_Start_IT(&htim6);

	__HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);

	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_BUF, (uint32_t)ADC1_BUF_LEN);	//Start ADC DMA mode

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_Delay(200);

	// Check if user data in flash is valid, if not - write default parameters
	if(!FlashCheckCRC()){
		FlashWrite(&default_flash_values);
		beep(1); //Beep once to indicate default parameters written to flash
		HAL_Delay(100);
	}

	/* Read flash data */
	FlashRead(&flash_values);

	/* Initiate display */
	LCD_init();
	LCD_SetRotation(flash_values.screen_rotation);

	/* Set startup state */
	change_state(HALTED);

	/* start settings menu */
	settings_menu();

	/* Set initial encoder timer value */
	TIM2->CNT = flash_values.startup_temperature;

	/* initialize moving average functions */
	Moving_Average_Init(&thermocouple_temperature_filter_struct,(uint32_t)2);
	Moving_Average_Init(&thermocouple_temperature_filtered_filter_struct,(uint32_t)flash_values.displayed_temp_filter*10);
	Moving_Average_Init(&requested_power_filtered_filter_struct,(uint32_t)20);
	Moving_Average_Init(&mcu_temperature_filter_struct,(uint32_t)100);
	Moving_Average_Init(&input_voltage_filterStruct,(uint32_t)25);
	Moving_Average_Init(&current_filterStruct,(uint32_t)5);
	Moving_Average_Init(&stand_sense_filterStruct,(uint32_t)20);
	Moving_Average_Init(&handle1_sense_filterStruct,(uint32_t)20);
	Moving_Average_Init(&handle2_sense_filterStruct,(uint32_t)20);

	/* initialize hysteresis functions */
	Hysteresis_Init(&thermocouple_temperature_filtered_hysteresis, 0.5);

	/* Initiate PID controller */
	PID(&TPID, &sensor_values.thermocouple_temperature, &sensor_values.requested_power, &PID_setpoint, 0, 0, 0, _PID_CD_DIRECT); //PID parameters are set depending on detected handle by set_handle_values()
	PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);
	PID_SetSampleTime(&TPID, PID_UPDATE_INTERVAL, 0); 		// Set PID update time to "PID_UPDATE_INTERVAL"
	PID_SetOutputLimits(&TPID, 0, PID_MAX_OUTPUT); 			// Set max and min output limit
	PID_SetILimits(&TPID, 0, 0);         					// Set max and min I limit
	PID_SetIminError(&TPID,PID_ADD_I_MIN_ERROR);
	PID_SetNegativeErrorIgainMult(&TPID, PID_NEG_ERROR_I_MULT, PID_NEG_ERROR_I_BIAS); // Set un-symmetric I gain parameters

	/* Init and fill filter structures with initial values */
	for (int i = 0; i<200;i++){
		get_bus_voltage();
		get_heater_current();
		get_mcu_temp();
		get_thermocouple_temperature();
		get_handle_type();
		set_handle_values();
		get_stand_status();
		handle_button_status();
		handle_cartridge_presence();
	}

	/* check STUSB4500 */
	HAL_StatusTypeDef halStatus;
	halStatus  = stusb_check_connection();
	if(halStatus != HAL_OK){
		//do error handling for STUSB
		debug_print_str(DEBUG_ERROR,"STUSB4500 unavailable");
	}else{
		debug_print_str(DEBUG_INFO,"STUSB4500 found");

		stusb_init();

		//1. check if cable is connected
		if(stusb_is_sink_connected()){

			//2. wait for sink to get ready
			while(!stusb_is_sink_ready()){
				debug_print_str(DEBUG_INFO,"Waiting for sink to get ready");
			}
			//if we are on USB-PD the sink needs some time to start
			HAL_Delay(500);

			stusb_soft_reset();

			//check if USB-PD is available
			STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef rdo;
			halStatus = stusb_read_rdo(&rdo);
			volatile uint8_t currendPdoIndex = rdo.b.Object_Pos;
			if(currendPdoIndex == 0){
				debug_print_str(DEBUG_INFO,"No USB-PD detected");
			}else{
				power_source = POWER_USB;
				//The usb devices need some time to transmit the messages and execute the soft reset
				//HAL_Delay(4);
				//poll alert status since we don't have the alert interrupt pin connected
				//depending on the source we may need a few tries
				bool sourceStatus = false;
				for(int i=0;i<500;i++){
					sourceStatus = poll_source();
					if(sourceStatus){
						debug_print_str(DEBUG_INFO,"Got PDOs");
						uint8_t maxPowerAvailable = 0;
						stusb_set_highest_pdo(&maxPowerAvailable, currendPdoIndex);

						sensor_values.USB_PD_power_limit = maxPowerAvailable*USB_PD_POWER_REDUCTION_FACTOR;
						debug_print_int(DEBUG_INFO,"Reduced max power to", maxPowerAvailable*USB_PD_POWER_REDUCTION_FACTOR);
						//re-negotiate
						break;
					}
				}
			}
		}else{
			debug_print_str(DEBUG_INFO,"No USB-PD sink connected");
		}
	}

	/* Draw the main screen decoration */
	LCD_draw_main_screen();

	/* Start-up beep */
	beep_startup(flash_values.startup_beep);

	//Flag to indicate that the startup sequence is done
	startup_done = 1;
	sensor_values.thermocouple_temperature_previous = sensor_values.thermocouple_temperature;

	while (1){
		if(HAL_GetTick() - previous_sensor_update_high_update >= interval_sensor_update_high_update){
			get_stand_status();
			get_handle_type();
			set_handle_values();
			get_set_temperature();
			handle_button_status();
			handle_emergency_shutdown();
			handle_cartridge_presence();
			//handle_delta_temperature();
			previous_sensor_update_high_update = HAL_GetTick();
		}

		if(HAL_GetTick() - previous_sensor_update_low_update >= interval_sensor_update_low_update){
			get_bus_voltage();
			get_heater_current();
			get_mcu_temp();
			previous_sensor_update_low_update = HAL_GetTick();
		}

		/* Compute PID */
		PID_Compute(&TPID);

		/* switch */
		switch(sensor_values.current_state) {
			case RUN: {
				PID_setpoint = sensor_values.set_temperature;
				break;
			}
			case STANDBY: {
			  if(flash_values.standby_temp > sensor_values.set_temperature){
				PID_setpoint = sensor_values.set_temperature;
			  }
			  else{
				PID_setpoint = flash_values.standby_temp;
			  }
			  break;
			}
			case SLEEP:
			case EMERGENCY_SLEEP:
			case HALTED: {
				PID_setpoint = 0;
				break;
			}
		}

		/* PID Tuning manual control */
		#ifdef PID_TUNING
		custom_temperature_on = 1;
		PID_SetTunings(&TPID, Kp_tuning, Ki_tuning, Kd_tuning/100.0);
		PID_SetILimits(&TPID, -PID_MAX_I_LIMIT_tuning, PID_MAX_I_LIMIT_tuning); 	// Set max and min I limit
		sensor_values.set_temperature = temperature_tuning;
		#endif

		/* Send debug information */
		if(flash_values.serial_debug_print == 1){
			if(HAL_GetTick() - previous_millis_debug >= interval_debug){
				UART_packet_length = 9*sizeof(float);
				pack_frame_start(UART_transmit_buffer, &UART_packet_index, UART_packet_length);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.thermocouple_temperature);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.thermocouple_temperature_filtered);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_setpoint);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.requested_power/PID_MAX_OUTPUT*100.0);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.requested_power_filtered/PID_MAX_OUTPUT*100.0);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_GetPpart(&TPID)/10.0);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_GetIpart(&TPID)/10.0);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_GetDpart(&TPID)/10.0);
				pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.heater_current);

				HAL_UART_Transmit_DMA(&huart1,(uint8_t*)UART_transmit_buffer, UART_packet_length+2); // Add two for starting bit and packet length
				previous_millis_debug = HAL_GetTick();
			}
		}

		/* Detect if a tip is present by sending a short voltage pulse and sense current */
		if (flash_values.current_measurement == 1){
			if(HAL_GetTick() - previous_measure_current_update >= interval_measure_current){
				if(thermocouple_measurement_done == 1){ //Only take current measurement if thermocouple measurement is not ongoing
					current_measurement_done = 0;
					set_heater_duty(PID_MAX_OUTPUT/2);
					current_measurement_requested = 1;
					previous_measure_current_update = HAL_GetTick();
				}
			}
		}
		else{
			sensor_values.heater_current = 1; // If the current is not measured, apply a dummy value to heater_current
		}

		/* Update display */
		if(HAL_GetTick() - previous_millis_display >= interval_display){
			sensor_values.requested_power_filtered = clamp(Moving_Average_Compute(sensor_values.requested_power, &requested_power_filtered_filter_struct), 0, PID_MAX_OUTPUT);
			update_display();
			previous_millis_display = HAL_GetTick();
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR_ADC1;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 2;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analog WatchDog 1
  */
  AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  AnalogWDGConfig.Channel = ADC_CHANNEL_2;
  AnalogWDGConfig.ITMode = ENABLE;
  AnalogWDGConfig.HighThreshold = 2000;
  AnalogWDGConfig.LowThreshold = 0;
  AnalogWDGConfig.FilteringConfig = ADC_AWD_FILTERING_NONE;
  if (HAL_ADC_AnalogWDGConfig(&hadc2, &AnalogWDGConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x30A0A7FB;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_1LINE;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 17-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 500;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_FALLING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 10000-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 10;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 17000-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 250;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 8500-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 9;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim7, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 65535;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OnePulse_Init(&htim8, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 17000-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 499;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 17000-1;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 49;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim17, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 256000;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, USR_2_Pin|USR_3_Pin|USR_4_Pin|SPI2_SD_CS_Pin
                          |SPI2_DC_Pin|SPI2_RST_Pin|SPI2_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USR_1_GPIO_Port, USR_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : VERSION_BIT_1_Pin VERSION_BIT_2_Pin VERSION_BIT_3_Pin */
  GPIO_InitStruct.Pin = VERSION_BIT_1_Pin|VERSION_BIT_2_Pin|VERSION_BIT_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : HANDLE_INP_1_Pin HANDLE_INP_2_Pin STAND_INP_Pin */
  GPIO_InitStruct.Pin = HANDLE_INP_1_Pin|HANDLE_INP_2_Pin|STAND_INP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_2_Pin */
  GPIO_InitStruct.Pin = SW_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW_2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USR_2_Pin USR_3_Pin USR_4_Pin SPI2_SD_CS_Pin
                           SPI2_DC_Pin SPI2_RST_Pin SPI2_CS_Pin */
  GPIO_InitStruct.Pin = USR_2_Pin|USR_3_Pin|USR_4_Pin|SPI2_SD_CS_Pin
                          |SPI2_DC_Pin|SPI2_RST_Pin|SPI2_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USR_1_Pin */
  GPIO_InitStruct.Pin = USR_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USR_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SW_1_Pin SW_3_Pin */
  GPIO_InitStruct.Pin = SW_1_Pin|SW_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

	// Stop the heater PWM and reconfigure as a GPIO and set to RESET
	heater_off();
	HAL_TIMEx_PWMN_Stop_IT(&htim1, TIM_CHANNEL_3);

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin : CURRENT_Pin */
	GPIO_InitStruct.Pin = HEATER_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOF, HEATER_Pin, GPIO_PIN_RESET);

	__disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
