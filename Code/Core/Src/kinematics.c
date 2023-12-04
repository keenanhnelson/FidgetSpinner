#include "kinematics.h"
#include "tim.h"

//80MHz clock, Prescaler=8000-1, Counter Period=1000, (1/80e6)*8000*1000 = 100ms for every interrupt
//If interrupt is every 0.1s and there are 6 magnets then the slowest rpm detectable is 100rpm=((1/6)/0.1)*60
#define REV_TIME_S 0.1f//The amount of time that passes while counting encoder ticks
#define NUM_MAGNETS 6

static int16_t prevCnt = 0;
static int16_t currentCnt = 0;
static int16_t diffCnt = 0;
static float rpm = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim6){
		currentCnt = getEncoderCnt();
		diffCnt = currentCnt - prevCnt;
		prevCnt = currentCnt;
	}
}

void initKinematics(){
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1);
	resetEncoderCnt();

	__HAL_TIM_CLEAR_FLAG(&htim6, TIM_SR_UIF);//Required so interrupt doesn't fire immediately on start
	HAL_TIM_Base_Start_IT(&htim6);//Timer used to measure speed
}

void resetEncoderCnt(){
	TIM1->CNT = 0;
}

int16_t getEncoderCnt(){
	return __HAL_TIM_GET_COUNTER(&htim1);
}

float getRpm(){
	rpm = ((float)diffCnt*60.0f)/((float)NUM_MAGNETS*REV_TIME_S);
	return rpm;
}
