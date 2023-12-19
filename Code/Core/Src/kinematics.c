#include "kinematics.h"
#include "tim.h"

//80MHz clock, Prescaler=80-1, 1TmrCnt=1us, MaxAmountOfTime=4294.967295s or 71.58m
//Fastest Rpm that can be measured is 10e6 rpm
//Slowest Rpm that can be measured is 2.3e-3 rpm
//rpm = (1rev/NumMagnets)(1/ElapsedTime)(60s/1m)
//rpm = (1rev/NumMagnets)(1/(TmrCnt*1us))(60s/1m) = (6e7/(NumMagnets*TmrCnt))

#define NUM_MAGNETS 6

static volatile uint32_t elapsedTmrCnt = 1e7;//Start at 1rpm
static volatile int16_t currentEncoderCnt = 0;
static volatile int16_t prevEncoderCnt = 0;
static volatile int8_t currentMagnetIndex = 0;
static volatile float rotationDir = 1;

//Called on every new encoder value change
void HAL_TIM_TriggerCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim1){
		//Keep track of magnet position index
		currentEncoderCnt = __HAL_TIM_GET_COUNTER(&htim1);
		if(currentEncoderCnt > prevEncoderCnt){
			rotationDir = 1;
			currentMagnetIndex++;
			if(currentMagnetIndex >= NUM_MAGNETS){
				currentMagnetIndex = 0;
			}
		}
		else{
			rotationDir = -1;
			currentMagnetIndex--;
			if(currentMagnetIndex < 0){
				currentMagnetIndex = NUM_MAGNETS - 1;
			}
		}
		prevEncoderCnt = currentEncoderCnt;

		elapsedTmrCnt = __HAL_TIM_GET_COUNTER(&htim2);//Get elapsed time
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

float getPosition(){
	uint32_t tmrCntBetweenMagnetsUs = __HAL_TIM_GET_COUNTER(&htim2);//Time elapsed since last magnet
	float revPerUs = (1.0f/(NUM_MAGNETS*elapsedTmrCnt));//Speed calculated at last magnet
	float revElapsed = revPerUs * tmrCntBetweenMagnetsUs;//Distance since last magnet
	if(revElapsed > 1/(float)NUM_MAGNETS){
		revElapsed = 1/(float)NUM_MAGNETS;
	}
	float currentRevPosition = (rotationDir * revElapsed) + (currentMagnetIndex/(float)NUM_MAGNETS);//Distance from first magnet
	return currentRevPosition;
}

float getRpm(){
	return (float)60000000/((float)NUM_MAGNETS*(float)elapsedTmrCnt);
}
