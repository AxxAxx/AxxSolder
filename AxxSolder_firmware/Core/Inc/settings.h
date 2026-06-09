#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

/* User settings stored in non-volatile flash.
 *
 * Loaded on boot via settings_init() and persisted whenever a field
 * changes via settings_save(). Fields are all `float` for historical
 * reasons (the original design used a single getter pattern that
 * returned float regardless of underlying type). */
typedef struct {
    float startup_temperature;
    float temperature_offset;
    float standby_temp;
    float standby_time;
    float emergency_time;
    float buzzer_enabled;
    float preset_temp_1;
    float preset_temp_2;
    float GPIO4_ON_at_run;
    float screen_rotation;
    float momentary_stand;
    float current_measurement;
    float startup_beep;
    float deg_celsius;
    float temp_cal_100;
    float temp_cal_200;
    float temp_cal_300;
    float temp_cal_350;
    float temp_cal_400;
    float temp_cal_450;
    float serial_debug_print;
    float displayed_temp_filter;
    float startup_temp_is_previous_temp;
    float three_button_mode;
    float beep_at_set_temp;
    float beep_tone;
    float power_unit;
    float detect_nt115;
    float power_limit_T245;
    float power_limit_T210;
    float power_limit_NT115;
    float power_limit_No_name;
    float display_graph;
    float delta_t_detection;
    float standby_delay;
    float show_profile_on_tip_change;
    float change_enc_dir;
    float encoder_step_idx;   /* index into encoder step table {1,2,5,10} */
} Flash_values;

/* The single live copy of user settings. Read directly from anywhere
 * that needs a value; mutate from settings menu / preset buttons / etc.
 * then call settings_save() to persist. */
extern Flash_values flash_values;

/* Default values, used by the settings menu to:
 *   1. show "default: X" next to each field
 *   2. handle the "Load Defaults" menu action (memcpy into flash_values)
 * Non-const to match the original (which exposed it as a writable global). */
extern Flash_values default_flash_values;

/* Set while the settings menu is being navigated. Various modules
 * check this to suppress side-effects (popup, redraw) while the user
 * is interacting with the menu. */
extern uint8_t settings_menu_active;

/* Init: load values from flash, writing defaults (and chirping the
 * buzzer + HAL_Delay) if the flash region fails its validity check.
 *
 * Call once on boot, AFTER the flash storage driver is ready and the
 * buzzer peripheral is initialized. */
void settings_init(void);

/* Persist the current flash_values to non-volatile storage. Call after
 * mutating any field that should survive reboot. */
void settings_save(void);

#endif /* SETTINGS_H */
