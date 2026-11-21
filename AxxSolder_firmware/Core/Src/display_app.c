#include "display_app.h"
#include "app_colors.h"   /* AxxSolder UI colour palette */
#include "main.h"
#include "util.h"
#include "settings.h"
#include "handle.h"
#include "heater.h"
#include "controller.h"   /* controller_force_off */
#include "power_source.h"
#include "sensors.h"      /* sensors_get_raw_tc (debug screen) */
#include "version.h"      /* fw version + hw revision (debug screen) */
#include "buttons.h"      /* buttons_take_press (PD info page) */
#include "watchdog.h"     /* iwdg_refresh */
#include "fault_record.h" /* boot fault readout */
#include "lcd.h"
#include "gui.h"
#include "graph.h"
#include "moving_average.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define POPUP_DWELL_MS      2000

static uint32_t previous_millis_display = 0;
static uint32_t previous_millis_popup   = 0;

static char DISPLAY_buffer[40];

/* Set when the SLEEP/STANDBY label has been drawn into the bar so we
 * don't redraw the same overlay every tick. File-scope so callers
 * (buttons.c after preset save) can clear them directly. */
uint8_t sleep_state_written_to_LCD   = 0;
uint8_t standby_state_written_to_LCD = 0;

static uint8_t popup_shown = 0;

/* Filter for the smoothed power-bar reading. main.c initializes it. */
FilterTypeDef requested_power_filtered_filter_struct;

/* Return the temperature in the correct unit */
float convert_temperature(float temperature){
	if (flash_values.deg_celsius == 1){
		return temperature;
	}
	else{
		return ((temperature * 9) + 3) / 5 + 32;
	}
}

/* Formatting float into a string as right-aligned, by adding spaces on the left */
static void format_number_right(float input, char* buffer) {
    int rounded = (int)clamp(roundf(input), 0, 999);

    char number_str[4]; // Enough for "100" + '\0'
    sprintf(number_str, "%d", rounded);

    int num_digits = strlen(number_str);
    int padding_spaces = (3 - num_digits) * 2;

    // Fill padding spaces first
    memset(buffer, ' ', padding_spaces);
    // Copy the number after the padding
    strcpy(buffer + padding_spaces, number_str);

    // Null terminate the full string
    buffer[padding_spaces + num_digits] = '\0';
}

/* Formatting float into a string as left-aligned, by adding spaces on the right */
static void format_number_left(float input, char* buffer) {
    int rounded = (int)clamp(roundf(input), 0, 999);

    char number_str[4]; // "100" + '\0'
    sprintf(number_str, "%d", rounded);

    int num_digits = strlen(number_str);
    int padding_spaces = (3 - num_digits) * 2;

    // Copy number into buffer
    strcpy(buffer, number_str);

    // Add spaces to the right
    memset(buffer + num_digits, ' ', padding_spaces);
    buffer[num_digits + padding_spaces] = '\0';
}

/* Function to update the standby and sleep timer countdown */
static void update_standby_sleep_display(void) {
    uint32_t now = HAL_GetTick();
    uint32_t countdown_ms = 0;
    char label[16] = {0};
    char display_buffer[40]  = {0};
    // clean field
    if(sensor_values.current_state == SLEEP || sensor_values.current_state == RUN || sensor_values.current_state == HALTED || sensor_values.current_state == EMERGENCY_SLEEP){
    	LCD_PutStr(11, 215, "                            ", FONT_arial_17X18, C_Header, C_Background);
    }

    switch(sensor_values.current_state) {
        case PRESTANDBY:
            // Countdown to STANDBY
            countdown_ms = flash_values.standby_delay * 1000UL - (now - previous_millis_prestandby);
            if(countdown_ms > flash_values.standby_delay * 1000UL) countdown_ms = 0; // sanity
            strcpy(label, "To Standby");
            break;

        case STANDBY:
            // Countdown to SLEEP
            countdown_ms = flash_values.standby_time * 60000UL - (now - previous_millis_standby);
            if(countdown_ms > flash_values.standby_time * 60000UL) countdown_ms = 0; // sanity
            strcpy(label, "To Sleep");
            break;

        default:
            countdown_ms = 0;
            label[0] = '\0'; // no label
            break;
    }

    if(countdown_ms > 0 && label[0] != '\0') {
        uint32_t min = countdown_ms / 60000UL;
        uint32_t sec = (countdown_ms % 60000UL) / 1000UL;
        sprintf(display_buffer, "%s: %02lu:%02lu     ", label, min, sec);
    } else {
        strcpy(display_buffer, "                    "); // clear display or show nothing
    }
	LCD_PutStr(11, 215, display_buffer, FONT_arial_17X18, C_Header, C_Background);
}

/* ------------------------------------------------------------------ */
/* Graph view update                                                  */
/* ------------------------------------------------------------------ */

static void update_graph_display(void) {
    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(140, 45, DISPLAY_buffer, FONT_arial_20X23, C_Text, C_Background);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(140, 70, " ---  ", FONT_arial_20X23, C_Text, C_Background);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(140, 70, DISPLAY_buffer, FONT_arial_20X23, C_Text, C_Background);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(162, 95, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        if      (attached_handle == T210)  LCD_PutStr(75, 95, "T210  ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == T245)  LCD_PutStr(75, 95, "T245  ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == NT115) LCD_PutStr(75, 95, "NT115", FONT_arial_17X18, C_Text, C_Background);

        UG_DrawFrame(208, 44, 229, 109, C_Text);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(209, 45, 228, 108, C_Highlight);
            LCD_PutStr(214, 50, "Z", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(215, 69, "z", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(214, 90, "Z", FONT_arial_17X18, C_Background, C_Highlight);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(209, 45, 228, 108, C_Highlight);
            LCD_PutStr(213, 50, "S", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(213, 70, "T", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(213, 90, "B", FONT_arial_17X18, C_Background, C_Highlight);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            UG_FillFrame(209, 45, 228, 108, C_PowerBar);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
    else {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(140, 45, DISPLAY_buffer, FONT_arial_20X23, C_Text, C_Background);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(140, 70, " ---  ", FONT_arial_20X23, C_Text, C_Background);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(140, 70, DISPLAY_buffer, FONT_arial_20X23, C_Text, C_Background);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(162, 95, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        if      (attached_handle == T210)  LCD_PutStr(75, 95, "T210  ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == T245)  LCD_PutStr(75, 95, "T245  ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == NT115) LCD_PutStr(75, 95, "NT115", FONT_arial_17X18, C_Text, C_Background);

        UG_DrawFrame(288, 44, 310, 109, C_Text);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(290, 45, 309, 108, C_Highlight);
            LCD_PutStr(294, 50, "Z", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(295, 69, "z", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(294, 90, "Z", FONT_arial_17X18, C_Background, C_Highlight);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(290, 45, 309, 108, C_Highlight);
            LCD_PutStr(294, 50, "S", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(294, 70, "T", FONT_arial_17X18, C_Background, C_Highlight);
            LCD_PutStr(294, 90, "B", FONT_arial_17X18, C_Background, C_Highlight);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            UG_FillFrame(290, 45, 309, 108, C_PowerBar);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Debug screen                                                       */
/* ------------------------------------------------------------------ */

/* When flash_values.debug_screen is enabled the main screen (and graph
 * view) is replaced by a full-screen debug view: FONT_7X12 text, one
 * "label: value" row per line. It shows everything the main screen shows
 * (temperatures, handle, voltage, MCU temp, power source and power, presets,
 * standby/sleep countdown) plus the raw runtime state: the real state incl.
 * previous (so SLEEP and EMERGENCY_SLEEP can be told apart), the stand and
 * handle sense inputs, filtered + instant tip temperature, heater current
 * (filtered + raw ADC), leak current, raw thermocouple ADC, USB-PD power
 * limit and uptime. The title line carries the firmware version and
 * hardware revision.
 *
 * The row grid (12 px pitch, title at the top) fits all rows in both
 * orientations (last row ends at y = 240). Static labels are drawn once by
 * LCD_draw_debug_screen(); update_debug_screen() rewrites only the value
 * fields that changed since the last tick, so the screen costs the main
 * loop next to nothing while it idles. */

/* Text origin inset so the display bezel does not cover the top-left. */
#define DEBUG_LABEL_X    14                 /* label column X */
#define DEBUG_TITLE_Y    11                 /* title row Y */
#define DEBUG_ROW_Y(i)   (24 + 12 * (i))    /* first row Y 24 */
#define DEBUG_VALUE_X    98                 /* value column: label X + 12 chars * 7 px */
#define DEBUG_VALUE_LEN  16                 /* value field width: ends at x = 210 < 240 */

enum {
    DBG_ROW_STATE = 0,     /* current (previous) */
    DBG_ROW_SET_TEMP,
    DBG_ROW_TIP_TEMP,      /* filtered/instant */
    DBG_ROW_RAW_ADC,
    DBG_ROW_HANDLE,
    DBG_ROW_VOLTAGE,
    DBG_ROW_MCU_TEMP,
    DBG_ROW_PWR_SRC,
    DBG_ROW_PD_LIMIT,
    DBG_ROW_MAX_POWER,
    DBG_ROW_REQ_POWER,
    DBG_ROW_CURRENT,       /* filtered A + raw ADC */
    DBG_ROW_LEAK,
    DBG_ROW_IN_STAND,
    DBG_ROW_H1_H2,
    DBG_ROW_PRESETS,
    DBG_ROW_COUNTDOWN,
    DBG_ROW_UPTIME,
    DBG_ROW_COUNT
};

_Static_assert(DEBUG_ROW_Y(DBG_ROW_COUNT - 1) + 12 <= 240, "debug rows overflow the panel");

static const char *debug_labels[DBG_ROW_COUNT] = {
    "STATE:",
    "SET TEMP:",
    "TIP TEMP:",
    "RAW ADC:",
    "HANDLE:",
    "VOLTAGE:",
    "MCU TEMP:",
    "PWR SRC:",
    "PD LIMIT:",
    "MAX POWER:",
    "REQ POWER:",
    "CURRENT:",
    "LEAK ADC:",
    "IN STAND:",
    "H1/H2:",
    "PRESETS:",
    "COUNTDOWN:",
    "UPTIME:",
};

/* Last value drawn per row; a row is only repainted when its text changes. */
static char debug_last[DBG_ROW_COUNT][DEBUG_VALUE_LEN + 1];

static const char *debug_state_str(mainstates state) {
    switch (state) {
        case RUN:             return "RUN";
        case PRESTANDBY:      return "PSTBY";
        case STANDBY:         return "STBY";
        case SLEEP:           return "SLEEP";
        case EMERGENCY_SLEEP: return "EMSLP";
        case HALTED:          return "HALT";
        default:              return "?";
    }
}

static const char *debug_handle_str(void) {
    switch (attached_handle) {
        case T210:  return "T210";
        case T245:  return "T245";
        case NT115: return "NT115";
        default:    return "---";
    }
}

static void LCD_draw_debug_screen(void) {
    char title[32];
    uint8_t hw = get_hw_version();
    UG_FillScreen(C_Background);
    memset(debug_last, 0, sizeof(debug_last));   /* panel cleared: repaint all values */
    snprintf(title, sizeof(title), "AxxSolder %u.%u.%u HW%u.%u",
             fw_version_major, fw_version_minor, fw_version_patch, hw / 10u, hw % 10u);
    LCD_PutStr(DEBUG_LABEL_X, DEBUG_TITLE_Y, title, FONT_7X12, C_Text, C_Background);
    for (uint8_t i = 0; i < DBG_ROW_COUNT; i++) {
        LCD_PutStr(DEBUG_LABEL_X, DEBUG_ROW_Y(i), (char *)debug_labels[i], FONT_7X12, C_Text, C_Background);
    }
}

/* Pad to a fixed width so shorter values erase stale characters; skip the
 * (slow) LCD write when the row is unchanged. */
static void put_debug_value(uint8_t row, const char *value) {
    char field[DEBUG_VALUE_LEN + 1];
    snprintf(field, sizeof(field), "%-*.*s", DEBUG_VALUE_LEN, DEBUG_VALUE_LEN, value);
    if (strcmp(field, debug_last[row]) == 0) return;
    memcpy(debug_last[row], field, sizeof(field));
    LCD_PutStr(DEBUG_VALUE_X, DEBUG_ROW_Y(row), field, FONT_7X12, C_Text, C_Background);
}

static void update_debug_screen(void) {
    char val[20];
    char unit = (flash_values.deg_celsius == 1) ? 'C' : 'F';

    snprintf(val, sizeof(val), "%s (%s)",
             debug_state_str(sensor_values.current_state),
             debug_state_str(sensor_values.previous_state));
    put_debug_value(DBG_ROW_STATE, val);

    snprintf(val, sizeof(val), "%d %c",
             (int)convert_temperature(sensor_values.set_temperature), unit);
    put_debug_value(DBG_ROW_SET_TEMP, val);

    /* filtered/instant - a large spread indicates filter lag or noise */
    if (cartridge_state == DETACHED) {
        snprintf(val, sizeof(val), "---/%d %c",
                 (int)convert_temperature(sensor_values.thermocouple_temperature), unit);
    } else {
        snprintf(val, sizeof(val), "%d/%d %c",
                 (int)convert_temperature(sensor_values.thermocouple_temperature_filtered),
                 (int)convert_temperature(sensor_values.thermocouple_temperature), unit);
    }
    put_debug_value(DBG_ROW_TIP_TEMP, val);

    snprintf(val, sizeof(val), "%d", (int)sensors_get_raw_tc());
    put_debug_value(DBG_ROW_RAW_ADC, val);

    put_debug_value(DBG_ROW_HANDLE, debug_handle_str());

    snprintf(val, sizeof(val), "%d.%d V",
             (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
    put_debug_value(DBG_ROW_VOLTAGE, val);

    snprintf(val, sizeof(val), "%d %c",
             (int)convert_temperature(sensor_values.mcu_temperature), unit);
    put_debug_value(DBG_ROW_MCU_TEMP, val);

    switch (power_source) {
        case POWER_DC:  put_debug_value(DBG_ROW_PWR_SRC, "DC");  break;
        case POWER_USB: put_debug_value(DBG_ROW_PWR_SRC, "USB"); break;
        case POWER_BAT: put_debug_value(DBG_ROW_PWR_SRC, "BAT"); break;
        default:        put_debug_value(DBG_ROW_PWR_SRC, "?");   break;
    }

    /* PD limit defaults to FLT_MAX ("no limit"); casting that to int is UB.
     * The range check also rejects NaN. */
    float pd_limit = sensor_values.USB_PD_power_limit;
    if (pd_limit >= 0.0f && pd_limit < 9999.0f) {
        snprintf(val, sizeof(val), "%d W", (int)pd_limit);
    } else {
        snprintf(val, sizeof(val), "-");
    }
    put_debug_value(DBG_ROW_PD_LIMIT, val);

    snprintf(val, sizeof(val), "%d W", (int)sensor_values.max_power_watt);
    put_debug_value(DBG_ROW_MAX_POWER, val);

    float power_fraction = sensor_values.requested_power / PID_MAX_OUTPUT;
    snprintf(val, sizeof(val), "%d W %d%%",
             (int)(sensor_values.max_power_watt * power_fraction),
             (int)(power_fraction * 100.0f));
    put_debug_value(DBG_ROW_REQ_POWER, val);

    snprintf(val, sizeof(val), "%d.%dA r%u",
             (int)sensor_values.heater_current, (int)(sensor_values.heater_current * 10) % 10,
             current_raw);
    put_debug_value(DBG_ROW_CURRENT, val);

    snprintf(val, sizeof(val), "%u", sensor_values.leak_current);
    put_debug_value(DBG_ROW_LEAK, val);

    snprintf(val, sizeof(val), "%d.%d",
             (int)sensor_values.in_stand, (int)(sensor_values.in_stand * 10) % 10);
    put_debug_value(DBG_ROW_IN_STAND, val);

    snprintf(val, sizeof(val), "%d.%d %d.%d",
             (int)sensor_values.handle1_sense, (int)(sensor_values.handle1_sense * 10) % 10,
             (int)sensor_values.handle2_sense, (int)(sensor_values.handle2_sense * 10) % 10);
    put_debug_value(DBG_ROW_H1_H2, val);

    snprintf(val, sizeof(val), "%d %d",
             (int)convert_temperature(flash_values.preset_temp_1),
             (int)convert_temperature(flash_values.preset_temp_2));
    put_debug_value(DBG_ROW_PRESETS, val);

    /* All in uint32: the float settings times are converted once, so an
     * expired timer gives 0 instead of a negative float cast to unsigned. */
    uint32_t now = HAL_GetTick();
    uint32_t countdown_ms = 0;
    if (sensor_values.current_state == PRESTANDBY) {
        uint32_t total = (flash_values.standby_delay > 0.0f) ? (uint32_t)flash_values.standby_delay * 1000UL : 0;
        uint32_t elapsed = now - previous_millis_prestandby;
        countdown_ms = (elapsed < total) ? (total - elapsed) : 0;
        snprintf(val, sizeof(val), "STBY %02lu:%02lu",
                 countdown_ms / 60000UL, (countdown_ms % 60000UL) / 1000UL);
    } else if (sensor_values.current_state == STANDBY) {
        uint32_t total = (flash_values.standby_time > 0.0f) ? (uint32_t)flash_values.standby_time * 60000UL : 0;
        uint32_t elapsed = now - previous_millis_standby;
        countdown_ms = (elapsed < total) ? (total - elapsed) : 0;
        snprintf(val, sizeof(val), "SLEEP %02lu:%02lu",
                 countdown_ms / 60000UL, (countdown_ms % 60000UL) / 1000UL);
    } else {
        snprintf(val, sizeof(val), "-");
    }
    put_debug_value(DBG_ROW_COUNTDOWN, val);

    uint32_t up_s = now / 1000UL;
    snprintf(val, sizeof(val), "%lu:%02lu:%02lu",
             up_s / 3600UL, (up_s / 60UL) % 60UL, up_s % 60UL);
    put_debug_value(DBG_ROW_UPTIME, val);
}

/* ------------------------------------------------------------------ */
/* Main-screen update                                                 */
/* ------------------------------------------------------------------ */

static void update_display(void) {
    if (flash_values.debug_screen == 1) {
        update_debug_screen();
        return;
    }
    if (flash_values.screen_rotation == 0) {
        update_standby_sleep_display();
    }
    float filtered_power_percent = sensor_values.requested_power_filtered / PID_MAX_OUTPUT;
    float filtered_power_watt    = sensor_values.max_power_watt * filtered_power_percent;
    uint16_t bar_top = 0;

    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(19, 70, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_Text, C_Background);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(15, 160, " ---  ", FONT_arial_36X44_NUMBERS, C_Text, C_Background);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(19, 160, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_Text, C_Background);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(125, 255, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (convert_temperature(sensor_values.mcu_temperature) < 99.5f) {
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(sensor_values.mcu_temperature));
        } else {
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(sensor_values.mcu_temperature));
        }
        LCD_PutStr(59, 275, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        if      (attached_handle == T210)  LCD_PutStr(125, 235, "T210   ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == T245)  LCD_PutStr(125, 235, "T245   ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == NT115) LCD_PutStr(125, 235, "NT115", FONT_arial_17X18, C_Text, C_Background);

        if (sensor_values.max_power_watt < 100) {
            sprintf(DISPLAY_buffer, "  %d W", (int)sensor_values.max_power_watt);
        } else {
            sprintf(DISPLAY_buffer, "%d W", (int)sensor_values.max_power_watt);
        }
        LCD_PutStr(183, 45, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (flash_values.power_unit == 0) {
            format_number_right(filtered_power_watt, DISPLAY_buffer);
        } else {
            format_number_right(100 * filtered_power_percent, DISPLAY_buffer);
        }
        LCD_PutStr(186, 275, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(210, 66, 230, 268, C_Highlight);
            LCD_PutStr(214, 73,  "Z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(216, 99,  "z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(214, 129, "Z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(216, 158, "z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(214, 191, "Z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(216, 217, "z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(214, 247, "Z", FONT_arial_20X23, C_Background, C_Highlight);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(210, 66, 230, 268, C_Highlight);
            LCD_PutStr(213, 73,  "S", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(213, 102, "T", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(213, 131, "A", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(213, 160, "N", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(213, 189, "D", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(213, 218, "B", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(213, 247, "Y", FONT_arial_20X23, C_Background, C_Highlight);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            bar_top = 268 - (uint16_t)(filtered_power_percent * 202.0f);
            UG_FillFrame(210, bar_top, 230, 268,     C_PowerBar);
            UG_FillFrame(210, 66,      230, bar_top, C_Background);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
    else {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(64, 35, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_Text, C_Background);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(60, 115, " ---  ", FONT_arial_36X44_NUMBERS, C_Text, C_Background);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(64, 115, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_Text, C_Background);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(170, 195, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (convert_temperature(sensor_values.mcu_temperature) < 99.5f) {
            sprintf(DISPLAY_buffer, "  %d", (int)convert_temperature(sensor_values.mcu_temperature));
        } else {
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(sensor_values.mcu_temperature));
        }
        LCD_PutStr(102, 215, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        if      (attached_handle == T210)  LCD_PutStr(170, 175, "T210   ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == T245)  LCD_PutStr(170, 175, "T245   ", FONT_arial_17X18, C_Text, C_Background);
        else if (attached_handle == NT115) LCD_PutStr(170, 175, "NT115", FONT_arial_17X18, C_Text, C_Background);

        if (sensor_values.max_power_watt < 100) {
            sprintf(DISPLAY_buffer, "%d W  ", (int)sensor_values.max_power_watt);
        } else {
            sprintf(DISPLAY_buffer, "%d W", (int)sensor_values.max_power_watt);
        }
        LCD_PutStr(2, 10, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (flash_values.power_unit == 0) { // power unit is set to W
            format_number_right(filtered_power_watt, DISPLAY_buffer);
        } else {
            format_number_right(100 * filtered_power_percent, DISPLAY_buffer);
        }
        LCD_PutStr(5, 215, DISPLAY_buffer, FONT_arial_17X18, C_Text, C_Background);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(10, 32, 30, 209, C_Highlight);
            LCD_PutStr(14, 41,  "Z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(16, 68,  "z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 95,  "Z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(16, 122, "z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 149, "Z", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(16, 176, "z", FONT_arial_20X23, C_Background, C_Highlight);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(10, 32, 30, 209, C_Highlight);
            LCD_PutStr(14, 41,  "S", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 63,  "T", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 85,  "A", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 107, "N", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 129, "D", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 151, "B", FONT_arial_20X23, C_Background, C_Highlight);
            LCD_PutStr(14, 173, "Y", FONT_arial_20X23, C_Background, C_Highlight);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            bar_top = 209 - (uint16_t)(filtered_power_percent * 177.0f);
            UG_FillFrame(10, bar_top, 30, 209,     C_PowerBar);
            UG_FillFrame(10, 32,      30, bar_top, C_Background);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Main-screen layout (drawn on boot and after popup dismisses)       */
/* ------------------------------------------------------------------ */

void LCD_draw_main_screen(void) {
    if (flash_values.debug_screen == 1) {
        LCD_draw_debug_screen();
        return;
    }
    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        UG_FillScreen(C_Background);

        LCD_PutStr(53, 8, "AxxSolder", FONT_arial_19X22, C_Header, C_Background);
        LCD_DrawLine(0, 36, 240, 36, C_Header);
        LCD_DrawLine(0, 37, 240, 37, C_Header);
        LCD_DrawLine(0, 38, 240, 38, C_Header);

        LCD_PutStr(19, 45, "Set temp", FONT_arial_20X23, C_Text, C_Background);
        UG_DrawCircle(128, 76, 5, C_Text);
        UG_DrawCircle(128, 76, 4, C_Text);
        UG_DrawCircle(128, 76, 3, C_Text);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(135, 70, "C", FONT_arial_36X44_C, C_Text, C_Background);
        } else {
            LCD_PutStr(135, 70, "F", FONT_arial_36X44_F, C_Text, C_Background);
        }
        LCD_PutStr(19, 135, "Actual temp", FONT_arial_20X23, C_Text, C_Background);
        UG_DrawCircle(128, 166, 5, C_Text);
        UG_DrawCircle(128, 166, 4, C_Text);
        UG_DrawCircle(128, 166, 3, C_Text);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(135, 160, "C", FONT_arial_36X44_C, C_Text, C_Background);
        } else {
            LCD_PutStr(135, 160, "F", FONT_arial_36X44_F, C_Text, C_Background);
        }
        UG_DrawFrame(11, 129, 187, 211, C_Text);
        UG_DrawFrame(10, 128, 188, 212, C_Text);

        LCD_PutStr(11, 235, "Handle type:", FONT_arial_17X18, C_Text, C_Background);
        LCD_PutStr(11, 255, "Input voltage:         V", FONT_arial_17X18, C_Text, C_Background);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(11, 275, "MCU:     °C", FONT_arial_17X18, C_Text, C_Background);
        } else {
            LCD_PutStr(11, 275, "MCU:      °F", FONT_arial_17X18, C_Text, C_Background);
        }
        LCD_PutStr(105, 275, "SRC:", FONT_arial_17X18, C_Text, C_Background);
        switch (power_source) {
            case POWER_DC:  LCD_PutStr(150, 275, "DC",  FONT_arial_17X18, C_Text, C_Background); break;
            case POWER_USB: LCD_PutStr(150, 275, "USB", FONT_arial_17X18, C_Text, C_Background); break;
            case POWER_BAT: LCD_PutStr(150, 275, "BAT", FONT_arial_17X18, C_Text, C_Background); break;
        }

        if (flash_values.power_unit == 0) { // power unit is set to W
            LCD_PutStr(219, 275, "W", FONT_arial_17X18, C_Text, C_Background);
        } else {
            LCD_PutStr(219, 275, "%", FONT_arial_17X18, C_Text, C_Background);
        }

        UG_DrawLine(0, 296, 240, 296, C_Footer);
        UG_DrawLine(0, 297, 240, 297, C_Footer);

        if (flash_values.three_button_mode == 1) {
            LCD_PutStr(11, 301, "TEMP          UP   DOWN", FONT_arial_20X23, C_Footer, C_Background);
        } else {
            LCD_PutStr(11, 301, "PRESETS", FONT_arial_20X23, C_Footer, C_Background);
            memset(DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(flash_values.preset_temp_1));
            LCD_PutStr(130, 301, DISPLAY_buffer, FONT_arial_20X23, C_Footer, C_Background);
            memset(DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(flash_values.preset_temp_2));
            LCD_PutStr(190, 301, DISPLAY_buffer, FONT_arial_20X23, C_Footer, C_Background);
        }

        UG_DrawFrame(208, 64, 232, 270, C_Text);
        UG_DrawFrame(209, 65, 231, 269, C_Text);
    }
    else {
        UG_FillScreen(C_Background);
        LCD_PutStr(165, 10, "AxxSolder", FONT_arial_20X23, C_Header, C_Background);

        LCD_PutStr(64, 10, "Set temp", FONT_arial_20X23, C_Text, C_Background);
        UG_DrawCircle(173, 41, 5, C_Text);
        UG_DrawCircle(173, 41, 4, C_Text);
        UG_DrawCircle(173, 41, 3, C_Text);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(180, 35, "C", FONT_arial_36X44_C, C_Text, C_Background);
        } else {
            LCD_PutStr(180, 35, "F", FONT_arial_36X44_F, C_Text, C_Background);
        }
        LCD_PutStr(64, 90, "Actual temp", FONT_arial_20X23, C_Text, C_Background);
        UG_DrawCircle(173, 121, 5, C_Text);
        UG_DrawCircle(173, 121, 4, C_Text);
        UG_DrawCircle(173, 121, 3, C_Text);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(180, 115, "C", FONT_arial_36X44_C, C_Text, C_Background);
        } else {
            LCD_PutStr(180, 115, "F", FONT_arial_36X44_F, C_Text, C_Background);
        }

        UG_DrawFrame(56, 84, 232, 170, C_Text);
        UG_DrawFrame(56, 83, 233, 171, C_Text);

        LCD_PutStr(56, 175, "Handle type:", FONT_arial_17X18, C_Text, C_Background);
        LCD_PutStr(56, 195, "Input voltage:          V", FONT_arial_17X18, C_Text, C_Background);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(56, 215, "MCU:      °C", FONT_arial_17X18, C_Text, C_Background);
        } else {
            LCD_PutStr(56, 215, "MCU:       °F", FONT_arial_17X18, C_Text, C_Background);
        }
        LCD_PutStr(160, 215, "SRC:", FONT_arial_17X18, C_Text, C_Background);
        switch (power_source) {
            case POWER_DC:  LCD_PutStr(210, 215, "DC",  FONT_arial_17X18, C_Text, C_Background); break;
            case POWER_USB: LCD_PutStr(210, 215, "USB", FONT_arial_17X18, C_Text, C_Background); break;
            case POWER_BAT: LCD_PutStr(210, 215, "BAT", FONT_arial_17X18, C_Text, C_Background); break;
        }

        UG_DrawLine(265, 0, 265, 240, C_Footer);
        UG_DrawLine(266, 0, 266, 240, C_Footer);
        UG_DrawLine(315, 0, 315, 240, C_Footer);
        UG_DrawLine(316, 0, 316, 240, C_Footer);

        if (flash_values.three_button_mode == 1) {
            LCD_PutStr(269, 90, "INC", FONT_arial_20X23, C_Footer, C_Background);
            LCD_PutStr(269, 10, "DEC", FONT_arial_20X23, C_Footer, C_Background);
        } else {
            LCD_PutStr(285, 75,  "P", FONT_arial_20X23, C_Footer, C_Background);
            LCD_PutStr(285, 97,  "R", FONT_arial_20X23, C_Footer, C_Background);
            LCD_PutStr(285, 119, "E", FONT_arial_20X23, C_Footer, C_Background);
            LCD_PutStr(285, 141, "S", FONT_arial_20X23, C_Footer, C_Background);
            LCD_PutStr(285, 163, "E", FONT_arial_20X23, C_Footer, C_Background);
            LCD_PutStr(285, 185, "T", FONT_arial_20X23, C_Footer, C_Background);
            LCD_PutStr(285, 207, "S", FONT_arial_20X23, C_Footer, C_Background);
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)flash_values.preset_temp_1);
            LCD_PutStr(272, 40, DISPLAY_buffer, FONT_arial_20X23, C_Footer, C_Background);
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)flash_values.preset_temp_2);
            LCD_PutStr(272, 10, DISPLAY_buffer, FONT_arial_20X23, C_Footer, C_Background);
        }

        UG_DrawFrame(8, 30, 32, 212, C_Text);
        UG_DrawFrame(9, 31, 31, 211, C_Text);

        if (flash_values.power_unit == 0) { // power unit is set to W
            LCD_PutStr(37, 215, "W", FONT_arial_17X18, C_Text, C_Background);
        } else {
            LCD_PutStr(37, 215, "%", FONT_arial_17X18, C_Text, C_Background);
        }
    }

    /* Callers (buttons.c preset-save, show_popup) clear
     * sleep_state_written_to_LCD as needed to trigger overlay redraw. */
}

/* ------------------------------------------------------------------ */
/* Popup overlay                                                      */
/* ------------------------------------------------------------------ */

void show_popup(char *text) {
    UG_FillFrame(10, 50, 235, 105, C_Highlight);
    UG_FillFrame(15, 55, 230, 100, C_Text);
    LCD_PutStr(20, 70, text, FONT_arial_20X23, C_Highlight, C_Text);
    popup_shown = 1;
    previous_millis_popup = HAL_GetTick();
    standby_state_written_to_LCD = 0;
    sleep_state_written_to_LCD = 0;
}

void LCD_draw_earth_fault_popup(void) {
    /* Order matters: this runs in the ADC watchdog ISR (priority 10),
     * which the TIM6/ADC1 heater chain (priority 0) CAN preempt. Zero
     * the requested power first so a TC-complete ISR firing in between
     * cannot re-apply PWM from stale PID output. Then kill the heater at
     * register level (MOE off, pin forced low) before the slow popup draw. */
    controller_force_off();
    heater_fault_kill();

    UG_FillFrame(10, 50, 205, 205, C_Highlight);
    UG_FillFrame(15, 55, 200, 200, C_Text);
    LCD_PutStr(20, 60,  "GROUNDING",   FONT_arial_20X23, C_Highlight, C_Text);
    LCD_PutStr(20, 80,  "ERROR",       FONT_arial_20X23, C_Highlight, C_Text);

    LCD_PutStr(20, 120, "CHECK",       FONT_arial_20X23, C_Highlight, C_Text);
    LCD_PutStr(20, 140, "CONNECTIONS", FONT_arial_20X23, C_Highlight, C_Text);
    LCD_PutStr(20, 160, "AND REBOOT",  FONT_arial_20X23, C_Highlight, C_Text);

    Error_Handler();
}

/* ------------------------------------------------------------------ */
/* Fault record readout (boot)                                        */
/* ------------------------------------------------------------------ */

#define FAULT_SCREEN_MS 5000u

static uint8_t any_button_down(void) {
    return (HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET) ||
           (HAL_GPIO_ReadPin(SW_2_GPIO_Port, SW_2_Pin) == GPIO_PIN_SET) ||
           (HAL_GPIO_ReadPin(SW_3_GPIO_Port, SW_3_Pin) == GPIO_PIN_SET);
}

void show_fault_record_screen(void) {
    const volatile fault_record_t *r = fault_record_get();
    if (r == NULL) return;

    char line[32];
    UG_FillScreen(C_Background);
    LCD_PutStr(10, 10, "FAULT", FONT_arial_20X23, C_Highlight, C_Background);

    if (r->flags & FAULT_F_FRAME_OK) {
        snprintf(line, sizeof(line), "PC  0x%08lX", (unsigned long)r->pc);
        LCD_PutStr(10, 45, line, FONT_arial_17X18, C_Text, C_Background);
        snprintf(line, sizeof(line), "LR  0x%08lX", (unsigned long)r->lr);
        LCD_PutStr(10, 67, line, FONT_arial_17X18, C_Text, C_Background);
    } else {
        LCD_PutStr(10, 45, "PC  ? (bad SP)", FONT_arial_17X18, C_Text, C_Background);
        snprintf(line, sizeof(line), "SP  0x%08lX", (unsigned long)r->sp);
        LCD_PutStr(10, 67, line, FONT_arial_17X18, C_Text, C_Background);
    }
    snprintf(line, sizeof(line), "CFSR 0x%08lX", (unsigned long)r->cfsr);
    LCD_PutStr(10, 89, line, FONT_arial_17X18, C_Text, C_Background);
    snprintf(line, sizeof(line), "HFSR 0x%08lX", (unsigned long)r->hfsr);
    LCD_PutStr(10, 111, line, FONT_arial_17X18, C_Text, C_Background);
    snprintf(line, sizeof(line), "BFAR 0x%08lX", (unsigned long)r->bfar);
    LCD_PutStr(10, 133, line, FONT_arial_17X18, C_Text, C_Background);
    snprintf(line, sizeof(line), "Count %lu", (unsigned long)r->faults);
    LCD_PutStr(10, 155, line, FONT_arial_17X18, C_Text, C_Background);
    LCD_PutStr(10, 195, "Press a button", FONT_arial_17X18, C_Footer, C_Background);

    /* Boot only, before the watchdog runs; the heater is off (sleep). Hold
     * until a button or the timeout, and let go before the boot continues. */
    uint32_t t0 = HAL_GetTick();
    while (!any_button_down() && (HAL_GetTick() - t0) < FAULT_SCREEN_MS) { HAL_Delay(10); }
    while (any_button_down() && (HAL_GetTick() - t0) < 2u * FAULT_SCREEN_MS) { HAL_Delay(10); }
    HAL_Delay(50);

    fault_record_clear();
    buttons_clear_all();
    UG_FillScreen(C_Background);
}

/* ------------------------------------------------------------------ */
/* PD information page (menu-invoked)                                 */
/* ------------------------------------------------------------------ */

#define PD_LINE_X       14
#define PD_ROW_Y(i)     (13 + 13 * (i))

/* Draw one text line at row `row`, right-padded so a shorter value fully
 * overwrites a previous longer one (needed for the live refresh). 31 chars
 * x 7 px + 14 px margin = 231 px, fits the 240 px short side. */
static void pd_put_line(uint8_t row, const char *text) {
    char padded[40];
    snprintf(padded, sizeof(padded), "%-31.31s", text);
    LCD_PutStr(PD_LINE_X, PD_ROW_Y(row), padded, FONT_7X12, C_Text, C_Background);
}

/* "20.0" from mV, rounded to 0.1 V. */
static void pd_fmt_v(char *buf, size_t len, uint16_t mv) {
    unsigned d = (mv + 50u) / 100u;
    snprintf(buf, len, "%u.%u", d / 10u, d % 10u);
}

/* "2.25" from mA, rounded to 10 mA (RDO/PDO currents are 10 mA steps). */
static void pd_fmt_a(char *buf, size_t len, uint16_t ma) {
    unsigned c = (ma + 5u) / 10u;
    snprintf(buf, len, "%u.%02u", c / 100u, c % 100u);
}

/* Compact current for the OFFER row ("3", "1.5", "2.25") so three
 * offered PDOs fit in 31 columns. */
static void pd_fmt_a_short(char *buf, size_t len, uint16_t ma) {
    unsigned c = (ma + 5u) / 10u;
    if (c % 100u == 0)     snprintf(buf, len, "%u", c / 100u);
    else if (c % 10u == 0) snprintf(buf, len, "%u.%u", c / 100u, (c % 100u) / 10u);
    else                   snprintf(buf, len, "%u.%02u", c / 100u, c % 100u);
}

/* Negotiation mode, <= 23 chars after "MODE:   ". */
static const char *pd_mode_str(pd_mode_t mode) {
    switch (mode) {
        case PD_MODE_AUTO:        return "auto (full current)";
        case PD_MODE_NVM_PENDING: return "ladder, NVM pending";
        case PD_MODE_LADDER:      return "ladder (NVM off)";
        case PD_MODE_NVM_ERROR:   return "ladder, NVM error";
        default:                  return "no PD";
    }
}

void show_pd_info_screen(void) {
    const pd_status_t *pd = power_source_get_pd_status();
    char line[40];
    char v[8], a[8];

    /* Consume the encoder click that opened this page (wait for release),
     * then drop the press it latched so it can't count as an exit click. */
    while (HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET) { iwdg_refresh(); }
    HAL_Delay(50);
    buttons_clear_all();

    /* The menu draws transparent text; the padded repaint below needs an
     * opaque background or the live rows smear. Restored on exit. */
    UG_U8 old_transparency = UG_FontGetTransparency();
    UG_FontSetTransparency(0);

    UG_FillScreen(C_Background);

    /* Repaint at ~5 Hz until the encoder is clicked. Static fields repaint
     * over themselves (no flicker); the LIVE block updates each pass. The
     * heater is already off in menu context, so blocking here is safe. */
    for (;;) {
        iwdg_refresh();   /* PD info page blocks the main loop */
        pd_live_t live;
        power_source_read_live(&live);

        uint8_t row = 0;
        pd_put_line(row++, "USB-PD INFORMATION");

        if (!pd->probed) {
            /* Menu opened during boot, before negotiation ran. */
            pd_put_line(row++, "PD: not probed");
        } else {
            snprintf(line, sizeof(line), "CTRL:   %s",
                     (pd->controller == PD_CTRL_STUSB4500) ? "STUSB4500" : "none");
            pd_put_line(row++, line);

            const char *src = (power_source == POWER_USB) ? "USB-PD" :
                              (power_source == POWER_BAT) ? "BAT"    : "DC";
            snprintf(line, sizeof(line), "SOURCE: %s", src);
            pd_put_line(row++, line);

            if (pd->controller == PD_CTRL_STUSB4500) {
                snprintf(line, sizeof(line), "USB-C:  %s", pd->sink_connected ? "attached" : "none");
                pd_put_line(row++, line);
            }

            if (pd->contract) {
                pd_fmt_a(a, sizeof(a), pd->neg_current_ma);
                if (pd->contract_mv) {
                    pd_fmt_v(v, sizeof(v), pd->contract_mv);
                    snprintf(line, sizeof(line), "CONTRACT:%sV %sA %uW", v, a, pd->neg_watts);
                } else {
                    snprintf(line, sizeof(line), "CONTRACT:%sA %uW", a, pd->neg_watts);
                }
            } else {
                snprintf(line, sizeof(line), "CONTRACT: %s", pd->sink_connected ? "none" : "no PD");
            }
            pd_put_line(row++, line);

            /* USB_PD_power_limit is FLT_MAX ("no limit") when not on USB-PD;
             * range-check before the cast (also rejects NaN). */
            float pd_lim = sensor_values.USB_PD_power_limit;
            if ((power_source == POWER_USB) && (pd_lim >= 0.0f) && (pd_lim < 1000.0f)) {
                snprintf(line, sizeof(line), "PWR LIM: %u W", (unsigned)pd_lim);
            } else {
                snprintf(line, sizeof(line), "PWR LIM: none");
            }
            pd_put_line(row++, line);

            if (pd->controller == PD_CTRL_STUSB4500) {
                snprintf(line, sizeof(line), "MODE:   %s", pd_mode_str(pd->mode));
                pd_put_line(row++, line);
                if (pd->mode == PD_MODE_NVM_PENDING) {
                    pd_put_line(row++, "Replug USB for full power");
                }
            }

            if (pd->controller == PD_CTRL_STUSB4500 && pd->sink_connected) {
                snprintf(line, sizeof(line), "SRC PDO#:%u MISMATCH:%s",
                         pd->object_pos, pd->capa_mismatch ? "YES" : "no");
                pd_put_line(row++, line);

                pd_fmt_a(a, sizeof(a), pd->rdo_max_current_ma);
                snprintf(line, sizeof(line), "MAX I:%sA COMM:%c GB:%c", a,
                         pd->usb_comm ? 'y' : 'n', pd->giveback ? 'y' : 'n');
                pd_put_line(row++, line);

                if (pd->req_pdo_count) {
                    char req[40];
                    int n = snprintf(req, sizeof(req), "OFFER:");
                    for (uint8_t i = 0; i < pd->req_pdo_count && n > 0 && n < (int)sizeof(req); i++) {
                        pd_fmt_a_short(a, sizeof(a), pd->req_pdo_ma[i]);
                        n += snprintf(req + n, sizeof(req) - n, " %uV/%sA",
                                      pd->req_pdo_mv[i] / 1000, a);
                    }
                    pd_put_line(row++, req);
                }
            }
        }

        pd_put_line(row++, "-- LIVE --");
        pd_fmt_v(v, sizeof(v), live.vbus_mv);
        snprintf(line, sizeof(line), "VBUS: %s V", v);
        pd_put_line(row++, line);

        pd_put_line(row + 1, "Click to return");

        /* Poll the encoder for ~200 ms; exit on a click (SW_1 == SET), or on
         * a short click the ISR latched while the page was repainting. */
        uint8_t clicked = buttons_take_press(BTN_1) ? 1 : 0;
        for (uint8_t t = 0; t < 20 && !clicked; t++) {
            if (HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET) { clicked = 1; break; }
            HAL_Delay(10);
        }
        if (clicked) { break; }
    }

    /* Wait for release so the click doesn't re-trigger the menu. */
    while (HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET) { iwdg_refresh(); }
    HAL_Delay(50);

    /* The menu repaint only clears its own list area, which in landscape
     * ends above the lower rows of this page: clear the whole panel. */
    UG_FillScreen(C_Background);
    UG_FontSetTransparency(old_transparency);
}

/* ------------------------------------------------------------------ */
/* Tick                                                               */
/* ------------------------------------------------------------------ */

void display_redraw_view(void){
	/* Debug screen takes priority over the graph view */
	if (flash_values.display_graph == 1 && flash_values.debug_screen == 0){
		UG_FillScreen(background);
		draw_graph_init();
	}
	else{
		LCD_draw_main_screen();
	}
}

void display_app_tick(void){
	if(popup_shown == 1){
		if(HAL_GetTick() - previous_millis_popup >= POPUP_DWELL_MS){
			popup_shown = 0;
			display_redraw_view(); // Clear the area where the popup was displayed
		}
	}
	else{
		/* Update display */
		if(HAL_GetTick() - previous_millis_display >= DISPLAY_TICK_INTERVAL_MS){
			sensor_values.requested_power_filtered = clamp(Moving_Average_Compute(sensor_values.requested_power, &requested_power_filtered_filter_struct), 0.0f, PID_MAX_OUTPUT);

			if (flash_values.display_graph == 0 || flash_values.debug_screen == 1){
				update_display();
			}
			else{
				float graf_percent = sensor_values.requested_power_filtered / (PID_MAX_OUTPUT/100);

				add_data_point((uint16_t) convert_temperature(sensor_values.thermocouple_temperature_filtered), (uint16_t) graf_percent, (uint16_t) sensor_values.set_temperature);
				draw_graph_update();
				update_graph_display();
			}

			previous_millis_display = HAL_GetTick();
		}
	}
}
