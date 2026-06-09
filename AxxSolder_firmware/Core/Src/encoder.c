#include "encoder.h"
#include "main.h"
#include "util.h"
#include "buzzer.h"

extern TIM_HandleTypeDef htim2;
extern uint8_t custom_temperature_on;

/* Get encoder value (Set temp.) and limit */
void encoder_read_set_temperature(void){
	if(custom_temperature_on != 0){
		return;
	}
	uint16_t prev_temp = sensor_values.set_temperature;

	if(flash_values.three_button_mode == 1){
		/* 3-button mode: TIM2->CNT is driven directly by the +/- buttons and
		   holds the temperature 1:1 (no physical rotary encoder). */
		TIM2->CNT = clamp(TIM2->CNT, MIN_SELECTABLE_TEMPERATURE, MAX_SELECTABLE_TEMPERATURE);
		sensor_values.set_temperature = (uint16_t)(TIM2->CNT);
	}
	else{
		/* Rotary encoder: it emits 2 counts per detent, so the adjustment step
		   must be applied per detent (counts/2), not by snapping the absolute
		   count. We track the count delta and move set_temperature by
		   "step" degrees per detent. */
		static const uint8_t enc_steps[4] = {1, 2, 5, 10};
		uint8_t idx = (uint8_t)flash_values.encoder_step_idx;
		if(idx > 3){ idx = 1; }   /* fall back to default (step 2) */
		int32_t step = enc_steps[idx];

		static uint8_t enc_synced = 0;
		static int32_t enc_last   = 0;
		int32_t cnt = (int32_t)(uint16_t)TIM2->CNT;
		if(!enc_synced){ enc_last = cnt; enc_synced = 1; }

		int32_t delta = cnt - enc_last;
		if(delta >  32768){ delta -= 65536; }   /* 16-bit counter wrap */
		if(delta < -32768){ delta += 65536; }

		int32_t detents = delta / 2;             /* 2 counts per detent */
		if(detents != 0){
			if(detents > 20 || detents < -20){
				/* Implausibly large jump for a hand-turned encoder in one
				   10 ms tick - e.g. returning from a menu that repurposed
				   TIM2->CNT. Re-anchor instead of applying it. */
				enc_last = cnt;
			}
			else{
				enc_last += detents * 2;         /* consume whole detents, keep odd remainder */
				float t = (float)sensor_values.set_temperature + (float)(detents * step);
				sensor_values.set_temperature = (uint16_t)clamp(t, MIN_SELECTABLE_TEMPERATURE, MAX_SELECTABLE_TEMPERATURE);
			}
		}
	}

	// Reset the flag if the set temperature has changed, so it beeps when reached
	if(sensor_values.set_temperature != prev_temp){
		buzzer_rearm_at_set_temp();
	}
}

/* Returns the current set temperature (used to save a preset). */
uint16_t encoder_get_count(void) {
    return (uint16_t)sensor_values.set_temperature;
}

/* Sets the set temperature directly (used to recall a preset). The rotary
   delta tracking in encoder_read_set_temperature is unaffected because
   TIM2->CNT is not touched here. */
void encoder_set_count(uint16_t value) {
    sensor_values.set_temperature = (uint16_t)clamp((float)value, MIN_SELECTABLE_TEMPERATURE, MAX_SELECTABLE_TEMPERATURE);
}

void encoder_add(int16_t delta) {
    TIM2->CNT = (uint16_t)((int32_t)TIM2->CNT + delta);
}

void encoder_isr_capture(void) {
    /* Click feedback for every encoder detent. */
    beep(flash_values.buzzer_enabled);
}
