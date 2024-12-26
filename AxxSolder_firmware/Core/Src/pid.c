#include "pid.h"

/* Initialize */
void PID_Init(PID_TypeDef *uPID)
{
	/* Set parameters */
	uPID->OutputSum = *uPID->MyOutput;
	uPID->LastInput = *uPID->MyInput;

	uPID->OutputSum = float_clamp(uPID->OutputSum, uPID->OutMin, uPID->OutMax);
}

void PID(PID_TypeDef *uPID, float *Input, float *Output, float *Setpoint, float Kp, float Ki, float Kd, PIDCD_TypeDef ControllerDirection)
{
	/* Set parameters */
	uPID->MyOutput   = Output;
	uPID->MyInput    = Input;
	uPID->MySetpoint = Setpoint;
	uPID->InAuto     = (PIDMode_TypeDef)0;

	PID_SetOutputLimits(uPID, 0, DEFAULT_PWM_MAX);
	uPID->SampleTime = DEFAULT_SAMPLE_TIME_MS;

	PID_SetControllerDirection(uPID, ControllerDirection);
	PID_SetTunings(uPID, Kp, Ki, Kd);

	uPID->LastTime = HAL_GetTick() - uPID->SampleTime;
}

/* Function to clamp d between the limits min and max */
float float_clamp(float d, float min, float max) {
	  const float t = d < min ? min : d;
	  return t > max ? max : t;
}

/* Function to check if clamping will occur */
uint8_t check_clamping(float d, float min, float max) {
	  if(d > max || d < min){
		  return 1;
	  }
	  else{
		  return 0;
	  }
}

/* Compute  */
uint8_t PID_Compute(PID_TypeDef *uPID){
	uint32_t now;
	uint32_t timeChange;
	float timeChange_in_seconds;

	float input;
	float error;
	float dInput;
	float output;

	/* Check PID mode */
	if (!uPID->InAuto){
		return 0;
	}

	/* Calculate time */
	now        = HAL_GetTick();
	timeChange = (now - uPID->LastTime);

	if ((timeChange >= uPID->SampleTime) || (uPID->updateOnEveryCall))
	{
		timeChange_in_seconds = timeChange/1000.0;
		/* Compute all the working error variables */
		input   = *uPID->MyInput;
		error   = *uPID->MySetpoint - input;
		dInput  = (input - uPID->LastInput);

		/* Calculate Proportional on Error */
		uPID->DispKp_part = uPID->Kp * error;
		output = uPID->DispKp_part;

		/* Calculate Derivative term and add to output sum */
		uPID->DispKd_part = - (uPID->Kd / timeChange_in_seconds) * dInput;
		output += uPID->DispKd_part;

		/* Conditional integration as anti-windup (clamping) */
		if(check_clamping(output + uPID->NegativeErrorIgainMultiplier*uPID->Ki * error  * timeChange_in_seconds, uPID->OutMin, uPID->OutMax) && (error*(output + uPID->OutputSum) > 0)){
			uPID->OutputSum     += 0;
		}
		else if(error > -1){// -1 to be able to hold a zero error without un-balanced control inpout
			uPID->OutputSum     += (uPID->Ki * error * timeChange_in_seconds);
		}
		else{
			uPID->OutputSum     += uPID->NegativeErrorIgainMultiplier*(uPID->Ki * error * timeChange_in_seconds);
		}

		/* Clamp Integral part */
		uPID->OutputSum = float_clamp(uPID->OutputSum, uPID->IMin, uPID->IMax);

		/* If Setpoint is set to 0, zero integral part */
		if(*uPID->MySetpoint == 0){
			uPID->OutputSum = 0;
		}

		/* only add I part if error is smaller than IminError and scale it from IminError to 0 */
		if(error > fabs(uPID->IminError)){
			uPID->OutputSum = 0;
		}

		uPID->DispKi_part = uPID->OutputSum;
		/* Final summation */
		output += uPID->DispKi_part;

		/* Clamp output */
		output = float_clamp(output, uPID->OutMin, uPID->OutMax);

		*uPID->MyOutput = output;

		/* Remember some variables for next time */
		uPID->LastInput = input;
		uPID->LastTime = now;

		return 1;
	}
	else{
		return 0;
	}
}

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
void PID_SetOutputLimits(PID_TypeDef *uPID, float Min, float Max){
	/* Check value */
	if (Min >= Max){
		return;
	}

	uPID->OutMin = Min;
	uPID->OutMax = Max;

	if (uPID->InAuto){
		/* Check value */
		*uPID->MyOutput = float_clamp(*uPID->MyOutput, uPID->OutMin, uPID->OutMax);

		/* Check out value */
		uPID->OutputSum = float_clamp(uPID->OutputSum, uPID->OutMin, uPID->OutMax);
	}
}

/* PID I-windup Limits */
void PID_SetILimits(PID_TypeDef *uPID, float Min, float Max){
	/* Check value */
	if (Min >= Max){
		return;
	}

	uPID->IMin = Min;
	uPID->IMax = Max;
}

/* Minimum error where I is added */
void PID_SetIminError(PID_TypeDef *uPID, float IminError){	/* Check value */
	if (IminError < 0){
		return;
	}

	uPID->IminError = IminError;
}

/* Set the I gain multiplier for negative error*/
void PID_SetNegativeErrorIgainMult(PID_TypeDef *uPID, float NegativeErrorIgainMultiplier, float NegativeErrorIgainBias){
	if (NegativeErrorIgainMultiplier < 0){
		return;
	}

	uPID->NegativeErrorIgainMultiplier = NegativeErrorIgainMultiplier;
	uPID->NegativeErrorIgainBias = NegativeErrorIgainBias;
}

/* PID Tunings */
void PID_SetTunings(PID_TypeDef *uPID, float Kp, float Ki, float Kd){
	/* Check value */
	if (Kp < 0 || Ki < 0 || Kd < 0){
		return;
	}

	uPID->DispKp = Kp;
	uPID->DispKi = Ki;
	uPID->DispKd = Kd;

	uPID->Kp = Kp;
	uPID->Ki = Ki;
	uPID->Kd = Kd;

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
	float ratio;

	/* Check value */
	if (NewSampleTime > 0){
		ratio = (float)NewSampleTime / (float)uPID->SampleTime;

		uPID->Ki *= ratio;
		uPID->Kd /= ratio;
		uPID->SampleTime = (uint32_t)NewSampleTime;
	}
}

/* Get Parameters */
float PID_GetKp(PID_TypeDef *uPID){
	return uPID->DispKp;
}
float PID_GetKi(PID_TypeDef *uPID){
	return uPID->DispKi;
}
float PID_GetKd(PID_TypeDef *uPID){
	return uPID->DispKd;
}

/* Get current contributions*/
float PID_GetPpart(PID_TypeDef *uPID){
	return uPID->DispKp_part;
}
float PID_GetIpart(PID_TypeDef *uPID){
	return uPID->DispKi_part;
}
float PID_GetDpart(PID_TypeDef *uPID){
	return uPID->DispKd_part;
}
