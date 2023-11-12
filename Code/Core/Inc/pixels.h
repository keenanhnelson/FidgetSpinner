#ifndef PIXELS_CONTROL_H_
#define PIXELS_CONTROL_H_

#include <stdint.h>

typedef enum{
	WS2812B_2020
}PixelsType;

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
	void (*sendSpiData)(uint8_t *data, uint32_t len);
	uint32_t spiClockFreq;
}PixelsInfo;

void initPixels(
		PixelsInfo *pixelInfo,
		PixelsType pixelsType,
		uint32_t numPixels,
		void (*sendSpiData)(uint8_t *data, uint32_t len),
		uint32_t spiClockFreq
);

void deinitPixels(PixelsInfo *pixelsInfo);

void setPixelsColors(PixelsInfo *pixelInfo, uint32_t *rgb);

#endif
