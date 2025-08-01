/*
 * hysteresis.c
 *
 *  Created on: Sep 20, 2024
 *      Author: Axel Johansson
 */


/* Includes ------------------------------------------------------------------*/
#include "hysteresis.h"

void Hysteresis_Init(Hysteresis_FilterTypeDef* hysteresis_struct, float hysteresis)
{
    hysteresis_struct->hysteresis = hysteresis;
    hysteresis_struct->previous_value = NAN;  // Status: not yet initialized
}

float Hysteresis_Add(float new_value, Hysteresis_FilterTypeDef* hysteresis_struct) {
    if (isnan(hysteresis_struct->previous_value)) {
        hysteresis_struct->previous_value = new_value;
        return new_value;
    }

    if ((new_value >= hysteresis_struct->previous_value + hysteresis_struct->hysteresis) ||
        (new_value <= hysteresis_struct->previous_value - hysteresis_struct->hysteresis)) {
        hysteresis_struct->previous_value = new_value;
    }

    return hysteresis_struct->previous_value;
}


