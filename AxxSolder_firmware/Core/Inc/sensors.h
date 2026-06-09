#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>
#include "moving_average.h"
#include "hysteresis.h"

/* Filter structs owned by sensors. File-scope so main.c can call
 * Moving_Average_Init on them at boot. Don't access outside sensors.c. */
extern FilterTypeDef thermocouple_temperature_filter_struct;
extern FilterTypeDef thermocouple_temperature_filtered_filter_struct;
extern FilterTypeDef mcu_temperature_filter_struct;
extern FilterTypeDef input_voltage_filterStruct;
extern FilterTypeDef current_filterStruct;
extern Hysteresis_FilterTypeDef thermocouple_temperature_filtered_hysteresis;

/* ADC1 DMA buffer accessors. */
uint32_t * sensors_adc1_buf(void);
uint32_t   sensors_adc1_buf_len(void);

/* Per-tick sensor readers. Each updates the matching field in sensor_values. */
void sensors_get_thermocouple_temperature(void);
void sensors_get_bus_voltage(void);
void sensors_get_heater_current(void);
void sensors_get_mcu_temp(void);

/* Sensor internals exposed to main.c. */
float sensors_get_raw_tc(void);                /* current raw TC value (post moving-average) */
void  sensors_reset_filtered_tc(float value);  /* preload the filtered-TC moving average */
void  sensors_start_adc_sample(void);          /* kick off one ADC1 DMA conversion */

/* True if no thermocouple sample is currently in flight (i.e. it's safe
 * to start a heater current-measurement pulse without colliding with
 * the next ADC1 conversion). */
bool sensors_thermocouple_sampling_idle(void);

/* ISR entry points. Call from main.c HAL callbacks after dispatching
 * by peripheral instance. */
void sensors_isr_thermocouple_window_start(void);  /* TIM6 fired: arm sample window */
void sensors_isr_thermocouple_complete(void);       /* ADC1 conversion complete */

#endif /* SENSORS_H */
