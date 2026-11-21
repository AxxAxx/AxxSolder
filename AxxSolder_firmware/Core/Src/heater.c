#include "heater.h"
#include "main.h"
#include "util.h"
#include "state_machine.h"   /* delta_t_fault_is_pending */
#include "sensors.h"         /* sensors_thermocouple_sampling_idle */

extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc2;

#define POWER_CONVERSION_FACTOR  0.123f
#define FIXED_MEASURE_DUTY       (PID_MAX_OUTPUT / 2)   /* short pulse for current sense */

/* RAW ADC from current measurement */
uint16_t adc2_dma_buffer[2]; // [0] = channel 2, [1] = channel 10
volatile uint16_t current_raw = 0;
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
	/* A latched delta-T fault keeps the heater off until the main loop
	 * consumes it, independent of any requested_power a PID_Compute that was
	 * in flight may have written after the ISR forced it to zero. */
	if (delta_t_fault_is_pending()){
		heater_set_duty(0);
		return;
	}
	if (sensor_values.bus_voltage <= 0.0f) return;
	float current = (sensor_values.max_power_watt*POWER_CONVERSION_FACTOR) / sensor_values.bus_voltage;
	float duty_cycle = sensor_values.requested_power * current;

	duty_cycle = clamp(duty_cycle, 0, PID_MAX_OUTPUT);
	heater_set_duty(duty_cycle);
}

/* Begin a one-shot current measurement: set FIXED_MEASURE_DUTY,
 * force timer update so the new duty takes effect immediately, then
 * arm the measurement flags. Returns true if armed. Caller is
 * responsible for rate-limiting.
 *
 * The whole arm runs with IRQs masked: the TIM6 window-start ISR turns
 * the heater off, so checking "sampling idle" and applying the pulse
 * must be indivisible or the pulse can land inside the thermocouple
 * settling window. */
bool heater_start_current_measurement(void) {
	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	/* No measurement pulse while a delta-T fault is pending: the heater
	 * must stay off until the main loop has consumed it. */
	if (!sensors_thermocouple_sampling_idle() || delta_t_fault_is_pending()) {
		__set_PRIMASK(primask);
		return false;
	}

	heater_set_duty(FIXED_MEASURE_DUTY);
	// Force the timer to update immediately so that the new value takes effect immediately
	htim1.Instance->EGR = TIM_EGR_UG;  // Apply duty cycle immediately
	/* Drop a CC1 match latched before the update so the pulse-finished ISR
	 * only sees the compare of this measurement pulse. */
	__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC1);

	current_measurement_done = 0;
	current_measurement_requested = 1; // Measurement needs to be performed

	__set_PRIMASK(primask);
	return true;
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

		/* Abort if the measurement duty was overwritten (TC window
		 * heater_off / PWM update): the pulse is gone, so drop the request
		 * and let the main loop re-arm. */
		if ( duty_now != FIXED_MEASURE_DUTY )
		{
			current_measurement_requested = 0;
			current_measurement_done = 1;
			return;
		}

		// check: is the level high on HEATER_Pin (CH3)
		if ( (HAL_GPIO_ReadPin(HEATER_GPIO_Port, HEATER_Pin) == GPIO_PIN_SET) )
		{
			//HAL_GPIO_WritePin(USR_1_GPIO_Port, USR_1_Pin, GPIO_PIN_SET);
			current_measurement_requested = 0;

			if (HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc2_dma_buffer, 2) != HAL_OK){
				/* No completion will come: end the measurement and restore
				 * regular duty unless the TC window is open. */
				current_measurement_done = 1;
				if (sensors_thermocouple_sampling_idle()) {
					heater_update_pwm();
				}
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

		/* Restore PWM only outside the thermocouple sampling window; the
		 * ADC1-complete ISR restores it at the end of the window. */
		if (sensors_thermocouple_sampling_idle()) {
			heater_update_pwm();
		}
	}
}
