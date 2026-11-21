#include "debug.h"

/* The whole module exists only in DEBUG builds; in release, debug.h maps the
 * debug_print_* calls to no-ops, so nothing here is needed or compiled. */
#ifdef DEBUG
#include <stdio.h>
#include "string.h"

extern UART_HandleTypeDef huart1;

/* Module-owned verbosity gate. Set at boot (could be made user-configurable
 * via flash_values later). All debug_print_* functions check this before
 * doing the synchronous UART transmit. */
DEBUG_VERBOSITY_t debugLevel = DEBUG_INFO;
static char buffer[64];

static const char *debug_level_str[] = {
	"",
	"[I]: ",
	"[W]: ",
	"[E]: ",
};


HAL_StatusTypeDef debug_print_str(DEBUG_VERBOSITY_t verbosity, char * str){
	HAL_StatusTypeDef ret = HAL_OK;
	if(debugLevel > 0 && verbosity >= debugLevel){
		memset(buffer, '\0', sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "%s %s\n", debug_level_str[verbosity], str);
		ret = HAL_UART_Transmit(&huart1, (uint8_t *) buffer, (uint16_t)strlen(buffer),100);
	}
	return ret;
}

HAL_StatusTypeDef debug_print_int(DEBUG_VERBOSITY_t verbosity, char * str, int i){
	HAL_StatusTypeDef ret = HAL_OK;
	if(debugLevel > 0 && verbosity >= debugLevel){
		memset(&buffer, '\0', sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "%s %s %i\n", debug_level_str[verbosity], str, i);
		ret = HAL_UART_Transmit(&huart1, (uint8_t *) buffer, (uint16_t)strlen(buffer),100);
	}
	return ret;
}

HAL_StatusTypeDef debug_print_int_int(DEBUG_VERBOSITY_t verbosity, char * str, int i, int j){
	HAL_StatusTypeDef ret = HAL_OK;
	if(debugLevel > 0 && verbosity >= debugLevel){
		memset(buffer, '\0', sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "%s %s %i %i\n", debug_level_str[verbosity], str, i, j);
		ret = HAL_UART_Transmit(&huart1, (uint8_t *) buffer, (uint16_t)strlen(buffer),100);
	}
	return ret;
}

#endif /* DEBUG */
