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
static int colors[] = {0x0f0000, 0x000f00, 0x00000f};
static int numColors = sizeof(colors)/sizeof(colors[0]);
static int colorIndex;

void displayPixelPattern(PixelsInfo *pixelsInfo, uint32_t *pixelsRgb, PixelPatternType pixelPattern){
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
			  pixelsRgb[prevLedIndex] = 0x000003;
			  pixelsRgb[currentLedIndex] = 0x0f0000;
			  prevCnt = currentCnt;
			  prevLedIndex = currentLedIndex;
			  setPixelsColors(pixelsInfo, pixelsRgb);
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
			  setPixelsColors(pixelsInfo, pixelsRgb);
			  prevCnt = currentCnt;
			}
			break;
		}
	}
}
