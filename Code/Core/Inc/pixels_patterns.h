#ifndef PIXELS_PATTERNS_H_
#define PIXELS_PATTERNS_H_

#include "pixels.h"
#include "user_interface.h"

typedef enum{
	PIXEL_PATTERN1,
	PIXEL_PATTERN2,
	PIXEL_PATTERN3,
	PIXEL_PATTERN4,
	PIXEL_PATTERN5
}MovingPixelPatternType;

typedef enum{
	STATIONARY_PATTERN1,
	STATIONARY_PATTERN2,
	STATIONARY_PATTERN3
}StationaryPixelPatternType;

void displayPixelPattern(PixelsInfo *pixelInfo, Rgb *pixelsRgb, MenuInfo *menuInfo);
void displayMovingPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, MovingPixelPatternType pixelPattern);
void displayStationaryPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, StationaryPixelPatternType pixelPattern);

#endif /* PIXELS_PATTERNS_H_ */
