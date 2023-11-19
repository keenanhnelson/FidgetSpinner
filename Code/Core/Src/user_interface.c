#include "user_interface.h"
#include "tim.h"
#include <stdlib.h>

#define SHORT_PRESS_MIN 100//Min amount of time(ms) button has to be down for short button press to be registered
#define SHORT_PRESS_MAX 1000//If button is down for longer than this then it will be considered a long press

typedef enum{
	ButtonDown,
	ButtonUp
}ButtonState;

uint8_t menuItemValues[NumMenuItems] = {1, 1, 1};
static uint32_t menuItemColors[NumMenuItems] = {0x030000, 0x000300, 0x000003};
static int16_t currentCnt = 0;
static int16_t prevCnt = 0;
static int16_t diffCnt = 0;

static uint32_t lastButtonPressTime = 0;
static uint32_t buttonHeldDownDuration = 0;
static ButtonState prevButtonState = ButtonUp;
static MenuItem menuItem = 0;

//Used to display the value of variable to the user
void displayLedCntAndColor(PixelsInfo *pixelInfo, uint8_t ledCnt, uint32_t rgbColor){
	uint32_t *rgb = calloc(pixelInfo->numPixels, sizeof(uint32_t));
	for(int i=0; i<ledCnt; i++){
		rgb[i] = rgbColor;
	}
	setPixelsColors(pixelInfo, rgb);
	free(rgb);
}

//Processes the button press for either short or long presses
ButtonPressType processButtonInput(){
	ButtonPressType buttonPress;
	ButtonState buttonState = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
	uint32_t currentTime = HAL_GetTick();

	//Save the time the button was initially pressed
	if(prevButtonState == ButtonUp && buttonState == ButtonDown){
		lastButtonPressTime = currentTime;
	}
	//Update the amount of time the button was pressed down for
	if(buttonState == ButtonDown){
		buttonHeldDownDuration = currentTime - lastButtonPressTime;
	}
	//Find if short button press occurred
	if(buttonHeldDownDuration > SHORT_PRESS_MIN && buttonHeldDownDuration < SHORT_PRESS_MAX && buttonState == ButtonUp){
		buttonPress = ButtonShortPress;
		buttonHeldDownDuration = 0;
	}
	//Find if long button press occurred
	else if(buttonHeldDownDuration > SHORT_PRESS_MAX && buttonState == ButtonDown){
		buttonPress = ButtonLongPress;
		buttonHeldDownDuration = 0;
	}
	else{
		buttonPress = ButtonNoPress;
	}
	prevButtonState = buttonState;

	return buttonPress;
}

//Displays a menu to the user that the use can interact with
void processMenu(PixelsInfo *pixelInfo, ButtonPressType buttonPress, MenuState *menuState){
	//If not in menu and short button press then show the start of the menu
	if(*menuState == NotInMenu && buttonPress == ButtonShortPress){
		menuItem = 0;
		prevCnt = TIM1->CNT;
		*menuState = InMenu;
		displayLedCntAndColor(pixelInfo, menuItemValues[menuItem], menuItemColors[menuItem]);
	}
	//Or if already in a menu and short button press then show next menu item
	else if(*menuState == InMenu && buttonPress == ButtonShortPress){
		menuItem++;
		if(menuItem >= NumMenuItems){
			menuItem = 0;
		}
		displayLedCntAndColor(pixelInfo, menuItemValues[menuItem], menuItemColors[menuItem]);
	}

	//Change state to NotInMenu on long button press
	if(buttonPress == ButtonLongPress){
		*menuState = NotInMenu;
	}

	//Exit if NotInMenu
	if(*menuState == NotInMenu){
		return;
	}

	//Increase or decrease menu items based on encoder value
	currentCnt = TIM1->CNT;
	diffCnt = currentCnt - prevCnt;
	prevCnt = currentCnt;
	if(diffCnt != 0){
		menuItemValues[menuItem] += diffCnt;
		if(menuItemValues[menuItem] < 1){
			menuItemValues[menuItem] = 1;
		}
		else if(menuItemValues[menuItem] > 10){
			menuItemValues[menuItem] = 10;
		}
		displayLedCntAndColor(pixelInfo, menuItemValues[menuItem], menuItemColors[menuItem]);
	}
}
