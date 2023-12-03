#include "user_interface.h"
#include "tim.h"
#include <string.h>
#include <stdlib.h>
#include "eeprom_emul.h"

#define SHORT_PRESS_MIN 100//Min amount of time(ms) button has to be down for short button press to be registered
#define SHORT_PRESS_MAX 1000//If button is down for longer than this then it will be considered a long press

typedef enum{
	ButtonDown,
	ButtonUp
}ButtonState;

static uint32_t lastButtonPressTime = 0;
static uint32_t buttonHeldDownDuration = 0;
static ButtonState prevButtonState = ButtonUp;
static MenuItem menuItem = 0;

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

void initMenu(MenuInfo *menuInfo, bool saveMenuState){
	//Set the mapping for brightness values
	const float brightnessToValue[10] = {0.01f, 0.025f, 0.05f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.8f, 1.0f};//Converts int menu value to hsv value
	memcpy(menuInfo->brightnessToValue, brightnessToValue, sizeof(brightnessToValue));

	//Set the menu colors
	Hsv itemColors[NumMenuItems] = {{0.0f, 1.0f, 0.05f}, {90.0f, 1.0f, 0.05f}, {180.0f, 1.0f, 0.05f}};
	memcpy(menuInfo->itemColors, itemColors, sizeof(itemColors));

	//Restore the previous menu values stored in flash
	EE_Status ee_status = EE_OK;
	HAL_FLASH_Unlock();
	ee_status = EE_Init(EE_FORCED_ERASE);
	if(ee_status != EE_OK) {Error_Handler();}
	for(int eeAddress=1, menuIndex=0; menuIndex<NumMenuItems; eeAddress++, menuIndex++){
		ee_status = EE_ReadVariable8bits(eeAddress, (uint8_t*)&menuInfo->itemValues[menuIndex]);
	}
	HAL_FLASH_Lock();

	menuInfo->state = NotInMenu;//Start not in menu
	menuInfo->saveMenuState = saveMenuState;
}

//Displays a menu to the user that the use can interact with
void processMenu(MenuInfo *menuInfo, PixelsInfo *pixelInfo, ButtonPressType buttonPress){
	//If not in menu and short button press then show the start of the menu
	if(menuInfo->state == NotInMenu && buttonPress == ButtonShortPress){
		menuItem = 0;
		menuInfo->prevEncoderCnt = TIM1->CNT;
		menuInfo->state = InMenu;
		menuInfo->itemColors[menuItem].v = menuInfo->brightnessToValue[menuInfo->itemValues[Brightness]];
		displayMenuValue(pixelInfo, menuInfo->itemValues[menuItem], menuInfo->itemColors[menuItem]);
	}
	//Or if already in a menu and short button press then show next menu item
	else if(menuInfo->state == InMenu && buttonPress == ButtonShortPress){
		menuItem++;
		if(menuItem >= NumMenuItems){
			menuItem = 0;
		}
		menuInfo->itemColors[menuItem].v = menuInfo->brightnessToValue[menuInfo->itemValues[Brightness]];
		displayMenuValue(pixelInfo, menuInfo->itemValues[menuItem], menuInfo->itemColors[menuItem]);
	}

	//On long button press save settings to flash and indicate menu exit
	if(buttonPress == ButtonLongPress){
		//Save settings to flash if desired
		if(menuInfo->saveMenuState){
			EE_Status ee_status = EE_OK;
			HAL_FLASH_Unlock();
			for(int eeAddress=1, menuIndex=0; menuIndex<NumMenuItems; eeAddress++, menuIndex++){
				//Try to write data to flash. If it fails then try to clean up. If it fails too many times then error out.
				for(int i=0; i<10; i++){
					ee_status = EE_WriteVariable8bits(eeAddress, menuInfo->itemValues[menuIndex]);
					if(ee_status == EE_OK){
						break;
					}
					else if(ee_status == EE_CLEANUP_REQUIRED){
						ee_status = EE_CleanUp();
					}
					else{
						Error_Handler();
					}
				}
				if(ee_status != EE_OK){Error_Handler();}
			}
			HAL_FLASH_Lock();
		}
		//Indicate menu exit
		menuInfo->state = NotInMenu;
	}

	//Exit if NotInMenu
	if(menuInfo->state == NotInMenu){
		return;
	}

	//Increase or decrease menu items based on encoder value
	int16_t currentCnt = TIM1->CNT;
	int16_t diffCnt = currentCnt - menuInfo->prevEncoderCnt;
	menuInfo->prevEncoderCnt = currentCnt;
	if(diffCnt != 0){
		menuInfo->itemValues[menuItem] += diffCnt;
		if(menuInfo->itemValues[menuItem] < 0){
			menuInfo->itemValues[menuItem] = 0;
		}
		else if(menuInfo->itemValues[menuItem] > 9){
			menuInfo->itemValues[menuItem] = 9;
		}
		menuInfo->itemColors[menuItem].v = menuInfo->brightnessToValue[menuInfo->itemValues[Brightness]];
		displayMenuValue(pixelInfo, menuInfo->itemValues[menuItem], menuInfo->itemColors[menuItem]);
	}
}
