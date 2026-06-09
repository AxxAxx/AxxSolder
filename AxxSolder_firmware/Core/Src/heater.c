#include "heater.h"
#include "main.h"
#include "util.h"

extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc2;

#define POWER_CONVERSION_FACTOR  0.123f
#define FIXED_MEASURE_DUTY       (PID_MAX_OUTPUT / 2)   /* short pulse for current sense */

/* RAW ADC from current measurement */
uint16_t adc2_dma_buffer[2]; // [0] = channel 2, [1] = channel 10
uint16_t current_raw = 0;
uint16_t current_leak = 0;

volatile uint8_t current_measurement_requested = 0;
volatile uint8_t current_measurement_done = 1;

/* Sets the duty cycle of timer controlling the heater */
void heater_set_duty(uint16_t dutycycle){
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, dutycycle);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint16_t)(dutycycle * 0.3f));
}

/* Disable the duty cycle of timer controlling the heater PWM*/
void heater_off(void){
	heater_set_duty(0);
}

/* Update the duty cycle of timer controlling the heater PWM */
void heater_update_pwm(void){
	if (sensor_values.bus_voltage <= 0.0f) return;
	float current = (sensor_values.max_power_watt*POWER_CONVERSION_FACTOR) / sensor_values.bus_voltage;
	float duty_cycle = sensor_values.requested_power * current;

	duty_cycle = clamp(duty_cycle, 0, PID_MAX_OUTPUT);
	heater_set_duty(duty_cycle);
}

/* Begin a one-shot current measurement: set FIXED_MEASURE_DUTY,
 * force timer update so the new duty takes effect immediately, then
 * arm the measurement flags. Caller is responsible for rate-limiting
 * and for ensuring the thermocouple sample is idle. */
void heater_start_current_measurement(void) {
	heater_set_duty(FIXED_MEASURE_DUTY);
	// Force the timer to update immediately so that the new value takes effect immediately
	htim1.Instance->EGR |= TIM_EGR_UG;  // Apply duty cycle immediately

	current_measurement_done = 0;
	current_measurement_requested = 1; // Measurement needs to be performed
}

bool heater_current_measurement_in_progress(void) {
    return current_measurement_done == 0;
}

/* ------------------------------------------------------------------ */
/* ISR entry points                                                   */
/* ------------------------------------------------------------------ */

/* Timer Callbacks */
void heater_isr_pwm_ch1_pulse_finished(void) {
	if (current_measurement_requested == 1 && current_measurement_done == 0)
	{
		uint32_t duty_now = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_3);

		if ( duty_now == FIXED_MEASURE_DUTY )
		{   // check: is the level high on HEATER_Pin (CH3)
			if ( (HAL_GPIO_ReadPin(HEATER_GPIO_Port, HEATER_Pin) == GPIO_PIN_SET) )
			{
				//HAL_GPIO_WritePin(USR_1_GPIO_Port, USR_1_Pin, GPIO_PIN_SET);
				current_measurement_requested = 0;

				HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc2_dma_buffer, 2);
			}
		}
	}
}

void heater_isr_current_complete(void) {
	if (current_measurement_done == 0)
	{
		current_measurement_done = 1;

		current_leak = adc2_dma_buffer[0];  // channel 2
		current_raw  = adc2_dma_buffer[1];  // channel 10

		sensor_values.leak_current = current_leak;

		heater_update_pwm();
	}
}
