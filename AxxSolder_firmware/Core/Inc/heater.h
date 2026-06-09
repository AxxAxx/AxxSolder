#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>
#include <stdbool.h>

/* Raw heater-current ADC reading. Set from the ADC2 ISR; sensors.c
 * reads it to compute the filtered heater_current. */
extern uint16_t current_raw;

/* Direct PWM control */
void heater_set_duty(uint16_t duty_cycle);
void heater_off(void);

/* Recomputes the heater duty from sensor_values (bus voltage, max
 * power, requested power). Called from the ADC2 ISR. */
void heater_update_pwm(void);

/* Begin a one-shot current-measurement pulse:
 * - set duty to FIXED_MEASURE_DUTY
 * - force the timer update event
 * - arm the ADC2 capture (completes in the ADC ISR)
 *
 * Caller is responsible for rate-limiting and for ensuring the sensors
 * module is not in the middle of a thermocouple sample. */
void heater_start_current_measurement(void);

/* True while a current measurement is armed-or-in-flight. */
bool heater_current_measurement_in_progress(void);

/* ISR entry points - call from main.c HAL callbacks. */
void heater_isr_pwm_ch1_pulse_finished(void);  /* TIM1 channel 1 PWM pulse done */
void heater_isr_current_complete(void);        /* ADC2 conversion complete */

#endif /* HEATER_H */
