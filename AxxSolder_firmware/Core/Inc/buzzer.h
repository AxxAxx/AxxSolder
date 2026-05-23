#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "stm32g4xx_hal.h"

/* Buzzer module: TIM4 PWM drives the piezo, TIM17 is a one-pulse
 * timer that bounds beep duration. */

/* ISR: call from HAL_TIM_PeriodElapsedCallback when TIM17 fires.
 * Stops the PWM and the one-pulse timer, ending the current beep. */
void buzzer_isr_beep_done(void);

/* Low-level primitives */
void beep(float buzzer_enabled);
void beep_double(float buzzer_enabled);
void set_tone(float frequency, float time_ms);

/* High-level: setpoint-reached beep.
 * Call every main-loop tick. Compares filtered tip temp to setpoint;
 * if within 5degC and the at-set-temp beep is currently armed, emits a
 * double-beep and disarms. Gated by flash_values.beep_at_set_temp. */
void beep_at_set_temp(void);

/* Re-arm the at-set-temp beep. Call when state enters RUN, when the
 * setpoint changes, or any other moment the user should be re-notified
 * once the iron next reaches target. */
void buzzer_rearm_at_set_temp(void);

#endif /* INC_BUZZER_H_ */
