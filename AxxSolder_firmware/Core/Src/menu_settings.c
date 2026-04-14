/* menu_settings.c*/

#include "menu_settings.h"
#include "menu_profiles.h"

/* ==== Menu item identifiers
 * Step-100 layout: each logical group owns a 100-wide range.
 * Items within a group can be inserted without renumbering other groups.
 *
 * Range → group
 *   0–  99  Mode / Behaviour
 * 100– 199  Presets
 * 200– 299  Profiles
 * 300– 399  Display
 * 400– 499  Sound
 * 500– 599  Calibration  (managed by profiles menu)
 * 600– 699  Power limits (managed by profiles menu)
 * 700– 799  System actions
 * ==== */
enum {
    /* ── Mode / Behaviour (0–99) ───────────────────────────────────── */
    MI_STARTUP_TEMP       = 0,
    MI_TEMP_OFFSET        = 1,
    MI_STANDBY_TEMP       = 2,
    MI_STANDBY_TIME       = 3,
    MI_SLEEP_TIME         = 4,
    MI_GPIO4_ON_AT_RUN    = 5,
    MI_MOMENTARY_STAND    = 6,
    MI_I_MEASUREMENT      = 7,
    MI_SERIAL_DEBUG       = 8,
    MI_START_PREV_TEMP    = 9,
    MI_THREE_BUTTON_MODE  = 10,
    MI_DETECT_NT115       = 11,
    MI_DELTA_T_DETECT     = 12,
    MI_STANDBY_DELAY      = 13,

    /* ── Presets (100–199) ─────────────────────────────────────────── */
    MI_PRESET_TEMP_1      = 100,
    MI_PRESET_TEMP_2      = 101,

    /* ── Profiles (200–299) ────────────────────────────────────────── */
    MI_PROFILE_ON_TIP_CHG = 200,

    /* ── Display (300–399) ─────────────────────────────────────────── */
    MI_SCREEN_ROTATION    = 300,
    MI_TEMP_UNIT          = 301,
    MI_DISP_TEMP_FILTER   = 302,
    MI_SHOW_POWER         = 303,
    MI_DISPLAY_GRAPH      = 304,

    /* ── Sound (400–499) ───────────────────────────────────────────── */
    MI_BUZZER_ENABLED     = 400,
    MI_STARTUP_BEEP       = 401,
    MI_BEEP_AT_SET_TEMP   = 402,
    MI_BEEP_TONE          = 403,

    /* ── Calibration, in profiles (500–599) ────────────────────────── */
    MI_TEMP_CAL_100       = 500,
    MI_TEMP_CAL_200       = 501,
    MI_TEMP_CAL_300       = 502,
    MI_TEMP_CAL_350       = 503,
    MI_TEMP_CAL_400       = 504,
    MI_TEMP_CAL_450       = 505,

    /* ── Power limits, in profiles (600–699) ───────────────────────── */
    MI_POWER_LIM_T245     = 600,
    MI_POWER_LIM_T210     = 601,
    MI_POWER_LIM_NT115    = 602,
    MI_POWER_LIM_NO_NAME  = 603,

    /* ── System actions (700–799) ──────────────────────────────────── */
    MI_LOAD_DEFAULT       = 700,
    MI_SAVE_REBOOT        = 701,
    MI_EXIT_NO_SAVE       = 702,
};

/* Sentinel: MI_ value has no Flash_values backing (system action). */
#define MI_NO_FLASH 0xFF

/* Maps an MI_ identifier to:
 *   fi   – sequential field index in Flash_values cast to float*
 *          (MI_NO_FLASH when the item is a system action, not stored in flash)
 *   name – display string shown in the menu
 */
typedef struct {
    uint16_t    mi;
    uint8_t     fi;
    const char *name;
} MI_Entry;

static const MI_Entry mi_table[] = {
    /* Mode */
    { MI_STARTUP_TEMP,       0,          "Startup Temp °C"      },
    { MI_TEMP_OFFSET,        1,          "Temp Offset °C"       },
    { MI_STANDBY_TEMP,       2,          "Standby Temp °C"      },
    { MI_STANDBY_TIME,       3,          "Standby Time [min]"   },
    { MI_SLEEP_TIME,         4,          "Sleep Time [min]"     },
    { MI_GPIO4_ON_AT_RUN,    8,          "GPIO4 ON at run"      },
    { MI_MOMENTARY_STAND,    10,         "Momentary stand"      },
    { MI_I_MEASUREMENT,      11,         "I Measurement"        },
    { MI_SERIAL_DEBUG,       20,         "Serial DEBUG"         },
    { MI_START_PREV_TEMP,    22,         "Start at prev. temp"  },
    { MI_THREE_BUTTON_MODE,  23,         "3-button mode"        },
    { MI_DETECT_NT115,       27,         "Detect NT115"         },
    { MI_DELTA_T_DETECT,     33,         "Delta T detect"       },
    { MI_STANDBY_DELAY,      34,         "Standby delay [s]"    },
    /* Presets */
    { MI_PRESET_TEMP_1,      6,          "Preset Temp 1 °C"     },
    { MI_PRESET_TEMP_2,      7,          "Preset Temp 2 °C"     },
    /* Profiles */
    { MI_PROFILE_ON_TIP_CHG, 35,         "Profile on tip chg"   },
    /* Display */
    { MI_SCREEN_ROTATION,    9,          "Screen Rotation"      },
    { MI_TEMP_UNIT,          13,         "Temperature unit"     },
    { MI_DISP_TEMP_FILTER,   21,         "Disp Temp. filter"    },
    { MI_SHOW_POWER,         26,         "Show power"           },
    { MI_DISPLAY_GRAPH,      32,         "Display graph"        },
    /* Sound */
    { MI_BUZZER_ENABLED,     5,          "Buzzer Enabled"       },
    { MI_STARTUP_BEEP,       12,         "Startup Beep"         },
    { MI_BEEP_AT_SET_TEMP,   24,         "Beep at set temp"     },
    { MI_BEEP_TONE,          25,         "Beep tone"            },
    /* Calibration */
    { MI_TEMP_CAL_100,       14,         "Temp cal 100"         },
    { MI_TEMP_CAL_200,       15,         "Temp cal 200"         },
    { MI_TEMP_CAL_300,       16,         "Temp cal 300"         },
    { MI_TEMP_CAL_350,       17,         "Temp cal 350"         },
    { MI_TEMP_CAL_400,       18,         "Temp cal 400"         },
    { MI_TEMP_CAL_450,       19,         "Temp cal 450"         },
    /* Power limits */
    { MI_POWER_LIM_T245,     28,         "Power lim T245"       },
    { MI_POWER_LIM_T210,     29,         "Power lim T210"       },
    { MI_POWER_LIM_NT115,    30,         "Power lim NT115"      },
    { MI_POWER_LIM_NO_NAME,  31,         "Power lim Nn"         },
    /* System actions — no flash backing */
    { MI_LOAD_DEFAULT,  MI_NO_FLASH,     "-Load Default-"       },
    { MI_SAVE_REBOOT,   MI_NO_FLASH,     "-Save and Reboot-"    },
    { MI_EXIT_NO_SAVE,  MI_NO_FLASH,     "-Exit no Save-"       },
};
#define MI_TABLE_COUNT (sizeof(mi_table) / sizeof(mi_table[0]))

/**
 * @brief Returns the Flash_values field index (fi) for an MI_ identifier.
 * @return 0–35 for data items, MI_NO_FLASH for system actions / unknown IDs.
 */
static uint8_t mi_to_fi(uint16_t mi) {
    for (uint8_t i = 0; i < MI_TABLE_COUNT; i++) {
        if (mi_table[i].mi == mi) return mi_table[i].fi;
    }
    return MI_NO_FLASH;
}

/**
 * @brief Returns the display name for an MI_ identifier.
 * @return Pointer to the name string, or "?" if not found.
 */
static const char* mi_name(uint16_t mi) {
    for (uint8_t i = 0; i < MI_TABLE_COUNT; i++) {
        if (mi_table[i].mi == mi) return mi_table[i].name;
    }
    return "?";
}

/* ==== MENU GROUPS (idx arrays hold MI_ identifiers, not sequential indices) ==== */
/* Groups can contain non-contiguous MI_ values. */

typedef struct {
    char* title;           // Menu group name (displayed at level=0).

    const uint16_t* idx;   // Table of MI_ identifiers belonging to this group.
                           // uint16_t because MI_ values exceed 255.

    uint8_t count;         // Number of items in the group (length of the idx array).
                           // For each group, 1 "Exit" item is added to count
                           // in the actual display.
} MenuGroup;


/* Mode / Behavior */
static const uint16_t GRP_MODE[] = {
    MI_STARTUP_TEMP, MI_TEMP_OFFSET, MI_STANDBY_TEMP, MI_STANDBY_DELAY,
    MI_STANDBY_TIME, MI_SLEEP_TIME, MI_GPIO4_ON_AT_RUN, MI_MOMENTARY_STAND,
    MI_I_MEASUREMENT, MI_SERIAL_DEBUG, MI_START_PREV_TEMP, MI_THREE_BUTTON_MODE,
    MI_DETECT_NT115, MI_DELTA_T_DETECT
};

/* Presets */
static const uint16_t GRP_PRESETS[] = { MI_PRESET_TEMP_1, MI_PRESET_TEMP_2 };

/* Display */
static const uint16_t GRP_DISPLAY[] = {
    MI_SCREEN_ROTATION, MI_TEMP_UNIT, MI_DISP_TEMP_FILTER, MI_SHOW_POWER,
    MI_DISPLAY_GRAPH
};

/* Sound */
static const uint16_t GRP_SOUND[] = {
    MI_BUZZER_ENABLED, MI_STARTUP_BEEP, MI_BEEP_AT_SET_TEMP, MI_BEEP_TONE
};

/* System (actions) */
static const uint16_t GRP_SYSTEM[] = {
    MI_LOAD_DEFAULT, MI_SAVE_REBOOT, MI_EXIT_NO_SAVE
};

/* Profiles — handled by separate profiles_menu(), idx/count unused */
static const uint16_t GRP_PROFILES_DUMMY[] = { MI_PROFILE_ON_TIP_CHG };

static const MenuGroup MENU_GROUPS[] = {
    { "Mode",        GRP_MODE,        sizeof(GRP_MODE)    / sizeof(GRP_MODE[0])    },
    { "Presets",     GRP_PRESETS,     sizeof(GRP_PRESETS) / sizeof(GRP_PRESETS[0]) },
    { "Profiles",    GRP_PROFILES_DUMMY, 0                                         },
    { "Display",     GRP_DISPLAY,     sizeof(GRP_DISPLAY) / sizeof(GRP_DISPLAY[0]) },
    { "Sound",       GRP_SOUND,       sizeof(GRP_SOUND)   / sizeof(GRP_SOUND[0])   },
    { "System",      GRP_SYSTEM,      sizeof(GRP_SYSTEM)  / sizeof(GRP_SYSTEM[0])  },
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

/* Function to left align a string from float */
void left_align_float(char* str, float number, int8_t len)
	{
		char tempstring[len];
		memset(&tempstring, '\0', len);
		sprintf(tempstring, "%d", (int)number);

		strcpy(str, tempstring);
	}

// ==== Enumeration strings ====
const char* bool_str[] = { "No ", "Yes " };
const char* screen_rotation_str[] = { "0°", "90°", "180°", "270°" };
const char* show_power_str[] = { "W", "%"};
const char* temp_unit_str[] = { "°F", "°C"};

// ==== Table of enumerated parameters ====
typedef struct {
    uint16_t index;
    const char** values;
    uint8_t count;
} EnumParam;

EnumParam enum_params[] = {
    { MI_BUZZER_ENABLED,     bool_str, 2 },
    { MI_GPIO4_ON_AT_RUN,    bool_str, 2 },
    { MI_SCREEN_ROTATION,    screen_rotation_str, 4 },
    { MI_MOMENTARY_STAND,    bool_str, 2 },
    { MI_I_MEASUREMENT,      bool_str, 2 },
    { MI_STARTUP_BEEP,       bool_str, 2 },
    { MI_TEMP_UNIT,          temp_unit_str, 2 },
    { MI_SERIAL_DEBUG,       bool_str, 2 },
    { MI_START_PREV_TEMP,    bool_str, 2 },
    { MI_THREE_BUTTON_MODE,  bool_str, 2 },
    { MI_BEEP_AT_SET_TEMP,   bool_str, 2 },
    { MI_SHOW_POWER,         show_power_str, 2 },
    { MI_DETECT_NT115,       bool_str, 2 },
    { MI_DISPLAY_GRAPH,      bool_str, 2 },
    { MI_DELTA_T_DETECT,     bool_str, 2 },
    { MI_PROFILE_ON_TIP_CHG, bool_str, 2 },
};

#define ENUM_PARAM_COUNT (sizeof(enum_params) / sizeof(enum_params[0]))

// ==== Get string value of a parameter ====
const char* get_enum_value_str(uint16_t index, float value) {
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
    uint8_t fi = mi_to_fi(index);
    if (fi == MI_NO_FLASH) return 0; // system actions have no flash backing

    float default_value = ((float*)&default_flash_values)[fi];

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
    uint8_t fi = mi_to_fi(index);
    if (fi == MI_NO_FLASH) return;
    float* p = &((float*)&flash_values)[fi];

    switch(index) {

        // --- Boolean or binary parameters: clamped to 0 or 1
        case MI_BUZZER_ENABLED: case MI_GPIO4_ON_AT_RUN:
        case MI_MOMENTARY_STAND: case MI_I_MEASUREMENT:
        case MI_STARTUP_BEEP: case MI_TEMP_UNIT:
        case MI_SERIAL_DEBUG: case MI_START_PREV_TEMP:
        case MI_THREE_BUTTON_MODE: case MI_BEEP_AT_SET_TEMP:
        case MI_SHOW_POWER: case MI_DETECT_NT115:
        case MI_DISPLAY_GRAPH: case MI_DELTA_T_DETECT:
        case MI_PROFILE_ON_TIP_CHG:
            *p = normalize_enum(*p, 2);
            break;

        // --- Screen rotation: 0..3
        case MI_SCREEN_ROTATION:
            *p = normalize_enum(*p, 4);
            break;

        // --- Display temp filter: 1..10
        case MI_DISP_TEMP_FILTER:
            *p = 1.0f + fmodf(roundf(fmodf(fabsf(*p), 10.0f)), 10.0f);
            break;

        // --- Temp offset: integer
        case MI_TEMP_OFFSET:
            *p = roundf(*p);
            break;

        // --- Temperature parameters: 0..MAX_SELECTABLE_TEMPERATURE
        case MI_STARTUP_TEMP: case MI_STANDBY_TEMP:
        case MI_PRESET_TEMP_1: case MI_PRESET_TEMP_2:
            *p = fmodf(roundf(fmodf(fabsf(*p), MAX_SELECTABLE_TEMPERATURE + 1)), MAX_SELECTABLE_TEMPERATURE + 1);
            break;

        // --- Power limits: 0..MAX_POWER (step 5)
        case MI_POWER_LIM_T245: case MI_POWER_LIM_T210:
        case MI_POWER_LIM_NT115: case MI_POWER_LIM_NO_NAME:
            *p = fmodf(roundf(fmodf(fabsf(*p), MAX_POWER + 5)), MAX_POWER + 5);
            break;

        // --- Default: absolute value
        default:
            *p = fabsf(*p);
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
    	handle_button_status(); // process SW_2/SW_3 button presses (up/down navigation)
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
            UG_FillFrame(0, 0, 239, 31 + menu_lines_on_screen * 26, RGB_to_BRG(C_BLACK));
            page_start = new_page_start;

            // Redraw header
            if (level == 0) {
                LCD_PutStr(5, 5, "SETTINGS", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
            } else {
                LCD_PutStr(5, 5, MENU_GROUPS[current_group].title, FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
            }
            LCD_DrawLine(0, 25, 240, 25, RGB_to_BRG(C_DARK_SEA_GREEN));
            LCD_DrawLine(0, 26, 240, 26, RGB_to_BRG(C_DARK_SEA_GREEN));
            LCD_DrawLine(0, 27, 240, 27, RGB_to_BRG(C_DARK_SEA_GREEN));

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
                	    uint16_t abs_mi = MENU_GROUPS[current_group].idx[pos];
                	    LCD_PutStr(5, line_y, mi_name(abs_mi), FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

                	    // Do not display values for the System group
                	    if (!(current_group == (GROUPS_COUNT - 1))) {
                	        uint8_t fi = mi_to_fi(abs_mi);
                	        display_menu_value_line(
                	            line,
                	            abs_mi,
                	            ((float*)&flash_values)[fi],
                	            0, 0,
                	            190, line_y,
								RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
                	        );
                	    }
                	}
                    else if (pos == MENU_GROUPS[current_group].count) {
                        // Back row
                        LCD_PutStr(5, line_y, "Back", FONT_arial_20X23, RGB_to_BRG(C_LIGHT_GRAY), RGB_to_BRG(C_BLACK));
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

            // MI_ identifier of the parameter being edited
            uint16_t abs_mi = MENU_GROUPS[current_group].idx[editing_index];
            uint8_t  fi     = mi_to_fi(abs_mi);

            // Increment/decrement from the edit "zero"
            int32_t delta = ((int32_t)TIM2->CNT - (int32_t)edit_cnt0) / 2;

            float new_val;
            if (abs_mi == MI_POWER_LIM_T245 || abs_mi == MI_POWER_LIM_T210 ||
                abs_mi == MI_POWER_LIM_NT115 || abs_mi == MI_POWER_LIM_NO_NAME) {
                new_val = old_value + (float)delta * 5.0f;
            } else {
                new_val = old_value + (float)delta;
            }

            ((float*)&flash_values)[fi] = new_val;
            normalize_param(abs_mi);  // original normalization/cycling function
            float norm_val = ((float*)&flash_values)[fi];

            // Redraw ONLY the value zone, inverting colors
            display_menu_value_line(
                line,
                abs_mi,
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

                // Profiles group — delegate to separate menu
                if (MENU_GROUPS[current_group].count == 0) {
                    wait_button_release();
                    profiles_menu();
                    // Return to group list
                    TIM2->CNT = 1000 + current_group * 2;
                    cursor = current_group;
                    page_start = 0xFFFF;
                    redraw_page = 1;
                } else {
                    level = 1;
                    // Reset cursor/page for the selected group
                    TIM2->CNT = 1000;
                    cursor = 0;
                    page_start = 0xFFFF;
                    redraw_page = 1;
                }

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
                    uint16_t abs_mi = MENU_GROUPS[current_group].idx[cursor];

                    if (abs_mi == MI_EXIT_NO_SAVE) {
                        settings_menu_active = 0;
                        HAL_NVIC_SystemReset();
                    } else if (abs_mi == MI_SAVE_REBOOT) {
                        STORAGE_SETTINGS_DRIVER->write(SETTINGS_PAGE, &flash_values, sizeof(Flash_values));
                        settings_menu_active = 0;
                        HAL_NVIC_SystemReset();
                    } else if (abs_mi == MI_LOAD_DEFAULT) {
                        flash_values = default_flash_values;
                        redraw_page = 1; // values changed - redraw
                    } else {
                        // Enter value editing
                        uint8_t fi = mi_to_fi(abs_mi);
                        old_value = ((float*)&flash_values)[fi];
                        editing_index = cursor;
                        edit_cnt0 = TIM2->CNT;      // "zero" for delta
                        level = 2;

                        // Remove cursor frame (level 2 does not draw it),
                        //   on entering edit mode redraw the value with inversion
                        uint8_t line = cursor % menu_lines_on_screen;
                        uint16_t line_y = 35 + line * 26;
                        display_menu_value_line(
                            line,
                            abs_mi,
                            ((float*)&flash_values)[fi],
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
                uint16_t abs_mi = MENU_GROUPS[current_group].idx[editing_index];
                uint8_t  fi     = mi_to_fi(abs_mi);
                display_menu_value_line(
                    line,
                    abs_mi,
                    ((float*)&flash_values)[fi],
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
