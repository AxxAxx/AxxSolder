#include "power_source.h"
#include "main.h"        /* sensor_values */
#include "stusb4500.h"
#include "sensors.h"     /* sensors_get_bus_voltage */
#include "debug.h"
#include <stdbool.h>
#include <math.h>        /* fabsf */

/* USB-PD power limit safety margin. The negotiated contract (measured bus
 * voltage x negotiated current) is multiplied by this factor to produce the
 * value we actually allow the heater to draw, giving headroom for inrush and
 * cable losses. Set to 1.0 = "trust the contract's number." */
#define USB_PD_POWER_REDUCTION_FACTOR 1.0f

/* Candidate sink profiles, requested HIGHEST POWER FIRST. We can't reliably
 * read the charger's advertised capabilities on this STUSB4500 (the PD message
 * buffer poll returns nothing here), so instead of guessing we PROBE: request
 * each profile in turn and let the charger accept or reject it. The STUSB4500
 * only forms a PDO2 contract if the source can supply that voltage AND current;
 * otherwise it falls back to PDO1 (5 V). We detect acceptance from the RDO
 * Object_Pos (the contracted SOURCE PDO index): == 1 is the 5 V fallback,
 * >= 2 means our profile matched. The first profile the charger accepts is
 * therefore the highest power it can deliver:
 *   20 V/5 A charger -> 100 W, 20 V/3 A -> 60 W, 15 V/3 A -> 45 W, 9 V/3 A -> 27 W.
 * The rungs are ordered by wattage (highest first) and include the 4.5 A and
 * 2.25 A ratings so a 90 W (20 V/4.5 A) or 45 W (20 V/2.25 A) charger is matched.
 * A 5 V-only source - or one that only offers high voltage below 3 A - accepts
 * none of these and we hold the safe 5 V baseline. A typical charger is matched
 * in the first 1-4 probes; the whole ladder is at most
 * 7 probes x PD_RENEGOTIATE_SETTLE_MS before the iron is ready. */
typedef struct { uint16_t mv; uint16_t ma; } pd_profile_t;
static const pd_profile_t pd_probe_ladder[] = {
	{20000, 5000},   /* 100 W */
	{20000, 4500},   /*  90 W */
	{20000, 3000},   /*  60 W */
	{20000, 2250},   /*  45 W  (20 V chargers without a 3 A rung) */
	{15000, 3000},   /*  45 W  (15 V-max chargers) */
	{12000, 3000},   /*  36 W */
	{ 9000, 3000},   /*  27 W */
};
#define PD_PROBE_COUNT (sizeof(pd_probe_ladder)/sizeof(pd_probe_ladder[0]))

/* Time to let a renegotiation settle before reading the RDO to learn whether
 * the requested PDO2 was accepted. */
#define PD_RENEGOTIATE_SETTLE_MS 400

/* --- Public state --- */
power_source_t power_source = POWER_DC;

/* Boot snapshot for the "PD information" page; written only by
 * power_source_init(), read via power_source_get_pd_status(). */
static pd_status_t pd_status;

const pd_status_t *power_source_get_pd_status(void){
	return &pd_status;
}

/* Live values for the info page. Cheap; safe to call repeatedly. */
void power_source_read_live(pd_live_t *out){
	if(out == NULL){ return; }
	float v = sensors_read_bus_voltage_instant();
	out->vbus_mv = (v > 0.0f) ? (uint16_t)(v * 1000.0f) : 0;
}

/* Apply PDO1=5V/3A baseline only (valid count 1) and renegotiate. This is the
 * guaranteed-safe contract: the source can always meet 5 V, so the MCU stays
 * powered no matter what. Used as the starting point and the final fallback. */
static void request_5v_baseline(void){
	stusb_update_pdo(1, 5000, 3000);
	stusb_set_valid_pdo(1);
	stusb_soft_reset();
}

/* Wait for VBUS to ramp to the newly negotiated voltage and hold steady.
 *
 * IMPORTANT: immediately after a soft-reset the OLD contract can still report
 * SNK_READY, and VBUS only ramps once the source accepts the new request. So we
 * must NOT sample right away (that reads the stale, lower voltage). Wait a fixed
 * minimum for the transition to begin, then poll the instantaneous bus voltage
 * until it has held steady. */
static void wait_for_vbus_settle(void){
	HAL_Delay(600);                 /* let the old contract tear down & VBUS start ramping */
	float v_prev = sensors_read_bus_voltage_instant();
	uint8_t stable_ticks = 0;
	uint32_t settle_timeout = HAL_GetTick() + 2000;
	while(HAL_GetTick() < settle_timeout){
		HAL_Delay(50);
		float v_now = sensors_read_bus_voltage_instant();
		stable_ticks = (fabsf(v_now - v_prev) < 0.2f) ? (stable_ticks + 1) : 0;
		v_prev = v_now;
		if(stable_ticks >= 3){ break; }   /* steady for ~150 ms => settled */
	}
}

/* check STUSB4500 */
static void power_source_probe(void){
	pd_status = (pd_status_t){0};
	pd_status.probes_total = PD_PROBE_COUNT;

	if(stusb_check_connection() != HAL_OK){
		//do error handling for STUSB
		debug_print_str(DEBUG_ERROR,"STUSB4500 unavailable");
		return;
	}
	debug_print_str(DEBUG_INFO,"STUSB4500 found");
	pd_status.controller = PD_CTRL_STUSB4500;

	stusb_init();

	/* 1. Is a USB-C cable actually attached? */
	if(!stusb_is_sink_connected()){
		debug_print_str(DEBUG_INFO,"No USB-PD sink connected");
		return;
	}
	pd_status.sink_connected = 1;

	/* 2. Start from the guaranteed-safe 5 V baseline. */
	request_5v_baseline();
	HAL_Delay(PD_RENEGOTIATE_SETTLE_MS);

	/* 3. Probe the ladder, highest power first; stop at the first profile the
	 * charger accepts. PDO1 stays 5 V/3 A throughout, so a rejected probe just
	 * falls back to 5 V (never 0 V) - no brown-out. */
	STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef rdo = {0};
	bool accepted = false;
	uint8_t accepted_rung = 0;
	for(uint8_t i = 0; i < PD_PROBE_COUNT; i++){
		pd_status.probes_tried = i + 1;
		stusb_update_pdo(2, pd_probe_ladder[i].mv, pd_probe_ladder[i].ma);
		stusb_set_valid_pdo(2);
		stusb_soft_reset();
		HAL_Delay(PD_RENEGOTIATE_SETTLE_MS);   /* let the renegotiation complete */

		stusb_read_rdo(&rdo);
		debug_print_int(DEBUG_INFO,"Probe mV", pd_probe_ladder[i].mv);
		debug_print_int(DEBUG_INFO," -> Object_Pos", rdo.b.Object_Pos);
		/* RDO Object_Pos is the SOURCE capability index that got contracted
		 * (1-based), NOT our sink slot. Source PDO 1 is always 5 V, so
		 * Object_Pos == 1 means PDO2 was rejected and the link fell back to 5 V.
		 * Any Object_Pos >= 2 means PDO2 matched a source PDO at the requested
		 * voltage => the charger supports this profile => accept it. */
		if(rdo.b.Object_Pos >= 2){
			accepted = true;
			accepted_rung = i;
			break;
		}
	}

	/* Snapshot what was offered in the final contract. PDO1 is always the
	 * 5 V/3 A baseline; PDO2 only counts if a ladder rung was accepted. */
	pd_status.probe_accepted = accepted ? 1 : 0;
	pd_status.req_pdo_mv[0] = 5000;
	pd_status.req_pdo_ma[0] = 3000;
	pd_status.req_pdo_count = 1;
	if(accepted){
		pd_status.req_pdo_mv[1] = pd_probe_ladder[accepted_rung].mv;
		pd_status.req_pdo_ma[1] = pd_probe_ladder[accepted_rung].ma;
		pd_status.req_pdo_count = 2;
	}

	/* 4. If nothing was accepted, return to a clean 5 V baseline contract. */
	if(!accepted){
		debug_print_str(DEBUG_WARNING,"No high-power PDO accepted - holding 5V");
		request_5v_baseline();
		HAL_Delay(PD_RENEGOTIATE_SETTLE_MS);
		stusb_read_rdo(&rdo);
	}

	/* RDO detail for the info page (final contract). */
	pd_status.object_pos         = rdo.b.Object_Pos;
	pd_status.rdo_max_current_ma = (uint16_t)(rdo.b.MaxCurrent * 10);
	pd_status.neg_current_ma     = (uint16_t)(rdo.b.OperatingCurrent * 10);
	pd_status.capa_mismatch      = rdo.b.CapaMismatch ? 1 : 0;
	pd_status.usb_comm           = rdo.b.UsbComCap ? 1 : 0;
	pd_status.giveback           = rdo.b.GiveBack ? 1 : 0;

	/* 5. Confirm an explicit PD contract exists. */
	if(rdo.b.Object_Pos == 0){
		debug_print_str(DEBUG_INFO,"No USB-PD contract");
		return;   /* leave power_source = POWER_DC */
	}
	power_source = POWER_USB;

	/* 6. Let VBUS settle at the negotiated voltage, then PRIME the bus-voltage
	 * moving-average filter so the displayed input voltage shows the negotiated
	 * value immediately instead of ramping across the filter window. */
	wait_for_vbus_settle();
	sensors_prime_bus_voltage();    /* fill the MA window with the settled reading */
	sensors_get_bus_voltage();      /* refresh sensor_values.bus_voltage from the primed filter */

	/* Derive the heater power limit from the ACTUAL contract: measured bus
	 * voltage (PA2) times the negotiated operating current (RDO, 10 mA units). */
	float negotiated_current_a = rdo.b.OperatingCurrent * 0.01f;   /* 10 mA units -> A */
	float watts = sensor_values.bus_voltage * negotiated_current_a * USB_PD_POWER_REDUCTION_FACTOR;
	sensor_values.USB_PD_power_limit = watts;
	debug_print_int(DEBUG_INFO,"USB-PD power limit (W)", (int)watts);

	pd_status.contract       = 1;
	pd_status.contract_mv    = accepted ? pd_probe_ladder[accepted_rung].mv : 5000;
	pd_status.neg_voltage_mv = (sensor_values.bus_voltage > 0.0f)
	                         ? (uint16_t)(sensor_values.bus_voltage * 1000.0f) : 0;
	pd_status.neg_watts      = (watts > 0.0f) ? (uint16_t)watts : 0;
}

void power_source_init(void){
	power_source_probe();
	pd_status.probed = 1;   /* on every exit path, incl. no STUSB4500 */
}
