#include "user_interface.h"
#include "tim.h"
#include <stdlib.h>

#define SHORT_PRESS_MIN 100//Min amount of time(ms) button has to be down for short button press to be registered
#define SHORT_PRESS_MAX 1000//If button is down for longer than this then it will be considered a long press

typedef enum{
	ButtonDown,
	ButtonUp
}ButtonState;

int8_t menuItemValues[NumMenuItems] = {0};
static Hsv menuItemColors[NumMenuItems] = {{0.0f, 1.0f, 0.05f}, {90.0f, 1.0f, 0.05f}, {180.0f, 1.0f, 0.05f}};
static int16_t currentCnt = 0;
static int16_t prevCnt = 0;
static int16_t diffCnt = 0;

static uint32_t lastButtonPressTime = 0;
static uint32_t buttonHeldDownDuration = 0;
static ButtonState prevButtonState = ButtonUp;
static MenuItem menuItem = 0;

const float menuBrightnessToValue[10] = {0.01f, 0.025f, 0.05f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.8f, 1.0f};//Converts int menu value to hsv value

//Used to display the value of variable to the user
void displayLedCntAndColor(PixelsInfo *pixelInfo, uint8_t ledCnt, Hsv color){
	Rgb *rgb = calloc(pixelInfo->numPixels, sizeof(Rgb));
	Rgb colorRgb = hsvToRgb(color);
	for(int i=0; i<ledCnt; i++){
		rgb[i] = colorRgb;
	}
	setPixelsRgb(pixelInfo, rgb);
	free(rgb);
}

void displayMenuValue(PixelsInfo *pixelInfo, uint8_t menuValue, Hsv color){
	uint8_t menuDisplayValue = menuValue + 1;//A value of zero will have one led turned on
	assert(menuDisplayValue <= pixelInfo->numPixels);//Make sure the menu value doesn't exceed the number of display leds
	displayLedCntAndColor(pixelInfo, menuDisplayValue, color);
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
		menuItemColors[menuItem].v = menuBrightnessToValue[menuItemValues[Brightness]];
		displayMenuValue(pixelInfo, menuItemValues[menuItem], menuItemColors[menuItem]);
	}
	//Or if already in a menu and short button press then show next menu item
	else if(*menuState == InMenu && buttonPress == ButtonShortPress){
		menuItem++;
		if(menuItem >= NumMenuItems){
			menuItem = 0;
		}
		menuItemColors[menuItem].v = menuBrightnessToValue[menuItemValues[Brightness]];
		displayMenuValue(pixelInfo, menuItemValues[menuItem], menuItemColors[menuItem]);
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
		if(menuItemValues[menuItem] < 0){
			menuItemValues[menuItem] = 0;
		}
		else if(menuItemValues[menuItem] > 9){
			menuItemValues[menuItem] = 9;
		}
		menuItemColors[menuItem].v = menuBrightnessToValue[menuItemValues[Brightness]];
		displayMenuValue(pixelInfo, menuItemValues[menuItem], menuItemColors[menuItem]);
	}
}
