#ifndef STAND_H
#define STAND_H

#include "moving_average.h"

/* Stand-sense filter. File-scope so main.c initializes it at boot. */
extern FilterTypeDef stand_sense_filterStruct;

/* Read STAND_INP GPIO (and HANDLE_INP_2 when NT115 detection is off),
 * debounce, update sensor_values.in_stand, and drive state-machine
 * transitions on stand entry/exit:
 *   stand entered:  RUN -> PRESTANDBY/STANDBY, STANDBY -> SLEEP after delay
 *   stand exited:   any non-RUN -> RUN
 *
 * Call once per fast sensor tick. */
void stand_update(void);

#endif /* STAND_H */
