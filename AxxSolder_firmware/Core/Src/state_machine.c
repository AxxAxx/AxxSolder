#include "state_machine.h"
#include "main.h"
#include "settings.h"
#include "buzzer.h"
#include "display_app.h"
#include "controller.h"   /* controller_force_off */
#include "heater.h"       /* heater_off (immediate cut on delta-T fault) */

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

/* Delta-T fault latch. Set from the ADC ISR, consumed in the main loop.
 * change_state() must only be called from main context: calling it from the
 * ISR races main-loop callers (e.g. stand_update reads current_state and then
 * writes RUN), which could silently overwrite the EMERGENCY_SLEEP transition. */
static volatile uint8_t delta_t_fault_pending = 0;

uint8_t delta_t_fault_is_pending(void){
	return delta_t_fault_pending;
}

/* Function to change the main state */
void change_state(mainstates new_state){
	// Save the previous state
	sensor_values.previous_state = sensor_values.current_state;
	// Set the new current state
	sensor_values.current_state = new_state;

	// If transitioning TO RUN STATE and the current temperature should be saved as the startup temperature.
	// Only touch flash when the value actually changed: settings_save() erases and
	// reprograms a whole page (~10k cycle endurance, ~30 ms with IRQs off), and RUN
	// is entered on every pickup from the stand.
	if((sensor_values.previous_state != RUN) && (sensor_values.current_state == RUN) && (flash_values.startup_temp_is_previous_temp == 1)
			&& (flash_values.startup_temperature != sensor_values.set_temperature)){
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
		/* Cut power NOW (this runs in the ADC ISR; the main loop only consumes
		 * the latch later). heater_off() also clears the duty heater_update_pwm
		 * programmed earlier in this same ISR. */
		controller_force_off();
		heater_off();
		delta_t_fault_pending = 1;
	}
	sensor_values.thermocouple_temperature_previous = sensor_values.thermocouple_temperature;
}

/* Function to set state to EMERGENCY_SLEEP */
void handle_emergency_shutdown(void){
	/* Consume a delta-T fault latched by the ADC ISR */
	if(delta_t_fault_pending == 1){
		/* Zero the PID state before releasing the latch, so the heater ISRs
		 * see zero power from the moment the gate opens. */
		controller_force_off();
		delta_t_fault_pending = 0;
		change_state(EMERGENCY_SLEEP);
	}
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
