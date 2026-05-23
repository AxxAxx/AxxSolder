#include "telemetry.h"
#include "main.h"            /* sensor_values, PID_MAX_OUTPUT */
#include "settings.h"        /* flash_values.serial_debug_print */
#include "controller.h"      /* TPID (for PID_Get*part), PID_setpoint */
#include "pid.h"             /* PID_GetPpart / Ipart / Dpart */
#include "type_packers.h"    /* pack_frame_start, pack_float */

/* CubeMX-owned UART handle; defined in main.c. */
extern UART_HandleTypeDef huart1;

/* UART send buffers */
#define MAX_BUFFER_LEN 250
uint8_t UART_transmit_buffer[MAX_BUFFER_LEN];
uint8_t UART_packet_index = 0;
uint8_t UART_packet_length = 0;

static uint32_t previous_millis_debug = 0;
static uint32_t interval_debug = 50;

/* Send debug information */
void telemetry_tick(void){
	if(flash_values.serial_debug_print == 1){
		if(HAL_GetTick() - previous_millis_debug >= interval_debug){
			UART_packet_length = 9*sizeof(float);
			pack_frame_start(UART_transmit_buffer, &UART_packet_index, UART_packet_length);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.thermocouple_temperature);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.thermocouple_temperature_filtered);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_setpoint);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.requested_power/PID_MAX_OUTPUT*100.0f);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.requested_power_filtered/PID_MAX_OUTPUT*100.0f);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_GetPpart(&TPID)/10.0f);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_GetIpart(&TPID)/10.0f);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)PID_GetDpart(&TPID)/10.0f);
			pack_float(UART_transmit_buffer, &UART_packet_index, (float)sensor_values.heater_current);

			HAL_UART_Transmit_DMA(&huart1,(uint8_t*)UART_transmit_buffer, UART_packet_length+2); // Add two for starting bit and packet length
			previous_millis_debug = HAL_GetTick();
		}
	}
}
