 /*
------------------------------------------------------------------------------
~ File   : pid.c
~ Author : Majid Derhambakhsh
~ Version: V1.0.0
~ Created: 02/11/2021 03:43:00 AM
~ Brief  :
~ Support:
		   E-Mail : Majid.do16@gmail.com (subject : Embedded Library Support)

		   Github : https://github.com/Majid-Derhambakhsh
------------------------------------------------------------------------------
~ Description:

~ Attention  :

~ Changes    :
------------------------------------------------------------------------------
*/

#include "pid.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~ Initialize ~~~~~~~~~~~~~~~~ */
void PID_Init(PID_TypeDef *uPID)
{
	/* ~~~~~~~~~~ Set parameter ~~~~~~~~~~ */
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

void PID(PID_TypeDef *uPID, double *Input, double *Output, double *Setpoint, double Kp, double Ki, double Kd, PIDPON_TypeDef POn, PIDCD_TypeDef ControllerDirection)
{
	/* ~~~~~~~~~~ Set parameter ~~~~~~~~~~ */
	uPID->MyOutput   = Output;
	uPID->MyInput    = Input;
	uPID->MySetpoint = Setpoint;
	uPID->InAuto     = (PIDMode_TypeDef)_FALSE;

	PID_SetOutputLimits(uPID, 0, _PID_8BIT_PWM_MAX);

	uPID->SampleTime = _PID_SAMPLE_TIME_MS_DEF; /* default Controller Sample Time is 0.1 seconds */

	PID_SetControllerDirection(uPID, ControllerDirection);
	PID_SetTunings2(uPID, Kp, Ki, Kd, POn);

	uPID->LastTime = GetTime() - uPID->SampleTime;

}

void PID2(PID_TypeDef *uPID, double *Input, double *Output, double *Setpoint, double Kp, double Ki, double Kd, PIDCD_TypeDef ControllerDirection)
{
	PID(uPID, Input, Output, Setpoint, Kp, Ki, Kd, _PID_P_ON_E, ControllerDirection);
}

/* ~~~~~~~~~~~~~~~~~ Computing ~~~~~~~~~~~~~~~~~ */
uint8_t PID_Compute(PID_TypeDef *uPID)
{

	uint32_t now;
	uint32_t timeChange;

	double input;
	double error;
	double dInput;
	double output;

	/* ~~~~~~~~~~ Check PID mode ~~~~~~~~~~ */
	if (!uPID->InAuto)
	{
		return _FALSE;
	}

	/* ~~~~~~~~~~ Calculate time ~~~~~~~~~~ */
	now        = GetTime();
	timeChange = (now - uPID->LastTime);

	if (timeChange >= uPID->SampleTime)
	{
		/* ..... Compute all the working error variables ..... */
		input   = *uPID->MyInput;
		error   = *uPID->MySetpoint - input;
		dInput  = (input - uPID->LastInput);

		uPID->OutputSum     += (uPID->Ki * error);

		/* ..... Add Proportional on Measurement, if P_ON_M is specified ..... */
		if (!uPID->POnE)
		{
			uPID->OutputSum -= uPID->Kp * dInput;
		}

		if (uPID->OutputSum > uPID->OutMax)
		{
			uPID->OutputSum = uPID->OutMax;
		}
		else if (uPID->OutputSum < uPID->OutMin)
		{
			uPID->OutputSum = uPID->OutMin;
		}
		else { }

		/* ..... Add Proportional on Error, if P_ON_E is specified ..... */
		if (uPID->POnE)
		{
			output = uPID->Kp * error;
		}
		else
		{
			output = 0;
		}

		/* ..... Compute Rest of PID Output ..... */
		output += uPID->OutputSum - uPID->Kd * dInput;

		if (output > uPID->OutMax)
		{
			output = uPID->OutMax;
		}
		else if (output < uPID->OutMin)
		{
			output = uPID->OutMin;
		}
		else { }

		*uPID->MyOutput = output;

		/* ..... Remember some variables for next time ..... */
		uPID->LastInput = input;
		uPID->LastTime = now;

		return _TRUE;

	}
	else
	{
		return _FALSE;
	}

}

/* ~~~~~~~~~~~~~~~~~ PID Mode ~~~~~~~~~~~~~~~~~~ */
void            PID_SetMode(PID_TypeDef *uPID, PIDMode_TypeDef Mode)
{

	uint8_t newAuto = (Mode == _PID_MODE_AUTOMATIC);

	/* ~~~~~~~~~~ Initialize the PID ~~~~~~~~~~ */
	if (newAuto && !uPID->InAuto)
	{
		PID_Init(uPID);
	}

	uPID->InAuto = (PIDMode_TypeDef)newAuto;

}
PIDMode_TypeDef PID_GetMode(PID_TypeDef *uPID)
{
	return uPID->InAuto ? _PID_MODE_AUTOMATIC : _PID_MODE_MANUAL;
}

/* ~~~~~~~~~~~~~~~~ PID Limits ~~~~~~~~~~~~~~~~~ */
void PID_SetOutputLimits(PID_TypeDef *uPID, double Min, double Max)
{
	/* ~~~~~~~~~~ Check value ~~~~~~~~~~ */
	if (Min >= Max)
	{
		return;
	}

	uPID->OutMin = Min;
	uPID->OutMax = Max;

	/* ~~~~~~~~~~ Check PID Mode ~~~~~~~~~~ */
	if (uPID->InAuto)
	{

		/* ..... Check out value ..... */
		if (*uPID->MyOutput > uPID->OutMax)
		{
			*uPID->MyOutput = uPID->OutMax;
		}
		else if (*uPID->MyOutput < uPID->OutMin)
		{
			*uPID->MyOutput = uPID->OutMin;
		}
		else { }

		/* ..... Check out value ..... */
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

}

/* ~~~~~~~~~~~~~~~~ PID Tunings ~~~~~~~~~~~~~~~~ */
void PID_SetTunings(PID_TypeDef *uPID, double Kp, double Ki, double Kd)
{
	PID_SetTunings2(uPID, Kp, Ki, Kd, uPID->POn);
}
void PID_SetTunings2(PID_TypeDef *uPID, double Kp, double Ki, double Kd, PIDPON_TypeDef POn)
{

	double SampleTimeInSec;

	/* ~~~~~~~~~~ Check value ~~~~~~~~~~ */
	if (Kp < 0 || Ki < 0 || Kd < 0)
	{
		return;
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	uPID->POn    = POn;
	uPID->POnE   = (PIDPON_TypeDef)(POn == _PID_P_ON_E);

	uPID->DispKp = Kp;
	uPID->DispKi = Ki;
	uPID->DispKd = Kd;

	/* ~~~~~~~~~ Calculate time ~~~~~~~~ */
	SampleTimeInSec = ((double)uPID->SampleTime) / 1000;

	uPID->Kp = Kp;
	uPID->Ki = Ki * SampleTimeInSec;
	uPID->Kd = Kd / SampleTimeInSec;

	/* ~~~~~~~~ Check direction ~~~~~~~~ */
	if (uPID->ControllerDirection == _PID_CD_REVERSE)
	{

		uPID->Kp = (0 - uPID->Kp);
		uPID->Ki = (0 - uPID->Ki);
		uPID->Kd = (0 - uPID->Kd);

	}

}

/* ~~~~~~~~~~~~~~~ PID Direction ~~~~~~~~~~~~~~~ */
void          PID_SetControllerDirection(PID_TypeDef *uPID, PIDCD_TypeDef Direction)
{
	/* ~~~~~~~~~~ Check parameters ~~~~~~~~~~ */
	if ((uPID->InAuto) && (Direction !=uPID->ControllerDirection))
	{

		uPID->Kp = (0 - uPID->Kp);
		uPID->Ki = (0 - uPID->Ki);
		uPID->Kd = (0 - uPID->Kd);

	}

	uPID->ControllerDirection = Direction;

}
PIDCD_TypeDef PID_GetDirection(PID_TypeDef *uPID)
{
	return uPID->ControllerDirection;
}

/* ~~~~~~~~~~~~~~~ PID Sampling ~~~~~~~~~~~~~~~~ */
void PID_SetSampleTime(PID_TypeDef *uPID, int32_t NewSampleTime)
{

	double ratio;

	/* ~~~~~~~~~~ Check value ~~~~~~~~~~ */
	if (NewSampleTime > 0)
	{

		ratio = (double)NewSampleTime / (double)uPID->SampleTime;

		uPID->Ki *= ratio;
		uPID->Kd /= ratio;
		uPID->SampleTime = (uint32_t)NewSampleTime;

	}

}

/* ~~~~~~~~~~~~~ Get Tunings Param ~~~~~~~~~~~~~ */
double PID_GetKp(PID_TypeDef *uPID)
{
	return uPID->DispKp;
}
double PID_GetKi(PID_TypeDef *uPID)
{
	return uPID->DispKi;
}
double PID_GetKd(PID_TypeDef *uPID)
{
	return uPID->DispKd;
}
