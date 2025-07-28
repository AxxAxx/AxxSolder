/* menu_settings.c*/

#include "menu_settings.h"

/* List of names for settings menu */
#define menu_length 29
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
		"Language",
		"-Load Default-",
		"-Save and Reboot-",
		"-Exit no Save-"
};

/* Function to left align a string from float */
void left_align_float(char* str, float number, int8_t len)
	{
		char tempstring[len];
		memset(&tempstring, '\0', len);
		sprintf(tempstring, "%.0f", number);

		strcpy(str, tempstring);
	}

// ==== Перечислимые строки ====
const char* bool_str[] = { " No", " Yes" };
const char* screen_rotation_str[] = { "0°", "90°", "180°", "270°" };

const char* bool_language_str[] = { " UA", " EN" };

// ==== Таблица перечислимых параметров ====
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
    { 9, screen_rotation_str, 4 },  // Screen rotation
    {28,  bool_language_str, 2 }     // Language
};

#define ENUM_PARAM_COUNT (sizeof(enum_params) / sizeof(enum_params[0]))

// ==== Получение символьного значения параметра ====
const char* get_enum_value_str(uint8_t index, float value) {
    for (uint8_t i = 0; i < ENUM_PARAM_COUNT; i++) {
        if (enum_params[i].index == index) {
            int v = (int)roundf(value);  // безопасное округление
            if (v >= 0 && v < enum_params[i].count) {
                return enum_params[i].values[v]; // корректный индекс
            } else {
                return "?";  // защита от выхода за пределы
            }
        }
    }
    return NULL;  // если не найден enum-параметр
}

// ==== Отображение строки параметра в меню ====
// Универсальный вывод значения пункта меню: символьное или числовое с выравниванием

#define MENU_VALUE_W   47      // ширина зоны значения
#define MENU_VALUE_H   18      // высота зоны значения

#define MAX_MENU_LINES 10

static char     prev_buf_screen[MAX_MENU_LINES][12] = {0};
static uint8_t  prev_flags_screen[MAX_MENU_LINES]   = {0};

uint8_t menu_lines_on_screen = 7; // динамически меняется в зависимости от поворота экрана


/* Функция вывода значения параметра с кэшированием, чтобы избежать лишнего перерисовывания */
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

    //Заливка по смене режима редактирования
    uint8_t prev_editing = (prev_flags >> 1) & 1;

    if (editing && !prev_editing) {
        // Вход в режим редактирования: заливаем белым
        UG_FillFrame(x, y, x + MENU_VALUE_W, y + MENU_VALUE_H, C_WHITE);
    }
    else if (!editing && prev_editing && selected) {
        // Выход из редактирования, если остаётся выбранным: заливаем чёрным
        UG_FillFrame(x, y, x + MENU_VALUE_W, y + MENU_VALUE_H, C_BLACK);
    }

    // === Отрисовка текста, если значение или флаги изменились ===
    if (strcmp(buf, prev_buf_screen[line_pos]) != 0 || prev_flags != flags) {
        strncpy(prev_buf_screen[line_pos], buf, sizeof(buf));
        prev_flags_screen[line_pos] = flags;

        uint16_t fg = (selected && editing) ? C_BLACK : fg_def;
        uint16_t bg = (selected && editing) ? C_WHITE : bg_def;

        UG_FillFrame(x, y, 190 + MENU_VALUE_W, y + MENU_VALUE_H, bg);
        LCD_PutStr(x, y, buf, FONT_arial_20X23, fg, bg);

    }
}
//Циклическое округление:
static inline float normalize_enum(float val, uint8_t count) {
    int v = (int)roundf(val);
    while (v < 0) v += count;
    v %= count;
    return (float)v;
}

//ограничения параметров меню
void normalize_param(uint16_t index) {
    float* p = &((float*)&flash_values)[index];

    switch(index) {

        // --- Булевые или бинарные параметры: ограничиваются значением 0 или 1
        case 5: case 8: case 11: case 12:
        case 13: case 20: case 22: case 23: case 24: case 28:
            *p = normalize_enum(*p, 2);  // 0/1, циклично
            break;

        // --- Параметры с диапазоном от 0 до 3 (4 значения)
        case 9:
        	 *p = normalize_enum(*p, 4);  // 0/1/2/3, циклично

            break;

        // --- Параметр с диапазоном от 1 до 10 (10 значений)
        case 21:
            *p = 1 + fmod(round(fmod(fabs(*p), 10)), 10);  // 1..10
            break;

        // --- Целочисленный параметр (например, тип датчика, флаг)
        case 1:
            *p = round(*p);  // Округление до ближайшего целого
            break;

        // --- Температурные параметры: ограничены до допустимого диапазона
        case 0: case 2: case 6: case 7:
            *p = fmod(round(fmod(fabs(*p), MAX_SELECTABLE_TEMPERATURE + 1)), MAX_SELECTABLE_TEMPERATURE + 1);
            break;

        // --- Параметр мощности: 0..(MAX_POWER + 4)
        case 10:
            *p = fmod(round(fmod(fabs(*p), MAX_POWER + 5)), MAX_POWER + 5);
            break;

        // --- По умолчанию: просто берём по модулю (только положительное значение)
        default:
            *p = fabs(*p);
            break;
    }
}

void settings_menu()
{
	//HAL_TIMEx_PWMN_Stop_IT(&htim1, TIM_CHANNEL_3);

	// Перевод системы в безопасное состояние (отключение нагрева)
    change_state(EMERGENCY_SLEEP);
    sensor_values.requested_power = 0; // отключить нагрев перед входом в меню

    settings_menu_active = 1; // флаг активности меню
    UG_FillScreen(C_BLACK);               // Очистка экрана
    UG_FontSetTransparency(1);              // Установка прозрачности фона текста


    // === Установка количества строк на экране в зависимости от ориентации
    // При повороте экрана 0° или 180° — больше вертикального пространства => 10 строк
    // При повороте экрана 90° или 270° — экран "высокий", помещается только 7 строк
    // Отображение версии прошивки и железа ===
    char str[64] = {0};
    if ((flash_values.screen_rotation == 0) || (flash_values.screen_rotation == 2)) {
        menu_lines_on_screen = 10;
    	sprintf(str, "fw mod: %d.%d.%d   hw: %d", fw_version_major, fw_version_minor, fw_version_patch, get_hw_version());
		LCD_PutStr(6, 300, str, FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_BLACK));

    } else {
        menu_lines_on_screen = 7;
    	sprintf(str, "fw mod: %d.%d.%d   hw: %d", fw_version_major, fw_version_minor, fw_version_patch, get_hw_version());
		LCD_PutStr(6, 215, str, FONT_arial_20X23, RGB_to_BRG(C_ORANGE), RGB_to_BRG(C_BLACK));

    }

    // === Заголовок меню ===
    LCD_PutStr(50, 5, "S E T T I N G S", FONT_arial_20X23, C_FOREST_GREEN, C_BLACK);
    LCD_DrawLine(0, 25, 240, 25, C_FOREST_GREEN);
    LCD_DrawLine(0, 26, 240, 26, C_FOREST_GREEN);
    //LCD_DrawLine(0, 27, 240, 27, B_FOREST_GREEN);

    // Начальное значение энкодера
    TIM2->CNT = 1000;

    // === Переменные управления ===
    uint16_t menu_cursor_position = 0;      // текущая позиция курсора
    uint8_t prev_menu_start = 0xFF;         // для отслеживания смены "страницы"
    uint8_t menu_level = 0;                 // 0 — навигация, 1 — редактирование значения
    uint8_t prev_cursor_position = 0xFF;    // предыдущая позиция курсора
    uint8_t prev_menu_level = 0xFF;         // предыдущий уровень редактирования
    uint8_t menu_active = 1;                // флаг выхода из меню

    float old_value = 0;                    // значение до редактирования
    int16_t prev_displayed_value = INT16_MIN; // предыдущее отображенное значение
    bool force_redraw_value = true;         // флаг принудительной перерисовки

    // === Ожидание отпускания кнопки перед входом в меню ===
    while (HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) {}

    // === Основной цикл работы меню ===
    while (menu_active)
    {
        handle_button_status(); // обновление состояния кнопок

        // === Навигация или редактирование параметра ===
        if (menu_level == 0) {
            // Курсор — по энкодеру
            TIM2->CNT = clamp(TIM2->CNT, 1000, 1000000);
            menu_cursor_position = (TIM2->CNT - 1000) / 2;
        } else if (menu_level == 1) {
            // === Режим редактирования параметра ===
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
                    true,   // selected
                    true,   // editing
                    190, line_y,
                    C_WHITE, C_BLACK
                );

                prev_displayed_value = norm_val;
                force_redraw_value = false;
            }
        }

        // === Ограничение по длине меню ===
        if (menu_cursor_position > menu_length - 1) {
            menu_cursor_position = menu_length - 1;
            TIM2->CNT = 1000 + (menu_length - 1) * 2;
        }

        // === Начало текущей "страницы" меню ===
        uint16_t new_menu_start = (menu_cursor_position / menu_lines_on_screen) * menu_lines_on_screen;

        // === Обработка нажатий кнопки ===
        if ((HAL_GPIO_ReadPin(GPIOB, SW_1_Pin) == 1) && (menu_cursor_position < menu_length - 3)) {
            if (menu_level == 0) {
                old_value = ((float*)&flash_values)[menu_cursor_position];
            }
            if (menu_level == 1) {
                TIM2->CNT = menu_cursor_position * 2 + 1000;
            }

            menu_level = !menu_level; // переключение режимов
            force_redraw_value = true;
            HAL_Delay(200); // антидребезг
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
            // === Загрузка значений по умолчанию ===
            //flash_values = default_flash_values;
        }

        // === Перерисовка новой страницы меню при смене "экрана" ===
        if (new_menu_start != prev_menu_start) {
            UG_FillFrame(0, 30, 239, 30 + menu_lines_on_screen * 25, C_BLACK); // очистка области
            prev_menu_start = new_menu_start;

            // очистка кэша строк
            for (int i = 0; i < menu_lines_on_screen; i++) {
                prev_buf_screen[i][0] = '\0';
                prev_flags_screen[i] = 0xFF;
            }

            // отрисовка строк параметров и значений
            for (int line = 0; line < menu_lines_on_screen && (new_menu_start + line) < menu_length; line++) {
                uint16_t index = new_menu_start + line;
                uint16_t line_y = 35 + line * 25;

                UG_FillFrame(5, line_y, 187, line_y + MENU_VALUE_H, C_BLACK);
                LCD_PutStr(5, line_y, menu_names[index], FONT_arial_20X23, C_WHITE, C_BLACK);

                if (index < menu_length - 3) {
                    display_menu_value_line(
                        line,
                        index,
                        ((float*)&flash_values)[index],
                        false, false,
                        190, line_y,
						C_WHITE, C_BLACK
                    );
                }
            }
        }

        // === Обработка перемещения курсора или смены режима ===
        if (menu_cursor_position != prev_cursor_position || menu_level != prev_menu_level) {
            // Стираем предыдущую рамку
            if (prev_cursor_position != 0xFF &&
                prev_cursor_position / menu_lines_on_screen == menu_cursor_position / menu_lines_on_screen) {
                uint8_t prev_line = prev_cursor_position % menu_lines_on_screen;
                uint16_t prev_line_y = 35 + prev_line * 25;

                UG_DrawFrame(0, prev_line_y - 4, 239, prev_line_y + 2 + MENU_VALUE_H, C_BLACK);
                UG_DrawFrame(1, prev_line_y - 3, 238, prev_line_y + 1 + MENU_VALUE_H, C_BLACK);

                // обновляем значение
                if (prev_cursor_position < menu_length - 3) {
                    display_menu_value_line(
                        prev_line,
                        prev_cursor_position,
                        ((float*)&flash_values)[prev_cursor_position],
                        false, false,
                        190, prev_line_y,
						C_WHITE, C_BLACK
                    );
                }
            }

            // Отрисовка новой рамки/подсветки
            if (menu_cursor_position / menu_lines_on_screen == prev_menu_start / menu_lines_on_screen) {
                uint8_t line = menu_cursor_position % menu_lines_on_screen;
                uint16_t line_y = 35 + line * 25;

                if (menu_level == 0) {
                    // курсор — рамка
                    UG_DrawFrame(0, line_y - 4, 239, line_y + 2 + MENU_VALUE_H, C_YELLOW);
                    UG_DrawFrame(1, line_y - 3, 238, line_y + 1 + MENU_VALUE_H, C_YELLOW);

                } else {
                    // редактирование — подсветка значения
                    display_menu_value_line(
                        line,
                        menu_cursor_position,
                        ((float*)&flash_values)[menu_cursor_position],
                        true, true,
                        190, line_y,
						C_WHITE, C_BLACK
                    );
                }
            }

            // Обновление прошлых значений
            prev_cursor_position = menu_cursor_position;
            prev_menu_level = menu_level;
        }
    }
}


