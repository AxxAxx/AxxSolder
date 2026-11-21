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
static uint8_t           cartridge_first_detect = 1;

/* Non-blocking TC settle after a cartridge insertion (replaces a HAL_Delay
 * that stalled the main loop): the filter is reseeded once the TC has read
 * valid for CARTRIDGE_TC_SETTLE_MS. */
#define CARTRIDGE_TC_SETTLE_MS 100
static uint8_t           settle_pending = 0;
static uint32_t          settle_start = 0;

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
	/* USB_PD_power_limit is a float that defaults to FLT_MAX ("no limit");
	 * converting an out-of-range float to uint16_t is undefined behaviour,
	 * so clamp both ends first (also catches negative and NaN). */
	float usb_limit_f = sensor_values.USB_PD_power_limit;
	uint16_t usb_limit = (usb_limit_f >= 65535.0f) ? 65535
	                   : ((usb_limit_f > 0.0f) ? (uint16_t)usb_limit_f : 0);
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
	uint8_t present = (sensor_values.heater_current >= 1) && (sensors_get_raw_tc() <= TC_OPEN_THRESHOLD); //NT115 at 9V draws 2.3
	if (!present){
		settle_pending = 0;
		cartridge_state = DETACHED;
		change_state(EMERGENCY_SLEEP);
		return;
	}
	if (cartridge_state == ATTACHED) return;

	/* Inserted cartridge: stay DETACHED (heater held off) until the TC has read
	 * valid for CARTRIDGE_TC_SETTLE_MS, without blocking the main loop. */
	if (!settle_pending){
		settle_pending = 1;
		settle_start = HAL_GetTick();
	}
	if ((HAL_GetTick() - settle_start) < CARTRIDGE_TC_SETTLE_MS){
		change_state(EMERGENCY_SLEEP);
		return;
	}
	settle_pending = 0;

	/* Fill the moving average filter with the settled TC measurement */
	sensors_reset_filtered_tc(sensor_values.thermocouple_temperature);
	cartridge_state = ATTACHED;

	/* Show profile selector popup on tip change (skip first detection after boot) */
	if (cartridge_first_detect) {
		cartridge_first_detect = 0;
	} else if (startup_done && flash_values.show_profile_on_tip_change && !settings_menu_active) {
		profiles_popup(attached_handle);
		LCD_draw_main_screen();
	}
}
