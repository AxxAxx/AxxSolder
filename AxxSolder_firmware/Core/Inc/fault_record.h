#ifndef FAULT_RECORD_H
#define FAULT_RECORD_H

#include <stdint.h>

/* Post-mortem HardFault record in .noinit RAM. It survives the IWDG reset
 * that follows a fault (fault handlers don't feed the watchdog) and is wiped
 * on power-on/brown-out or a bad checksum. Shown on screen once at boot and
 * then cleared; the counters keep running until the next power cycle. */
#define FAULT_F_RECORDED 0x1u   /* a record is present */
#define FAULT_F_FRAME_OK 0x2u   /* stacked r0..xpsr were inside RAM and read */

typedef struct {
	uint32_t magic;
	uint32_t check;       /* checksum over every word after this one */
	uint32_t boots;       /* boots since power-on */
	uint32_t faults;      /* HardFaults since power-on */
	uint32_t boot_csr;    /* RCC->CSR of the most recent boot */
	uint32_t flags;       /* FAULT_F_* */
	/* first fault since power-on / since the last readout */
	uint32_t reset_csr;   /* RCC->CSR of the boot the fault happened in */
	uint32_t cfsr, hfsr, mmfar, bfar, exc_return, sp;
	uint32_t r0, r1, r2, r3, r12, lr, pc, xpsr;
} fault_record_t;

/* Once at boot, after reset_flags_capture(). */
void fault_record_boot(void);

/* The pending record, or NULL if there is none. */
const volatile fault_record_t *fault_record_get(void);

/* Drop the pending record (after it has been shown). */
void fault_record_clear(void);

/* C part of the HardFault handler, entered from the naked shim in
 * stm32g4xx_it.c with the exception frame and EXC_RETURN. Never returns. */
void fault_handler_c(uint32_t *frame, uint32_t exc_return) __attribute__((noreturn));

#endif /* FAULT_RECORD_H */
