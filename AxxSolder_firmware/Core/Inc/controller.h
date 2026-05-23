#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "pid.h"

/* The PID controller instance that drives the heater. Public so
 * handle.c can call tip_profiles_apply_pid(handle, &TPID) and
 * telemetry.c can read P/I/D contributions. */
extern PID_TypeDef TPID;

/* The PID's target temperature. Updated each controller_tick() based on
 * the current state machine state. Read by PID_Compute via pointer. */
extern float PID_setpoint;

/* Asymmetric I-term gain applied when the temperature error is negative
 * (under-target), to give the integrator more authority during heat-up
 * without overshooting on the high side. */
extern float PID_NEG_ERROR_I_MULT;
extern float PID_NEG_ERROR_I_BIAS;

/* Init the PID engine: pointers to sensor inputs and outputs, sample
 * time, output/I-term limits, asymmetric I-gain. */
void controller_init(void);

/* Per-main-loop tick. Runs PID_Compute(), then updates PID_setpoint
 * based on the current state machine state (RUN/PRESTANDBY -> set_temp,
 * STANDBY -> standby_temp clamp, SLEEP/HALTED/EMERGENCY_SLEEP -> 0). */
void controller_tick(void);

#endif /* CONTROLLER_H */
