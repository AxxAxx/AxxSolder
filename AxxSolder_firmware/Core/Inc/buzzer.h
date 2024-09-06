#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "stm32g4xx_hal.h"

/* Includes ------------------------------------------------------------------*/

/* Definitions ---------------------------------------------------------------*/

/* TypeDefs ------------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
void beep(double buzzer_enabled);
void beep_startup(double buzzer_enabled);

#endif /* INC_BUZZER_H_ */
