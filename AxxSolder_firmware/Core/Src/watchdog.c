#include "watchdog.h"
#include "main.h"

/* LSI ~32 kHz / 256 = 125 Hz; reload 500 -> ~4 s (LSI tolerance gives ~3-5 s).
 * KR keys: 0xCCCC start, 0x5555 unlock PR/RLR, 0xAAAA reload. */
#define IWDG_KEY_START       0x0000CCCCu
#define IWDG_KEY_UNLOCK      0x00005555u
#define IWDG_KEY_RELOAD      0x0000AAAAu
#define IWDG_PRESCALER_256   0x06u
#define IWDG_RELOAD_4S       500u
#define IWDG_SR_TIMEOUT_MS   50u

static volatile uint8_t  iwdg_started = 0;
static volatile uint32_t tc_samples = 0;       /* written by the TC ISR only */
static uint32_t          tc_samples_at_reload = 0;
static volatile uint32_t main_heartbeat = 0;   /* written by controller_tick only */
static uint32_t          boot_reset_csr = 0;

void reset_flags_capture(void){
	boot_reset_csr = RCC->CSR;
	RCC->CSR |= RCC_CSR_RMVF;
}

uint32_t reset_flags(void){
	return boot_reset_csr;
}

bool reset_was_watchdog(void){
	return (boot_reset_csr & RCC_CSR_IWDGRSTF) != 0u;
}

void iwdg_start(void){
	__HAL_DBGMCU_FREEZE_IWDG();          /* a core halted by the debugger doesn't reset */
	IWDG->KR  = IWDG_KEY_START;          /* also starts the LSI */
	IWDG->KR  = IWDG_KEY_UNLOCK;
	IWDG->PR  = IWDG_PRESCALER_256;
	IWDG->RLR = IWDG_RELOAD_4S;
	/* PR/RLR take a few LSI cycles to land; bounded so a dead LSI can't hang
	 * the boot (a dead LSI can't reset us either). */
	uint32_t t0 = HAL_GetTick();
	while ((IWDG->SR != 0u) && ((HAL_GetTick() - t0) < IWDG_SR_TIMEOUT_MS)){ }
	IWDG->KR = IWDG_KEY_RELOAD;
	tc_samples_at_reload = tc_samples;
	iwdg_started = 1;
}

/* Reload only when a new TC sample completed since the last reload: called
 * many times per 25 ms sample, so a live chain reloads every sample while a
 * stalled one stops reloading and the IWDG fires. Main context only. */
void iwdg_refresh(void){
	if (!iwdg_started) return;
	uint32_t n = tc_samples;
	if (n != tc_samples_at_reload){
		tc_samples_at_reload = n;
		IWDG->KR = IWDG_KEY_RELOAD;
	}
}

/* Error_Handler keeps its message on screen until a power cycle: the heater
 * is already killed there, so it feeds unconditionally (IRQs are off). */
void iwdg_refresh_latched(void){
	IWDG->KR = IWDG_KEY_RELOAD;
}

void watchdog_main_alive(void){
	main_heartbeat++;
}

/* One call per completed TC sample. Returns false once the main loop has not
 * ticked for WATCHDOG_MAIN_STALE_SAMPLES samples in a row. */
bool watchdog_isr_sample(void){
	static uint32_t heartbeat_seen = 0;
	static uint8_t  stale = 0;

	tc_samples++;

	uint32_t hb = main_heartbeat;
	if (hb != heartbeat_seen){
		heartbeat_seen = hb;
		stale = 0;
	}
	else if (stale < WATCHDOG_MAIN_STALE_SAMPLES){
		stale++;
	}
	return stale < WATCHDOG_MAIN_STALE_SAMPLES;
}
