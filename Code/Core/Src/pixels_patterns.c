#include "pixels_patterns.h"
#include "tim.h"

//General variables
static int16_t currentCnt;
static int prevCnt = -1;

//Pattern1 variables
static int currentLedIndex = -1;
static int prevLedIndex = -1;
static int16_t diffCnt;

//Pattern2 variables
static Rgb colors[] = {{0x0f, 0x00, 0x00}, {0x00, 0x0f, 0x00}, {0x00, 0x00, 0x0f}};
static int numColors = sizeof(colors)/sizeof(Rgb);
static int colorIndex;

void displayPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, PixelPatternType pixelPattern){
	switch(pixelPattern){
		case PIXEL_PATTERN1:{
			currentCnt = TIM1->CNT;
			diffCnt = currentCnt - prevCnt;
			//Initialize prevLedIndex to the last led index at the start of the program
			if(prevLedIndex == -1){
				prevLedIndex = pixelsInfo->numPixels-1;
			}
			if(diffCnt > 0){
			  currentLedIndex += 1;
			  if(currentLedIndex >= pixelsInfo->numPixels){
				  currentLedIndex = 0;
			  }
			}
			if(diffCnt < 0){
			  currentLedIndex -= 1;
			  if(currentLedIndex < 0){
				  currentLedIndex = pixelsInfo->numPixels-1;
			  }
			}
			if(prevCnt != currentCnt){
				pixelsRgb[prevLedIndex] = (Rgb){0x00, 0x00, 0x03};
				pixelsRgb[currentLedIndex] = (Rgb){0x0f, 0x00, 0x00};
				prevCnt = currentCnt;
				prevLedIndex = currentLedIndex;
				setPixelsRgb(pixelsInfo, pixelsRgb);
			}
			break;
		}

		case PIXEL_PATTERN2:{
			currentCnt = TIM1->CNT;
			if(prevCnt != currentCnt){
				diffCnt = currentCnt - prevCnt;
				if(diffCnt > 0){
					colorIndex += 1;
				  if(colorIndex >= numColors){
					  colorIndex = 0;
				  }
				}
				if(diffCnt < 0){
					colorIndex -= 1;
				  if(colorIndex < 0){
					  colorIndex = numColors-1;
				  }
				}
			  for(int i=0; i<pixelsInfo->numPixels; i++){
				  pixelsRgb[i] = colors[colorIndex];
			  }
			  setPixelsRgb(pixelsInfo, pixelsRgb);
			  prevCnt = currentCnt;
			}
			break;
		}
	}
}
