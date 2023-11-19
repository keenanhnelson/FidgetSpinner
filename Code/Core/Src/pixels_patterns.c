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
static Hsv colors[] = {{45, 1, 0.1f}, {135, 1, 0.1f}, {225, 1, 0.1f}, {315, 1, 0.1f}, };
static int numColors = sizeof(colors)/sizeof(Hsv);
static int colorIndex = 0;

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
				  colorIndex++;
				  if(colorIndex >= numColors){
					  colorIndex = 0;
				  }
			  }
			}
			if(diffCnt < 0){
			  currentLedIndex -= 1;
			  if(currentLedIndex < 0){
				  currentLedIndex = pixelsInfo->numPixels-1;
				  colorIndex--;
				  if(colorIndex < 0){
					  colorIndex = numColors - 1;
				  }
			  }
			}
			if(prevCnt != currentCnt){
				pixelsRgb[prevLedIndex] = (Rgb){0x00, 0x00, 0x03};
				pixelsRgb[currentLedIndex] = hsvToRgb(colors[colorIndex]);
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
				Rgb color = hsvToRgb(colors[colorIndex]);
				for(int i=0; i<pixelsInfo->numPixels; i++){
					pixelsRgb[i] = color;
				}
				setPixelsRgb(pixelsInfo, pixelsRgb);
				prevCnt = currentCnt;
			}
			break;
		}

//		case PIXEL_PATTERN3:{
//			currentCnt = TIM1->CNT;
//			if(prevCnt != currentCnt){
//				diffCnt = currentCnt - prevCnt;
//
//			  for(int i=0; i<pixelsInfo->numPixels; i++){
//				  pixelsRgb[i] = colors[colorIndex];
//			  }
//			  setPixelsRgb(pixelsInfo, pixelsRgb);
//			  prevCnt = currentCnt;
//			}
//			break;
//		}
	}
}
