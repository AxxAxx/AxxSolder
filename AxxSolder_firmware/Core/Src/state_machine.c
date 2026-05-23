#include "state_machine.h"
#include "main.h"
#include "settings.h"
#include "buzzer.h"
#include "display_app.h"

extern uint8_t startup_done;

#define EMERGENCY_SHUTDOWN_TEMPERATURE    490
#define MIN_BUSVOLTAGE                    8.0f
#define MIN_BUSPOWER                      8.0f
/* Largest delta temperature before detecting a faulty or missing cartridge */
#define MAX_TC_DELTA_FAULTDETECTION       50

/* State-entry timestamps */
uint32_t previous_millis_left_stand = 0;
uint32_t previous_millis_standby    = 0;
uint32_t previous_millis_prestandby = 0;

/* Function to change the main state */
void change_state(mainstates new_state){
	// Save the previous state
	sensor_values.previous_state = sensor_values.current_state;
	// Set the new current state
	sensor_values.current_state = new_state;

	// If transitioning TO RUN STATE and the current temperature should be saved as the startup temperature
	if((sensor_values.previous_state != RUN) && (sensor_values.current_state == RUN) && (flash_values.startup_temp_is_previous_temp == 1)){
		flash_values.startup_temperature = sensor_values.set_temperature;
		settings_save();
	}

	// Enable GPIO4 when transitioning to RUN, if the corresponding setting is active
	if((sensor_values.current_state == RUN) && (flash_values.GPIO4_ON_at_run == 1)){
		HAL_GPIO_WritePin(GPIOB, USR_4_Pin, GPIO_PIN_SET); // Set the USR_4 output to high level
	}
	else{
		HAL_GPIO_WritePin(GPIOB, USR_4_Pin, GPIO_PIN_RESET); // Set the USR_4 output to low level
	}

	// Reset the beep flag upon reaching the temperature, only when entering RUN
	if((sensor_values.previous_state != RUN) && (sensor_values.current_state == RUN)){
			buzzer_rearm_at_set_temp();
	}
}

/* Function to check if the delta temperature is larger than expected */
void handle_delta_temperature(void){
	if ((startup_done == 1) &&
			( (sensor_values.thermocouple_temperature - sensor_values.thermocouple_temperature_previous > MAX_TC_DELTA_FAULTDETECTION) ||
			  (sensor_values.thermocouple_temperature - sensor_values.thermocouple_temperature_previous < -MAX_TC_DELTA_FAULTDETECTION) )){
		//heater_off();
		sensor_values.requested_power = 0.0f;
		//sensor_values.heater_current = 0;
		change_state(EMERGENCY_SLEEP);
		//show_popup("No or Faulty tip!");
	}
	sensor_values.thermocouple_temperature_previous = sensor_values.thermocouple_temperature;
}

/* Function to set state to EMERGENCY_SLEEP */
void handle_emergency_shutdown(void){
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
	/* Set state to EMERGENCY_SLEEP if iron is over max allowed temp */
	if((sensor_values.thermocouple_temperature_filtered > EMERGENCY_SHUTDOWN_TEMPERATURE) && (sensor_values.current_state == RUN)){
		change_state(EMERGENCY_SLEEP);
		show_popup("OVERTEMP");
	}
	/* Set state to EMERGENCY_SLEEP if input power is too low */
	else if((sensor_values.max_power_watt <= MIN_BUSPOWER) && (sensor_values.current_state == RUN)){
		change_state(EMERGENCY_SLEEP);
		show_popup("Inp. Power too low");
	}
	/* Set state to EMERGENCY_SLEEP if no tip detected (no current draw) */
	else if((sensor_values.heater_current < 1) && (sensor_values.current_state == RUN)){ //NT115 at 9V draws 2.3
		change_state(EMERGENCY_SLEEP);
		show_popup("No tip detected");
	}
}
