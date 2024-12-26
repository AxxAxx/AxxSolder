#ifndef INC_PID_H
#define INC_PID_H

/* Include */
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "stm32g4xx_hal.h"       /* Import HAL library */
#include <math.h>

#define DEFAULT_SAMPLE_TIME_MS 100
#define DEFAULT_PWM_MAX 100

/* PID Mode */
typedef enum{
	_PID_MODE_MANUAL    = 0,
	_PID_MODE_AUTOMATIC = 1
}PIDMode_TypeDef;

/* PID Control direction */
typedef enum{
	_PID_CD_DIRECT  = 0,
	_PID_CD_REVERSE = 1
}PIDCD_TypeDef;

/* PID Structure */
typedef struct{

	PIDMode_TypeDef InAuto;
	PIDCD_TypeDef   ControllerDirection;

	uint32_t        LastTime;
	uint32_t        SampleTime;

	uint32_t        updateOnEveryCall;

	float          DispKp;
	float          DispKi;
	float          DispKd;

	float          Kp;
	float          Ki;
	float          Kd;

	float          DispKp_part;
	float          DispKi_part;
	float          DispKd_part;

	float          *MyInput;
	float          *MyOutput;
	float          *MySetpoint;

	float          OutputSum;
	float          LastInput;

	float          OutMin;
	float          OutMax;

	float          IMin;
	float          IMax;

	float			IminError;

	float			NegativeErrorIgainMultiplier;
	float			NegativeErrorIgainBias;

}PID_TypeDef;

/* Init */
void PID_Init(PID_TypeDef *uPID);

void PID(PID_TypeDef *uPID, float *Input, float *Output, float *Setpoint, float Kp, float Ki, float Kd, PIDCD_TypeDef ControllerDirection);

/* Function to clamp d between the limits min and max */
float float_clamp(float d, float min, float max);

/* Function to check if clamping will occur */
uint8_t check_clamping(float d, float min, float max);

/* Computing */
uint8_t PID_Compute(PID_TypeDef *uPID);

/* PID Limits */
void PID_SetOutputLimits(PID_TypeDef *uPID, float Min, float Max);

/* PID I-windup Limits */
void PID_SetILimits(PID_TypeDef *uPID, float Min, float Max);

/* Minimum error where I is added */
void PID_SetIminError(PID_TypeDef *uPID, float IminError);

/* Set the I gain multiplier for negative error*/
void PID_SetNegativeErrorIgainMult(PID_TypeDef *uPID, float NegativeErrorIgainMultiplier, float NegativeErrorIgainBias);

/* PID Tunings */
void PID_SetTunings(PID_TypeDef *uPID, float Kp, float Ki, float Kd);

/* PID Direction */
void          PID_SetControllerDirection(PID_TypeDef *uPID, PIDCD_TypeDef Direction);
PIDCD_TypeDef PID_GetDirection(PID_TypeDef *uPID);

/* PID Sampling */
void PID_SetSampleTime(PID_TypeDef *uPID, int32_t NewSampleTime, int32_t updateOnCall);

void PID_SetMode(PID_TypeDef *uPID, PIDMode_TypeDef Mode);

/* Get Tunings Param */
float PID_GetKp(PID_TypeDef *uPID);
float PID_GetKi(PID_TypeDef *uPID);
float PID_GetKd(PID_TypeDef *uPID);

/* Get current contributions */
float PID_GetPpart(PID_TypeDef *uPID);
float PID_GetIpart(PID_TypeDef *uPID);
float PID_GetDpart(PID_TypeDef *uPID);

#endif
