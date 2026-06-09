#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/* Read the encoder count, clamp to MIN/MAX_SELECTABLE_TEMPERATURE, and
 * write to sensor_values.set_temperature. Honors flash_values.three_button_mode
 * for step granularity (1degC vs 2degC). */
void encoder_read_set_temperature(void);

/* Encoder count read/write/increment helpers. Used by buttons.c to
 * bump the setpoint via preset-temp buttons. */
uint16_t encoder_get_count(void);
void     encoder_set_count(uint16_t value);
void     encoder_add(int16_t delta);

/* ISR: called from HAL_TIM_IC_CaptureCallback when TIM2's input-capture
 * channel fires (one click of the rotary encoder). */
void encoder_isr_capture(void);

#endif /* ENCODER_H */
