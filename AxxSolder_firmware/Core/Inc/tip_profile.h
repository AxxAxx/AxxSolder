#ifndef INC_TIP_PROFILE_H_
#define INC_TIP_PROFILE_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "storage.h"

#define PROFILE_NAME_LEN   24
#define MAX_PROFILES       24  /* 16 + 24*72 = 1744 bytes, fits in 2 KB PROFILES page */
#define PROFILES_MAGIC      0x54495031  /* "TIP1" */
#define PROFILES_VERSION    2
#define NUM_CAL_POINTS      6

/* Single tip profile */
typedef struct {
	char    name[PROFILE_NAME_LEN];
	uint8_t handle_type;             /* enum handles value */
	uint8_t _pad[3];
	float   kp;
	float   ki;
	float   kd;
	float   max_i;
	float   power_limit;              /* 0 = no limit (use handle default) */
	float   temp_cal[NUM_CAL_POINTS]; /* 100, 200, 300, 350, 400, 450 */
} TipProfile;                         /* 72 bytes */

/* Stored block: header + profiles array */
typedef struct {
	uint32_t   magic;
	uint8_t    version;
	uint8_t    count;                 /* valid profiles 0..MAX_PROFILES */
	uint8_t    active_idx[4];         /* per handle type: NT115=0, T210=1, T245=2, No_name=3 */
	                                  /* 0xFF = no profile active */
	uint8_t    _reserved[6];
	TipProfile profiles[MAX_PROFILES];
} ProfileStore;                       /* 16 + 24*72 = 1744 bytes */

/* PID_TypeDef is defined in pid.h — include it for apply_pid prototype */
#include "pid.h"

/* Lifecycle */
void        tip_profiles_init(const StorageDriver *drv);
StoreResult tip_profiles_save(void);

/* CRUD */
uint8_t     tip_profiles_count(void);
TipProfile *tip_profiles_get(uint8_t index);
StoreResult tip_profiles_add(const TipProfile *p);
StoreResult tip_profiles_delete(uint8_t index);
StoreResult tip_profiles_update(uint8_t index, const TipProfile *p);

/* Active profile per handle */
void        tip_profiles_set_active(enum handles h, uint8_t idx);
uint8_t     tip_profiles_get_active(enum handles h);

/* Apply active profile's PID to controller */
void        tip_profiles_apply_pid(enum handles h, PID_TypeDef *pid);

/* Get calibration value from active profile.
 * cal_index 0..5 maps to 100/200/300/350/400/450.
 * Returns identity value if no profile active. */
float       tip_profiles_get_cal(enum handles h, uint8_t cal_index);

/* Get power limit from active profile (0 = no limit). */
float       tip_profiles_get_power_limit(enum handles h);

/* Identity calibration values (100, 200, 300, 350, 400, 450) */
extern const float identity_cal[NUM_CAL_POINTS];

#endif /* INC_TIP_PROFILE_H_ */
