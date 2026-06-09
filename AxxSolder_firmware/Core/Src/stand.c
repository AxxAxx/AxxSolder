#include "stand.h"
#include "main.h"
#include "state_machine.h"
#include "buzzer.h"
#include "moving_average.h"

/* Filter + debounce state */
FilterTypeDef stand_sense_filterStruct;
uint32_t previous_stand_debounce = 0;
uint32_t interval_stand_debounce = 100;
uint8_t previous_stand_status   = 0;
uint8_t stand_debounced_flag    = 0;

/* Get the status of handle in/on stand to trigger SLEEP */
void stand_update(void){
	// Read stand input: low level = handle is in the stand
	uint8_t stand_status = 0;
	if(flash_values.detect_nt115 == 1){
		stand_status = (HAL_GPIO_ReadPin(GPIOA, STAND_INP_Pin) == GPIO_PIN_RESET) ? 1 : 0;
	}
	else{
		stand_status = ((HAL_GPIO_ReadPin(GPIOA, STAND_INP_Pin) == GPIO_PIN_RESET) ||
		                (HAL_GPIO_ReadPin(GPIOA, HANDLE_INP_2_Pin) == GPIO_PIN_RESET)) ? 1 : 0;
	}

	/* If the momentary stand function is not used */
	if(flash_values.momentary_stand == 0){
		sensor_values.in_stand = Moving_Average_Compute(stand_status, &stand_sense_filterStruct);
	}
	/* If the momentary stand function is used, de-bounce the stand input */
	else{
		if(stand_status != previous_stand_status){
			previous_stand_debounce = HAL_GetTick();
			previous_stand_status = stand_status;
			stand_debounced_flag = 0;
		}
		else if(!stand_debounced_flag && (HAL_GetTick() - previous_stand_debounce) > interval_stand_debounce){
			stand_debounced_flag = 1;
			if(stand_status == 1){
				if(sensor_values.in_stand == 0){
					beep(flash_values.buzzer_enabled);
					sensor_values.in_stand = 1;
				}
				else{
					beep_double(flash_values.buzzer_enabled);
					sensor_values.in_stand = 0;
				}
			}
	    }
	}

	/* Handle stand logic */
	if(sensor_values.in_stand >= 0.2f){

		/* RUN -> PRESTANDBY or STANDBY */
		if(sensor_values.current_state == RUN){
			if(flash_values.standby_delay != 0){
				change_state(PRESTANDBY);
				previous_millis_prestandby = HAL_GetTick();
			}
			else{
				change_state(STANDBY);
				previous_millis_standby = HAL_GetTick();
			}
		}

		/* PRESTANDBY -> STANDBY after delay */
		if((sensor_values.current_state == PRESTANDBY) &&
		   (HAL_GetTick() - previous_millis_prestandby >= flash_values.standby_delay * 1000.0f)){
			change_state(STANDBY);
			previous_millis_standby = HAL_GetTick();
		}

		/* STANDBY -> SLEEP */
		if((sensor_values.current_state == STANDBY) &&
		   (HAL_GetTick() - previous_millis_standby >= flash_values.standby_time * 60000.0f)){
			change_state(SLEEP);
		}

		/* Force sleep from emergency states */
		if ((sensor_values.current_state == EMERGENCY_SLEEP) ||
		    (sensor_values.current_state == HALTED)){
			change_state(SLEEP);
		}
	}

	/* Handle removed from stand → always go to RUN */
	if(sensor_values.in_stand < 0.2f){
		if ((sensor_values.current_state == SLEEP) ||
		    (sensor_values.current_state == STANDBY) ||
		    (sensor_values.current_state == PRESTANDBY) ||
		    (sensor_values.current_state == RUN)){
			change_state(RUN);
		}
	}
}
