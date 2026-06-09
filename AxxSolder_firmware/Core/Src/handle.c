#include "handle.h"
#include "main.h"
#include "sensors.h"
#include "tip_profile.h"
#include "menu_profiles.h"
#include "display_app.h"
#include "controller.h"
#include "moving_average.h"

extern uint8_t startup_done;

/* Per-handle max power limits (W) */
#define NT115_MAX_POWER     22
#define T210_MAX_POWER      65
#define T245_MAX_POWER      130
#define No_name_MAX_POWER   150

/* Filters for handle-detect GPIOs (20-sample moving average to debounce
 * connector contact noise). */
FilterTypeDef handle1_sense_filterStruct;
FilterTypeDef handle2_sense_filterStruct;

/* --- Cartridge tracking (private) --- */
static cartridge_state_t previous_cartridge_state = ATTACHED;
static uint8_t           cartridge_first_detect = 1;

/* --- Public state --- */
enum handles      attached_handle;
cartridge_state_t cartridge_state = ATTACHED;

/* Automatically detect handle type, NT115, T210 or T245 based on HANDLE_INP_1_Pin and HANDLE_INP_2_Pin.*/
void handle_detect(void){
	uint8_t handle1_status = (HAL_GPIO_ReadPin (GPIOA, HANDLE_INP_1_Pin) == GPIO_PIN_RESET) ? 0 : 1;
	sensor_values.handle1_sense = Moving_Average_Compute(handle1_status, &handle1_sense_filterStruct); // Moving average filter

	uint8_t handle2_status = (HAL_GPIO_ReadPin (GPIOA, HANDLE_INP_2_Pin) == GPIO_PIN_RESET) ? 0 : 1;
	sensor_values.handle2_sense = Moving_Average_Compute(handle2_status, &handle2_sense_filterStruct); // Moving average filte

	/* If NT115 should not be detected, force 1 to sensor_values.handle2_sense*/
	if(flash_values.detect_nt115 == 0){
		sensor_values.handle2_sense = 1;
	}

	/* Determine if NT115 handle is detected */
	if((sensor_values.handle1_sense >= 0.5f) && (sensor_values.handle2_sense < 0.5f)){
		attached_handle = NT115;
	}
	/* Determine if T210 handle is detected */
	else if((sensor_values.handle1_sense < 0.5f) && (sensor_values.handle2_sense >= 0.5f)){
		attached_handle = T210;
	}
	/* Determine if T245 handle is detected */
	else{
		attached_handle = T245;
	}
}

/* Function to set heating values depending on detected handle */
void handle_apply_settings(void){
	uint16_t usb_limit = sensor_values.USB_PD_power_limit;
    uint16_t WATT;

	uint16_t profile_limit = (uint16_t)tip_profiles_get_power_limit(attached_handle);
	uint16_t handle_max;

	switch (attached_handle) {
    case NT115:   handle_max = NT115_MAX_POWER;   break;
    case T210:    handle_max = T210_MAX_POWER;    break;
    case T245:    handle_max = T245_MAX_POWER;    break;
    case No_name:
    default:      handle_max = No_name_MAX_POWER; break;
	}

	WATT = (profile_limit != 0) ? profile_limit : handle_max;
	/* max_power_watt = min(usb_limit, handle_max, WATT) */
	uint16_t min_ab = (usb_limit < handle_max) ? usb_limit : handle_max;
	sensor_values.max_power_watt = (min_ab < WATT) ? min_ab : WATT;

	/* Apply PID from active tip profile */
	tip_profiles_apply_pid(attached_handle, &TPID);
}

/* Function to handle the cartridge presence */
void handle_check_cartridge(void){
	if((sensor_values.heater_current < 1) || (sensors_get_raw_tc() > 4096-10)) { //NT115 at 9V draws 2.3
		cartridge_state = DETACHED;
		change_state(EMERGENCY_SLEEP);
	}
	else{
		cartridge_state = ATTACHED;
	}
	/* When a inserted cartridge is detected - fill the moving average filter with TC measurements */
	if ((previous_cartridge_state == DETACHED) && (cartridge_state == ATTACHED)){
		HAL_Delay(100);
		sensors_reset_filtered_tc(sensor_values.thermocouple_temperature);

		  /* Show profile selector popup on tip change (skip first detection after boot) */
		  if (cartridge_first_detect) {
			  cartridge_first_detect = 0;
		  } else if (startup_done && flash_values.show_profile_on_tip_change && !settings_menu_active) {
			profiles_popup(attached_handle);
			LCD_draw_main_screen();
		}
	}
	previous_cartridge_state = cartridge_state;
}
