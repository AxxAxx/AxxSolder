#ifndef INC_HYSTERESIS_H
#define INC_HYSTERESIS_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* TypeDefs ------------------------------------------------------------------*/
typedef struct{
	double previous_value;	// previous value
	double hysteresis;		// hysteresis
}Hysteresis_FilterTypeDef;

/* Function prototypes -------------------------------------------------------*/
void Hysteresis_Init(Hysteresis_FilterTypeDef* hysteresis_struct, float hysteresis);
double Hysteresis_Add(double new_value, Hysteresis_FilterTypeDef* hysteresis_struct);

#endif
