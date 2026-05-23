#ifndef VERSION_H
#define VERSION_H

#include <stdint.h>

/* Build identity.
 *
 * Firmware version is a compile-time constant. Hardware revision is
 * read from 3 GPIO straps on the PCB; the result is 3..10 because
 * version numbering starts from board rev 3. */

extern uint8_t fw_version_major;
extern uint8_t fw_version_minor;
extern uint8_t fw_version_patch;

/* Returns the hardware revision (3..10) by reading VERSION_BIT_1..3 pins. */
uint8_t get_hw_version(void);

#endif /* VERSION_H */
