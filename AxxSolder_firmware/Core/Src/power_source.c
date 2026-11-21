/*
 * power_source.c - boot-time USB-PD negotiation (STUSB4500 on I2C1).
 *
 * The STUSB4500 is an autonomous sink that hides the charger's source PDO
 * list. Max power relies on the NVM REQ_SRC_CURRENT bit, provisioned once and
 * checked every boot before the attach gate (so a DC-powered board is
 * provisioned or healed too). With the bit live the chip books the source's
 * full current at the highest matching voltage (20 V/5 A on a 100 W charger,
 * 20 V/3.25 A on a 65 W one, ...), read back from the RDO.
 *
 * The chip loads its NVM only at power-up. While the bit is not live (just
 * written, or written without a power cycle since), cannot be written
 * (STUSB_NVM_PROVISION 0) or the NVM is in error, the proven probe ladder
 * below is used instead.
 */
#include "power_source.h"
#include "main.h"        /* sensor_values */
#include "stusb4500.h"
#include "sensors.h"     /* sensors_get_bus_voltage */
#include "debug.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>        /* fabsf */

/* Heater limit: contract x margin, current clamped to the 5 A a PD fixed PDO
 * can carry, capped at the chip's 100 W. Without a confirmed contract the
 * limit fails closed at measured VBUS x STUSB_FAILSAFE_CURRENT_A. */
#define STUSB_LIMIT_MARGIN        0.95f
#define STUSB_MAX_CURRENT_MA      5000u
#define STUSB_MAX_WATTS           100.0f
#define STUSB_FAILSAFE_CURRENT_A  0.5f

/* Max wait for the chip's own power-up contract (PE_SNK_READY) before the
 * first soft reset. */
#define STUSB_READY_TIMEOUT_MS    500u

/* Settled at 5 V: the RDO names source PDO 1 (always 5 V) or the measured
 * VBUS is below this. */
#define PD_5V_FALLBACK_THRESHOLD_V 7.0f

/* Auto-current sink-PDO sets (REQ_SRC_CURRENT live). Slot order [0]=PDO1
 * (5 V forced), [1]=PDO2, [2]=PDO3 (highest priority). Currents are match
 * floors - the granted current is the source's full max regardless, so a low
 * floor maximises compatibility, and a grant AT the floor means the bit is
 * not live. AUTO_LOW is the step-down for sub-15 V bricks, taken only when
 * AUTO_HIGH settled at 5 V. */
#define STUSB_AUTO_FLOOR_MA 1000u
typedef struct { uint16_t mv[3]; uint16_t ma[3]; } sink_pdo_set_t;
static const sink_pdo_set_t SINK_SET_AUTO_HIGH = {{5000,15000,20000},{STUSB_AUTO_FLOOR_MA,STUSB_AUTO_FLOOR_MA,STUSB_AUTO_FLOOR_MA}};
static const sink_pdo_set_t SINK_SET_AUTO_LOW  = {{5000, 9000,12000},{STUSB_AUTO_FLOOR_MA,STUSB_AUTO_FLOOR_MA,STUSB_AUTO_FLOOR_MA}};
static const sink_pdo_set_t *const SINK_SETS_AUTO[] = {&SINK_SET_AUTO_HIGH, &SINK_SET_AUTO_LOW};
#define SET_COUNT(a) ((uint8_t)(sizeof(a) / sizeof((a)[0])))

/* Fallback probe ladder, requested HIGHEST POWER FIRST. Without the NVM bit
 * the chip requests exactly the sink-PDO current, so instead of guessing we
 * PROBE: request each profile in turn and let the charger accept or reject
 * it. The STUSB4500 only forms a PDO2 contract if the source can supply that
 * voltage AND current; otherwise it falls back to PDO1 (5 V). We detect
 * acceptance from the RDO Object_Pos (the contracted SOURCE PDO index): == 1
 * is the 5 V fallback, >= 2 means our profile matched. The first profile the
 * charger accepts is therefore the highest power it can deliver:
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

/* What a negotiation produces; consumed by the limit calc and pd_status. */
typedef struct {
	bool      contract;
	float     vbus_v;          /* measured, settled */
	uint16_t  current_ma;      /* granted operating current */
	uint16_t  contract_mv;     /* nominal contract voltage (0 = unknown) */
	pd_mode_t mode;
	STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef rdo;
	uint16_t  offer_mv[3];     /* sink PDOs offered in the final contract */
	uint16_t  offer_ma[3];
	uint8_t   offer_count;
} pd_result_t;

/* Wait for VBUS to ramp to the newly negotiated voltage and hold steady.
 *
 * IMPORTANT: immediately after a soft-reset the OLD contract can still report
 * SNK_READY, and VBUS only ramps once the source accepts the new request. So we
 * must NOT sample right away (that reads the stale, lower voltage). Wait a fixed
 * minimum for the transition to begin, then poll the instantaneous bus voltage
 * until it has held steady. Returns false on timeout. */
static bool wait_for_vbus_settle(void){
	HAL_Delay(600);                 /* let the old contract tear down & VBUS start ramping */
	float v_prev = sensors_read_bus_voltage_instant();
	uint8_t stable_ticks = 0;
	uint32_t start = HAL_GetTick();
	while((HAL_GetTick() - start) < 2000u){
		HAL_Delay(50);
		float v_now = sensors_read_bus_voltage_instant();
		stable_ticks = (fabsf(v_now - v_prev) < 0.2f) ? (stable_ticks + 1) : 0;
		v_prev = v_now;
		if(stable_ticks >= 3){ return true; }   /* steady for ~150 ms => settled */
	}
	return false;
}

/* Settle, then PRIME the bus-voltage moving-average filter so the displayed
 * input voltage shows the negotiated value immediately instead of ramping
 * across the filter window. Returns the settled bus voltage. */
static float settle_and_read_vbus(void){
	if(!wait_for_vbus_settle()){
		HAL_Delay(300);   /* slow charger: extra grace, don't misread as 5 V */
	}
	sensors_prime_bus_voltage();
	sensors_get_bus_voltage();
	return sensor_values.bus_voltage;
}

/* Let the chip finish its own power-up contract before the first soft reset,
 * so the Soft_Reset doesn't land mid-negotiation. Proceeds after the timeout
 * anyway (a source without PD never gets there). */
static bool stusb_wait_sink_ready(void){
	uint32_t start = HAL_GetTick();
	for(;;){
		if(stusb_is_sink_ready()){ return true; }
		if((HAL_GetTick() - start) >= STUSB_READY_TIMEOUT_MS){ return false; }
		HAL_Delay(20);
	}
}

static bool stusb_is_5v_contract(const STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef *rdo, float vbus){
	return (rdo->b.Object_Pos == 1) || (vbus < PD_5V_FALLBACK_THRESHOLD_V);
}

/* Auto-current: negotiate the sets in order until one settles above 5 V.
 * Every HAL step is checked - an I2C failure yields a zeroed RDO (no
 * contract), never a garbage power budget. */
static void negotiate_auto(pd_result_t *r){
	for(uint8_t i = 0; i < SET_COUNT(SINK_SETS_AUTO); i++){
		const sink_pdo_set_t *set = SINK_SETS_AUTO[i];
		memset(&r->rdo, 0, sizeof(r->rdo));
		r->vbus_v = 0.0f;
		r->offer_count = 3;
		memcpy(r->offer_mv, set->mv, sizeof(r->offer_mv));
		memcpy(r->offer_ma, set->ma, sizeof(r->offer_ma));
		if((stusb_set_sink_pdos(set->mv, set->ma) != HAL_OK) ||
		   (stusb_soft_reset() != HAL_OK)){
			r->contract = false;
			return;
		}
		r->vbus_v = settle_and_read_vbus();
		(void)stusb_read_rdo(&r->rdo);   /* zeroed on failure */
		r->contract = (r->rdo.b.Object_Pos != 0);
		if(!r->contract || !stusb_is_5v_contract(&r->rdo, r->vbus_v)){ return; }
	}
}

/* Apply PDO1=5V/3A baseline only (valid count 1) and renegotiate. This is the
 * guaranteed-safe contract: the source can always meet 5 V, so the MCU stays
 * powered no matter what. Used as the starting point and the final fallback. */
static void request_5v_baseline(void){
	stusb_update_pdo(1, 5000, 3000);
	stusb_set_valid_pdo(1);
	stusb_soft_reset();
}

/* Fallback: probe the ladder, highest power first. */
static void negotiate_ladder(pd_result_t *r){
	/* Start from the guaranteed-safe 5 V baseline. */
	request_5v_baseline();
	HAL_Delay(PD_RENEGOTIATE_SETTLE_MS);

	/* Stop at the first profile the charger accepts. PDO1 stays 5 V/3 A
	 * throughout, so a rejected probe just falls back to 5 V (never 0 V) - no
	 * brown-out. */
	STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef rdo = {0};
	bool accepted = false;
	uint8_t accepted_rung = 0;
	for(uint8_t i = 0; i < PD_PROBE_COUNT; i++){
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

	/* Offered in the final contract: PDO1 is always the 5 V/3 A baseline;
	 * PDO2 only counts if a ladder rung was accepted. */
	r->offer_mv[0] = 5000;
	r->offer_ma[0] = 3000;
	r->offer_count = 1;
	if(accepted){
		r->offer_mv[1] = pd_probe_ladder[accepted_rung].mv;
		r->offer_ma[1] = pd_probe_ladder[accepted_rung].ma;
		r->offer_count = 2;
	}

	/* If nothing was accepted, return to a clean 5 V baseline contract. */
	if(!accepted){
		debug_print_str(DEBUG_WARNING,"No high-power PDO accepted - holding 5V");
		request_5v_baseline();
		HAL_Delay(PD_RENEGOTIATE_SETTLE_MS);
		stusb_read_rdo(&rdo);
	}

	r->rdo = rdo;
	r->contract = (rdo.b.Object_Pos != 0);
	r->vbus_v = settle_and_read_vbus();
}

static void negotiate_stusb4500(pd_result_t *r, stusb_nvm_state_t nvm){
	if(nvm == STUSB_NVM_SET){
		negotiate_auto(r);
		if(!r->contract){ return; }
		/* A live bit grants the source's max, above the floor. A grant AT the
		 * floor means the bit was written without a chip power cycle since. */
		if((uint32_t)r->rdo.b.OperatingCurrent * 10u > STUSB_AUTO_FLOOR_MA){
			r->mode = PD_MODE_AUTO;
		}
		else{
			r->mode = PD_MODE_NVM_PENDING;
		}
	}
	else{
		r->mode = (nvm == STUSB_NVM_WRITTEN) ? PD_MODE_NVM_PENDING :
		          (nvm == STUSB_NVM_CLEAR)   ? PD_MODE_LADDER : PD_MODE_NVM_ERROR;
	}
	if(r->mode != PD_MODE_AUTO){
		negotiate_ladder(r);
	}
	if(!r->contract){
		return;
	}

	r->current_ma = (uint16_t)(r->rdo.b.OperatingCurrent * 10);

	/* The RDO carries no voltage. A 5 V settle is 5 V; otherwise the contract
	 * matched one of our offered voltages - take the one within 1 V of the
	 * measured VBUS (unknown if none is). */
	if(stusb_is_5v_contract(&r->rdo, r->vbus_v)){
		r->contract_mv = 5000;
	}
	else{
		uint16_t vbus_mv = (uint16_t)(r->vbus_v * 1000.0f);
		for(uint8_t i = 1; i < r->offer_count; i++){
			uint16_t mv = r->offer_mv[i];
			uint16_t diff = (vbus_mv > mv) ? (vbus_mv - mv) : (mv - vbus_mv);
			if(diff <= 1000){ r->contract_mv = mv; }
		}
	}
}

/* Heater limit. Confirmed contract: min(measured, nominal) voltage x granted
 * current x margin - never more than the contract even if VBUS reads high.
 * Attached without one (no PD reply, I2C failure): fail closed at measured
 * VBUS x 0.5 A instead of the unlimited DC default. */
static float stusb_power_limit(pd_result_t *r){
	if(r->contract){
		float v = r->vbus_v;
		if((r->contract_mv != 0) && (v > (r->contract_mv / 1000.0f))){
			v = r->contract_mv / 1000.0f;
		}
		uint16_t ma = (r->current_ma > STUSB_MAX_CURRENT_MA) ? STUSB_MAX_CURRENT_MA : r->current_ma;
		float watts = v * (ma / 1000.0f) * STUSB_LIMIT_MARGIN;
		return (watts > STUSB_MAX_WATTS) ? STUSB_MAX_WATTS : watts;
	}
	sensors_prime_bus_voltage();
	sensors_get_bus_voltage();
	r->vbus_v = sensor_values.bus_voltage;
	float v = (r->vbus_v > 5.0f) ? r->vbus_v : 5.0f;
	return v * STUSB_FAILSAFE_CURRENT_A;
}

/* The only writer of the negotiation part of pd_status. */
static void pd_status_publish(const pd_result_t *r, float watts){
	pd_status.mode           = r->mode;
	pd_status.contract       = r->contract ? 1 : 0;
	pd_status.contract_mv    = r->contract ? r->contract_mv : 0;
	pd_status.neg_voltage_mv = (r->vbus_v > 0.0f) ? (uint16_t)(r->vbus_v * 1000.0f) : 0;
	pd_status.neg_current_ma = r->current_ma;
	pd_status.neg_watts      = (watts > 0.0f) ? (uint16_t)watts : 0;
	pd_status.object_pos     = r->rdo.b.Object_Pos;
	pd_status.rdo_max_current_ma = (uint16_t)(r->rdo.b.MaxCurrent * 10);
	pd_status.capa_mismatch  = r->rdo.b.CapaMismatch ? 1 : 0;
	pd_status.usb_comm       = r->rdo.b.UsbComCap ? 1 : 0;
	pd_status.giveback       = r->rdo.b.GiveBack ? 1 : 0;
	pd_status.req_pdo_count  = r->offer_count;
	for(uint8_t i = 0; i < r->offer_count && i < 3; i++){
		pd_status.req_pdo_mv[i] = r->offer_mv[i];
		pd_status.req_pdo_ma[i] = r->offer_ma[i];
	}
}

/* check STUSB4500 */
static void power_source_probe(void){
	pd_status = (pd_status_t){0};
	pd_result_t r;
	memset(&r, 0, sizeof(r));

	if(stusb_check_connection() != HAL_OK){
		debug_print_str(DEBUG_ERROR,"STUSB4500 unavailable");
		return;
	}
	debug_print_str(DEBUG_INFO,"STUSB4500 found");
	pd_status.controller = PD_CTRL_STUSB4500;

	stusb_init();

	/* 1. NVM REQ_SRC_CURRENT check/provision, before the attach gate so a
	 * DC-powered board is provisioned or healed as well. */
	stusb_nvm_state_t nvm = stusb_nvm_provision();
	debug_print_int(DEBUG_INFO,"STUSB4500 NVM state", (int)nvm);
	r.mode = (nvm == STUSB_NVM_ERROR) ? PD_MODE_NVM_ERROR : PD_MODE_NONE;

	/* 2. Is a USB-C cable actually attached? */
	int8_t attached = stusb_sink_attached();
	if(attached == 0){
		debug_print_str(DEBUG_INFO,"No USB-PD sink connected");
		pd_status_publish(&r, 0.0f);
		return;
	}
	pd_status.sink_connected = 1;

	/* 3. Negotiate: auto-current if the NVM bit is live, else the ladder.
	 * Attach state unknown (I2C failure): skip straight to fail closed. */
	if(attached > 0){
		(void)stusb_wait_sink_ready();
		negotiate_stusb4500(&r, nvm);
	}
	if(!r.contract){
		debug_print_str(DEBUG_WARNING,"No USB-PD contract - limiting power");
	}

	/* 4. Heater limit from the contract, or fail closed. */
	float watts = stusb_power_limit(&r);
	power_source = POWER_USB;
	sensor_values.USB_PD_power_limit = watts;
	debug_print_int(DEBUG_INFO,"USB-PD power limit (W)", (int)watts);

	pd_status_publish(&r, watts);
}

void power_source_init(void){
	power_source_probe();
	pd_status.probed = 1;   /* on every exit path, incl. no STUSB4500 */
}
