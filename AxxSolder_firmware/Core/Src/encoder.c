#include "encoder.h"
#include "main.h"
#include "util.h"
#include "buzzer.h"

extern TIM_HandleTypeDef htim2;
extern uint8_t custom_temperature_on;

/* Get encoder value (Set temp.) and limit */
void encoder_read_set_temperature(void){
	if(custom_temperature_on == 0){
		uint16_t prev_temp = sensor_values.set_temperature;

		TIM2->CNT = clamp(TIM2->CNT, MIN_SELECTABLE_TEMPERATURE, MAX_SELECTABLE_TEMPERATURE);

		if(flash_values.three_button_mode == 1){
			sensor_values.set_temperature = (uint16_t)(TIM2->CNT);
		}
		else{
			static const uint8_t enc_steps[4] = {1, 2, 5, 10};
			uint8_t idx = (uint8_t)flash_values.encoder_step_idx;
			if(idx > 3){ idx = 1; }   /* fall back to default (step 2) */
			uint8_t step = enc_steps[idx];
			sensor_values.set_temperature = (uint16_t)((TIM2->CNT / step) * step);
		}
		// Reset the flag if the set temperature has changed, so it beeps when reached
		if(sensor_values.set_temperature != prev_temp){
			buzzer_rearm_at_set_temp();
		}
	}
}

uint16_t encoder_get_count(void) {
    return (uint16_t)TIM2->CNT;
}

void encoder_set_count(uint16_t value) {
    TIM2->CNT = value;
}

void encoder_add(int16_t delta) {
    TIM2->CNT = (uint16_t)((int32_t)TIM2->CNT + delta);
}

void encoder_isr_capture(void) {
    /* Click feedback for every encoder detent. */
    beep(flash_values.buzzer_enabled);
}
