#ifndef POWER_SOURCE_H
#define POWER_SOURCE_H

/* The power source currently feeding the iron.
 *
 * Set during boot by power_source_init() after the STUSB4500 chip
 * has been queried over I2C:
 *   - POWER_DC  : 9-24 V from the barrel jack (default)
 *   - POWER_USB : USB-PD source negotiated successfully
 *   - POWER_BAT : reserved for a future portable variant
 *
 * Read by the display code to render the "DC" / "USB" / "BAT" label
 * on the main screen. */
typedef enum {
    POWER_DC,
    POWER_USB,
    POWER_BAT
} power_source_t;

extern power_source_t power_source;

/* Boot-time power-source detection. Talks to the STUSB4500 over I2C,
 * waits for the sink to become ready, asks for the available PDOs,
 * picks the highest-power one, and writes sensor_values.USB_PD_power_limit.
 *
 * Blocking. May take up to ~3.5 seconds in the worst case (3 s sink
 * timeout + 500 ms wait + ~100 ms for PDO read).
 *
 * Call once on boot, after the I2C peripheral is ready. */
void power_source_init(void);

#endif /* POWER_SOURCE_H */
