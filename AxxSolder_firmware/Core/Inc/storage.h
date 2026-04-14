#ifndef INC_STORAGE_H_
#define INC_STORAGE_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	STORE_OK = 0,
	STORE_ERR_CRC,
	STORE_ERR_WRITE,
	STORE_ERR_FULL,
	STORE_ERR_INVALID
} StoreResult;

typedef struct {
	StoreResult (*read)(uint32_t addr, void *buf, uint16_t size);
	StoreResult (*write)(uint32_t addr, const void *buf, uint16_t size);
	bool        (*verify)(uint32_t addr, uint16_t size);
} StorageDriver;

/* Flash backend */
extern const StorageDriver storage_flash;

/* ---- Linker symbols for flash sections ---- */
extern uint32_t _sprofiles;
extern uint32_t _eprofiles;
extern uint32_t _ssettings;
extern uint32_t _esettings;

#define PROFILES_PAGE    ((uint32_t)&_sprofiles)
#define SETTINGS_PAGE    ((uint32_t)&_ssettings)

/* ---- Select profiles storage driver via build flag ---- */
#if defined(STORAGE_PROFILES_FLASH)
  #define STORAGE_PROFILES_DRIVER (&storage_flash)
#elif defined(STORAGE_PROFILES_EEPROM)
  extern const StorageDriver storage_eeprom;
  #define STORAGE_PROFILES_DRIVER (&storage_eeprom)
#elif defined(STORAGE_PROFILES_SDCARD)
  extern const StorageDriver storage_sdcard;
  #define STORAGE_PROFILES_DRIVER (&storage_sdcard)
#else
  #define STORAGE_PROFILES_DRIVER (&storage_flash)
#endif

/* ---- Select settings storage driver via build flag ---- */
#if defined(STORAGE_SETTINGS_FLASH)
  #define STORAGE_SETTINGS_DRIVER (&storage_flash)
#elif defined(STORAGE_SETTINGS_EEPROM)
  extern const StorageDriver storage_eeprom;
  #define STORAGE_SETTINGS_DRIVER (&storage_eeprom)
#elif defined(STORAGE_SETTINGS_SDCARD)
  extern const StorageDriver storage_sdcard;
  #define STORAGE_SETTINGS_DRIVER (&storage_sdcard)
#else
  #define STORAGE_SETTINGS_DRIVER (&storage_flash)
#endif

#endif /* INC_STORAGE_H_ */
