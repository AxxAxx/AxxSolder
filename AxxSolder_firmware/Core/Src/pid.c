#include "pid.h"

/* Initialize */
void PID_Init(PID_TypeDef *uPID)
{
	/* Set parameters */
	uPID->OutputSum = *uPID->MyOutput;
	uPID->LastInput = *uPID->MyInput;

	if (uPID->OutputSum > uPID->OutMax)
	{
		uPID->OutputSum = uPID->OutMax;
	}
	else if (uPID->OutputSum < uPID->OutMin)
	{
		uPID->OutputSum = uPID->OutMin;
	}
	else { }
}

void PID(PID_TypeDef *uPID, double *Input, double *Output, double *Setpoint, double Kp, double Ki, double Kd, PIDCD_TypeDef ControllerDirection)
{
	/* Set parameters */
	uPID->MyOutput   = Output;
	uPID->MyInput    = Input;
	uPID->MySetpoint = Setpoint;
	uPID->InAuto     = (PIDMode_TypeDef)_FALSE;

	PID_SetOutputLimits(uPID, 0, _PID_8BIT_PWM_MAX);

	uPID->SampleTime = _PID_SAMPLE_TIME_MS_DEF;

	PID_SetControllerDirection(uPID, ControllerDirection);
	PID_SetTunings2(uPID, Kp, Ki, Kd);

	uPID->LastTime = HAL_GetTick() - uPID->SampleTime;
}

void PID2(PID_TypeDef *uPID, double *Input, double *Output, double *Setpoint, double Kp, double Ki, double Kd, PIDCD_TypeDef ControllerDirection)
{
	PID(uPID, Input, Output, Setpoint, Kp, Ki, Kd, ControllerDirection);
}

/* Compute  */
uint8_t PID_Compute(PID_TypeDef *uPID){
	uint32_t now;
	uint32_t timeChange;

	double input;
	double error;
	double dInput;
	double output;

	/* Check PID mode */
	if (!uPID->InAuto)
	{
		return _FALSE;
	}

	/* Calculate time */
	now        = HAL_GetTick();
	timeChange = (now - uPID->LastTime);

	if ((timeChange >= uPID->SampleTime) || (uPID->updateOnEveryCall))
	{
		/* Compute all the working error variables */
		input   = *uPID->MyInput;
		error   = *uPID->MySetpoint - input;
		dInput  = (input - uPID->LastInput);


		/* Calculate Proportional on Error */
		output = uPID->Kp * error;
		uPID->DispKp_part = output;

		/* ..... Calculate Derivative term and sum */
		output -= uPID->Kd * dInput;
		uPID->DispKd_part = - uPID->Kd * dInput;

		/* Conditional integration as anti-windup (clamping) */
		if((((output + uPID->Ki * error) > uPID->OutMax || (output + uPID->Ki * error) < uPID->OutMin)) && (error*(output + uPID->OutputSum + uPID->Ki * error) > 0)){
			uPID->OutputSum     += 0;
		}
		else{
			uPID->OutputSum     += (uPID->Ki * error);
		}
		/* If Setpoint is set to 0, zero integral part */
		if(*uPID->MySetpoint == 0){
			uPID->OutputSum = 0;
		}
		uPID->DispKi_part = uPID->OutputSum;

		/* Final summation */
		output += uPID->OutputSum;

		/* Clamp output */
		if (output > uPID->OutMax){
			output = uPID->OutMax;
		}
		else if (output < uPID->OutMin){
			output = uPID->OutMin;
		}

		*uPID->MyOutput = output;


		/* Remember some variables for next time */
		uPID->LastInput = input;
		uPID->LastTime = now;

		return _TRUE;

	}
	else
	{
		return _FALSE;
	}

}

/* PID Mode */
void PID_SetMode(PID_TypeDef *uPID, PIDMode_TypeDef Mode){
	uint8_t newAuto = (Mode == _PID_MODE_AUTOMATIC);

	/* Initialize the PID */
	if (newAuto && !uPID->InAuto){
		PID_Init(uPID);
	}
	uPID->InAuto = (PIDMode_TypeDef)newAuto;
}

PIDMode_TypeDef PID_GetMode(PID_TypeDef *uPID){
	return uPID->InAuto ? _PID_MODE_AUTOMATIC : _PID_MODE_MANUAL;
}

/* PID Limits */
void PID_SetOutputLimits(PID_TypeDef *uPID, double Min, double Max){
	/* Check value */
	if (Min >= Max){
		return;
	}

	uPID->OutMin = Min;
	uPID->OutMax = Max;

	/* Check PID Mode */
	if (uPID->InAuto){

		/* Check value */
		if (*uPID->MyOutput > uPID->OutMax){
			*uPID->MyOutput = uPID->OutMax;
		}
		else if (*uPID->MyOutput < uPID->OutMin){
			*uPID->MyOutput = uPID->OutMin;
		}
		else { }

		/* Check out value */
		if (uPID->OutputSum > uPID->OutMax){
			uPID->OutputSum = uPID->OutMax;
		}
		else if (uPID->OutputSum < uPID->OutMin){
			uPID->OutputSum = uPID->OutMin;
		}
		else { }
	}
}

/* PID Tunings */
void PID_SetTunings(PID_TypeDef *uPID, double Kp, double Ki, double Kd){
	PID_SetTunings2(uPID, Kp, Ki, Kd);
}

void PID_SetTunings2(PID_TypeDef *uPID, double Kp, double Ki, double Kd){
	double SampleTimeInSec;

	/* Check value */
	if (Kp < 0 || Ki < 0 || Kd < 0){
		return;
	}

	uPID->DispKp = Kp;
	uPID->DispKi = Ki;
	uPID->DispKd = Kd;

	/* Calculate time */
	SampleTimeInSec = ((double)uPID->SampleTime) / 1000;

	uPID->Kp = Kp;
	uPID->Ki = Ki * SampleTimeInSec;
	uPID->Kd = Kd / SampleTimeInSec;

	/* Check direction */
	if (uPID->ControllerDirection == _PID_CD_REVERSE){

		uPID->Kp = (0 - uPID->Kp);
		uPID->Ki = (0 - uPID->Ki);
		uPID->Kd = (0 - uPID->Kd);
	}
}

/* PID Direction */
void PID_SetControllerDirection(PID_TypeDef *uPID, PIDCD_TypeDef Direction){
	/* Check parameters */
	if ((uPID->InAuto) && (Direction !=uPID->ControllerDirection)){
		uPID->Kp = (0 - uPID->Kp);
		uPID->Ki = (0 - uPID->Ki);
		uPID->Kd = (0 - uPID->Kd);
	}

	uPID->ControllerDirection = Direction;
}

PIDCD_TypeDef PID_GetDirection(PID_TypeDef *uPID){
	return uPID->ControllerDirection;
}

/* PID Sampling */
void PID_SetSampleTime(PID_TypeDef *uPID, int32_t NewSampleTime, int32_t updateOnCall){
	if(updateOnCall > 0){
		updateOnCall = 1;
	}
	uPID->updateOnEveryCall = updateOnCall;
	double ratio;

	/* Check value */
	if (NewSampleTime > 0){

		ratio = (double)NewSampleTime / (double)uPID->SampleTime;

		uPID->Ki *= ratio;
		uPID->Kd /= ratio;
		uPID->SampleTime = (uint32_t)NewSampleTime;
	}
}

/* Get Parameters */
double PID_GetKp(PID_TypeDef *uPID){
	return uPID->DispKp;
}
double PID_GetKi(PID_TypeDef *uPID){
	return uPID->DispKi;
}
double PID_GetKd(PID_TypeDef *uPID){
	return uPID->DispKd;
}

/* Get current contributions*/
double PID_GetPpart(PID_TypeDef *uPID){
	return uPID->DispKp_part;
}
double PID_GetIpart(PID_TypeDef *uPID){
	return uPID->DispKi_part;
}
double PID_GetDpart(PID_TypeDef *uPID){
	return uPID->DispKd_part;
}
