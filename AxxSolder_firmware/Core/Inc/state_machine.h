#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>

/* Top-level runtime states. Stored in sensor_values.current_state. */
typedef enum {
    RUN,
    PRESTANDBY,
    STANDBY,
    SLEEP,
    EMERGENCY_SLEEP,
    HALTED
} mainstates;

/* State-entry timestamps. Set when the corresponding state transition
 * happens; read by the display countdown logic and the stand transition
 * logic to time the next transition. */
extern uint32_t previous_millis_left_stand;   /* RUN entry timestamp (for emergency_time safety) */
extern uint32_t previous_millis_standby;      /* STANDBY entry timestamp (for SLEEP transition) */
extern uint32_t previous_millis_prestandby;   /* PRESTANDBY entry timestamp (for STANDBY transition) */

/* Transition to a new state. Side effects:
 * - records previous state in sensor_values.previous_state
 * - sets sensor_values.current_state
 * - on entering RUN: writes startup temp to flash (if enabled), resets
 *   the beep-at-temp flag, raises GPIO4 output (if enabled).
 * - on leaving RUN: drops GPIO4 output. */
void change_state(mainstates new_state);

/* Safety check called from the main loop. Transitions to EMERGENCY_SLEEP
 * if any of these conditions are met:
 * - iron has been outside the stand longer than flash_values.emergency_time
 * - bus voltage drops below MIN_BUSVOLTAGE (also in PRESTANDBY/STANDBY)
 * - tip temperature exceeds EMERGENCY_SHUTDOWN_TEMPERATURE
 * - max available power drops below MIN_BUSPOWER
 * - no current draw (no tip detected) */
void handle_emergency_shutdown(void);

/* Detects an unphysical jump in TC reading (more than
 * MAX_TC_DELTA_FAULTDETECTION degC, still displaced two samples later), cuts
 * heater power and latches a fault that handle_emergency_shutdown() turns
 * into EMERGENCY_SLEEP from main context. Called from the ADC ISR on every
 * sample; the fault action only fires when delta_t_detection is on. */
void handle_delta_temperature(void);

/* True while a delta-T fault is latched but not yet consumed by
 * handle_emergency_shutdown(). controller_tick() and heater_update_pwm()
 * read it so power stays zero in the gap between the ISR latching the
 * fault and the main loop changing state. */
uint8_t delta_t_fault_is_pending(void);

#endif /* STATE_MACHINE_H */
