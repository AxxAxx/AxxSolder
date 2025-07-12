#include "buzzer.h"

extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim17;

/* Beep the beeper */
void beep(float buzzer_enabled){
	if(buzzer_enabled == 1){
		HAL_TIM_Base_Start_IT(&htim17);
		HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_2);

	}
}

/* Beep the beeper twice*/
void beep_double(float buzzer_enabled){
	beep(buzzer_enabled);
	HAL_Delay(100);
	beep(buzzer_enabled);
}

/* Set the beeper tone*/
void set_tone(float frequency, float time_ms){
	htim4.Init.Prescaler = 17000000/(10*frequency);
	HAL_TIM_PWM_Init(&htim4);

	htim17.Init.Period = time_ms*10;
	HAL_TIM_Base_Init(&htim17);
	HAL_TIM_OnePulse_Init(&htim17, TIM_OPMODE_SINGLE);
}
