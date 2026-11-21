#ifndef POWER_SOURCE_H
#define POWER_SOURCE_H

#include <stdint.h>

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

/* Which USB-PD sink controller was found on the bus at boot (if any). */
typedef enum {
    PD_CTRL_NONE = 0,
    PD_CTRL_STUSB4500
} pd_ctrl_t;

/* Boot snapshot of the USB-PD state, filled by power_source_init() and
 * shown read-only by the "PD information" settings-menu page. Static: it
 * reflects what was negotiated at boot, not live values.
 *
 * The STUSB4500 cannot expose the charger's advertised source-PDO list (its
 * autonomous policy engine consumes Source_Capabilities before firmware can
 * read it), so the page shows what we OFFERED and what was CONTRACTED. */
typedef struct {
    uint8_t   probed;             /* power_source_init() has run (0 = menu opened before it) */
    pd_ctrl_t controller;
    uint8_t   sink_connected;     /* USB-C source attached */
    uint8_t   contract;           /* an explicit PD contract exists */
    uint16_t  contract_mv;        /* nominal contract voltage (0 = none) */
    uint16_t  neg_voltage_mv;     /* MEASURED settled VBUS (not the requested voltage) */
    uint16_t  neg_current_ma;     /* negotiated operating current (RDO) */
    uint16_t  neg_watts;          /* derived heater power limit */
    uint8_t   object_pos;         /* contracted source-PDO index (0 = no contract) */

    /* Sink PDOs offered in the final contract (PDO1 = 5 V baseline). */
    uint16_t  req_pdo_mv[2];
    uint16_t  req_pdo_ma[2];
    uint8_t   req_pdo_count;

    /* Probe ladder result: rungs tried, and whether one was accepted. */
    uint8_t   probes_tried;
    uint8_t   probes_total;
    uint8_t   probe_accepted;

    /* RDO detail. */
    uint16_t  rdo_max_current_ma;
    uint8_t   capa_mismatch;      /* sink asked for more than the source granted */
    uint8_t   usb_comm;           /* USB communications capable */
    uint8_t   giveback;           /* GiveBack flag */
} pd_status_t;

/* Live (continuously-refreshing) readings for the info page. */
typedef struct {
    uint16_t vbus_mv;             /* measured VBUS from the ADC */
} pd_live_t;

/* Read-only access to the boot PD snapshot. Never NULL. */
const pd_status_t *power_source_get_pd_status(void);

/* Sample live values for the info page. Cheap; safe to call repeatedly. */
void power_source_read_live(pd_live_t *out);

/* Boot-time power-source detection. Talks to the STUSB4500 over I2C and,
 * if a USB-C source is attached, probes a ladder of sink profiles (highest
 * power first) until the charger accepts one, then writes
 * sensor_values.USB_PD_power_limit from measured VBUS x negotiated current.
 *
 * Blocking. Worst case ~6 s (baseline + 7 probes + 5 V fallback at 400 ms
 * each, plus up to 2.6 s for VBUS to settle).
 *
 * Call once on boot, after the I2C peripheral is ready. */
void power_source_init(void);

#endif /* POWER_SOURCE_H */
