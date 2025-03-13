#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "stm32g4xx_hal.h"

/* Includes ------------------------------------------------------------------*/

/* Definitions ---------------------------------------------------------------*/

/* TypeDefs ------------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
void beep(float buzzer_enabled);
void beep_double(float buzzer_enabled);

#endif /* INC_BUZZER_H_ */
