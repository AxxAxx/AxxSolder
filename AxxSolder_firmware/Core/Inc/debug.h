#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include "stm32g4xx_hal.h"
#include "string.h"

typedef enum{
	DEBUG_OFF = 0,
	DEBUG_INFO,
	DEBUG_WARNING,
	DEBUG_ERROR,
	DEBUG_MAX_LEVELS
}DEBUG_VERBOSITY_t;

/* Serial debug prints on USART1 (256000 8N1). Compiled in ONLY when DEBUG is
 * defined (the STM32CubeIDE Debug configuration, or -DDEBUG in a CMake build);
 * in release builds every debug_print_* call and its string argument is removed
 * by the preprocessor, so there is no flash or run-time cost. */
#ifdef DEBUG
HAL_StatusTypeDef debug_print_str(DEBUG_VERBOSITY_t verbosity, char * str);
HAL_StatusTypeDef debug_print_int(DEBUG_VERBOSITY_t verbosity, char * str, int i);
HAL_StatusTypeDef debug_print_int_int(DEBUG_VERBOSITY_t verbosity, char * str, int i, int j);
#else
#define debug_print_str(verbosity, str)             ((void)0)
#define debug_print_int(verbosity, str, i)          ((void)0)
#define debug_print_int_int(verbosity, str, i, j)   ((void)0)
#endif

#endif /* INC_DEBUG_H_ */
