#include "sensors.h"
#include "heater.h"           // current_raw, heater_off, heater_update_pwm
#include "handle.h"           // enum handles, attached_handle
#include "state_machine.h"    // handle_delta_temperature (called from ADC1 complete)
#include "moving_average.h"   // Moving_Average_Compute / _Set_Value
#include "hysteresis.h"       // Hysteresis_Add
#include "tip_profile.h"      // tip_profiles_get_cal
#include "main.h"             // sensor_values, flash_values, GPIO macros
#include "util.h"             // clamp
#include <math.h>             // fabsf

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern ADC_HandleTypeDef hadc1;

/* --- Thermocouple state --- */
static float    TC_temp = 0;                  /* post-outlier-filter, post-moving-average raw value */
static float    TC_temp_from_ADC = 0;
static float    TC_temp_from_ADC_previous = 0;
static float    TC_temp_from_ADC_diff = 0;
static uint16_t TC_outliers_detected = 0;

/* --- Thermocouple sample-coordination flag ---
 * Set to 0 when TIM6 fires (window opens); set to 1 when the ADC1
 * conversion completes. main.c reads it via sensors_thermocouple_sampling_idle()
 * to gate the heater current-measurement pulse so the two ADCs don't
 * fire on top of each other. */
static volatile uint8_t thermocouple_measurement_done = 1;

/* --- Moving-average filters (file-scope; main.c initializes them) --- */
FilterTypeDef thermocouple_temperature_filter_struct;
FilterTypeDef thermocouple_temperature_filtered_filter_struct;
FilterTypeDef mcu_temperature_filter_struct;
FilterTypeDef input_voltage_filterStruct;
FilterTypeDef current_filterStruct;

/* --- Hysteresis filter --- */
Hysteresis_FilterTypeDef thermocouple_temperature_filtered_hysteresis;

/* --- TC compensation polynomials (per handle family) --- */
#define TC_OUTLIERS_THRESHOLD 300

#define TC_COMPENSATION_X2_T210 (4.223931712905644e-06f)
#define TC_COMPENSATION_X1_T210 0.31863796444354214f
#define TC_COMPENSATION_X0_T210 20.968033870812942f

#define TC_COMPENSATION_X2_T245 (-4.735112838956741e-07f)
#define TC_COMPENSATION_X1_T245 0.11936452029674384f
#define TC_COMPENSATION_X0_T245 23.777399955382318f

#define TC_COMPENSATION_X2_NT115 (5.1026665462522864e-05f)
#define TC_COMPENSATION_X1_NT115 0.42050803230712813f
#define TC_COMPENSATION_X0_NT115 20.14538589052425f

/* --- ADC scaling constants --- */
#define VOLTAGE_COMPENSATION 0.00840442388f
#define CURRENT_COMPENSATION 0.002864f

/* MCU internal temperature sensor */
#define V30        0.76f             /* from datasheet */
#define VSENSE     (3.3f / 4096.0f)
#define Avg_Slope  0.0025f           /* 2.5 mV / °C from datasheet */

/* --- ADC1 DMA buffer (3 channels, 19 samples each, interleaved) --- */
#define ADC1_BUF_LEN 57   /* 3 * 19 */
static uint16_t ADC1_BUF[ADC1_BUF_LEN];

/* ADC1 DMA buffer accessors (used by main.c to start the DMA). */
uint32_t * sensors_adc1_buf(void)   { return (uint32_t*)ADC1_BUF; }
uint32_t   sensors_adc1_buf_len(void) { return (uint32_t)ADC1_BUF_LEN; }

/**
 * @brief Returns the average value of 100 readings at index + 3*n in the ADC_buffer vector.
 * It is assumed that ADC1_BUF contains data from 3 channels, collected sequentially.
 * For example, if index == 0, elements 0, 3, 6, ... are used;
 * if index == 1, then 1, 4, 7, ... and so on.
 * @param index Channel index (0, 1, or 2)
 * @return Average value for that channel
 */
static float get_mean_ADC_reading_indexed(uint8_t index){
    if (index > 2) return 0.0f;  // Incorrect Index Protection

    float ADC_filter_mean = 0.0f;
    int count = 0;

    for (int n = index; n < ADC1_BUF_LEN; n += 3) {
        ADC_filter_mean += ADC1_BUF[n];
        count++;
    }

    return (count > 0) ? (ADC_filter_mean / (float)count) : 0.0f;
}

void sensors_get_thermocouple_temperature(){
	/* --- Step 1: Outlier filter on raw ADC data --- */
	TC_temp_from_ADC_previous = TC_temp_from_ADC;
	TC_temp_from_ADC = get_mean_ADC_reading_indexed(1);
	TC_temp_from_ADC_diff = fabsf(TC_temp_from_ADC_previous - TC_temp_from_ADC);

	if((TC_temp_from_ADC_diff > TC_OUTLIERS_THRESHOLD) && (TC_outliers_detected < 2)){
		TC_outliers_detected++;
		if(TC_outliers_detected < 2){
			TC_temp_from_ADC = TC_temp_from_ADC_previous;
		}
	}
	else{
		TC_outliers_detected = 0;
	}

	/* --- Step 2: Moving average filter --- */
	TC_temp = Moving_Average_Compute(TC_temp_from_ADC, &thermocouple_temperature_filter_struct); /* Moving average */

	/* --- Step 3: Apply handle-specific compensation polynomial --- */
	if(attached_handle == T210){
		sensor_values.thermocouple_temperature = TC_temp*TC_temp*TC_COMPENSATION_X2_T210 + TC_temp*TC_COMPENSATION_X1_T210 + TC_COMPENSATION_X0_T210;
	}
	else if(attached_handle == T245){
		sensor_values.thermocouple_temperature = TC_temp*TC_temp*TC_COMPENSATION_X2_T245 + TC_temp*TC_COMPENSATION_X1_T245 + TC_COMPENSATION_X0_T245;
	}
	else if(attached_handle == NT115){
		sensor_values.thermocouple_temperature = TC_temp*TC_temp*TC_COMPENSATION_X2_NT115 + TC_temp*TC_COMPENSATION_X1_NT115 + TC_COMPENSATION_X0_NT115;
	}
	else{
		/* Unknown handle - report temperature as setpoint so PID output is zero (heater off) */
		sensor_values.thermocouple_temperature = sensor_values.set_temperature;
	}

	/* --- Step 4: Adjust measured temperature to fit calibrated values from active tip profile */
	float cal_100 = tip_profiles_get_cal(attached_handle, 0);
	float cal_200 = tip_profiles_get_cal(attached_handle, 1);
	float cal_300 = tip_profiles_get_cal(attached_handle, 2);
	float cal_350 = tip_profiles_get_cal(attached_handle, 3);
	float cal_400 = tip_profiles_get_cal(attached_handle, 4);
	float cal_450 = tip_profiles_get_cal(attached_handle, 5);

	if(sensor_values.thermocouple_temperature < 100){
		sensor_values.thermocouple_temperature = sensor_values.thermocouple_temperature*(cal_100)/100.0f;
		}
	else if(sensor_values.thermocouple_temperature < 200){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 100.0f)*(cal_200-cal_100)/100.0f + cal_100;
		}
	else if(sensor_values.thermocouple_temperature < 300){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 200.0f)*(cal_300-cal_200)/100.0f + cal_200;
	}
	else if(sensor_values.thermocouple_temperature < 350){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 300.0f)*(cal_350-cal_300)/50.0f + cal_300;
		}
	else if(sensor_values.thermocouple_temperature < 400){
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 350.0f)*(cal_400-cal_350)/50.0f + cal_350;
		}
	else{
		sensor_values.thermocouple_temperature = (sensor_values.thermocouple_temperature - 400.0f)*(cal_450-cal_400)/50.0f + cal_400;
		}
	// Add temperature offset value
	sensor_values.thermocouple_temperature += flash_values.temperature_offset;
	// Clamp
	sensor_values.thermocouple_temperature = clamp(sensor_values.thermocouple_temperature ,0 ,500);

	sensor_values.thermocouple_temperature_filtered = Moving_Average_Compute(sensor_values.thermocouple_temperature, &thermocouple_temperature_filtered_filter_struct); // Moving average filter
	sensor_values.thermocouple_temperature_filtered = Hysteresis_Add(sensor_values.thermocouple_temperature_filtered, &thermocouple_temperature_filtered_hysteresis); // Hysteresis filter
}

/* Function to get the filtered MCU temperature */
void sensors_get_mcu_temp(void){
	sensor_values.mcu_temperature =	Moving_Average_Compute((((get_mean_ADC_reading_indexed(2) * VSENSE) - V30) / Avg_Slope + 25), &mcu_temperature_filter_struct); //Index 2 is MCU temp
}

/* Function to get the filtered bus voltage */
void sensors_get_bus_voltage(void){
	sensor_values.bus_voltage = Moving_Average_Compute(get_mean_ADC_reading_indexed(0), &input_voltage_filterStruct)*VOLTAGE_COMPENSATION; //Index 0 is bus voltage
}

/* Function to get the filtered heater current */
void sensors_get_heater_current(void){
	sensor_values.heater_current = Moving_Average_Compute(current_raw, &current_filterStruct)*CURRENT_COMPENSATION;
}

/* ---- Bridge API (used by main.c until further modules are extracted) ---- */

float sensors_get_raw_tc(void){
    return TC_temp;
}

void sensors_reset_filtered_tc(float value){
    Moving_Average_Set_Value(value, &thermocouple_temperature_filtered_filter_struct);
}

void sensors_start_adc_sample(void){
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_BUF, ADC1_BUF_LEN);
}

/* ---- Sample-coordination accessor ---- */

bool sensors_thermocouple_sampling_idle(void) {
    return thermocouple_measurement_done == 1;
}

/* ---- ISR entry points ---- */

/* TIM6 (25 ms tick) fires: open the sample window.
 *   - Mark the flag as "sampling in progress"
 *   - Turn the heater off so the thermocouple voltage isn't contaminated
 *     by the switching MOSFET
 *   - Enable TIM7; its 0.5 ms timeout will trigger the ADC1 conversion. */
void sensors_isr_thermocouple_window_start(void) {
    thermocouple_measurement_done = 0;
    heater_off();
    __HAL_TIM_ENABLE(&htim7);
}

/* ADC1 conversion done: read the sample, run PID, optionally detect
 * a temperature-jump fault. */
void sensors_isr_thermocouple_complete(void) {
    if (thermocouple_measurement_done != 0) return;   /* spurious / already handled */

    HAL_ADC_Stop_DMA(&hadc1);
    sensors_get_thermocouple_temperature();
    heater_update_pwm();
    thermocouple_measurement_done = 1;

    if (flash_values.delta_t_detection == 1) {
        handle_delta_temperature();
    }
}