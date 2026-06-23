# USB-PD: negotiate the maximum power the charger can provide

Date: 2026-06-23
Status: Approved
Area: `AxxSolder_firmware` — `power_source.c`, `stusb4500.c/.h`

## Problem

The STUSB4500 exposes only **3 sink PDOs**, and PDO1 is forced to 5 V by spec.
The current firmware advertises a fixed-current ladder:

- PDO3 = 20 V / 5 A
- PDO2 = 15 V / 5 A
- PDO1 = 5 V / 3 A

USB-PD only matches a sink PDO when the source can supply the **requested
current** at that voltage. A 20 V / 3 A (60 W) charger cannot satisfy 20 V/5 A
*or* 15 V/5 A, so negotiation falls all the way to PDO1 and the iron runs at
**5 V**. With only two usable high-voltage slots, a fixed-current ladder cannot
cover every charger's current rating.

A previous dynamic implementation read the source capabilities and rewrote PDO2,
but when the capability poll returned nothing it wrote a **0 V PDO** and
soft-reset again, thrashing the contract and **browning out marginal boards**
(iron powered up, then died). That regression is why the dynamic path was
removed.

## Goal

Request the **highest-power** profile the charger actually advertises, capped at
the STUSB4500's 20 V ceiling — without ever writing a 0 V / empty PDO.

Expected results: 20 V/5 A charger → 100 W, 20 V/3 A → 60 W, 15 V/3 A → 45 W,
dead-comms charger → safe 5 V, DC barrel → unchanged.

## Decisions

- **Selection metric:** highest power (V × I), capped at V ≤ 20 V.
- **Fallback when source caps cannot be read:** hold a guaranteed 5 V/3 A
  contract (USB powered, low power). Never write a 0 V PDO. The iron simply will
  not reach high power until reconnected.

## Design

### `power_source_init()` flow

1. **Connect / init / attach checks** — unchanged. Return `POWER_DC` if the
   STUSB4500 is absent or no sink is attached.
2. **Baseline contract** — write PDO1 = 5 V/3 A, valid count = 1, one
   `stusb_soft_reset()`. Guarantees a safe 5 V rail (MCU stays powered) and
   triggers the source to (re)send its capabilities.
3. **Poll source capabilities** — call `poll_source()` in a retry loop with a
   ~500 ms timeout until it captures the source PDO list (`numPDOs > 0`).
4. **Select & request best (caps read OK):** choose the highest-wattage source
   PDO with V ≤ 20 V, write it into PDO2, valid count = 2, one
   `stusb_soft_reset()`. Then run the existing VBUS-settle loop, confirm an
   explicit RDO contract (`Object_Pos != 0`), prime the bus-voltage filter, and
   derive the heater power limit from **measured VBUS × negotiated current**.
5. **Safe fallback (caps unreadable):** leave the 5 V/3 A baseline in place
   (`POWER_USB`, low power), never write a 0 V PDO, log a warning.

Worst case is two soft-resets (baseline → high power), both to valid non-zero
contracts.

### Driver change (`stusb4500.c` / `.h`)

Add one encapsulated selection function so `power_source.c` never touches the
driver's internal `pdos` global:

```c
/* Scan the last-received source capabilities; return the highest-power
 * fixed PDO at or below max_voltage_mv. false if none / none read. */
bool stusb_find_highest_power_pdo(uint16_t max_voltage_mv,
                                  uint16_t *voltage_mv, uint16_t *current_ma);
```

Reuses the existing wattage math; converts units (source PDO is 50 mV / 10 mA).

Remove the now-redundant `stusb_set_highest_pdo()` (currently unused, lacks the
20 V cap, performs its own delay/reset) to avoid two competing selection paths.

### Unchanged

VBUS settle loop, voltage-filter priming (`sensors_prime_bus_voltage` /
`sensors_get_bus_voltage`), RDO-based power-limit derivation, and
`USB_PD_POWER_REDUCTION_FACTOR`.

## Testing / verification

No host test harness (bare-metal STM32G4). Verification is on-hardware:

- 100 W (20 V/5 A) charger → negotiates 20 V, ~100 W limit shown.
- 60 W (20 V/3 A) charger → negotiates 20 V, ~60 W limit shown.
- ~45 W (15 V/3 A) charger → negotiates 15 V, ~45 W limit shown.
- Marginal / dead-comms charger → stays at 5 V, no brown-out cycling.
- DC barrel input → `POWER_DC`, unchanged behavior.
