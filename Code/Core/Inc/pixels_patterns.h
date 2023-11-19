#ifndef PIXELS_PATTERNS_H_
#define PIXELS_PATTERNS_H_

#include "pixels.h"

typedef enum{
	PIXEL_PATTERN1,
	PIXEL_PATTERN2,
	PIXEL_PATTERN3
}PixelPatternType;

void displayPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, PixelPatternType pixelPattern);

#endif /* PIXELS_PATTERNS_H_ */
