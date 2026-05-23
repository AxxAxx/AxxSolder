#include "buttons.h"
#include "main.h"
#include "state_machine.h"
#include "encoder.h"
#include "buzzer.h"
#include "menu_settings.h"
#include "display_app.h"
#include "graph.h"
#include "settings.h"

extern TIM_HandleTypeDef htim16;

uint32_t previous_millis_heating_halted_update = 0;

/* Define time for long button press */
#define BTN_LONG_PRESS 15 //*50ms (htim16 interval) --> 15 = 750ms

/* Button flags */
volatile uint8_t SW_ready = 1;
volatile uint8_t SW_1_pressed = 0;
volatile uint8_t SW_2_pressed = 0;
volatile uint8_t SW_3_pressed = 0;
volatile uint8_t SW_1_pressed_long = 0;
volatile uint8_t SW_2_pressed_long = 0;
volatile uint8_t SW_3_pressed_long = 0;

static volatile uint16_t btnPressed = 0;
static volatile uint16_t debounceDone = 0;

/* ------------------------------------------------------------------ */
/* Press accessors                                                    */
/* ------------------------------------------------------------------ */

bool buttons_take_press(button_id_t id) {
    volatile uint8_t *flag = NULL;
    switch (id) {
        case BTN_1: flag = &SW_1_pressed; break;
        case BTN_2: flag = &SW_2_pressed; break;
        case BTN_3: flag = &SW_3_pressed; break;
        default: return false;
    }
    if (*flag) {
        *flag = 0;
        return true;
    }
    return false;
}

bool buttons_take_long_press(button_id_t id) {
    volatile uint8_t *flag = NULL;
    switch (id) {
        case BTN_1: flag = &SW_1_pressed_long; break;
        case BTN_2: flag = &SW_2_pressed_long; break;
        case BTN_3: flag = &SW_3_pressed_long; break;
        default: return false;
    }
    if (*flag) {
        *flag = 0;
        return true;
    }
    return false;
}

void buttons_clear_all(void) {
    SW_1_pressed      = 0;
    SW_2_pressed      = 0;
    SW_3_pressed      = 0;
    SW_1_pressed_long = 0;
    SW_2_pressed_long = 0;
    SW_3_pressed_long = 0;
}

/* ------------------------------------------------------------------ */
/* Main-loop handler                                                  */
/* ------------------------------------------------------------------ */

/* Function to toggle between RUN and HALTED at each press of the encoder button */
void buttons_handle(void){
	if(buttons_take_press(BTN_1)){
		// toggle between RUN and HALTED
		if ((sensor_values.current_state == RUN) || (sensor_values.current_state == STANDBY)){
			change_state(HALTED);
		}
		else if ((sensor_values.current_state == HALTED) || (sensor_values.current_state == EMERGENCY_SLEEP)){
			change_state(RUN);
		}
		previous_millis_heating_halted_update = HAL_GetTick();
	}

	if(buttons_take_long_press(BTN_1)){
		/* start settings menu */
		settings_menu();
	}

	/* Set "set temp" to preset temp 1 */
	if(buttons_take_press(BTN_2)){
		if(settings_menu_active == 1){
			encoder_add(+2);
		}
		else if(flash_values.three_button_mode == 1){
			encoder_add(+5);
		}
		else{
			encoder_set_count((uint16_t)flash_values.preset_temp_1);
		}
	}
	if(buttons_take_long_press(BTN_2)){
		if(settings_menu_active == 0){
			if(flash_values.three_button_mode == 0){
				flash_values.preset_temp_1 = encoder_get_count();
				settings_save();
				/* Draw the main screen decoration */
				if (flash_values.display_graph == 0){
					LCD_draw_main_screen();
				}
				else{
					draw_graph_init();
				}
				sleep_state_written_to_LCD = 0;
			}
		}
	}

	/* Set "set temp" to preset temp 2 */
	if(buttons_take_press(BTN_3)){
		if(settings_menu_active == 1){
			encoder_add(-2);
		}
		else if(flash_values.three_button_mode == 1){
				encoder_add(-5);
			}
		else{
			encoder_set_count((uint16_t)flash_values.preset_temp_2);
		}
	}
	if(buttons_take_long_press(BTN_3)){
		if(settings_menu_active == 0){
			if(flash_values.three_button_mode == 0){
				flash_values.preset_temp_2 = encoder_get_count();
				settings_save();
				/* Draw the main screen decoration */
				if (flash_values.display_graph == 0){
					LCD_draw_main_screen();
				}
				else{
					draw_graph_init();
				}
				sleep_state_written_to_LCD = 0;
			}
		}
	}
}

/* ------------------------------------------------------------------ */
/* ISR entry points                                                   */
/* ------------------------------------------------------------------ */

/* Interrupts at button press */
void buttons_isr_exti(uint16_t gpio_pin)
{
    if(((gpio_pin == SW_1_Pin) || (gpio_pin == SW_2_Pin) || (gpio_pin == SW_3_Pin)) && (SW_ready == 1)){ //A button is pressed
    	btnPressed = gpio_pin;
		HAL_TIM_Base_Start_IT(&htim16);
		SW_ready = 0;
		debounceDone = 0;
    }
}

/* Button De-bounce timer (50 ms) */
void buttons_isr_debounce_tick(void) {
    static uint8_t timerCycles = 0;

    if (SW_ready == 0 && debounceDone == 0) {
        timerCycles = 0;
        if ((btnPressed == SW_1_Pin && HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET) ||
            (btnPressed == SW_2_Pin && HAL_GPIO_ReadPin(SW_2_GPIO_Port, SW_2_Pin) == GPIO_PIN_SET) ||
            (btnPressed == SW_3_Pin && HAL_GPIO_ReadPin(SW_3_GPIO_Port, SW_3_Pin) == GPIO_PIN_SET)) {
            debounceDone = 1;
        }
        else {
            HAL_TIM_Base_Stop_IT(&htim16);
            SW_ready = 1;
        }
    }

    if (debounceDone == 1 && SW_ready == 0) {
        HAL_TIM_Base_Stop_IT(&htim16);
        if (btnPressed == SW_1_Pin && HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_RESET) {
            SW_ready = 1;
            SW_1_pressed = 1;
            beep(flash_values.buzzer_enabled);
            timerCycles = 0;
        }
        else if (timerCycles > BTN_LONG_PRESS && btnPressed == SW_1_Pin && HAL_GPIO_ReadPin(SW_1_GPIO_Port, SW_1_Pin) == GPIO_PIN_SET) {
            SW_ready = 1;
            beep(flash_values.buzzer_enabled);
            SW_1_pressed_long = 1;
            timerCycles = 0;
        }
        else if (btnPressed == SW_2_Pin && HAL_GPIO_ReadPin(SW_2_GPIO_Port, SW_2_Pin) == GPIO_PIN_RESET) {
            SW_ready = 1;
            SW_2_pressed = 1;
            beep(flash_values.buzzer_enabled);
            timerCycles = 0;
        }
        else if (timerCycles > BTN_LONG_PRESS && btnPressed == SW_2_Pin && HAL_GPIO_ReadPin(SW_2_GPIO_Port, SW_2_Pin) == GPIO_PIN_SET) {
            SW_ready = 1;
            beep(flash_values.buzzer_enabled);
            SW_2_pressed_long = 1;
            timerCycles = 0;
        }
        else if (btnPressed == SW_3_Pin && HAL_GPIO_ReadPin(SW_3_GPIO_Port, SW_3_Pin) == GPIO_PIN_RESET) {
            SW_ready = 1;
            SW_3_pressed = 1;
            beep(flash_values.buzzer_enabled);
            timerCycles = 0;
        }
        else if (timerCycles > BTN_LONG_PRESS && btnPressed == SW_3_Pin && HAL_GPIO_ReadPin(SW_3_GPIO_Port, SW_3_Pin) == GPIO_PIN_SET) {
            SW_ready = 1;
            beep(flash_values.buzzer_enabled);
            SW_3_pressed_long = 1;
            timerCycles = 0;
        }
        else {
            HAL_TIM_Base_Start_IT(&htim16);
            timerCycles++;
        }
    }
}
