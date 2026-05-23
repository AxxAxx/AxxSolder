#include "buzzer.h"
#include "main.h"      /* sensor_values */
#include "settings.h"  /* flash_values */
#include <math.h>      /* fabsf */

extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim17;

/* --- "Setpoint reached" beep state (private) --- */
static uint8_t beeped_at_set_temp = 0;

/* ------------------------------------------------------------------ */
/* ISR                                                                */
/* ------------------------------------------------------------------ */

void buzzer_isr_beep_done(void) {
    /* TIM17 fired: beep duration elapsed. Stop the PWM and the
     * one-pulse timer. */
    HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_2);
    HAL_TIM_Base_Stop_IT(&htim17);
}

/* ------------------------------------------------------------------ */
/* Low-level primitives                                               */
/* ------------------------------------------------------------------ */

/* Single short beep. No-op if the user has disabled the buzzer. */
void beep(float buzzer_enabled) {
    if (buzzer_enabled == 1) {
        HAL_TIM_Base_Start_IT(&htim17);
        HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_2);
    }
}

/* Two short beeps separated by 100 ms. Blocking. */
void beep_double(float buzzer_enabled) {
    beep(buzzer_enabled);
    HAL_Delay(100);
    beep(buzzer_enabled);
}

/* Set the beep tone frequency (Hz) and duration (ms). Reconfigures
 * TIM4 prescaler and TIM17 period. */
void set_tone(float frequency, float time_ms) {
    htim4.Init.Prescaler = 17000000 / (10 * frequency);
    HAL_TIM_PWM_Init(&htim4);

    htim17.Init.Period = time_ms * 10;
    HAL_TIM_Base_Init(&htim17);
    HAL_TIM_OnePulse_Init(&htim17, TIM_OPMODE_SINGLE);
}

/* ------------------------------------------------------------------ */
/* "Setpoint reached" beep                                            */
/* ------------------------------------------------------------------ */

void beep_at_set_temp(void) {
    if (!flash_values.beep_at_set_temp) return;
    if (beeped_at_set_temp) return;

    float temp   = sensor_values.thermocouple_temperature_filtered;
    float target = sensor_values.set_temperature;
    float delta  = fabsf(temp - target);

    if (delta <= 5.0f) {
        beeped_at_set_temp = 1;
        beep_double(flash_values.buzzer_enabled);
    }
}

void buzzer_rearm_at_set_temp(void) {
    beeped_at_set_temp = 0;
}
