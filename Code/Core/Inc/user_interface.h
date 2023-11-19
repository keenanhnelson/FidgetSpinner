#ifndef USER_INTERFACE_H_
#define USER_INTERFACE_H_

#include "pixels.h"

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

extern uint8_t menuItemValues[NumMenuItems];

ButtonPressType processButtonInput();
void processMenu(PixelsInfo *pixelInfo, ButtonPressType buttonPress, MenuState *menuState);

#endif /* USER_INTERFACE_H_ */
