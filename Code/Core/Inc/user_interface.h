#ifndef USER_INTERFACE_H_
#define USER_INTERFACE_H_

#include "pixels.h"
#include <stdbool.h>

typedef enum{
	ButtonNoPress,
	ButtonShortPress,
	ButtonLongPress
}ButtonPressType;

typedef enum{
	NotInMenu,
	InMenu
}MenuState;

typedef enum{
	PatternMoving,
	Brightness,
	PatternStationary,
	NumMenuItems
}MenuItem;

typedef struct{
	int8_t itemValues[NumMenuItems];
	float brightnessToValue[10];
	MenuState state;
	Hsv itemColors[NumMenuItems];
	int16_t prevEncoderCnt;
	bool saveMenuState;
}MenuInfo;

ButtonPressType processButtonInput();
void initMenu(MenuInfo *menuInfo, bool saveMenuState);
void processMenu(MenuInfo *menuInfo, PixelsInfo *pixelInfo, ButtonPressType buttonPress);

#endif /* USER_INTERFACE_H_ */
