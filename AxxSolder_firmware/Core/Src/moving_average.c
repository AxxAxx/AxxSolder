/**
  ******************************************************************************
  * @file    moving_average.c
  * @author  Mohammad Hussein Tavakoli Bina, Sepehr Hashtroudi
  * @brief   This file contains an efficient implementation of
	*					 moving average filter.
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

/* Includes ------------------------------------------------------------------*/
#include "moving_average.h"

/**
  * @brief  This function initializes filter's data structure.
	* @param  filter_struct : Data structure
  * @retval None.
  */
void Moving_Average_Init(FilterTypeDef* filter_struct, uint32_t window_length)
{
	filter_struct->WindowLength = window_length;

	filter_struct->Sum = 0;
	filter_struct->WindowPointer = 0;

	for(uint32_t i=0; i<(uint32_t)filter_struct->WindowLength; i++)
	{
		filter_struct->History[i] = 0;
	}
}

/**
  * @brief  This function filters data with moving average filter.
	* @param  raw_data : input raw sensor data.
	* @param  filter_struct : Data structure
  * @retval Filtered value.
  */
float Moving_Average_Compute(float raw_data, FilterTypeDef* filter_struct)
{
	filter_struct->Sum += raw_data;
	filter_struct->Sum -= filter_struct->History[filter_struct->WindowPointer];
	filter_struct->History[filter_struct->WindowPointer] = raw_data;
	if(filter_struct->WindowPointer < (uint32_t)(filter_struct->WindowLength - 1.0f))
	{
		filter_struct->WindowPointer += 1;
	}
	else
	{
		filter_struct->WindowPointer = 0;
	}
	return (float)filter_struct->Sum/(float)filter_struct->WindowLength;
}

/* Fill moving average filter history with raw_data */
void Moving_Average_Set_Value(float raw_data, FilterTypeDef* filter_struct){
	filter_struct->Sum = raw_data*filter_struct->WindowLength;
	filter_struct->WindowPointer = 0;

	for(uint32_t i=0; i<(uint32_t)filter_struct->WindowLength; i++)
	{
		filter_struct->History[i] = raw_data;
	}
}

