#include "tip_profile.h"
#include "storage.h"
#include <string.h>

/* RAM cache */
static ProfileStore cache;
static const StorageDriver *driver;

/* Identity calibration values (no adjustment) — also used by menu_profiles.c */
const float identity_cal[NUM_CAL_POINTS] = {
	100.0f, 200.0f, 300.0f, 350.0f, 400.0f, 450.0f
};

/* Default PID values per handle type (moved from main.c #defines) */
static const TipProfile default_profiles[] = {
	{
		.name = "T245 Default",
		.handle_type = T245,
		.kp = 8.0f, .ki = 2.0f, .kd = 0.5f, .max_i = 300.0f,
		.power_limit = 0.0f,
		.temp_cal = {100.0f, 200.0f, 300.0f, 350.0f, 400.0f, 450.0f}
	},
	{
		.name = "T210 Default",
		.handle_type = T210,
		.kp = 7.0f, .ki = 4.0f, .kd = 0.3f, .max_i = 300.0f,
		.power_limit = 0.0f,
		.temp_cal = {100.0f, 200.0f, 300.0f, 350.0f, 400.0f, 450.0f}
	},
	{
		.name = "NT115 Default",
		.handle_type = NT115,
		.kp = 5.0f, .ki = 2.0f, .kd = 0.3f, .max_i = 300.0f,
		.power_limit = 0.0f,
		.temp_cal = {100.0f, 200.0f, 300.0f, 350.0f, 400.0f, 450.0f}
	},
	{
		.name = "No_name Default",
		.handle_type = No_name,
		.kp = 8.0f, .ki = 2.0f, .kd = 0.5f, .max_i = 300.0f,
		.power_limit = 0.0f,
		.temp_cal = {100.0f, 200.0f, 300.0f, 350.0f, 400.0f, 450.0f}
	},
};

#define NUM_DEFAULT_PROFILES (sizeof(default_profiles) / sizeof(default_profiles[0]))

/* Map enum handles to active_idx array index (0..3) */
static uint8_t handle_to_slot(enum handles h)
{
	switch (h) {
	case NT115:   return 0;
	case T210:    return 1;
	case T245:    return 2;
	case No_name: return 3;
	default:      return 3;
	}
}

/* Reverse mapping: slot index (0..3) to enum handles */
static enum handles slot_to_handle(uint8_t slot)
{
	switch (slot) {
	case 0:  return NT115;
	case 1:  return T210;
	case 2:  return T245;
	case 3:  return No_name;
	default: return No_name;
	}
}

/* Find the active profile for a handle, returns NULL if none */
static TipProfile *get_active_profile(enum handles h)
{
	uint8_t slot = handle_to_slot(h);
	uint8_t idx = cache.active_idx[slot];

	if (idx >= cache.count) {
		return NULL;
	}

	return &cache.profiles[idx];
}

/* Find first profile matching handle type */
static TipProfile *find_by_handle(enum handles h)
{
	for (uint8_t i = 0; i < cache.count; i++) {
		if (cache.profiles[i].handle_type == (uint8_t)h) {
			return &cache.profiles[i];
		}
	}
	return NULL;
}

static void create_defaults(void)
{
	memset(&cache, 0, sizeof(cache));
	cache.magic = PROFILES_MAGIC;
	cache.version = PROFILES_VERSION;
	cache.count = NUM_DEFAULT_PROFILES;
	memset(cache.active_idx, 0xFF, sizeof(cache.active_idx));

	for (uint8_t i = 0; i < NUM_DEFAULT_PROFILES; i++) {
		cache.profiles[i] = default_profiles[i];
		/* Set each default profile as active for its handle type */
		cache.active_idx[handle_to_slot((enum handles)default_profiles[i].handle_type)] = i;
	}
}

void tip_profiles_init(const StorageDriver *drv)
{
	driver = drv;

	StoreResult res = driver->read(PROFILES_PAGE, &cache, sizeof(ProfileStore));

	if (res != STORE_OK || cache.magic != PROFILES_MAGIC || cache.version != PROFILES_VERSION) {
		create_defaults();
		driver->write(PROFILES_PAGE, &cache, sizeof(ProfileStore));
	}
}

StoreResult tip_profiles_save(void)
{
	return driver->write(PROFILES_PAGE, &cache, sizeof(ProfileStore));
}

uint8_t tip_profiles_count(void)
{
	return cache.count;
}

TipProfile *tip_profiles_get(uint8_t index)
{
	if (index >= cache.count) {
		return NULL;
	}
	return &cache.profiles[index];
}

StoreResult tip_profiles_add(const TipProfile *p)
{
	if (cache.count >= MAX_PROFILES) {
		return STORE_ERR_FULL;
	}

	cache.profiles[cache.count] = *p;
	cache.count++;
	return STORE_OK;
}

StoreResult tip_profiles_delete(uint8_t index)
{
	if (index >= cache.count) {
		return STORE_ERR_INVALID;
	}

	/* Remember which slots had this profile as active before shifting */
	uint8_t was_active[4] = {0};
	for (uint8_t s = 0; s < 4; s++) {
		if (cache.active_idx[s] == index) was_active[s] = 1;
	}

	/* Shift remaining profiles down */
	for (uint8_t i = index; i < cache.count - 1; i++) {
		cache.profiles[i] = cache.profiles[i + 1];
	}
	cache.count--;

	/* Fix active_idx references */
	for (uint8_t s = 0; s < 4; s++) {
		if (was_active[s]) {
			/* Reassign to another profile for the same handle type */
			cache.active_idx[s] = 0xFF;
			uint8_t handle = (uint8_t)slot_to_handle(s);
			for (uint8_t i = 0; i < cache.count; i++) {
				if (cache.profiles[i].handle_type == handle) {
					cache.active_idx[s] = i;
					break;
				}
			}
		} else if (cache.active_idx[s] != 0xFF && cache.active_idx[s] > index) {
			cache.active_idx[s]--;
		}
	}

	return STORE_OK;
}

StoreResult tip_profiles_update(uint8_t index, const TipProfile *p)
{
	if (index >= cache.count) {
		return STORE_ERR_INVALID;
	}

	cache.profiles[index] = *p;
	return STORE_OK;
}

void tip_profiles_set_active(enum handles h, uint8_t idx)
{
	if (idx != 0xFF && idx >= cache.count) return;
	cache.active_idx[handle_to_slot(h)] = idx;
}

uint8_t tip_profiles_get_active(enum handles h)
{
	return cache.active_idx[handle_to_slot(h)];
}

void tip_profiles_apply_pid(enum handles h, PID_TypeDef *pid)
{
	TipProfile *p = get_active_profile(h);
	if (!p) {
		p = find_by_handle(h);
	}
	if (!p) {
		return;
	}

	PID_SetTunings(pid, p->kp, p->ki, p->kd);
	PID_SetILimits(pid, -p->max_i, p->max_i);
}

float tip_profiles_get_power_limit(enum handles h)
{
	TipProfile *p = get_active_profile(h);
	if (!p) {
		p = find_by_handle(h);
	}
	return p ? p->power_limit : 0.0f;
}

float tip_profiles_get_cal(enum handles h, uint8_t cal_index)
{
	if (cal_index >= NUM_CAL_POINTS) {
		return 0.0f;
	}

	TipProfile *p = get_active_profile(h);
	if (!p) {
		p = find_by_handle(h);
	}
	if (!p) {
		return identity_cal[cal_index];
	}

	return p->temp_cal[cal_index];
}
