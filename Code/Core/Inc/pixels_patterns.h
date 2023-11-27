#ifndef PIXELS_PATTERNS_H_
#define PIXELS_PATTERNS_H_

#include "pixels.h"

typedef enum{
	PIXEL_PATTERN1,
	PIXEL_PATTERN2,
	PIXEL_PATTERN3
}MovingPixelPatternType;

typedef enum{
	STATIONARY_PATTERN1,
	STATIONARY_PATTERN2,
	STATIONARY_PATTERN3
}StationaryPixelPatternType;

void displayPixelPattern(PixelsInfo *pixelInfo, Rgb *pixelsRgb, int8_t *menuItemValues);
void displayMovingPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, MovingPixelPatternType pixelPattern);
void displayStationaryPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, StationaryPixelPatternType pixelPattern);

#endif /* PIXELS_PATTERNS_H_ */
