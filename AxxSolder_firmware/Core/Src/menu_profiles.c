#include "menu_profiles.h"
#include "menu_settings.h"
#include "tip_profile.h"
#include "storage.h"
#include "lcd.h"
#include "gui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---- Shared state from menu_settings / main ---- */
extern volatile uint8_t SW_1_pressed;
extern volatile uint8_t SW_1_pressed_long;
extern volatile uint8_t SW_2_pressed;
extern volatile uint8_t SW_3_pressed;
extern uint8_t settings_menu_active;
extern uint8_t menu_lines_on_screen;

/* ---- LCD helpers (same patterns as menu_settings.c) ---- */

static inline void wait_btn_release(void)
{
	while (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {}
	HAL_Delay(50);
}

static inline int16_t enc_sel(uint32_t cnt0, volatile uint32_t *pCnt, uint16_t max_items)
{
	if (max_items == 0) return 0;
	if (*pCnt < cnt0) *pCnt = cnt0;
	if (*pCnt > 1000000) *pCnt = 1000000;
	uint16_t sel = (*pCnt - cnt0) / 2;
	if (sel >= max_items) {
		sel = max_items - 1;
		*pCnt = cnt0 + sel * 2;
	}
	return sel;
}

static void draw_header(const char *title)
{
	UG_FillFrame(0, 0, 239, 29, C_BLACK);
	LCD_PutStr(5, 5, (char *)title, FONT_arial_20X23,
	           C_DARK_SEA_GREEN, C_BLACK);
	LCD_DrawLine(0, 25, 240, 25, C_DARK_SEA_GREEN);
	LCD_DrawLine(0, 26, 240, 26, C_DARK_SEA_GREEN);
	LCD_DrawLine(0, 27, 240, 27, C_DARK_SEA_GREEN);
}

static void draw_line(uint8_t line, const char *text, uint16_t fg, uint16_t bg)
{
	uint16_t y = 35 + line * 26;
	UG_FillFrame(0, y - 2, 239, y + 22, bg);
	LCD_PutStr(5, y, (char *)text, FONT_arial_20X23, fg, bg);
}

static void draw_cursor(uint8_t line)
{
	uint16_t y = 35 + line * 26;
	UG_DrawFrame(0, y - 3, 239, y + 23, C_YELLOW);
	UG_DrawFrame(1, y - 2, 238, y + 22, C_YELLOW);
}

static void erase_cursor(uint8_t line)
{
	uint16_t y = 35 + line * 26;
	UG_DrawFrame(0, y - 3, 239, y + 23, C_BLACK);
	UG_DrawFrame(1, y - 2, 238, y + 22, C_BLACK);
}

/* ---- Handle type to string ---- */
static const char *handle_str[] = {
	[NT115]   = "NT115",
	[T210]    = "T210",
	[T245]    = "T245",
	[No_name] = "No_name",
};

static const char *handle_to_str(uint8_t h)
{
	if (h <= No_name) return handle_str[h];
	return "?";
}

/* ---- Profile field names for edit submenu ---- */
typedef enum {
	PF_NAME = 0,
	PF_HANDLE,
	PF_KP,
	PF_KI,
	PF_KD,
	PF_MAX_I,
	PF_POWER_LIM,
	PF_CAL100,
	PF_CAL200,
	PF_CAL300,
	PF_CAL350,
	PF_CAL400,
	PF_CAL450,
	PF_SET_ACTIVE,
	PF_DELETE,
	PF_BACK,
	PF_COUNT
} ProfileField;

static const char *field_names[] = {
	"Name",
	"Handle",
	"Kp",
	"Ki",
	"Kd",
	"Max I",
	"Power Lim",
	"Cal 100",
	"Cal 200",
	"Cal 300",
	"Cal 350",
	"Cal 400",
	"Cal 450",
	"-Set Active-",
	"-Delete-",
	"Back"
};

/* ---- Value zone helpers (split layout like settings_menu) ---- */
#define PROF_VALUE_X  130

static void draw_prof_value(uint8_t line, const char *text, uint16_t fg, uint16_t bg)
{
	uint16_t y = 35 + line * 26;
	UG_FillFrame(PROF_VALUE_X, y - 2, 239, y + 22, bg);
	LCD_PutStr(PROF_VALUE_X + 3, y, (char *)text, FONT_arial_20X23, fg, bg);
}

static void format_float1(char *buf, uint8_t len, float v)
{
	int whole = (int)v;
	int frac  = (int)(fabsf(v - (float)whole) * 10.0f + 0.5f);
	if (frac >= 10) { whole += (v >= 0) ? 1 : -1; frac = 0; }
	snprintf(buf, len, "%d.%d", whole, frac);
}

static void format_prof_value(uint8_t fi, const TipProfile *e, char *buf, uint8_t len)
{
	switch (fi) {
	case PF_NAME:      snprintf(buf, len, "%s", e->name); break;
	case PF_HANDLE:    snprintf(buf, len, "%s", handle_to_str(e->handle_type)); break;
	case PF_KP:        format_float1(buf, len, e->kp); break;
	case PF_KI:        format_float1(buf, len, e->ki); break;
	case PF_KD:        format_float1(buf, len, e->kd); break;
	case PF_MAX_I:     snprintf(buf, len, "%d", (int)e->max_i); break;
	case PF_POWER_LIM: snprintf(buf, len, "%d", (int)e->power_limit); break;
	case PF_CAL100:    snprintf(buf, len, "%d", (int)e->temp_cal[0]); break;
	case PF_CAL200:    snprintf(buf, len, "%d", (int)e->temp_cal[1]); break;
	case PF_CAL300:    snprintf(buf, len, "%d", (int)e->temp_cal[2]); break;
	case PF_CAL350:    snprintf(buf, len, "%d", (int)e->temp_cal[3]); break;
	case PF_CAL400:    snprintf(buf, len, "%d", (int)e->temp_cal[4]); break;
	case PF_CAL450:    snprintf(buf, len, "%d", (int)e->temp_cal[5]); break;
	default:           buf[0] = '\0'; break;
	}
}

static float *get_editable_field(uint8_t fi, TipProfile *e)
{
	switch (fi) {
	case PF_KP:     return &e->kp;
	case PF_KI:     return &e->ki;
	case PF_KD:     return &e->kd;
	case PF_MAX_I:     return &e->max_i;
	case PF_POWER_LIM: return &e->power_limit;
	case PF_CAL100: return &e->temp_cal[0];
	case PF_CAL200: return &e->temp_cal[1];
	case PF_CAL300: return &e->temp_cal[2];
	case PF_CAL350: return &e->temp_cal[3];
	case PF_CAL400: return &e->temp_cal[4];
	case PF_CAL450: return &e->temp_cal[5];
	default:        return NULL;
	}
}

/* ---- Name editor ---- */
/*
 * Controls:
 *   Encoder        – cycle character at cursor through the charset
 *   SW_1 (short)   – accept current char, advance cursor right
 *   SW_2 (short)   – move cursor left
 *   SW_3 (short)   – delete char at cursor (shift tail left)
 *   SW_1 (long)    – confirm name and exit
 */

static const char name_charset[] =
    " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.";
#define NAME_CHARSET_LEN ((uint8_t)(sizeof(name_charset) - 1))

static uint8_t char_to_charset_idx(char c)
{
    for (uint8_t i = 0; i < NAME_CHARSET_LEN; i++)
        if (name_charset[i] == c) return i;
    return 0;
}

/* Scrolling name row: FONT_arial_20X23 (20 px wide), 12-char window.
 * The window slides so the cursor is always visible.
 * Cursor character is drawn inverted (black on white). */
#define NAME_CW        20   /* char width in pixels  */
#define NAME_CH        23   /* char height in pixels */
#define NAME_WIN       12   /* visible chars         */
#define NAME_ROW_Y     38

static void draw_name_row(const char *name, uint8_t cursor, uint8_t maxlen)
{
    const uint16_t fg = C_WHITE;
    const uint16_t bg = C_BLACK;

    /* Slide window so cursor stays in view */
    uint8_t win = (cursor >= NAME_WIN) ? cursor - NAME_WIN + 1 : 0;

    UG_FillFrame(0, NAME_ROW_Y - 2, 239, NAME_ROW_Y + NAME_CH + 2, bg);

    for (uint8_t i = 0; i < NAME_WIN; i++) {
        uint8_t si = win + i;
        if (si >= maxlen - 1) break;
        char c = (si < (uint8_t)strnlen(name, maxlen - 1)) ? name[si] : ' ';
        uint16_t x = i * NAME_CW;
        if (si == cursor) {
            UG_FillFrame(x, NAME_ROW_Y - 1, x + NAME_CW - 1, NAME_ROW_Y + NAME_CH + 1, fg);
            LCD_PutChar(x, NAME_ROW_Y, c, (UG_FONT*)&FONT_arial_20X23, bg, fg);
        } else {
            LCD_PutChar(x, NAME_ROW_Y, c, (UG_FONT*)&FONT_arial_20X23, fg, bg);
        }
    }

    /* Scroll indicators */
    if (win > 0)
        LCD_PutStr(0, NAME_ROW_Y + NAME_CH + 4, "<",
                   FONT_arial_12X15, C_YELLOW, bg);
    else
        UG_FillFrame(0, NAME_ROW_Y + NAME_CH + 4, 14, NAME_ROW_Y + NAME_CH + 18, bg);

    uint8_t len = (uint8_t)strnlen(name, maxlen - 1);
    if (win + NAME_WIN < len)
        LCD_PutStr(225, NAME_ROW_Y + NAME_CH + 4, ">",
                   FONT_arial_12X15, C_YELLOW, bg);
    else
        UG_FillFrame(225, NAME_ROW_Y + NAME_CH + 4, 239, NAME_ROW_Y + NAME_CH + 18, bg);
}

static void edit_name(char *name, uint8_t maxlen)
{
    const uint16_t bg = C_BLACK;

    /* Ensure null termination */
    name[maxlen - 1] = '\0';
    uint8_t len = (uint8_t)strnlen(name, maxlen - 1);

    /* Pad to at least one char so there is always a cursor position */
    if (len == 0) { name[0] = 'A'; name[1] = '\0'; len = 1; }

    uint8_t cursor = 0;

    /* Clear SW flags so stale presses don't fire immediately */
    SW_1_pressed      = 0;
    SW_1_pressed_long = 0;
    SW_2_pressed      = 0;
    SW_3_pressed      = 0;

    /* Init encoder to the charset position of the current char */
    TIM2->CNT = 1000 + char_to_charset_idx(name[cursor]) * 2;

    /* Full redraw */
    UG_FillScreen(C_BLACK);
    draw_header("EDIT NAME");

    /* Hints (static — drawn once) */
    LCD_PutStr(5, 265, "SW1:next  SW2:<  SW3:del",
               FONT_arial_16X18, C_DARK_SEA_GREEN, bg);
    LCD_PutStr(5, 288, "Hold SW1 to confirm",
               FONT_arial_16X18, C_DARK_SEA_GREEN, bg);

    draw_name_row(name, cursor, maxlen);

    uint8_t prev_chr_idx = char_to_charset_idx(name[cursor]);

    while (1) {
        /* ---- Encoder: update char under cursor ---- */
        uint8_t chr_idx = (uint8_t)enc_sel(1000, &TIM2->CNT, NAME_CHARSET_LEN);
        if (chr_idx != prev_chr_idx) {
            name[cursor] = name_charset[chr_idx];
            prev_chr_idx = chr_idx;
            draw_name_row(name, cursor, maxlen);
        }

        /* ---- SW_1 short: advance cursor ---- */
        if (SW_1_pressed) {
            SW_1_pressed = 0;
            if (cursor < maxlen - 2) {
                cursor++;
                /* Extend string if cursor moved past current end */
                if (cursor >= (uint8_t)strnlen(name, maxlen - 1)) {
                    name[cursor]     = ' ';
                    name[cursor + 1] = '\0';
                }
                TIM2->CNT = 1000 + char_to_charset_idx(name[cursor]) * 2;
                prev_chr_idx = char_to_charset_idx(name[cursor]);
                draw_name_row(name, cursor, maxlen);
            }
        }

        /* ---- SW_2: cursor left ---- */
        if (SW_2_pressed) {
            SW_2_pressed = 0;
            if (cursor > 0) {
                cursor--;
                TIM2->CNT = 1000 + char_to_charset_idx(name[cursor]) * 2;
                prev_chr_idx = char_to_charset_idx(name[cursor]);
                draw_name_row(name, cursor, maxlen);
            }
        }

        /* ---- SW_3: delete char at cursor ---- */
        if (SW_3_pressed) {
            SW_3_pressed = 0;
            uint8_t l = (uint8_t)strnlen(name, maxlen - 1);
            if (l > 1) {
                /* Shift tail left */
                for (uint8_t i = cursor; i < l - 1; i++)
                    name[i] = name[i + 1];
                name[l - 1] = '\0';
                /* Keep cursor in range */
                l--;
                if (cursor >= l) cursor = l - 1;
            } else {
                /* Last char: replace with space rather than empty string */
                name[0] = ' ';
            }
            TIM2->CNT = 1000 + char_to_charset_idx(name[cursor]) * 2;
            prev_chr_idx = char_to_charset_idx(name[cursor]);
            draw_name_row(name, cursor, maxlen);
        }

        /* ---- SW_1 long: confirm ---- */
        if (SW_1_pressed_long) {
            SW_1_pressed_long = 0;
            break;
        }

        HAL_Delay(10);
    }

    /* Trim trailing spaces */
    uint8_t l = (uint8_t)strnlen(name, maxlen - 1);
    while (l > 0 && name[l - 1] == ' ') l--;
    name[l] = '\0';
    /* Guard: prevent empty name */
    if (l == 0) { strncpy(name, "Profile", maxlen - 1); name[maxlen - 1] = '\0'; }
}

/* ---- Edit single profile ---- */
static void edit_profile(uint8_t prof_idx)
{
	TipProfile *p = tip_profiles_get(prof_idx);
	if (!p) return;

	TipProfile edit = *p; /* work on copy */
	uint16_t fg = C_WHITE;
	uint16_t bg = C_BLACK;

	int16_t cursor = 0, prev_cursor = -1;
	uint8_t redraw = 1;
	uint8_t running = 1;

	/* Edit state (like level 2 in settings_menu) */
	uint8_t editing = 0;
	float old_value = 0;
	uint32_t edit_cnt0 = 0;
	float *edit_val = NULL;
	int32_t prev_delta = 0;

	TIM2->CNT = 1000;

	while (running) {
		handle_button_status();

		/* Encoder controls cursor in browse mode, value in edit mode */
		if (!editing) {
			cursor = enc_sel(1000, &TIM2->CNT, PF_COUNT);
		}

		uint8_t page_start = (cursor / menu_lines_on_screen) * menu_lines_on_screen;

		/* ---- Full page redraw ---- */
		if (redraw) {
			UG_FillFrame(0, 33, 239, 319, bg);
			draw_header(edit.name);

			for (uint8_t i = 0; i < menu_lines_on_screen && (page_start + i) < PF_COUNT; i++) {
				uint8_t fi = page_start + i;

				if (fi == PF_NAME || fi >= PF_SET_ACTIVE) {
					/* Full-width items: Name (read-only), actions */
					char line[38];
					uint16_t line_fg = fg;
					if (fi == PF_NAME) {
						snprintf(line, sizeof(line), "Name: %s [...]", edit.name);
					} else if (fi == PF_DELETE) {
						snprintf(line, sizeof(line), "%s", field_names[fi]);
						line_fg = C_RED;
					} else if (fi == PF_BACK) {
						snprintf(line, sizeof(line), "%s", field_names[fi]);
						line_fg = C_LIGHT_GRAY;
					} else {
						snprintf(line, sizeof(line), "%s", field_names[fi]);
						line_fg = C_YELLOW;
					}
					draw_line(i, line, line_fg, bg);
				} else {
					/* Split layout: name left, value right */
					uint16_t y = 35 + i * 26;
					UG_FillFrame(0, y - 2, PROF_VALUE_X - 1, y + 22, bg);
					LCD_PutStr(5, y, (char *)field_names[fi], FONT_arial_20X23, fg, bg);

					char val_buf[16];
					format_prof_value(fi, &edit, val_buf, sizeof(val_buf));
					draw_prof_value(i, val_buf, fg, bg);
				}
			}
			draw_cursor(cursor - page_start);
			prev_cursor = cursor;
			redraw = 0;
		}

		uint8_t vis = cursor - page_start;

		/* ---- Cursor update (browse mode only) ---- */
		if (!editing && cursor != prev_cursor) {
			if (prev_cursor >= 0) {
				uint8_t prev_vis = prev_cursor - page_start;
				if (prev_vis < menu_lines_on_screen) {
					erase_cursor(prev_vis);
				}
			}
			draw_cursor(vis);

			/* Page changed? */
			uint8_t prev_page = (prev_cursor >= 0)
				? (prev_cursor / menu_lines_on_screen) * menu_lines_on_screen : 0xFF;
			if (prev_page != page_start) redraw = 1;

			prev_cursor = cursor;
		}

		/* ---- Edit mode: update value zone only ---- */
		if (editing && edit_val) {
			int32_t delta = ((int32_t)TIM2->CNT - (int32_t)edit_cnt0) / 2;
			if (delta != prev_delta) {
				prev_delta = delta;
				*edit_val = old_value + (float)delta;
				if (*edit_val < 0) *edit_val = 0;

				char val_buf[16];
				format_prof_value(cursor, &edit, val_buf, sizeof(val_buf));
				draw_prof_value(vis, val_buf, C_BLACK, C_WHITE);
				draw_cursor(vis);
			}
		}

		/* ---- Button press ---- */
		if (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {
			wait_btn_release();

			if (editing) {
				/* Exit edit mode — restore value to normal colors */
				editing = 0;
				TIM2->CNT = 1000 + cursor * 2;

				char val_buf[16];
				format_prof_value(cursor, &edit, val_buf, sizeof(val_buf));
				draw_prof_value(vis, val_buf, fg, bg);
				draw_cursor(vis);

				edit_val = NULL;
				prev_cursor = cursor;
			} else {
				switch (cursor) {
				case PF_NAME: {
					edit_name(edit.name, sizeof(edit.name));
					/* Restore full editor screen after name editing */
					UG_FillScreen(C_BLACK);
					draw_header(edit.name);
					redraw = 1;
					TIM2->CNT = 1000 + cursor * 2;
					break;
				}
				case PF_HANDLE: {
					/* Cycle handle type — update value zone only */
					edit.handle_type = (edit.handle_type + 1) % 4;
					char val_buf[16];
					format_prof_value(PF_HANDLE, &edit, val_buf, sizeof(val_buf));
					draw_prof_value(vis, val_buf, fg, bg);
					draw_cursor(vis);
					break;
				}
				case PF_KP: case PF_KI: case PF_KD: case PF_MAX_I:
				case PF_POWER_LIM:
				case PF_CAL100: case PF_CAL200: case PF_CAL300:
				case PF_CAL350: case PF_CAL400: case PF_CAL450: {
					/* Enter edit mode */
					edit_val = get_editable_field(cursor, &edit);
					if (edit_val) {
						old_value = *edit_val;
						edit_cnt0 = TIM2->CNT;
						prev_delta = 0;
						editing = 1;

						/* Draw value inverted */
						char val_buf[16];
						format_prof_value(cursor, &edit, val_buf, sizeof(val_buf));
						draw_prof_value(vis, val_buf, C_BLACK, C_WHITE);
						draw_cursor(vis);
					}
					break;
				}
				case PF_SET_ACTIVE: {
					StoreResult res = tip_profiles_update(prof_idx, &edit);
					if (res == STORE_OK) {
						tip_profiles_set_active((enum handles)edit.handle_type, prof_idx);
					}
					running = 0;
					break;
				}
				case PF_DELETE: {
					StoreResult res = tip_profiles_delete(prof_idx);
					running = 0;
					break;
				}
				case PF_BACK: {
					/* Only write flash if profile was actually modified */
					if (memcmp(&edit, p, sizeof(TipProfile)) != 0) {
						tip_profiles_update(prof_idx, &edit);
					}
					running = 0;
					break;
				}
				default:
					break;
				}
			}
		}

		HAL_Delay(10);
	}
}

/* ---- Main profiles menu ---- */
void profiles_menu(void)
{
	uint16_t fg = C_WHITE;
	uint16_t bg = C_BLACK;

	int16_t cursor = 0, prev_cursor = -1;
	uint8_t redraw = 1;
	uint8_t running = 1;

	TIM2->CNT = 1000;

	while (running) {
		handle_button_status();

		uint8_t count = tip_profiles_count();
		uint8_t can_add = (count < MAX_PROFILES);
		/* Items: profiles[0..count-1] + (optional)"+ Add New" + "Back" */
		uint16_t list_len = count + 1 + can_add;
		uint16_t add_idx  = count; /* index of "+ Add New" row */

		cursor = enc_sel(1000, &TIM2->CNT, list_len);

		uint8_t page_start = (cursor / menu_lines_on_screen) * menu_lines_on_screen;

		if (redraw) {
			UG_FillFrame(0, 33, 239, 319, bg);
			draw_header("PROFILES");
			char cnt[12];
			snprintf(cnt, sizeof(cnt), "[%d:%d]", count, MAX_PROFILES);
			LCD_PutStr(239 - (int)strlen(cnt) * 13, 5, cnt,
			           FONT_arial_20X23, C_DARK_SEA_GREEN, C_BLACK);

			for (uint8_t i = 0; i < menu_lines_on_screen && (page_start + i) < list_len; i++) {
				uint8_t li = page_start + i;

				if (li < count) {
					TipProfile *p = tip_profiles_get(li);
					uint8_t active_for_current = tip_profiles_get_active(attached_handle);
					char line[38];
					snprintf(line, sizeof(line), "%s %s", handle_to_str(p->handle_type), p->name);
					draw_line(i, line, fg, bg);
					if (active_for_current == li) {
						uint16_t y = 35 + i * 26;
						LCD_PutStr(200, y, "[A]", FONT_arial_20X23, C_YELLOW, bg);
					}
				} else if (can_add && li == add_idx) {
					draw_line(i, "+ Add New", C_DARK_SEA_GREEN, bg);
				} else {
					draw_line(i, "Back", C_LIGHT_GRAY, bg);
				}
			}
			draw_cursor(cursor - page_start);
			prev_cursor = cursor;
			redraw = 0;
		}

		/* Cursor */
		uint8_t vis = cursor - page_start;
		uint8_t prev_vis = prev_cursor - page_start;

		if (cursor != prev_cursor) {
			if (prev_cursor >= 0 && prev_vis < menu_lines_on_screen) {
				erase_cursor(prev_vis);
			}
			draw_cursor(vis);

			uint8_t prev_page = (prev_cursor >= 0) ? (prev_cursor / menu_lines_on_screen) * menu_lines_on_screen : 0xFF;
			if (prev_page != page_start) redraw = 1;

			prev_cursor = cursor;
		}

		/* Button */
		if (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {
			wait_btn_release();

			if (cursor < count) {
				/* Edit existing profile */
				edit_profile(cursor);
				TIM2->CNT = 1000;
				prev_cursor = -1;
				redraw = 1;
			} else if (can_add && cursor == add_idx) {
				/* Add new profile with neutral defaults, then open editor */
				TipProfile new_p;
				memset(&new_p, 0, sizeof(new_p));
				snprintf(new_p.name, sizeof(new_p.name), "Profile %d", count + 1);
				new_p.handle_type = T245;
				new_p.kp = 8.0f; new_p.ki = 2.0f; new_p.kd = 0.5f;
				new_p.max_i = 300.0f;
				new_p.power_limit = 0.0f;
				for (uint8_t ci = 0; ci < NUM_CAL_POINTS; ci++)
					new_p.temp_cal[ci] = identity_cal[ci];

				tip_profiles_add(&new_p);
				edit_profile(count); /* count == index of the just-added profile */

				TIM2->CNT = 1000;
				prev_cursor = -1;
				redraw = 1;
			} else {
				/* Back */
				running = 0;
			}
		}

		HAL_Delay(10);
	}
}

/* ---- Popup: quick profile select on handle change ---- */
void profiles_popup(enum handles h)
{
	uint16_t fg = C_WHITE;
	uint16_t bg = C_BLACK;

	/* Collect profiles matching this handle */
	uint8_t match_idx[MAX_PROFILES];
	uint8_t match_count = 0;

	for (uint8_t i = 0; i < tip_profiles_count(); i++) {
		TipProfile *p = tip_profiles_get(i);
		if (p && p->handle_type == (uint8_t)h) {
			match_idx[match_count++] = i;
		}
	}

	if (match_count == 0) return;

	/* Disable heater while popup blocks the main loop */
	sensor_values.requested_power = 0;

	/* Draw popup */
	uint16_t popup_x = 10;
	uint16_t popup_y = 50;
	uint16_t popup_w = 200;
	uint16_t popup_h = 30 + match_count * 26 + 10;

	UG_FillFrame(popup_x, popup_y, popup_x + popup_w, popup_y + popup_h, bg);
	UG_DrawFrame(popup_x, popup_y, popup_x + popup_w, popup_y + popup_h, C_YELLOW);
	UG_DrawFrame(popup_x + 1, popup_y + 1, popup_x + popup_w - 1, popup_y + popup_h - 1, C_YELLOW);
	UG_DrawFrame(popup_x + 2, popup_y + 2, popup_x + popup_w - 2, popup_y + popup_h - 2, C_YELLOW);

	LCD_PutStr(popup_x + 5, popup_y + 5, "Select profile:", FONT_arial_20X23, fg, bg);

	for (uint8_t i = 0; i < match_count; i++) {
		TipProfile *p = tip_profiles_get(match_idx[i]);
		uint16_t y = popup_y + 30 + i * 26;
		LCD_PutStr(popup_x + 10, y, p->name, FONT_arial_20X23, fg, bg);
	}

	/* Pre-select the currently active profile for this handle, if any */
	uint8_t active_idx = tip_profiles_get_active(h);
	int16_t initial_cursor = 0;
	for (uint8_t i = 0; i < match_count; i++) {
		if (match_idx[i] == active_idx) { initial_cursor = i; break; }
	}

	/* Selection with 5-second timeout (500 * 10ms) to prevent
	 * blocking the main loop indefinitely while heater is off */
	uint32_t saved_cnt = TIM2->CNT;
	TIM2->CNT = 1000 + initial_cursor * 2;
	int16_t cursor = initial_cursor, prev_cursor = -1;
	uint16_t timeout_ticks = 300;

	while (1) {
		handle_button_status();
		cursor = enc_sel(1000, &TIM2->CNT, match_count);

		if (cursor != prev_cursor) {
			if (prev_cursor >= 0) {
				uint16_t py = popup_y + 30 + prev_cursor * 26;
				UG_DrawFrame(popup_x + 3, py - 2, popup_x + popup_w - 3, py + 22, bg);
				UG_DrawFrame(popup_x + 4, py - 1, popup_x + popup_w - 4, py + 21, bg);
			}
			uint16_t cy = popup_y + 30 + cursor * 26;
			UG_DrawFrame(popup_x + 3, cy - 2, popup_x + popup_w - 3, cy + 22, C_YELLOW);
			UG_DrawFrame(popup_x + 4, cy - 1, popup_x + popup_w - 4, cy + 21, C_YELLOW);
			prev_cursor = cursor;
			timeout_ticks = 500; /* reset timeout on user interaction */
		}

		if (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {
			while (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {}
			HAL_Delay(50);

			tip_profiles_set_active(h, match_idx[cursor]);
			tip_profiles_save();
			TIM2->CNT = saved_cnt;
			return;
		}

		/* SW_2: cancel — keep current active profile */
		if (SW_2_pressed) {
			SW_2_pressed = 0;
			TIM2->CNT = saved_cnt;
			return;
		}

		/* Timeout: auto-select current cursor position */
		if (--timeout_ticks == 0) {
			tip_profiles_set_active(h, match_idx[cursor]);
			tip_profiles_save();
			TIM2->CNT = saved_cnt;
			return;
		}

		HAL_Delay(10);
	}
}

void profiles_reset(void)
{
	tip_profiles_reset();
}

void profiles_save(void)
{
	tip_profiles_save();
}
