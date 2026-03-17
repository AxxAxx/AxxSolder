/* menu_settings.c*/

#include "menu_settings.h"

/* ==== MENU GROUPS (indices correspond to menu_names / flash_values) ==== */
/* Groups can contain NON-contiguous indices - we use index tables. */

typedef struct {
    char* title;          // Menu group name (displayed at level=0).

    const uint8_t* idx;   // Table of absolute menu item indices from the menu_names[] array
                          // and the flash_values[] structure. Allows building a group from scattered items.

    uint8_t count;        // Number of items in the group (length of the idx array).
                          // For each group, 1 "Exit" item is added to count
                          // in the actual display.
} MenuGroup;


/* Mode / Behavior */
static const uint8_t GRP_MODE[]        = {0, 1, 2, 3, 4, 8, 10, 11, 20, 22, 23};

/* Presets */
static const uint8_t GRP_PRESETS[]     = {6, 7};

/* Tips (tip shapes) */
static const uint8_t GRP_TIPS[]        = {28, 29, 30, 31};

/* Calibration */
static const uint8_t GRP_CALIBRATION[] = {14, 15, 16, 17, 18, 19};

/* Display */
static const uint8_t GRP_DISPLAY[]     = {9, 13, 21, 26, 27, 32};

/* Sound */
static const uint8_t GRP_SOUND[]       = {5, 12, 24, 25};

/* System (actions) */
static const uint8_t GRP_SYSTEM[]      = {33, 34, 35}; // -Load Default-, -Save and Reboot-, -Exit no Save-

static const MenuGroup MENU_GROUPS[] = {
    { "Mode",        GRP_MODE,        sizeof(GRP_MODE)        },
    { "Presets",     GRP_PRESETS,     sizeof(GRP_PRESETS)     },
    { "Tips",        GRP_TIPS,        sizeof(GRP_TIPS)        },
	{ "Calibration", GRP_CALIBRATION, sizeof(GRP_CALIBRATION) },
    { "Display",     GRP_DISPLAY,     sizeof(GRP_DISPLAY)     },
    { "Sound",       GRP_SOUND,       sizeof(GRP_SOUND)       },
    { "System",      GRP_SYSTEM,      sizeof(GRP_SYSTEM)      },
};
// Number of elements in the MENU_GROUPS array (i.e. the number of menu groups).
#define GROUPS_COUNT (sizeof(MENU_GROUPS)/sizeof(MENU_GROUPS[0]))

/* -------------------------------------------------------------------
 * Helper utilities for button and encoder handling
 * -------------------------------------------------------------------*/

/**
 * @brief Wait for the encoder button to be released.
 * Used for debouncing and preventing multiple triggers
 * from a single press.
 */
static inline void wait_button_release(void) {
    while (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {
        // Wait until the button is released
    }
    HAL_Delay(50); // short delay for debouncing
}

/**
 * @brief Converts the TIM2->CNT encoder counter to a menu item index.
 *
 * @param cnt0     - base counter offset (usually 1000 at startup).
 * @param pCnt     - pointer to the encoder counter register (volatile).
 * @param maxItems - total number of items in the current list.
 *
 * @return int16_t - current cursor position (0..maxItems-1).
 *
 * The function ensures that TIM2->CNT:
 *   - does not go below cnt0
 *   - does not overflow the range
 *   - stays bound to the range 0..maxItems-1
 * Counter step = 2 (i.e. two ticks = move to the next item).
 */
static inline int16_t enc_to_sel_bounded(uint32_t cnt0, volatile uint32_t* pCnt, uint16_t maxItems)
{
    // Guard against the counter going below the lower boundary
    if (*pCnt < cnt0) *pCnt = cnt0;

    // Upper limit (safeguard against register overflow)
    if (*pCnt > 1000000) *pCnt = 1000000;

    // Convert to an offset from cnt0
    uint32_t span = *pCnt - cnt0;

    // Divide by 2 (encoder step = 2)
    uint16_t sel = span / 2;

    // Clamp cursor value to valid range
    if (sel >= maxItems) {
        sel = maxItems - 1;
        *pCnt = cnt0 + sel * 2; // lock TIM2->CNT to the last valid value
    }

    return sel;
}

/* List of names for settings menu */
#define menu_length 36
char menu_names[menu_length][36] = {
		"Startup Temp °C",//0
		"Temp Offset °C",//1
		"Standby Temp °C",//2
		"Standby Time [min]",//3
		"Sleep Time [min]",//4
		"Buzzer Enabled",//5
		"Preset Temp 1 °C",//6
		"Preset Temp 2 °C",//7
		"GPIO4 ON at run",//8
		"Screen Rotation",//9
		"Momentary stand",//10
		"I Measurement",//11
		"Startup Beep",//12
		"Temp in Celsius",//13
		"Temp cal 100",//14
		"Temp cal 200",//15
		"Temp cal 300",//16
		"Temp cal 350",//17
		"Temp cal 400",//18
		"Temp cal 450",//19
		"Serial DEBUG",//20
		"Disp Temp. filter",//21
		"Start at prev. temp",//22
		"3-button mode",//23
		"Beep at set temp",//24
		"Beep tone",//25
		"Show power",//26
		"Detect NT115",//27
	    "Power lim T245",//28
	    "Power lim T210",//29
	    "Power lim NT115",//30
		"Power lim Nn",//31
		"Display graph",//32
		"-Load Default-",//33
		"-Save and Reboot-",//34
		"-Exit no Save-"//35
};

/* Function to left align a string from float */
void left_align_float(char* str, float number, int8_t len)
	{
		char tempstring[len];
		memset(&tempstring, '\0', len);
		sprintf(tempstring, "%.0f", number);

		strcpy(str, tempstring);
	}

// ==== Enumeration strings ====
const char* bool_str[] = { "No ", "Yes " };
const char* screen_rotation_str[] = { "0°", "90°", "180°", "270°" };
const char* show_power_str[] = { "W", "%"};

// ==== Table of enumerated parameters ====
typedef struct {
    uint8_t index;
    const char** values;
    uint8_t count;
} EnumParam;

EnumParam enum_params[] = {
    { 5,  bool_str, 2 },    // Buzzer Enabled
    { 8,  bool_str, 2 },    // GPIO4 ON at run
	{ 9, screen_rotation_str, 4 },  // Screen rotation
	{10,  bool_str, 2 },    // Momentary stand
    {11,  bool_str, 2 },    // I Measurement
    {12,  bool_str, 2 },    // Startup Beep
    {13,  bool_str, 2 },    // Temp in Celsius
    {20,  bool_str, 2 },    // Serial DEBUG
    {22,  bool_str, 2 },    // Start at previous temp
    {23,  bool_str, 2 },    // 3-button mode
    {24,  bool_str, 2 },    // Beep at set temp
	{26,  show_power_str, 2 }, 	// Power unit on screen
	{27,  bool_str, 2 },			// Detect the NT115 handle
    {32,  bool_str, 2 }     // Displaying a graph
};

#define ENUM_PARAM_COUNT (sizeof(enum_params) / sizeof(enum_params[0]))

// ==== Get string value of a parameter ====
const char* get_enum_value_str(uint8_t index, float value) {
    for (uint8_t i = 0; i < ENUM_PARAM_COUNT; i++) {
        if (enum_params[i].index == index) {
            int v = (int)roundf(value);  // safe rounding
            if (v >= 0 && v < enum_params[i].count) {
                return enum_params[i].values[v]; // valid index
            } else {
                return "?";  // out-of-bounds guard
            }
        }
    }
    return NULL;  // enum parameter not found
}

// ==== Display a parameter row in the menu ====
// Universal output of a menu item value: enum string or numeric with alignment

#define MENU_VALUE_W   47      // value zone width
#define MENU_VALUE_H   18      // value zone height

#define MAX_MENU_LINES 10

static char     prev_buf_screen[MAX_MENU_LINES][12] = {0};
static uint8_t  prev_flags_screen[MAX_MENU_LINES]   = {0};

uint8_t menu_lines_on_screen = 7; // dynamically changes depending on screen rotation

/* Function to check if value differs from default */
static uint8_t is_value_different_from_default(uint16_t index, float current_value) {
        if (index >= menu_length - 3) return 0; // Skip special menu items

        float default_value = ((float*)&default_flash_values)[index];

        // Compare with small tolerance for floating point values
        return (fabsf(current_value - default_value) > 0.1f);
}

/* Function for rendering a parameter value with caching to avoid unnecessary redraws */
static void display_menu_value_line(uint8_t line_pos, uint16_t index,
                                    float value, uint8_t selected, uint8_t editing,
                                    uint16_t x, uint16_t y,
                                    uint16_t fg_def, uint16_t bg_def)
{
    char buf[12] = {0};
    const char *s = get_enum_value_str(index, value);

    if (s) strncpy(buf, s, sizeof(buf) - 1);
    else   left_align_float(buf, value, sizeof(buf));

    uint8_t prev_flags = prev_flags_screen[line_pos];
    uint8_t flags = (editing << 1) | selected;

    //Fill on edit mode change
    uint8_t prev_editing = (prev_flags >> 1) & 1;

    if (editing && !prev_editing) {
        // Entering edit mode: fill with white
        UG_FillFrame(x, y-2, x + MENU_VALUE_W, y + MENU_VALUE_H+2, RGB_to_BRG(C_WHITE));
    }
    else if (!editing && prev_editing && selected) {
        // Exiting edit mode while still selected: fill with black
        UG_FillFrame(x, y-2, x + MENU_VALUE_W, y + MENU_VALUE_H+2, RGB_to_BRG(C_BLACK));
    }

    // === Redraw text if the value or flags have changed ===
    if (strcmp(buf, prev_buf_screen[line_pos]) != 0 || prev_flags != flags) {
        strncpy(prev_buf_screen[line_pos], buf, sizeof(buf));
        prev_flags_screen[line_pos] = flags;

        // Determine text color based on whether value differs from default
        uint16_t fg;
        fg = (selected && editing) ? RGB_to_BRG(C_BLACK) : fg_def;
        if (selected && editing) {
                fg = RGB_to_BRG(C_BLACK);
        } else if (is_value_different_from_default(index, value)) {
                fg = RGB_to_BRG(C_ORANGE);  // Red for non-default values
        } else {
                fg = fg_def;  // Default color for default values
        }

        uint16_t bg = (selected && editing) ? RGB_to_BRG(C_WHITE) : bg_def;

        UG_FillFrame(x, y-2, 190 + MENU_VALUE_W, y + MENU_VALUE_H+2, bg);
        LCD_PutStr(x+3, y, buf, FONT_arial_20X23, fg, bg);

    }
}
//Cyclic rounding:
static inline float normalize_enum(float val, uint8_t count) {
    int v = (int)roundf(val);
    while (v < 0) v += count;
    v %= count;
    return (float)v;
}

//Menu parameter constraints
void normalize_param(uint16_t index) {
    float* p = &((float*)&flash_values)[index];

    switch(index) {

        // --- Boolean or binary parameters: clamped to 0 or 1
        case 5: case 8: case 10: case 11: case 12:
        case 13: case 20: case 22: case 23: case 24: case 26: case 27: case 32:
            *p = normalize_enum(*p, 2);  // 0/1, cyclic
            break;

        // --- Parameters with range 0 to 3 (4 values)
        case 9:
        	 *p = normalize_enum(*p, 4);  // 0/1/2/3, cyclic

            break;

        // --- Parameter with range 1 to 10 (10 values)
        case 21:
            *p = 1 + fmod(round(fmod(fabs(*p), 10)), 10);  // 1..10
            break;

        // --- Integer parameter (e.g. sensor type, flag)
        case 1:
            *p = round(*p);  // Round to nearest integer
            break;

        // --- Temperature parameters: clamped to the allowed range
        case 0: case 2: case 6: case 7:
            *p = fmod(round(fmod(fabs(*p), MAX_SELECTABLE_TEMPERATURE + 1)), MAX_SELECTABLE_TEMPERATURE + 1);
            break;

        // --- Power parameter: 0..(MAX_POWER + 4)
       case 28: case 29: case 30: case 31:
            *p = fmod(round(fmod(fabs(*p), MAX_POWER + 5)), MAX_POWER + 5);
            break;

        // --- Default: take absolute value (positive only)
        default:
            *p = fabs(*p);
            break;
    }
}

void settings_menu()
{
    // Transition system to safe state (disable heating)
    change_state(EMERGENCY_SLEEP);
    sensor_values.requested_power = 0;
    settings_menu_active = 1;

    LCD_SetRotation(flash_values.screen_rotation);
    UG_FillScreen(RGB_to_BRG(C_BLACK));
    UG_FontSetTransparency(1);

    // --- Firmware version string ---
    char str[64] = {0};
    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        menu_lines_on_screen = 10;
        sprintf(str, "fw mod: %d.%d.%d   hw: %d", fw_version_major, fw_version_minor, fw_version_patch, get_hw_version());
        LCD_PutStr(6, 296, str, FONT_arial_20X23, RGB_to_BRG(C_RED), RGB_to_BRG(C_BLACK));
    } else {
        menu_lines_on_screen = 7;
        sprintf(str, "fw mod: %d.%d.%d   hw: %d", fw_version_major, fw_version_minor, fw_version_patch, get_hw_version());
        LCD_PutStr(6, 215, str, FONT_arial_20X23, RGB_to_BRG(C_RED), RGB_to_BRG(C_BLACK));
    }

    // Header
	LCD_PutStr(70, 5, "SETTINGS", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
	LCD_DrawLine(0, 25, 240, 25, RGB_to_BRG(C_DARK_SEA_GREEN));
	LCD_DrawLine(0, 26, 240, 26, RGB_to_BRG(C_DARK_SEA_GREEN));
	LCD_DrawLine(0, 27, 240, 27, RGB_to_BRG(C_DARK_SEA_GREEN));

    // Value rendering cache
    for (int i = 0; i < MAX_MENU_LINES; i++) {
        prev_buf_screen[i][0] = '\0';
        prev_flags_screen[i] = 0xFF;
    }

    // Levels:
    //   0 - group selection
    //   1 - list of items in the selected group (+ "Back" at the end)
    //   2 - editing the value of the selected item

    uint8_t level = 0;
    uint8_t current_group = 0;

    // Current cursor position within the current list
    uint16_t cursor = 0;
    uint16_t prev_cursor = 0xFFFF;
    uint16_t page_start = 0xFFFF;

    // Edit mode helpers
    float old_value = 0;
    uint32_t edit_cnt0 = 0;   // encoder "zero" at the start of editing
    uint8_t editing_index = 0xFF;  // position within the group (not an absolute index)
    uint8_t redraw_page = 1;

    // Wait for button release before entering the menu
    wait_button_release();

    // Starting encoder position - mid-scale for smooth operation
    TIM2->CNT = 1000;

    while (settings_menu_active)
    {
        // ---- Current list length ----
        uint16_t list_len = 0;
        if (level == 0) {
            list_len = GROUPS_COUNT;
        } else if (level == 1 || level == 2) {
            // group items + Back
            list_len = MENU_GROUPS[current_group].count + 1;
        }

        // ---- Update position from encoder (in modes 0/1) ----
        if (level != 2) {
            cursor = enc_to_sel_bounded(1000, &TIM2->CNT, list_len);

        }

        // ---- Page grouping of items ----
        uint16_t new_page_start = (cursor / menu_lines_on_screen) * menu_lines_on_screen;

        // ---- Full page redraw ----
        if (redraw_page || (new_page_start != page_start)) {
            UG_FillFrame(0, 30, 239, 31 + menu_lines_on_screen * 26, RGB_to_BRG(C_BLACK));
            page_start = new_page_start;

            // Reset value row cache - important so display_menu_value_line() redraws
            for (int i = 0; i < menu_lines_on_screen; i++) {
                prev_buf_screen[i][0] = '\0';
                prev_flags_screen[i] = 0xFF;
            }

            // Render rows of the current list
            for (uint16_t line = 0; line < menu_lines_on_screen; line++) {
                uint16_t pos = page_start + line;
                uint16_t line_y = 35 + line * 26;

                if (pos >= list_len) break;

                UG_FillFrame(5, line_y, 187, line_y + MENU_VALUE_H, RGB_to_BRG(C_BLACK));

                if (level == 0) {
                    // Group list - names only
                    LCD_PutStr(5, line_y, MENU_GROUPS[pos].title, FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));
                } else {
                    // Group item list + Back
                	if (pos < MENU_GROUPS[current_group].count) {
                	    uint8_t abs_index = MENU_GROUPS[current_group].idx[pos];
                	    LCD_PutStr(5, line_y, menu_names[abs_index], FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

                	    // Do not display values for the System group
                	    if (!(current_group == (GROUPS_COUNT - 1))) {
                	        display_menu_value_line(
                	            line,
                	            abs_index,
                	            ((float*)&flash_values)[abs_index],
                	            0, 0,
                	            190, line_y,
								RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
                	        );
                	    }
                	}
                    else if (pos == MENU_GROUPS[current_group].count) {
                        // Back row
                        LCD_PutStr(5, line_y, "Back", FONT_arial_20X23, RGB_to_BRG(C_RED), RGB_to_BRG(C_BLACK));
                    }
                }
            }

            redraw_page = 0;
            // On page change - redraw the cursor frame, then continue with normal logic
            prev_cursor = 0xFFFF;
        }

        // ---- Cursor/line highlight (frame) - levels 0 and 1 only ----
        if (level != 2 && cursor != prev_cursor) {
            // Erase the old frame if it is on the same page
            if (prev_cursor != 0xFFFF &&
                (prev_cursor / menu_lines_on_screen) == (cursor / menu_lines_on_screen)) {
                uint8_t prev_line = prev_cursor % menu_lines_on_screen;
                uint16_t prev_y = 35 + prev_line * 26;
                UG_DrawFrame(0, prev_y - 4, 239, prev_y + 4 + MENU_VALUE_H, RGB_to_BRG(C_BLACK));
                UG_DrawFrame(1, prev_y - 3, 238, prev_y + 3 + MENU_VALUE_H, RGB_to_BRG(C_BLACK));
            }

            // Draw the new frame
            uint8_t line = cursor % menu_lines_on_screen;
            uint16_t line_y = 35 + line * 26;
            UG_DrawFrame(0, line_y - 4, 239, line_y + 4 + MENU_VALUE_H, RGB_to_BRG(C_YELLOW));
            UG_DrawFrame(1, line_y - 3, 238, line_y + 3 + MENU_VALUE_H, RGB_to_BRG(C_YELLOW));

            prev_cursor = cursor;
        }

        // ---- Edit mode (level 2) - highlight value area only, do NOT touch names ----
        if (level == 2) {
            // Local line on screen
            uint8_t line = editing_index % menu_lines_on_screen;
            uint16_t line_y = 35 + line * 26;

            // Absolute parameter index
            uint8_t abs_index = MENU_GROUPS[current_group].idx[editing_index];

            // Increment/decrement from the edit "zero"
            int32_t delta = ((int32_t)TIM2->CNT - (int32_t)edit_cnt0) / 2;

            float new_val;
            if (abs_index == 28 || abs_index == 29 || abs_index == 30|| abs_index == 31) { // Limit Power - step 5
                new_val = old_value + (float)delta * 5.0f;
            } else {
                new_val = old_value + (float)delta;
            }

            ((float*)&flash_values)[abs_index] = new_val;
            normalize_param(abs_index);  // original normalization/cycling function
            float norm_val = ((float*)&flash_values)[abs_index];

            // Redraw ONLY the value zone, inverting colors
            display_menu_value_line(
                line,
                abs_index,
                norm_val,
                1, // selected
                1, // editing
                190, line_y,
				RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
            );
        }

        // ---- Button press handling ----
        if (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {

            if (level == 0) {
                // Select group
                current_group = cursor;
                level = 1;

                // Reset cursor/page for the selected group
                TIM2->CNT = 1000;
                cursor = 0;
                page_start = 0xFFFF;
                redraw_page = 1;

            } else if (level == 1) {
                // Press on an item inside a group
                if (cursor == MENU_GROUPS[current_group].count) {
                    // Back - return to group list
                    level = 0;
                    // Restore cursor to the current group
                    TIM2->CNT = 1000 + current_group * 2;
                    cursor = current_group;
                    page_start = 0xFFFF;
                    redraw_page = 1;
                } else {
                    // Regular parameter or system action
                    uint8_t abs_index = MENU_GROUPS[current_group].idx[cursor];

                    if (abs_index == 35) {                // -Exit no Save-
                        settings_menu_active = 0;
                        HAL_NVIC_SystemReset();
                    } else if (abs_index == 34) {         // -Save and Reboot-
                        FlashWrite(&flash_values);
                        settings_menu_active = 0;
                        HAL_NVIC_SystemReset();
                    } else if (abs_index == 33) {         // -Load Default-
                        flash_values = default_flash_values;
                        redraw_page = 1; // values changed - redraw
                    } else {
                        // Enter value editing
                        old_value = ((float*)&flash_values)[abs_index];
                        editing_index = cursor;
                        edit_cnt0 = TIM2->CNT;      // "zero" for delta
                        level = 2;

                        // Remove cursor frame (level 2 does not draw it),
                        //   on entering edit mode redraw the value with inversion
                        uint8_t line = cursor % menu_lines_on_screen;
                        uint16_t line_y = 35 + line * 26;
                        display_menu_value_line(
                            line,
                            abs_index,
                            ((float*)&flash_values)[abs_index],
                            1, 1, 190, line_y,
							RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
                        );
                    }
                }

            }
            else if (level == 2) {
                /* Confirm / exit editing - return to group item list */
                level = 1;

                /* Reset encoder: cursor must stay on the same row */
                uint16_t safe_cursor = editing_index;
                if (safe_cursor >= MENU_GROUPS[current_group].count) {
                    safe_cursor = MENU_GROUPS[current_group].count - 1; // safety guard in case idx is broken
                }
                TIM2->CNT = 1000 + safe_cursor * 2;

                /* After exiting editing the value stays rendered in normal style */
                uint8_t line = editing_index % menu_lines_on_screen;
                uint16_t line_y = 35 + line * 26;
                uint8_t abs_index = MENU_GROUPS[current_group].idx[editing_index];
                display_menu_value_line(
                    line,
                    abs_index,
                    ((float*)&flash_values)[abs_index],
                    0, 0,
                    190, line_y,
					RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
                );

                /* Restore cursor appearance: draw frame for the current row */
                prev_cursor = 0xFFFF;
            }
            wait_button_release(); // debounce + one click = one action
        }
    }
}
