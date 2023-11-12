#ifndef PIXELS_PATTERNS_H_
#define PIXELS_PATTERNS_H_

#include "pixels.h"

typedef enum{
	PIXEL_PATTERN1,
	PIXEL_PATTERN2
}PixelPatternType;

void displayPixelPattern(PixelsInfo *pixelsInfo, uint32_t *pixelsRgb, PixelPatternType pixelPattern);

#endif /* PIXELS_PATTERNS_H_ */
