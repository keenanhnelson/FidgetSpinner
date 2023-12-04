#ifndef PIXELS_CONTROL_H_
#define PIXELS_CONTROL_H_

#include <stdint.h>

typedef enum{
	WS2812B_2020
}PixelsType;

typedef struct {
    float h; // Hue [0, 360]
    float s; // Saturation [0, 1]
    float v; // Value [0, 1]
} Hsv;

typedef struct {
    uint8_t r; // Red [0, 255]
    uint8_t g; // Green [0, 255]
    uint8_t b; // Blue [0, 255]
} Rgb;

typedef struct{
	uint32_t numPixels;
	uint8_t *spiData;
	PixelsType pixelsType;

	uint16_t numSpiBytesPerPixel;
	uint16_t numSpiBitsPerPixel;
	uint16_t numSpiBitsHighFor0;
	uint16_t numSpiBitsHighFor1;
	uint16_t numSpiBytesEndSignal;
	uint32_t totalNumSpiBytesToSend;
	uint32_t spiClockFreq;
}PixelsInfo;

void initPixels(
		PixelsInfo *pixelInfo,
		PixelsType pixelsType,
		uint32_t numPixels,
		uint32_t spiClockFreq
);
void deinitPixels(PixelsInfo *pixelsInfo);
Rgb hsvToRgb(Hsv hsv);
void setPixelsRgb(PixelsInfo *pixelInfo, Rgb *rgb);
void setPixelsHsv(PixelsInfo *pixelsInfo, Rgb *rgb, Hsv *hsv);

#endif
