#include "pixels.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

void setPixelsColors(PixelsInfo *pixelInfo, uint32_t *rgb){
    //First make sure the all the bytes start off zero so only setting bits is required
	memset(pixelInfo->spiData, 0, pixelInfo->totalNumSpiBytesToSend);

	uint32_t pixelDataBitIndex = 0;
    for(int k=0; k<pixelInfo->numPixels; k++){
        //Convert rgb to grb color which is needed by the smart pixel
        uint8_t r = (uint8_t)((rgb[k] >> 16) & 0xFF);
        uint8_t g = (uint8_t)((rgb[k] >> 8) & 0xFF);
        uint8_t b = (uint8_t)((rgb[k] >> 0) & 0xFF);
        uint32_t dataToSend = (g << 16) | (r << 8) | (b<<0);

        //Generate bit stream
        for(int i=0; i<24; i++){//24 bits for one pixel
            uint8_t colorBit = (uint8_t)((dataToSend & 0x800000) >> 23);//Grab the MSb
            dataToSend <<= 1;//shift to new data for next time
            int byteIndex, subBitIndex;//Used to determine spi location
            uint8_t numOfHighSpiBits;
            if(colorBit == 0){
            	numOfHighSpiBits = pixelInfo->numSpiBitsHighFor0;
            }
            else{
            	numOfHighSpiBits = pixelInfo->numSpiBitsHighFor1;
            }
            for(int j=0; j<pixelInfo->numSpiBitsPerPixel; j++){//Number of spi bits in one smart pixel bit
                byteIndex = pixelDataBitIndex / 8;//Determines current spi byte
                subBitIndex = 7 - (pixelDataBitIndex % 8);//Determines which bit needs to be change in the current spi byte
                if(j<numOfHighSpiBits){//how many spi bits is the signal high for
                	pixelInfo->spiData[byteIndex] |= 1 << subBitIndex;//logic high
                }else{
//                    pixelInfo->spiData[byteIndex] &= ~(1 << subBitIndex);//logic low
                }
                pixelDataBitIndex++;
            }
        }
    }

    //Transmit all the data needed to light up all of the smart pixels
    pixelInfo->sendSpiData(pixelInfo->spiData, pixelInfo->totalNumSpiBytesToSend);
}
