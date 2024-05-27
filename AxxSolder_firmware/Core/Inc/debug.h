#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include "stm32g4xx_hal.h"
#include "string.h"
#include "stusb4500.h"

typedef enum{
	DEBUG_OFF = 0,
	DEBUG_INFO,
	DEBUG_WARNING,
	DEBUG_ERROR,
	DEBUG_MAX_LEVELS
}DEBUG_VERBOSITY_t;

HAL_StatusTypeDef debug_print_pdos(DEBUG_VERBOSITY_t verbosity, PDO_container_t *pdos);
HAL_StatusTypeDef debug_print_str(DEBUG_VERBOSITY_t verbosity, char * str);
HAL_StatusTypeDef debug_print_int(DEBUG_VERBOSITY_t verbosity, char * str, int i);

#endif /* INC_DEBUG_H_ */
