#include "state_machine.h"
#include "main.h"
#include "settings.h"
#include "buzzer.h"
#include "display_app.h"
#include "controller.h"   /* controller_force_off */
#include "heater.h"       /* heater_off (immediate cut on delta-T fault) */
#include "handle.h"       /* cartridge_state, TC_OPEN_THRESHOLD */
#include "sensors.h"      /* sensors_get_raw_tc */

extern uint8_t startup_done;

#define EMERGENCY_SHUTDOWN_TEMPERATURE    490
#define MIN_BUSVOLTAGE                    8.0f
#define MIN_BUSPOWER                      8.0f
/* Largest delta temperature before detecting a faulty or missing cartridge */
#define MAX_TC_DELTA_FAULTDETECTION       50
/* Samples after a jump at which it is re-checked against the pre-jump
 * baseline. Must be 2: the raw TC moving average (length 2) stretches a
 * single raw glitch over two outputs, so a 1-sample confirm can't filter it. */
#define DELTA_T_CONFIRM_SAMPLES           2
/* Samples (25 ms each) to ignore after the cartridge becomes present, so the
 * settling TC after insertion isn't taken for a step fault. */
#define DELTA_T_REARM_SAMPLES             4

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

/* Function to check if the delta temperature is larger than expected.
 *
 * A real TC fault (intermittent contact, failing thermocouple) STEPS to a
 * wrong value and stays there; non-genuine cartridges instead produce short
 * glitches during heat-up (heater switching contaminating the TC reading).
 * So a jump is not trusted immediately: remember the pre-jump baseline and
 * only fault if the sample DELTA_T_CONFIRM_SAMPLES later (50 ms) is still
 * displaced from that baseline.
 *
 * Called from the ADC ISR on EVERY sample so the previous value is always
 * tracked; only the fault action depends on the delta_t_detection setting. */
void handle_delta_temperature(void){
	static float   previous = 0;
	static float   baseline = 0;                    /* pre-jump reference */
	static uint8_t confirm  = 0;                    /* samples left until the re-check */
	static uint8_t holdoff  = DELTA_T_REARM_SAMPLES;

	float temp = sensor_values.thermocouple_temperature;
	float step = temp - previous;

	if ((startup_done != 1) || (cartridge_state == DETACHED)){
		/* Not armed. Re-arm only after a short holdoff once the cartridge is
		 * present, so the settling TC after insertion isn't seen as a step. */
		confirm = 0;
		holdoff = DELTA_T_REARM_SAMPLES;
	}
	else if (sensors_get_raw_tc() > TC_OPEN_THRESHOLD){
		/* Raw TC reads open: that is a cartridge being pulled out (inherently
		 * safe - the temperature clamps to 500 C and the PID output goes to
		 * zero), not a TC fault. handle_check_cartridge() handles it. */
		confirm = 0;
	}
	else if (holdoff > 0){
		holdoff--;
		confirm = 0;
	}
	else if (confirm > 0){
		if (--confirm == 0){
			float diff_from_baseline = temp - baseline;
			if (((diff_from_baseline > MAX_TC_DELTA_FAULTDETECTION) || (diff_from_baseline < -MAX_TC_DELTA_FAULTDETECTION)) &&
			    (flash_values.delta_t_detection == 1)){
				/* Still displaced from the pre-jump baseline: sustained step.
				 * Cut power NOW (this runs in the ADC ISR; the main loop only
				 * consumes the latch later). heater_off() also clears the duty
				 * heater_update_pwm programmed earlier in this same ISR, and
				 * heater_update_pwm keeps it off until the latch is consumed. */
				controller_force_off();
				heater_off();
				delta_t_fault_pending = 1;
			}
			/* else: glitch - ignore */
		}
	}
	else if ((step > MAX_TC_DELTA_FAULTDETECTION) || (step < -MAX_TC_DELTA_FAULTDETECTION)){
		baseline = previous;
		confirm  = DELTA_T_CONFIRM_SAMPLES;
	}
	previous = temp;
	sensor_values.thermocouple_temperature_previous = temp;
}

/* Function to set state to EMERGENCY_SLEEP */
void handle_emergency_shutdown(void){
	/* Consume a delta-T fault latched by the ADC ISR */
	if(delta_t_fault_pending == 1){
		/* Zero the PID state before releasing the latch, so the heater ISRs
		 * see zero power from the moment the gate opens. */
		controller_force_off();
		delta_t_fault_pending = 0;
		/* Only a heating state needs the transition and the popup; a fault
		 * seen while already asleep (setpoint 0) needs neither. */
		if((sensor_values.current_state == RUN) ||
		   (sensor_values.current_state == PRESTANDBY) ||
		   (sensor_values.current_state == STANDBY)){
			change_state(EMERGENCY_SLEEP);
			show_popup("TC delta fault");
		}
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
	/* Set state to EMERGENCY_SLEEP if input voltage is too low. Also in the
	 * standby states, which still heat: a sagging or 5 V-only supply must not
	 * hold a tip at standby temperature either. */
	if((sensor_values.bus_voltage <= MIN_BUSVOLTAGE) &&
	   ((sensor_values.current_state == RUN) ||
	    (sensor_values.current_state == PRESTANDBY) ||
	    (sensor_values.current_state == STANDBY))){
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
