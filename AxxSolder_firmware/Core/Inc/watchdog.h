#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>
#include <stdbool.h>

/* Independent watchdog (IWDG) and main-loop liveness.
 *
 * The IWDG (~4 s) is started once, right before the main loop: the boot
 * (USB-PD negotiation) blocks for many seconds. From then on iwdg_refresh()
 * must be called from the main loop and from inside every blocking loop
 * (menus, popups, button-release waits, flash writes), or it will trip.
 * A refresh only reloads the IWDG when the thermocouple ISR has completed a
 * new sample since the previous reload, so a stalled TIM6 -> ADC1 sampling
 * chain (heater PWM frozen at its last duty) resets the MCU too. Never call
 * it from an ISR or a fault handler. On the reset the iron boots into sleep.
 *
 * The other direction: controller_tick() calls watchdog_main_alive(), and
 * the TC-complete ISR calls watchdog_isr_sample(). When that returns false
 * the main loop has been silent for WATCHDOG_MAIN_STALE_SAMPLES samples and
 * the ISR must stop driving the heater from a stale requested_power. */
#define WATCHDOG_MAIN_STALE_SAMPLES 16u   /* x 25 ms = 400 ms */

void iwdg_start(void);
void iwdg_refresh(void);
void iwdg_refresh_latched(void);   /* unconditional; Error_Handler only */

void watchdog_main_alive(void);    /* main context (controller_tick) */
bool watchdog_isr_sample(void);    /* TC-complete ISR only */

/* Reset cause. RCC->CSR is read once, first thing at boot, and its flags
 * cleared so every boot sees only its own cause; everyone else uses the
 * copy. */
void reset_flags_capture(void);
uint32_t reset_flags(void);
bool reset_was_watchdog(void);

#endif /* WATCHDOG_H */
