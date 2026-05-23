#ifndef TELEMETRY_H
#define TELEMETRY_H

/* Telemetry: structured binary packet stream emitted over UART1 for
 * external graphing (e.g. AxxTerm). Gated by flash_values.serial_debug_print
 * and rate-limited internally. */

/* Call every main-loop iteration. The module decides internally whether
 * to actually emit a packet based on the rate-limit and the user setting. */
void telemetry_tick(void);

#endif /* TELEMETRY_H */
