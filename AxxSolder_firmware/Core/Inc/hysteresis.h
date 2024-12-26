#ifndef INC_HYSTERESIS_H
#define INC_HYSTERESIS_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* TypeDefs ------------------------------------------------------------------*/
typedef struct{
	float previous_value;	// previous value
	float hysteresis;		// hysteresis
}Hysteresis_FilterTypeDef;

/* Function prototypes -------------------------------------------------------*/
void Hysteresis_Init(Hysteresis_FilterTypeDef* hysteresis_struct, float hysteresis);
float Hysteresis_Add(float new_value, Hysteresis_FilterTypeDef* hysteresis_struct);

#endif
