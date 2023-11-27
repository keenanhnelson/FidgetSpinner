#include "pixels.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "stm32l4xx_hal.h"

void initPixels(
		PixelsInfo *pixelInfo,
		PixelsType pixelsType,
		uint32_t numPixels,
		void (*sendSpiData)(uint8_t *data, uint32_t len),
		uint32_t spiClockFreq
		)
{
	if(pixelsType == WS2812B_2020){
		//WS2812B_2020 required SPI configuration
		//10MHz spi clock
		//100ns resolution for each spi bit
		//Low bit requires 300ns on and 600ns off. Which is about 3 spi high and 6 spi low
		//High bit requires 600ns on and 300ns off. Which is about 6 spi high and 3 spi low
		//9 spi bits to 1 ws2812b bit
		//216 spi bits for 24 ws2812b bits
		//27 spi bytes per 1 ws2812b pixel
		//After all the color data bits are sent the data line needs to say low for at least 280us or 2800 spi bits or 350 spi bytes

		pixelInfo->pixelsType = pixelsType;
		pixelInfo->numPixels = numPixels;

		assert(spiClockFreq == 10000000);//Spi clock frequency needs to be 10MHz for everything to work properly
		pixelInfo->numSpiBytesPerPixel = 27;
		pixelInfo->numSpiBitsPerPixel = 9;
		pixelInfo->numSpiBitsHighFor0 = 3;
		pixelInfo->numSpiBitsHighFor1 = 6;
		pixelInfo->numSpiBytesEndSignal = 350;
		pixelInfo->totalNumSpiBytesToSend = numPixels*pixelInfo->numSpiBytesPerPixel + pixelInfo->numSpiBytesEndSignal;

		pixelInfo->spiData = malloc(pixelInfo->totalNumSpiBytesToSend);
		pixelInfo->sendSpiData = sendSpiData;
	}
	else{
		assert(0);//Pixel type not supported
	}
}

void deinitPixels(PixelsInfo *pixelsInfo){
	free(pixelsInfo->spiData);
}

Rgb hsvToRgb(Hsv hsv){
    Rgb rgb;
    int i;
    float f, p, q, t;

    if (hsv.s == 0) {
        // Achromatic (gray)
        rgb.r = rgb.g = rgb.b = (int)(hsv.v * 255);
        return rgb;
    }

    hsv.h /= 60; // sector 0 to 5
    i = (int)hsv.h;
    f = hsv.h - i; // factorial part of h
    p = hsv.v * (1 - hsv.s);
    q = hsv.v * (1 - hsv.s * f);
    t = hsv.v * (1 - hsv.s * (1 - f));

    switch (i) {
        case 0: rgb.r = hsv.v * 255; rgb.g = t * 255; rgb.b = p * 255; break;
        case 1: rgb.r = q * 255; rgb.g = hsv.v * 255; rgb.b = p * 255; break;
        case 2: rgb.r = p * 255; rgb.g = hsv.v * 255; rgb.b = t * 255; break;
        case 3: rgb.r = p * 255; rgb.g = q * 255; rgb.b = hsv.v * 255; break;
        case 4: rgb.r = t * 255; rgb.g = p * 255; rgb.b = hsv.v * 255; break;
        default: rgb.r = hsv.v * 255; rgb.g = p * 255; rgb.b = q * 255; break;
    }

    return rgb;
}

void setPixelsRgb(PixelsInfo *pixelsInfo, Rgb *rgb){
    //First make sure the all the bytes start off zero so only setting bits is required
	memset(pixelsInfo->spiData, 0, pixelsInfo->totalNumSpiBytesToSend);

	uint32_t pixelDataBitIndex = 0;
    for(int k=0; k<pixelsInfo->numPixels; k++){
        //Convert rgb to grb color which is needed by the smart pixel
        uint32_t dataToSend = (rgb[k].g << 16) | (rgb[k].r << 8) | (rgb[k].b << 0);

        //Generate bit stream
        for(int i=0; i<24; i++){//24 bits for one pixel
            uint8_t colorBit = (uint8_t)((dataToSend & 0x800000) >> 23);//Grab the MSb
            dataToSend <<= 1;//shift to new data for next time
            int byteIndex, subBitIndex;//Used to determine spi location
            uint8_t numOfHighSpiBits;
            if(colorBit == 0){
            	numOfHighSpiBits = pixelsInfo->numSpiBitsHighFor0;
            }
            else{
            	numOfHighSpiBits = pixelsInfo->numSpiBitsHighFor1;
            }
            for(int j=0; j<pixelsInfo->numSpiBitsPerPixel; j++){//Number of spi bits in one smart pixel bit
                byteIndex = pixelDataBitIndex / 8;//Determines current spi byte
                subBitIndex = 7 - (pixelDataBitIndex % 8);//Determines which bit needs to be change in the current spi byte
                if(j<numOfHighSpiBits){//how many spi bits is the signal high for
                	pixelsInfo->spiData[byteIndex] |= 1 << subBitIndex;//logic high
                }else{
//                    pixelInfo->spiData[byteIndex] &= ~(1 << subBitIndex);//logic low
                }
                pixelDataBitIndex++;
            }
        }
    }

    //Transmit all the data needed to light up all of the smart pixels
    pixelsInfo->sendSpiData(pixelsInfo->spiData, pixelsInfo->totalNumSpiBytesToSend);

}

void setPixelsHsv(PixelsInfo *pixelsInfo, Rgb *rgb, Hsv *hsv){
	for(int i=0; i<pixelsInfo->numPixels; i++){
		rgb[i] = hsvToRgb(hsv[i]);
	}
	setPixelsRgb(pixelsInfo, rgb);
}
