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

/* The voltage/current we ask the source for. PDO1 is forced to 5 V (spec) as a
 * safe fallback; PDO2 is our high-power target. If the source cannot supply
 * PDO2 the STUSB4500 falls back on its own (e.g. to 9 V or 5 V). */
#define USB_PD_TARGET_VOLTAGE_MV 20000
#define USB_PD_TARGET_CURRENT_MA 3000

/* --- Public state --- */
power_source_t power_source = POWER_DC;

/* check STUSB4500 */
void power_source_init(void){
	if(stusb_check_connection() != HAL_OK){
		//do error handling for STUSB
		debug_print_str(DEBUG_ERROR,"STUSB4500 unavailable");
		return;
	}
	debug_print_str(DEBUG_INFO,"STUSB4500 found");

	stusb_init();

	/* 1. Is a USB-C cable actually attached? */
	if(!stusb_is_sink_connected()){
		debug_print_str(DEBUG_INFO,"No USB-PD sink connected");
		return;
	}

	/* 2. Request our profile and renegotiate with a SINGLE clean soft-reset.
	 *
	 * The previous implementation polled the source capabilities and then
	 * force-wrote the "highest" source PDO via a second soft-reset. When the
	 * capability poll returned nothing it wrote a 0 V PDO and reset again,
	 * thrashing the contract - which browned out marginal boards (the iron
	 * powered up then died). Requesting our fixed target once is robust: the
	 * STUSB4500 negotiates the highest profile the source can satisfy, or
	 * falls back by itself, with no contract thrashing. */
	stusb_update_pdo(1, 5000,                    USB_PD_TARGET_CURRENT_MA); /* PDO1 = 5 V fallback */
	stusb_update_pdo(2, USB_PD_TARGET_VOLTAGE_MV, USB_PD_TARGET_CURRENT_MA); /* PDO2 = 20 V target */
	stusb_set_valid_pdo(2);
	stusb_soft_reset();

	/* 3. Wait for the renegotiation to complete and VBUS to settle.
	 *
	 * IMPORTANT: immediately after the soft-reset the OLD contract can still
	 * report SNK_READY, and VBUS only ramps to the new voltage once the source
	 * accepts the request. So we must NOT sample right away (that reads the
	 * stale, lower voltage). Wait a fixed minimum for the transition to begin,
	 * then poll the instantaneous bus voltage until it has held steady. */
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

	/* 4. Confirm an explicit PD contract was established. */
	STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef rdo;
	stusb_read_rdo(&rdo);
	if(rdo.b.Object_Pos == 0){
		debug_print_str(DEBUG_INFO,"No USB-PD contract");
		return;   /* leave power_source = POWER_DC */
	}
	power_source = POWER_USB;

	/* 5. VBUS has settled at the negotiated voltage: PRIME the bus-voltage
	 * moving-average filter with it so the displayed input voltage shows the
	 * negotiated value immediately instead of ramping across the filter window. */
	sensors_prime_bus_voltage();    /* fill the MA window with the settled reading */
	sensors_get_bus_voltage();      /* refresh sensor_values.bus_voltage from the primed filter */

	/* Derive the heater power limit from the ACTUAL contract: measured bus
	 * voltage (PA2) times the negotiated operating current (RDO, 10 mA units). */
	float negotiated_current_a = rdo.b.OperatingCurrent * 0.01f;   /* 10 mA units -> A */
	float watts = sensor_values.bus_voltage * negotiated_current_a * USB_PD_POWER_REDUCTION_FACTOR;
	sensor_values.USB_PD_power_limit = watts;
	debug_print_int(DEBUG_INFO,"USB-PD power limit (W)", (int)watts);
}
