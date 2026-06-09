#include "controller.h"
#include "main.h"            /* sensor_values, PID_MAX_OUTPUT, PID_UPDATE_INTERVAL, PID_ADD_I_MIN_ERROR */
#include "settings.h"        /* flash_values.standby_temp */
#include "state_machine.h"   /* mainstates: RUN, PRESTANDBY, STANDBY, SLEEP, EMERGENCY_SLEEP, HALTED */

/* PID instance */
PID_TypeDef TPID;
float PID_setpoint = 0.0f;

float PID_NEG_ERROR_I_MULT = 7;
float PID_NEG_ERROR_I_BIAS = 1;

/* Custom tuning parameters */
float Kp_tuning = 0;
float Ki_tuning = 0;
float Kd_tuning = 0;
float temperature_tuning = 100;
float PID_MAX_I_LIMIT_tuning = 0;

/* Allow use of custom temperature, used for tuning */
uint8_t custom_temperature_on = 0;

/* ------------------------------------------------------------------ */
/* Init                                                               */
/* ------------------------------------------------------------------ */

void controller_init(void) {
	/* Initiate PID controller */
	PID(&TPID, &sensor_values.thermocouple_temperature, &sensor_values.requested_power, &PID_setpoint, 0, 0, 0, _PID_CD_DIRECT); //PID parameters are set depending on detected handle by set_handle_values()
	PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);
	PID_SetSampleTime(&TPID, PID_UPDATE_INTERVAL, 0); 		// Set PID update time to "PID_UPDATE_INTERVAL"
	PID_SetOutputLimits(&TPID, 0, PID_MAX_OUTPUT); 			// Set max and min output limit
	PID_SetILimits(&TPID, 0, 0);         					// Set max and min I limit
	PID_SetIminError(&TPID,PID_ADD_I_MIN_ERROR);			// Set I min Error
	PID_SetNegativeErrorIgainMult(&TPID, PID_NEG_ERROR_I_MULT, PID_NEG_ERROR_I_BIAS); // Set un-symmetric I gain parameters
}

/* ------------------------------------------------------------------ */
/* Tick                                                               */
/* ------------------------------------------------------------------ */

void controller_tick(void) {
	/* Compute PID */
	PID_Compute(&TPID);

	/* switch */
	switch(sensor_values.current_state) {
		case RUN:
		case PRESTANDBY: {
			PID_setpoint = sensor_values.set_temperature;
			break;
		}
		case STANDBY: {
		  if(flash_values.standby_temp > sensor_values.set_temperature){
			PID_setpoint = sensor_values.set_temperature;
		  }
		  else{
			PID_setpoint = flash_values.standby_temp;
		  }
		  break;
		}
		case SLEEP:
		case EMERGENCY_SLEEP:
		case HALTED: {
			PID_setpoint = 0;
			break;
		}
	}

	/* PID Tuning manual control */
	#ifdef PID_TUNING
	custom_temperature_on = 1;
	PID_SetTunings(&TPID, Kp_tuning, Ki_tuning, Kd_tuning/100.0f);
	PID_SetILimits(&TPID, -PID_MAX_I_LIMIT_tuning, PID_MAX_I_LIMIT_tuning); 	// Set max and min I limit
	sensor_values.set_temperature = temperature_tuning;
	#endif
}
