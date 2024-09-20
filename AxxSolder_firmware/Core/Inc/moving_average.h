/**
  ******************************************************************************
  * @file    moving_average.h
  * @author  Mohammad Hussein Tavakoli Bina, Sepehr Hashtroudi.
  * @brief   This file contains function prototype of moving average filter.
  * @remark  2024-09-19 - Edited by Axel Johansson, Adding function
  *  			Moving_Average_Set_Value
  ******************************************************************************
  *MIT License
  *
  *Copyright (c) 2018 Mohammad Hussein Tavakoli Bina
  *
  *Permission is hereby granted, free of charge, to any person obtaining a copy
  *of this software and associated documentation files (the "Software"), to deal
  *in the Software without restriction, including without limitation the rights
  *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  *copies of the Software, and to permit persons to whom the Software is
  *furnished to do so, subject to the following conditions:
  *
  *The above copyright notice and this permission notice shall be included in all
  *copies or substantial portions of the Software.
  *
  *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  *SOFTWARE.
  */

#ifndef INC_MOVING_AVERAGE_H
#define INC_MOVING_AVERAGE_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* Definitions ---------------------------------------------------------------*/
#define Max_WindowLength 200

/* TypeDefs ------------------------------------------------------------------*/
typedef struct{
	float WindowLength;
	float History[Max_WindowLength]; /*Array to store values of filter window*/
	float Sum;	/* Sum of filter window's elements*/
	uint32_t WindowPointer; /* Pointer to the first element of window*/
}FilterTypeDef;

/* Function prototypes -------------------------------------------------------*/
void Moving_Average_Init(FilterTypeDef* filter_struct, uint32_t window_length);
float Moving_Average_Compute(float raw_data, FilterTypeDef* filter_struct);
void Moving_Average_Set_Value(float raw_data, FilterTypeDef* filter_struct);

#endif
