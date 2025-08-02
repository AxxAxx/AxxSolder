/* menu_settings.c*/

#include "menu_settings.h"

/* List of names for settings menu */
#define menu_length 30
char menu_names[menu_length][30] = {
		"Startup Temp °C",
		"Temp Offset °C",
		"Standby Temp °C",
		"Standby Time [min]",
		"Sleep Time [min]",
		"Buzzer Enabled",
		"Preset Temp 1 °C",
		"Preset Temp 2 °C",
		"GPIO4 ON at run",
		"Screen Rotation",
		"Limit Power [W]",
		"I Measurement",
		"Startup Beep",
		"Temp in Celsius",
		"Temp cal 100",
		"Temp cal 200",
		"Temp cal 300",
		"Temp cal 350",
		"Temp cal 400",
		"Temp cal 450",
		"Serial DEBUG",
		"Disp Temp. filter",
		"Start at prev. temp",
		"3-button mode",
		"Beep at set temp",
		"Beep tone",
		"Momentary stand",
		"-Load Default-",
		"-Save and Reboot-",
		"-Exit no Save-"
};


uint8_t redraw_menu_screen = 0;

/* Function to left align a string from float */
void left_align_float(char* str, float number, int8_t len)
{
	char tempstring[len];
	memset(&tempstring, '\0', len);
	sprintf(tempstring, "%.0f", number);
	strcpy(str, tempstring);
}

// ==== Enumerated strings ====
const char* bool_str[] = { "No", "Yes" };
const char* screen_rotation_str[] = { "0°", "90°", "180°", "270°" };

// ==== Table of enumerated parameters ====
typedef struct {
	uint8_t index;
	const char** values;
	uint8_t count;
} EnumParam;

EnumParam enum_params[] = {
	{ 5,  bool_str, 2 },    // Buzzer Enabled
	{ 8,  bool_str, 2 },    // GPIO4 ON at run
	{11,  bool_str, 2 },    // I Measurement
	{12,  bool_str, 2 },    // Startup Beep
	{13,  bool_str, 2 },    // Temp in Celsius
	{20,  bool_str, 2 },    // Serial DEBUG
	{22,  bool_str, 2 },    // Start at previous temp
	{23,  bool_str, 2 },    // 3-button mode
	{24,  bool_str, 2 },    // Beep at set temp
	{9, screen_rotation_str, 4 },  // Screen rotation
	{26,  bool_str, 2 }     // Momentary stand
};

#define ENUM_PARAM_COUNT (sizeof(enum_params) / sizeof(enum_params[0]))

// ==== Getting string value of a parameter ====
const char* get_enum_value_str(uint8_t index, float value) {
	for (uint8_t i = 0; i < ENUM_PARAM_COUNT; i++) {
		if (enum_params[i].index == index) {
			int v = (int)roundf(value);  // safe rounding
			if (v >= 0 && v < enum_params[i].count) {
				return enum_params[i].values[v]; // valid index
			} else {
				return "?";  // protection from out-of-bounds
			}
		}
	}
	return NULL;  // if enum parameter not found
}

// ==== Display parameter string in menu ====
// Universal output of menu item value: string or number with alignment

#define MENU_VALUE_W   47      // value field width
#define MENU_VALUE_H   18      // value field height

#define MAX_MENU_LINES 10

static char     prev_buf_screen[MAX_MENU_LINES][12] = {0};
static uint8_t  prev_flags_screen[MAX_MENU_LINES]   = {0};

uint8_t menu_lines_on_screen = 7; // dynamically changes based on screen rotation


// ==== Function to check if value differs from default ====
static uint8_t is_value_different_from_default(uint16_t index, float current_value) {
        if (index >= menu_length - 3) return 0; // Skip special menu items

        float default_value = ((float*)&default_flash_values)[index];

        // Compare with small tolerance for floating point values
        return (fabsf(current_value - default_value) > 0.1f);
}

/* Display value of a parameter with caching to avoid unnecessary redraw */
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

	// Fill on edit mode change
	uint8_t prev_editing = (prev_flags >> 1) & 1;

	if (editing && !prev_editing) {
		// Entering edit mode: fill white
		UG_FillFrame(x, y, x + MENU_VALUE_W, y + MENU_VALUE_H, RGB_to_BRG(C_WHITE));
	}
	else if (!editing && prev_editing && selected) {
		// Exiting edit mode while still selected: fill black
		UG_FillFrame(x, y, x + MENU_VALUE_W, y + MENU_VALUE_H, RGB_to_BRG(C_BLACK));
	}

	/* Drawing text if value or flags have changed */
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

		UG_FillFrame(x, y, 190 + MENU_VALUE_W, y + MENU_VALUE_H, bg);
		LCD_PutStr(x, y, buf, FONT_arial_20X23, fg, bg);
	}
}

// Cyclical rounding:
static inline float normalize_enum(float val, uint8_t count) {
	int v = (int)roundf(val);
	while (v < 0) v += count;
	v %= count;
	return (float)v;
}

// menu parameter constraints
void normalize_param(uint16_t index) {
	float* p = &((float*)&flash_values)[index];

	switch(index) {
		// --- Boolean or binary parameters: constrained to 0 or 1
		case 5: case 8: case 11: case 12:
		case 13: case 20: case 22: case 23: case 24: case 26: case 28:
			*p = normalize_enum(*p, 2);  // 0/1, cyclic
			break;

		// --- Parameters with range 0 to 3 (4 values)
		case 9: case 25:
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

		// --- Temperature parameters: constrained to acceptable range
		case 0: case 2: case 6: case 7:
			*p = fmod(round(fmod(fabs(*p), MAX_SELECTABLE_TEMPERATURE + 1)), MAX_SELECTABLE_TEMPERATURE + 1);
			break;

		// --- Power parameter: 0..(MAX_POWER + 4)
		case 10:
			*p = fmod(round(fmod(fabs(*p), MAX_POWER + 5)), MAX_POWER + 5);
			break;

		// --- Default: just take absolute value (positive only)
		default:
			*p = fabs(*p);
			break;
	}
}

void settings_menu()
{
	// Transition system to safe state (disable heating)
	change_state(EMERGENCY_SLEEP);
	sensor_values.requested_power = 0; // disable heating before menu

	settings_menu_active = 1; // menu activity flag
	UG_FillScreen(RGB_to_BRG(C_BLACK)); // Clear screen
	UG_FontSetTransparency(1); // Set text background transparency

	// === Set number of lines on screen based on orientation
	// For screen rotation 0° or 180° — more vertical space => 10 lines
	// For screen rotation 90° or 270° — "tall" screen, fits only 7 lines
	// Display firmware and hardware version ===
	char str[64] = {0};
	if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
		menu_lines_on_screen = 10;
		sprintf(str, "fw: %d.%d.%d   hw: %d", fw_version_major, fw_version_minor, fw_version_patch, get_hw_version());
		LCD_PutStr(6, 300, str, FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_BLACK));
	} else {
		menu_lines_on_screen = 7;
		sprintf(str, "fw: %d.%d.%d   hw: %d", fw_version_major, fw_version_minor, fw_version_patch, get_hw_version());
		LCD_PutStr(6, 215, str, FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_BLACK));
	}

	// === Menu title ===
	LCD_PutStr(70, 5, "SETTINGS", FONT_arial_20X23, RGB_to_BRG(C_DARK_SEA_GREEN), RGB_to_BRG(C_BLACK));
	LCD_DrawLine(0, 25, 240, 25, RGB_to_BRG(C_DARK_SEA_GREEN));
	LCD_DrawLine(0, 26, 240, 26, RGB_to_BRG(C_DARK_SEA_GREEN));
	LCD_DrawLine(0, 27, 240, 27, RGB_to_BRG(C_DARK_SEA_GREEN));

	// Initial encoder value
	TIM2->CNT = 1000;

	// === Control variables ===
	uint16_t menu_cursor_position = 0;
	uint8_t prev_menu_start = 0xFF;
	uint8_t menu_level = 0;
	uint8_t prev_cursor_position = 0xFF;
	uint8_t prev_menu_level = 0xFF;
	uint8_t menu_active = 1;

	float old_value = 0;
	int16_t prev_displayed_value = INT16_MIN;
	bool force_redraw_value = true;

	// === Wait for button release before entering menu ===
	while (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {}

	// === Main menu loop ===
	while (menu_active)
	{
		handle_button_status(); // update button states

		// === Navigation or parameter editing ===
		if (menu_level == 0) {
			// Cursor — by encoder
			TIM2->CNT = clamp(TIM2->CNT, 1000, 1000000);
			menu_cursor_position = (TIM2->CNT - 1000) / 2;
		} else if (menu_level == 1) {
			// === Parameter editing mode ===
			float new_val;
			if (menu_cursor_position == 10) {
				new_val = old_value + round(((float)(TIM2->CNT - 1000.0) / 2.0 - menu_cursor_position)) * 5;
			} else {
				new_val = old_value + ((float)(TIM2->CNT - 1000.0) / 2.0 - menu_cursor_position);
			}
			((float*)&flash_values)[menu_cursor_position] = new_val;
			normalize_param(menu_cursor_position);
			float norm_val = ((float*)&flash_values)[menu_cursor_position];

			if (force_redraw_value || abs((int)norm_val - prev_displayed_value) >= 1) {
				uint8_t line = menu_cursor_position % menu_lines_on_screen;
				uint16_t line_y = 35 + line * 25;

				display_menu_value_line(
					line,
					menu_cursor_position,
					norm_val,
					true, true,
					190, line_y,
					RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
				);
				prev_displayed_value = norm_val;
				force_redraw_value = false;
			}
		}

		// === Limit menu length ===
		if (menu_cursor_position > menu_length - 1) {
			menu_cursor_position = menu_length - 1;
			TIM2->CNT = 1000 + (menu_length - 1) * 2;
		}

		// === Start of current menu "page" ===
		uint16_t new_menu_start = (menu_cursor_position / menu_lines_on_screen) * menu_lines_on_screen;

		// === Handle button presses ===
		if ((HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) && (menu_cursor_position < menu_length - 3)) {
			if (menu_level == 0) old_value = ((float*)&flash_values)[menu_cursor_position];
			if (menu_level == 1) TIM2->CNT = menu_cursor_position * 2 + 1000;

			menu_level = !menu_level; // toggle modes
			force_redraw_value = true;
			HAL_Delay(200); // debounce
		}
		else if ((HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) && (menu_cursor_position == menu_length - 1)) {
			// === RESET ===
			menu_active = 0;
			HAL_NVIC_SystemReset();
		}
		else if ((HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) && (menu_cursor_position == menu_length - 2)) {
			// === SAVE + RESET ===
			menu_active = 0;
			FlashWrite(&flash_values);
			HAL_NVIC_SystemReset();
		}
		else if ((HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) && (menu_cursor_position == menu_length - 3)) {
			// === Load default values ===
			flash_values = default_flash_values;
			redraw_menu_screen = 1;
		}

		// === Redraw new menu page when "screen" changes ===
		if ((new_menu_start != prev_menu_start) | redraw_menu_screen) {
			UG_FillFrame(0, 30, 239, 30 + menu_lines_on_screen * 25, RGB_to_BRG(C_BLACK)); // clear area
			prev_menu_start = new_menu_start;

			// clear line cache
			for (int i = 0; i < menu_lines_on_screen; i++) {
				prev_buf_screen[i][0] = '\0';
				prev_flags_screen[i] = 0xFF;
			}

			// draw parameter lines and values
			for (int line = 0; line < menu_lines_on_screen && (new_menu_start + line) < menu_length; line++) {
				uint16_t index = new_menu_start + line;
				uint16_t line_y = 35 + line * 25;

				UG_FillFrame(5, line_y, 187, line_y + MENU_VALUE_H, RGB_to_BRG(C_BLACK));
				LCD_PutStr(5, line_y, menu_names[index], FONT_arial_20X23, RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK));

				if (index < menu_length - 3) {
					display_menu_value_line(
						line,
						index,
						((float*)&flash_values)[index],
						false, false,
						190, line_y,
						RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
					);
				}
			}
		}

		// === Handle cursor movement or mode change ===
		if ((menu_cursor_position != prev_cursor_position || menu_level != prev_menu_level) | redraw_menu_screen) {
			redraw_menu_screen = 0;
			// Erase previous highlight box
			if (prev_cursor_position != 0xFF &&
				prev_cursor_position / menu_lines_on_screen == menu_cursor_position / menu_lines_on_screen) {
				uint8_t prev_line = prev_cursor_position % menu_lines_on_screen;
				uint16_t prev_line_y = 35 + prev_line * 25;

				UG_DrawFrame(2, prev_line_y - 4, 237, prev_line_y + 2 + MENU_VALUE_H, RGB_to_BRG(C_BLACK));
				UG_DrawFrame(3, prev_line_y - 3, 236, prev_line_y + 1 + MENU_VALUE_H, RGB_to_BRG(C_BLACK));

				// update value
				if (prev_cursor_position < menu_length - 3) {
					display_menu_value_line(
						prev_line,
						prev_cursor_position,
						((float*)&flash_values)[prev_cursor_position],
						false, false,
						190, prev_line_y,
						RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
					);
				}
			}

			// Draw new box/highlight
			if (menu_cursor_position / menu_lines_on_screen == prev_menu_start / menu_lines_on_screen) {
				uint8_t line = menu_cursor_position % menu_lines_on_screen;
				uint16_t line_y = 35 + line * 25;

				if (menu_level == 0) {
					// cursor — frame
					UG_DrawFrame(2, line_y - 4, 237, line_y + 2 + MENU_VALUE_H, RGB_to_BRG(C_YELLOW));
					UG_DrawFrame(3, line_y - 3, 236, line_y + 1 + MENU_VALUE_H, RGB_to_BRG(C_YELLOW));
				} else {
					// editing — highlight value
					display_menu_value_line(
						line,
						menu_cursor_position,
						((float*)&flash_values)[menu_cursor_position],
						true, true,
						190, line_y,
						RGB_to_BRG(C_WHITE), RGB_to_BRG(C_BLACK)
					);
				}
			}

			// Update previous values
			prev_cursor_position = menu_cursor_position;
			prev_menu_level = menu_level;
		}
	}
}
