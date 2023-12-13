#include "kinematics.h"
#include "tim.h"

//80MHz clock, Prescaler=80-1, 1TmrCnt=1us, MaxAmountOfTime=4294.967295s or 71.58m
//Fastest Rpm that can be measured is 10e6 rpm
//Slowest Rpm that can be measured is 2.3e-3 rpm
//rpm = (1rev/NumMagnets)(1/ElapsedTime)(60s/1m)
//rpm = (1rev/NumMagnets)(1/(TmrCnt*1us))(60s/1m) = (6e7/(NumMagnets*TmrCnt))

#define NUM_MAGNETS 6

static float rpm = 0;
static uint32_t elapsedTmrCnt = 1e7;//Start at 1rpm

void HAL_TIM_TriggerCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim1){
		elapsedTmrCnt = __HAL_TIM_GET_COUNTER(&htim2);
		__HAL_TIM_SET_COUNTER(&htim2, 0);//Reset timer
	}
}

void initKinematics(){
	//Start encoder
	HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL);
	resetEncoderCnt();

	//Enable encoder interrupt on count change
	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	sSlaveConfig.SlaveMode = htim1.Instance->SMCR && TIM_SMCR_SMS;//Save the encoder mode
	sSlaveConfig.InputTrigger = TIM_TS_TI1F_ED;//Activate interrupt on encoder count change
	HAL_StatusTypeDef halStatus = HAL_TIM_SlaveConfigSynchro_IT(&htim1, &sSlaveConfig);
	if(halStatus != HAL_OK){
		Error_Handler();
	}

	HAL_TIM_Base_Start(&htim2);//Timer used to measure speed
}

void resetEncoderCnt(){
	TIM1->CNT = 0;
}

int16_t getEncoderCnt(){
	return __HAL_TIM_GET_COUNTER(&htim1);
}

float getRpm(){
	rpm = ((float)60000000/((float)NUM_MAGNETS*(float)elapsedTmrCnt));
	return rpm;
}
