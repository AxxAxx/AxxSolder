/* Post-mortem HardFault record. User-owned file (CubeMX never touches it);
 * the naked HardFault_Handler shim in stm32g4xx_it.c branches here. */
#include "fault_record.h"
#include "main.h"
#include "heater.h"     /* heater_fault_kill */
#include "watchdog.h"   /* reset_flags */

extern uint32_t _sram;     /* ORIGIN(RAM), linker script */
extern uint32_t _estack;   /* ORIGIN(RAM) + LENGTH(RAM), linker script */

#define FAULT_RECORD_MAGIC 0xFA07CAFEu
#define FAULT_RECORD_WORDS (sizeof(fault_record_t) / sizeof(uint32_t))

/* Not static: easy to find over SWD. */
__attribute__((section(".noinit"))) volatile fault_record_t fault_record;

static uint32_t record_sum(void){
	const volatile uint32_t *w = (const volatile uint32_t *)&fault_record;
	uint32_t s = 0x5A5A5A5Au;
	for (uint32_t i = 2; i < FAULT_RECORD_WORDS; i++){
		s = ((s << 1) | (s >> 31)) ^ w[i];
	}
	return s;
}

static int record_valid(void){
	return (fault_record.magic == FAULT_RECORD_MAGIC) && (fault_record.check == record_sum());
}

static void record_wipe(void){
	volatile uint32_t *w = (volatile uint32_t *)&fault_record;
	for (uint32_t i = 0; i < FAULT_RECORD_WORDS; i++){
		w[i] = 0u;
	}
	fault_record.magic = FAULT_RECORD_MAGIC;
}

void fault_record_boot(void){
	uint32_t csr = reset_flags();

	/* SRAM is undefined after power-on/brown-out (BORRSTF covers POR on the
	 * G4) and can be half-decayed after a short dip, magic intact but body
	 * rotten: wipe on BOR or a bad checksum. */
	if ((csr & RCC_CSR_BORRSTF) || !record_valid()){
		record_wipe();
	}
	fault_record.boots++;
	fault_record.boot_csr = csr;
	fault_record.check = record_sum();
}

const volatile fault_record_t *fault_record_get(void){
	if (record_valid() && (fault_record.flags & FAULT_F_RECORDED)){
		return &fault_record;
	}
	return NULL;
}

void fault_record_clear(void){
	fault_record.flags = 0u;
	fault_record.check = record_sum();
}

/* Entered from the shim with MOE already cleared and a usable MSP. */
__attribute__((used)) void fault_handler_c(uint32_t *frame, uint32_t exc_return){
	heater_fault_kill();

	if (!record_valid()){   /* fault before fault_record_boot(), or a corrupt record */
		record_wipe();
	}
	fault_record.faults++;

	/* Keep the FIRST record: later faults only bump the counter. */
	if ((fault_record.flags & FAULT_F_RECORDED) == 0u){
		uint32_t flags = FAULT_F_RECORDED;
		fault_record.reset_csr  = fault_record.boot_csr;
		fault_record.cfsr       = SCB->CFSR;
		fault_record.hfsr       = SCB->HFSR;
		fault_record.mmfar      = SCB->MMFAR;
		fault_record.bfar       = SCB->BFAR;
		fault_record.exc_return = exc_return;
		fault_record.sp         = (uint32_t)frame;

		/* Dereference the frame only if all 8 words lie inside RAM: a wild SP
		 * would otherwise fault again inside the HardFault handler (lockup). */
		uint32_t f = (uint32_t)frame;
		if (((f & 3u) == 0u) && (f >= (uint32_t)&_sram) && (f <= (uint32_t)&_estack - 32u)){
			fault_record.r0   = frame[0];
			fault_record.r1   = frame[1];
			fault_record.r2   = frame[2];
			fault_record.r3   = frame[3];
			fault_record.r12  = frame[4];
			fault_record.lr   = frame[5];
			fault_record.pc   = frame[6];
			fault_record.xpsr = frame[7];
			flags |= FAULT_F_FRAME_OK;
		}
		fault_record.flags = flags;
	}
	fault_record.check = record_sum();

	/* No IWDG feed: the watchdog reset recovers (if it was started); the
	 * heater is off either way. */
	for (;;){ }
}
