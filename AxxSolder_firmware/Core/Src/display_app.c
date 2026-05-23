#include "display_app.h"
#include "main.h"
#include "util.h"
#include "settings.h"
#include "handle.h"
#include "heater.h"
#include "power_source.h"
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
    	LCD_PutStr(11, 215, "                            ", FONT_arial_17X18, C_YELLOW, C_BLACK);
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
	LCD_PutStr(11, 215, display_buffer, FONT_arial_17X18, C_YELLOW, C_BLACK);
}

/* ------------------------------------------------------------------ */
/* Graph view update                                                  */
/* ------------------------------------------------------------------ */

static void update_graph_display(void) {
    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(140, 45, DISPLAY_buffer, FONT_arial_20X23, C_WHITE, C_BLACK);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(140, 70, " ---  ", FONT_arial_20X23, C_WHITE, C_BLACK);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(140, 70, DISPLAY_buffer, FONT_arial_20X23, C_WHITE, C_BLACK);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(162, 95, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        if      (attached_handle == T210)  LCD_PutStr(75, 95, "T210  ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == T245)  LCD_PutStr(75, 95, "T245  ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == NT115) LCD_PutStr(75, 95, "NT115", FONT_arial_17X18, C_WHITE, C_BLACK);

        UG_DrawFrame(208, 44, 229, 109, C_WHITE);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(209, 45, 228, 108, C_ORANGE);
            LCD_PutStr(214, 50, "Z", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(215, 69, "z", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(214, 90, "Z", FONT_arial_17X18, C_BLACK, C_ORANGE);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(209, 45, 228, 108, C_ORANGE);
            LCD_PutStr(213, 50, "S", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 70, "T", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 90, "B", FONT_arial_17X18, C_BLACK, C_ORANGE);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            UG_FillFrame(209, 45, 228, 108, C_LIGHT_SKY_BLUE);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
    else {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(140, 45, DISPLAY_buffer, FONT_arial_20X23, C_WHITE, C_BLACK);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(140, 70, " ---  ", FONT_arial_20X23, C_WHITE, C_BLACK);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(140, 70, DISPLAY_buffer, FONT_arial_20X23, C_WHITE, C_BLACK);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(162, 95, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        if      (attached_handle == T210)  LCD_PutStr(75, 95, "T210  ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == T245)  LCD_PutStr(75, 95, "T245  ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == NT115) LCD_PutStr(75, 95, "NT115", FONT_arial_17X18, C_WHITE, C_BLACK);

        UG_DrawFrame(288, 44, 310, 109, C_WHITE);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(290, 45, 309, 108, C_ORANGE);
            LCD_PutStr(294, 50, "Z", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(295, 69, "z", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(294, 90, "Z", FONT_arial_17X18, C_BLACK, C_ORANGE);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(290, 45, 309, 108, C_ORANGE);
            LCD_PutStr(294, 50, "S", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(294, 70, "T", FONT_arial_17X18, C_BLACK, C_ORANGE);
            LCD_PutStr(294, 90, "B", FONT_arial_17X18, C_BLACK, C_ORANGE);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            UG_FillFrame(290, 45, 309, 108, C_LIGHT_SKY_BLUE);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Main-screen update                                                 */
/* ------------------------------------------------------------------ */

static void update_display(void) {
    if (flash_values.screen_rotation == 0) {
        update_standby_sleep_display();
    }
    float filtered_power_percent = sensor_values.requested_power_filtered / PID_MAX_OUTPUT;
    float filtered_power_watt    = sensor_values.max_power_watt * filtered_power_percent;
    uint16_t bar_top = 0;

    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(19, 70, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_WHITE, C_BLACK);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(15, 160, " ---  ", FONT_arial_36X44_NUMBERS, C_WHITE, C_BLACK);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(19, 160, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_WHITE, C_BLACK);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(125, 255, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (convert_temperature(sensor_values.mcu_temperature) < 99.5f) {
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(sensor_values.mcu_temperature));
        } else {
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(sensor_values.mcu_temperature));
        }
        LCD_PutStr(59, 275, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        if      (attached_handle == T210)  LCD_PutStr(125, 235, "T210   ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == T245)  LCD_PutStr(125, 235, "T245   ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == NT115) LCD_PutStr(125, 235, "NT115", FONT_arial_17X18, C_WHITE, C_BLACK);

        if (sensor_values.max_power_watt < 100) {
            sprintf(DISPLAY_buffer, "  %d W", (int)sensor_values.max_power_watt);
        } else {
            sprintf(DISPLAY_buffer, "%d W", (int)sensor_values.max_power_watt);
        }
        LCD_PutStr(183, 45, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (flash_values.power_unit == 0) {
            format_number_right(filtered_power_watt, DISPLAY_buffer);
        } else {
            format_number_right(100 * filtered_power_percent, DISPLAY_buffer);
        }
        LCD_PutStr(186, 275, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(210, 66, 230, 268, C_ORANGE);
            LCD_PutStr(214, 73,  "Z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(216, 99,  "z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(214, 129, "Z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(216, 158, "z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(214, 191, "Z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(216, 217, "z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(214, 247, "Z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(210, 66, 230, 268, C_ORANGE);
            LCD_PutStr(213, 73,  "S", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 102, "T", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 131, "A", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 160, "N", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 189, "D", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 218, "B", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(213, 247, "Y", FONT_arial_20X23, C_BLACK, C_ORANGE);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            bar_top = 268 - (uint16_t)(filtered_power_percent * 202.0f);
            UG_FillFrame(210, bar_top, 230, 268,     C_LIGHT_SKY_BLUE);
            UG_FillFrame(210, 66,      230, bar_top, C_BLACK);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
    else {
        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        format_number_left(convert_temperature(sensor_values.set_temperature), DISPLAY_buffer);
        LCD_PutStr(64, 35, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_WHITE, C_BLACK);

        if (cartridge_state == DETACHED) {
            LCD_PutStr(60, 115, " ---  ", FONT_arial_36X44_NUMBERS, C_WHITE, C_BLACK);
        } else {
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            format_number_left(convert_temperature(sensor_values.thermocouple_temperature_filtered), DISPLAY_buffer);
            LCD_PutStr(64, 115, DISPLAY_buffer, FONT_arial_36X44_NUMBERS, C_WHITE, C_BLACK);
        }

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        sprintf(DISPLAY_buffer, "%d.%d", (int)sensor_values.bus_voltage, (int)(sensor_values.bus_voltage * 10) % 10);
        LCD_PutStr(170, 195, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (convert_temperature(sensor_values.mcu_temperature) < 99.5f) {
            sprintf(DISPLAY_buffer, "  %d", (int)convert_temperature(sensor_values.mcu_temperature));
        } else {
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(sensor_values.mcu_temperature));
        }
        LCD_PutStr(102, 215, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        if      (attached_handle == T210)  LCD_PutStr(170, 175, "T210   ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == T245)  LCD_PutStr(170, 175, "T245   ", FONT_arial_17X18, C_WHITE, C_BLACK);
        else if (attached_handle == NT115) LCD_PutStr(170, 175, "NT115", FONT_arial_17X18, C_WHITE, C_BLACK);

        if (sensor_values.max_power_watt < 100) {
            sprintf(DISPLAY_buffer, "%d W  ", (int)sensor_values.max_power_watt);
        } else {
            sprintf(DISPLAY_buffer, "%d W", (int)sensor_values.max_power_watt);
        }
        LCD_PutStr(2, 10, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
        if (flash_values.power_unit == 0) { // power unit is set to W
            format_number_right(filtered_power_watt, DISPLAY_buffer);
        } else {
            format_number_right(100 * filtered_power_percent, DISPLAY_buffer);
        }
        LCD_PutStr(5, 215, DISPLAY_buffer, FONT_arial_17X18, C_WHITE, C_BLACK);

        if ((sensor_values.current_state == SLEEP || sensor_values.current_state == EMERGENCY_SLEEP || sensor_values.current_state == HALTED) && !sleep_state_written_to_LCD) {
            UG_FillFrame(10, 32, 30, 209, C_ORANGE);
            LCD_PutStr(14, 41,  "Z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(16, 68,  "z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 95,  "Z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(16, 122, "z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 149, "Z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(16, 176, "z", FONT_arial_20X23, C_BLACK, C_ORANGE);
            sleep_state_written_to_LCD = 1;
            standby_state_written_to_LCD = 0;
        }
        else if ((sensor_values.current_state == STANDBY) && !standby_state_written_to_LCD) {
            UG_FillFrame(10, 32, 30, 209, C_ORANGE);
            LCD_PutStr(14, 41,  "S", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 63,  "T", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 85,  "A", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 107, "N", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 129, "D", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 151, "B", FONT_arial_20X23, C_BLACK, C_ORANGE);
            LCD_PutStr(14, 173, "Y", FONT_arial_20X23, C_BLACK, C_ORANGE);
            standby_state_written_to_LCD = 1;
            sleep_state_written_to_LCD = 0;
        }
        else if (sensor_values.current_state == RUN) {
            bar_top = 209 - (uint16_t)(filtered_power_percent * 177.0f);
            UG_FillFrame(10, bar_top, 30, 209,     C_LIGHT_SKY_BLUE);
            UG_FillFrame(10, 32,      30, bar_top, C_BLACK);
            standby_state_written_to_LCD = 0;
            sleep_state_written_to_LCD = 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Main-screen layout (drawn on boot and after popup dismisses)       */
/* ------------------------------------------------------------------ */

void LCD_draw_main_screen(void) {
    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        UG_FillScreen(C_BLACK);

        LCD_PutStr(53, 8, "AxxSolder", FONT_arial_19X22, C_YELLOW, C_BLACK);
        LCD_DrawLine(0, 36, 240, 36, C_YELLOW);
        LCD_DrawLine(0, 37, 240, 37, C_YELLOW);
        LCD_DrawLine(0, 38, 240, 38, C_YELLOW);

        LCD_PutStr(19, 45, "Set temp", FONT_arial_20X23, C_WHITE, C_BLACK);
        UG_DrawCircle(128, 76, 5, C_WHITE);
        UG_DrawCircle(128, 76, 4, C_WHITE);
        UG_DrawCircle(128, 76, 3, C_WHITE);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(135, 70, "C", FONT_arial_36X44_C, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(135, 70, "F", FONT_arial_36X44_F, C_WHITE, C_BLACK);
        }
        LCD_PutStr(19, 135, "Actual temp", FONT_arial_20X23, C_WHITE, C_BLACK);
        UG_DrawCircle(128, 166, 5, C_WHITE);
        UG_DrawCircle(128, 166, 4, C_WHITE);
        UG_DrawCircle(128, 166, 3, C_WHITE);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(135, 160, "C", FONT_arial_36X44_C, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(135, 160, "F", FONT_arial_36X44_F, C_WHITE, C_BLACK);
        }
        UG_DrawFrame(11, 129, 187, 211, C_WHITE);
        UG_DrawFrame(10, 128, 188, 212, C_WHITE);

        LCD_PutStr(11, 235, "Handle type:", FONT_arial_17X18, C_WHITE, C_BLACK);
        LCD_PutStr(11, 255, "Input voltage:         V", FONT_arial_17X18, C_WHITE, C_BLACK);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(11, 275, "MCU:     °C", FONT_arial_17X18, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(11, 275, "MCU:      °F", FONT_arial_17X18, C_WHITE, C_BLACK);
        }
        LCD_PutStr(105, 275, "SRC:", FONT_arial_17X18, C_WHITE, C_BLACK);
        switch (power_source) {
            case POWER_DC:  LCD_PutStr(150, 275, "DC",  FONT_arial_17X18, C_WHITE, C_BLACK); break;
            case POWER_USB: LCD_PutStr(150, 275, "USB", FONT_arial_17X18, C_WHITE, C_BLACK); break;
            case POWER_BAT: LCD_PutStr(150, 275, "BAT", FONT_arial_17X18, C_WHITE, C_BLACK); break;
        }

        if (flash_values.power_unit == 0) { // power unit is set to W
            LCD_PutStr(219, 275, "W", FONT_arial_17X18, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(219, 275, "%", FONT_arial_17X18, C_WHITE, C_BLACK);
        }

        UG_DrawLine(0, 296, 240, 296, C_DARK_SEA_GREEN);
        UG_DrawLine(0, 297, 240, 297, C_DARK_SEA_GREEN);

        if (flash_values.three_button_mode == 1) {
            LCD_PutStr(11, 301, "TEMP          UP   DOWN", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
        } else {
            LCD_PutStr(11, 301, "PRESETS", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            memset(DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(flash_values.preset_temp_1));
            LCD_PutStr(130, 301, DISPLAY_buffer, FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            memset(DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)convert_temperature(flash_values.preset_temp_2));
            LCD_PutStr(190, 301, DISPLAY_buffer, FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
        }

        UG_DrawFrame(208, 64, 232, 270, C_WHITE);
        UG_DrawFrame(209, 65, 231, 269, C_WHITE);
    }
    else {
        UG_FillScreen(C_BLACK);
        LCD_PutStr(165, 10, "AxxSolder", FONT_arial_20X23, C_YELLOW, C_BLACK);

        LCD_PutStr(64, 10, "Set temp", FONT_arial_20X23, C_WHITE, C_BLACK);
        UG_DrawCircle(173, 41, 5, C_WHITE);
        UG_DrawCircle(173, 41, 4, C_WHITE);
        UG_DrawCircle(173, 41, 3, C_WHITE);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(180, 35, "C", FONT_arial_36X44_C, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(180, 35, "F", FONT_arial_36X44_F, C_WHITE, C_BLACK);
        }
        LCD_PutStr(64, 90, "Actual temp", FONT_arial_20X23, C_WHITE, C_BLACK);
        UG_DrawCircle(173, 121, 5, C_WHITE);
        UG_DrawCircle(173, 121, 4, C_WHITE);
        UG_DrawCircle(173, 121, 3, C_WHITE);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(180, 115, "C", FONT_arial_36X44_C, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(180, 115, "F", FONT_arial_36X44_F, C_WHITE, C_BLACK);
        }

        UG_DrawFrame(56, 84, 232, 170, C_WHITE);
        UG_DrawFrame(56, 83, 233, 171, C_WHITE);

        LCD_PutStr(56, 175, "Handle type:", FONT_arial_17X18, C_WHITE, C_BLACK);
        LCD_PutStr(56, 195, "Input voltage:          V", FONT_arial_17X18, C_WHITE, C_BLACK);
        if (flash_values.deg_celsius == 1) {
            LCD_PutStr(56, 215, "MCU:      °C", FONT_arial_17X18, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(56, 215, "MCU:       °F", FONT_arial_17X18, C_WHITE, C_BLACK);
        }
        LCD_PutStr(160, 215, "SRC:", FONT_arial_17X18, C_WHITE, C_BLACK);
        switch (power_source) {
            case POWER_DC:  LCD_PutStr(210, 215, "DC",  FONT_arial_17X18, C_WHITE, C_BLACK); break;
            case POWER_USB: LCD_PutStr(210, 215, "USB", FONT_arial_17X18, C_WHITE, C_BLACK); break;
            case POWER_BAT: LCD_PutStr(210, 215, "BAT", FONT_arial_17X18, C_WHITE, C_BLACK); break;
        }

        UG_DrawLine(265, 0, 265, 240, C_DARK_SEA_GREEN);
        UG_DrawLine(266, 0, 266, 240, C_DARK_SEA_GREEN);
        UG_DrawLine(315, 0, 315, 240, C_DARK_SEA_GREEN);
        UG_DrawLine(316, 0, 316, 240, C_DARK_SEA_GREEN);

        if (flash_values.three_button_mode == 1) {
            LCD_PutStr(269, 90, "INC", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            LCD_PutStr(269, 10, "DEC", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
        } else {
            LCD_PutStr(285, 75,  "P", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            LCD_PutStr(285, 97,  "R", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            LCD_PutStr(285, 119, "E", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            LCD_PutStr(285, 141, "S", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            LCD_PutStr(285, 163, "E", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            LCD_PutStr(285, 185, "T", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            LCD_PutStr(285, 207, "S", FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)flash_values.preset_temp_1);
            LCD_PutStr(272, 40, DISPLAY_buffer, FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
            memset(&DISPLAY_buffer, '\0', sizeof(DISPLAY_buffer));
            sprintf(DISPLAY_buffer, "%d", (int)flash_values.preset_temp_2);
            LCD_PutStr(272, 10, DISPLAY_buffer, FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);
        }

        UG_DrawFrame(8, 30, 32, 212, C_WHITE);
        UG_DrawFrame(9, 31, 31, 211, C_WHITE);

        if (flash_values.power_unit == 0) { // power unit is set to W
            LCD_PutStr(37, 215, "W", FONT_arial_17X18, C_WHITE, C_BLACK);
        } else {
            LCD_PutStr(37, 215, "%", FONT_arial_17X18, C_WHITE, C_BLACK);
        }
    }

    /* Callers (buttons.c preset-save, show_popup) clear
     * sleep_state_written_to_LCD as needed to trigger overlay redraw. */
}

/* ------------------------------------------------------------------ */
/* Popup overlay                                                      */
/* ------------------------------------------------------------------ */

void show_popup(char *text) {
    UG_FillFrame(10, 50, 235, 105, C_ORANGE);
    UG_FillFrame(15, 55, 230, 100, C_WHITE);
    LCD_PutStr(20, 70, text, FONT_arial_20X23, C_ORANGE, C_WHITE);
    popup_shown = 1;
    previous_millis_popup = HAL_GetTick();
    standby_state_written_to_LCD = 0;
    sleep_state_written_to_LCD = 0;
}

void LCD_draw_earth_fault_popup(void) {
    heater_off();
    sensor_values.requested_power = 0.0f;

    UG_FillFrame(10, 50, 205, 205, C_ORANGE);
    UG_FillFrame(15, 55, 200, 200, C_WHITE);
    LCD_PutStr(20, 60,  "GROUNDING",   FONT_arial_20X23, C_ORANGE, C_WHITE);
    LCD_PutStr(20, 80,  "ERROR",       FONT_arial_20X23, C_ORANGE, C_WHITE);

    LCD_PutStr(20, 120, "CHECK",       FONT_arial_20X23, C_ORANGE, C_WHITE);
    LCD_PutStr(20, 140, "CONNECTIONS", FONT_arial_20X23, C_ORANGE, C_WHITE);
    LCD_PutStr(20, 160, "AND REBOOT",  FONT_arial_20X23, C_ORANGE, C_WHITE);

    Error_Handler();
}

/* ------------------------------------------------------------------ */
/* Tick                                                               */
/* ------------------------------------------------------------------ */

void display_app_tick(void){
	if(popup_shown == 1){
		if(HAL_GetTick() - previous_millis_popup >= POPUP_DWELL_MS){
			popup_shown = 0;

			if (flash_values.display_graph == 0){
				LCD_draw_main_screen(); // Clear the area where the emergency message is displayed
			}
			else{
				UG_FillScreen(background);
				draw_graph_init();
			}
		}
	}
	else{
		/* Update display */
		if(HAL_GetTick() - previous_millis_display >= DISPLAY_TICK_INTERVAL_MS){
			sensor_values.requested_power_filtered = clamp(Moving_Average_Compute(sensor_values.requested_power, &requested_power_filtered_filter_struct), 0.0f, PID_MAX_OUTPUT);

			if (flash_values.display_graph == 0){
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
