#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

/* The three physical buttons on the front panel. */
typedef enum {
    BTN_1 = 0,   /* encoder press (RUN/HALT, long = settings menu) */
    BTN_2 = 1,   /* preset 1 / increment in settings */
    BTN_3 = 2    /* preset 2 / decrement in settings */
} button_id_t;

/* Main-loop handler: dispatch any pending button events to their actions.
 * Call once per fast sensor tick. */
void buttons_handle(void);

/* Press accessors used by external code (menu_profiles, settings_menu)
 * to consume button events without reaching into module-private flags. */
bool buttons_take_press(button_id_t id);        /* returns true and clears the flag if a short press is pending */
bool buttons_take_long_press(button_id_t id);   /* same for long press */
void buttons_clear_all(void);                    /* clear every pending event (used when switching UI contexts) */

/* ISR entry points called from main.c's HAL callbacks. */
void buttons_isr_exti(uint16_t gpio_pin);   /* from HAL_GPIO_EXTI_Callback */
void buttons_isr_debounce_tick(void);        /* from HAL_TIM_PeriodElapsedCallback when TIM16 fires */

#endif /* BUTTONS_H */
