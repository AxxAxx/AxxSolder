#ifndef HANDLE_H
#define HANDLE_H

#include <stdint.h>
#include <stdbool.h>
#include "moving_average.h"

/* JBC handle families. The detector reads two binary GPIO inputs to
 * distinguish them; the value selects the right thermocouple
 * compensation polynomial (in sensors.c), PID gains (in tip_profile.c),
 * and max power limit (in this module).
 *
 * The enum order/values matter: they index tip-profile arrays and
 * are referenced by the per-family flash_values entries. */
enum handles {
    NT115,
    T210,
    T245,
    No_name
};

/* Whether a cartridge is currently inserted in the handle.
 * Set by handle_check_cartridge() based on heater current draw and
 * thermocouple ADC reading. */
typedef enum {
    ATTACHED,
    DETACHED
} cartridge_state_t;

/* Currently-detected handle type. Owned by handle.c, written by
 * handle_detect(). Read by sensors (polynomial selection), tip_profile,
 * UI, and state-machine code. */
extern enum handles attached_handle;

/* Current cartridge presence. Owned by handle.c, written by
 * handle_check_cartridge(). Read by display code to render "---" when
 * no tip is plugged in. */
extern cartridge_state_t cartridge_state;

/* Filter structs for handle1/2 sense GPIOs. File-scope so main.c
 * initializes them at boot. */
extern FilterTypeDef handle1_sense_filterStruct;
extern FilterTypeDef handle2_sense_filterStruct;

/* Read HANDLE_INP_1/2 GPIOs, filter, and update attached_handle.
 * Call every main-loop tick. */
void handle_detect(void);

/* Apply per-handle max power and PID gains to sensor_values / TPID,
 * factoring in USB-PD budget and any active tip profile's overrides.
 * Call after handle_detect(). */
void handle_apply_settings(void);

/* Detect cartridge presence (low heater current OR rail-high TC reading
 * means no tip is connected). On DETACHED, transitions to EMERGENCY_SLEEP.
 * On DETACHED -> ATTACHED, primes the TC filter and (after first boot)
 * may show the tip-profile selection popup. */
void handle_check_cartridge(void);

#endif /* HANDLE_H */
