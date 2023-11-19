#include "user_interface.h"
#include "tim.h"
#include <stdlib.h>

#define BUTTON_DEBOUNCE_TIME_MS 1000

static volatile int8_t buttonPressed = 0;
static volatile int8_t buttonHeldDown = 0;
static volatile uint32_t prevButtonTimestamp = 0;
static volatile uint32_t debugbutton = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == BUTTON_Pin){
		GPIO_PinState buttonState = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
		uint32_t timestamp = HAL_GetTick();
		if((buttonState == GPIO_PIN_RESET)){
			__HAL_TIM_CLEAR_FLAG(&htim6, TIM_SR_UIF);//Required so interrupt doesn't fire immediately on start
			HAL_TIM_Base_Start_IT(&htim6);//Interrupt to determine how long the button is held down for
			if(timestamp > (prevButtonTimestamp + BUTTON_DEBOUNCE_TIME_MS)){
				debugbutton++;
				buttonPressed = 1;
				prevButtonTimestamp = timestamp;
			}


		}
		else if(buttonState == GPIO_PIN_SET){
			HAL_TIM_Base_Stop_IT(&htim6);//Stop held down timer if button is not pressed
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim6){
		buttonHeldDown = 1;
		HAL_TIM_Base_Stop_IT(&htim6);
	}
}

void initUserInput(){

}

void displayLedCntAndColor(PixelsInfo *pixelInfo, uint8_t ledCnt, uint32_t rgbColor){
	uint32_t *rgb = calloc(pixelInfo->numPixels, sizeof(uint32_t));

	for(int i=0; i<ledCnt; i++){
		rgb[i] = rgbColor;
	}

	setPixelsColors(pixelInfo, rgb);

	free(rgb);
}

typedef enum{
	Brightness,
	PatternMoving,
	PatternStationary,
	NumMenuItems
}MenuItem;

uint8_t menuItemValues[NumMenuItems] = {1};
uint32_t menuItemColors[NumMenuItems] = {0x030000, 0x000300, 0x000003};
int16_t currentCnt = 0;
int16_t prevCnt = 0;
int16_t diffCnt = 0;

void processUserInput(PixelsInfo *pixelInfo){
	if(buttonPressed == 0){
		return;
	}

	buttonPressed = 0;
	MenuItem menuItem = 0;
	uint8_t menuDisplayStart = 1;
	TIM1->CNT = 0;
	while(buttonHeldDown != 1){
		currentCnt = TIM1->CNT;
		diffCnt = currentCnt - prevCnt;
		prevCnt = currentCnt;

		if(menuDisplayStart || (diffCnt != 0)){
			menuItemValues[menuItem] += diffCnt;
			if(menuItemValues[menuItem] < 1){
				menuItemValues[menuItem] = 1;
			}
			else if(menuItemValues[menuItem] > 10){
				menuItemValues[menuItem] = 10;
			}
			displayLedCntAndColor(pixelInfo, menuItemValues[menuItem], menuItemColors[menuItem]);
			menuDisplayStart = 0;
		}



		if(buttonPressed){
			menuItem++;
			if(menuItem >= NumMenuItems){
				menuItem = 0;
			}
			buttonPressed = 0;
			menuDisplayStart = 1;
		}
	}

	buttonPressed = 0;
	buttonHeldDown = 0;
}




//			switch(menuItem){
//			case(Brightness):{
//				menuItemValues[Brightness] += diffCnt;
//				if(menuItemValues[Brightness] < 1){
//					menuItemValues[Brightness] = 1;
//				}
//				else if(menuItemValues[Brightness] > 10){
//					menuItemValues[Brightness] = 10;
//				}
//				displayLedCntAndColor(pixelInfo, menuItemValues[Brightness], 0x030000);
//				buttonPressed = 0;
//				break;
//			}
//			case(PatternMoving):{
//				break;
//			}
//			case(PatternStationary):{
//				break;
//			}
//			}
