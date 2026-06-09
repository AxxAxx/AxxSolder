#include "power_source.h"
#include "main.h"        /* sensor_values */
#include "stusb4500.h"
#include "debug.h"
#include <stdbool.h>

/* USB-PD power limit safety margin. The STUSB4500 returns the source's
 * advertised maximum; multiplying by this factor produces the value we
 * actually allow the heater to draw, giving headroom for inrush and
 * cable losses. Set to 1.0 = "trust the source's number." */
#define USB_PD_POWER_REDUCTION_FACTOR 1.0f

/* --- Public state --- */
power_source_t power_source = POWER_DC;

/* check STUSB4500 */
void power_source_init(void){
	HAL_StatusTypeDef halStatus;
	halStatus  = stusb_check_connection();
	if(halStatus != HAL_OK){
		//do error handling for STUSB
		debug_print_str(DEBUG_ERROR,"STUSB4500 unavailable");
	}else{
		debug_print_str(DEBUG_INFO,"STUSB4500 found");

		stusb_init();

		//1. check if cable is connected
		if(stusb_is_sink_connected()){

			//2. wait for sink to get ready (with timeout to prevent infinite hang)
			uint32_t sink_ready_timeout = HAL_GetTick() + 3000;
			while(!stusb_is_sink_ready()){
				debug_print_str(DEBUG_INFO,"Waiting for sink to get ready");
				if(HAL_GetTick() > sink_ready_timeout){
					debug_print_str(DEBUG_ERROR,"Sink ready timeout");
					break;
				}
			}
			//if we are on USB-PD the sink needs some time to start
			HAL_Delay(500);

			stusb_soft_reset();

			//check if USB-PD is available
			STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef rdo;
			halStatus = stusb_read_rdo(&rdo);
			volatile uint8_t currendPdoIndex = rdo.b.Object_Pos;
			if(currendPdoIndex == 0){
				debug_print_str(DEBUG_INFO,"No USB-PD detected");
			}else{
				power_source = POWER_USB;
				//The usb devices need some time to transmit the messages and execute the soft reset
				//HAL_Delay(4);
				//poll alert status since we don't have the alert interrupt pin connected
				//depending on the source we may need a few tries
				bool sourceStatus = false;
				for(int i=0;i<500;i++){
					sourceStatus = poll_source();
					if(sourceStatus){
						debug_print_str(DEBUG_INFO,"Got PDOs");
						uint8_t maxPowerAvailable = 0;
						stusb_set_highest_pdo(&maxPowerAvailable, currendPdoIndex);

						sensor_values.USB_PD_power_limit = maxPowerAvailable*USB_PD_POWER_REDUCTION_FACTOR;
						debug_print_int(DEBUG_INFO,"Reduced max power to", maxPowerAvailable*USB_PD_POWER_REDUCTION_FACTOR);
						//re-negotiate
						break;
					}
				}
			}
		}else{
			debug_print_str(DEBUG_INFO,"No USB-PD sink connected");
		}
	}
}
