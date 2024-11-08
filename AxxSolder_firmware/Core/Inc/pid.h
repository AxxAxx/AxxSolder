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

	double          DispKp;
	double          DispKi;
	double          DispKd;

	double          Kp;
	double          Ki;
	double          Kd;

	double          DispKp_part;
	double          DispKi_part;
	double          DispKd_part;

	double          *MyInput;
	double          *MyOutput;
	double          *MySetpoint;

	double          OutputSum;
	double          LastInput;

	double          OutMin;
	double          OutMax;

	double          IMin;
	double          IMax;

	double			IminError;

	double			NegativeErrorIgainMultiplier;

}PID_TypeDef;

/* Init */
void PID_Init(PID_TypeDef *uPID);

void PID(PID_TypeDef *uPID, double *Input, double *Output, double *Setpoint, double Kp, double Ki, double Kd, PIDCD_TypeDef ControllerDirection);

/* Function to clamp d between the limits min and max */
double double_clamp(double d, double min, double max);

/* Function to check if clamping will occur */
uint8_t check_clamping(double d, double min, double max);

/* Computing */
uint8_t PID_Compute(PID_TypeDef *uPID);

/* PID Limits */
void PID_SetOutputLimits(PID_TypeDef *uPID, double Min, double Max);

/* PID I-windup Limits */
void PID_SetILimits(PID_TypeDef *uPID, double Min, double Max);

/* Minimum error where I is added */
void PID_SetIminError(PID_TypeDef *uPID, double IminError);

/* Set the I gain multiplier for negative error*/
void PID_SetNegativeErrorIgainMult(PID_TypeDef *uPID, double NegativeErrorIgainMultiplier);

/* PID Tunings */
void PID_SetTunings(PID_TypeDef *uPID, double Kp, double Ki, double Kd);

/* PID Direction */
void          PID_SetControllerDirection(PID_TypeDef *uPID, PIDCD_TypeDef Direction);
PIDCD_TypeDef PID_GetDirection(PID_TypeDef *uPID);

/* PID Sampling */
void PID_SetSampleTime(PID_TypeDef *uPID, int32_t NewSampleTime, int32_t updateOnCall);

void PID_SetMode(PID_TypeDef *uPID, PIDMode_TypeDef Mode);

/* Get Tunings Param */
double PID_GetKp(PID_TypeDef *uPID);
double PID_GetKi(PID_TypeDef *uPID);
double PID_GetKd(PID_TypeDef *uPID);

/* Get current contributions */
double PID_GetPpart(PID_TypeDef *uPID);
double PID_GetIpart(PID_TypeDef *uPID);
double PID_GetDpart(PID_TypeDef *uPID);

#endif
