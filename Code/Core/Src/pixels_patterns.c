#include "pixels_patterns.h"
#include "tim.h"
#include "revolution_speed.h"
#include <math.h>
#include "user_interface.h"

//General variables
static int16_t currentCnt;
static int16_t prevCnt = -1;

//Pattern1 variables
static int currentLedIndex = -1;
static int prevLedIndex = -1;
static int16_t diffCnt;

//Pattern2 variables
static Hsv colors[] = {{45, 1, 0.1f}, {135, 1, 0.1f}, {225, 1, 0.1f}, {315, 1, 0.1f}, };
static int numColors = sizeof(colors)/sizeof(colors[0]);
static int colorIndex = 0;

//Pattern3 variables
static float currentRpm = 0;
static float prevRpm = 0;
static float lowToHighRpmBoundaries[] = {1000, 1300, 1600};//Threshold to pass whem coming from low rpm to high rpm before changing color
static float highToLowRpmBoundaries[] = {1000, 1300, 1600};//Threshold to pass when coming from high rpm to low rpm before changing color
static int numBoundaries = sizeof(lowToHighRpmBoundaries)/sizeof(lowToHighRpmBoundaries[0]);
static Hsv areaColors[] = {{0, 1, 0.1f}, {90, 1, 0.1f}, {180, 1, 0.1f}, {270, 1, 0.1f},};
//static int numAreas = sizeof(areaColors)/sizeof(areaColors[0]);
static int currentAreaIndex = 0;
static int prevAreaIndex = sizeof(areaColors)/sizeof(areaColors[0]) - 1;

//Stationary pattern variables
//Pattern1
static uint32_t currentTime = 0;
static uint32_t nextTimeAction = 0;
static uint32_t betweenTime = 100;
static float rotatingHue = 0;//0-360

void displayPixelPattern(PixelsInfo *pixelInfo, Rgb *pixelsRgb, uint8_t *menuItemValues){
	currentRpm = fabsf(getRpm());
	currentTime = HAL_GetTick();
	if(currentRpm > 100){
		//Display moving pattern selected in menu
		MovingPixelPatternType movingPattern = menuItemValues[PatternMoving] - 1;
		displayMovingPixelPattern(pixelInfo, pixelsRgb, movingPattern);
	}
	else{
		//Display stationary pattern
		StationaryPixelPatternType stationaryPattern = menuItemValues[PatternStationary] - 1;
		displayStationaryPixelPattern(pixelInfo, pixelsRgb, stationaryPattern);
	}
}

void displayMovingPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, MovingPixelPatternType pixelPattern){
	switch(pixelPattern){
		case PIXEL_PATTERN3:{
			currentCnt = getEncoderCnt();
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
			currentCnt = getEncoderCnt();
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

		case PIXEL_PATTERN1:{

			uint8_t foundBoundaryCrossing = 0;

			//First check if currentRpm has passed any higher boundaries
			for(int i=prevAreaIndex; i<numBoundaries; i++){
				if(currentRpm > lowToHighRpmBoundaries[i]){
					currentAreaIndex = i + 1;
					foundBoundaryCrossing = 1;
				}
			}

			//If current rpm hasn't passed any higher boundaries check if it has decreased passed any lower boundaries
			if(!foundBoundaryCrossing){
				for(int i=prevAreaIndex-1; i>=0; i--){
					if(currentRpm < highToLowRpmBoundaries[i]){
						currentAreaIndex = i;
						foundBoundaryCrossing = 1;
					}
				}
			}

			if(currentAreaIndex != prevAreaIndex){
				Rgb color = hsvToRgb(areaColors[currentAreaIndex]);
				for(int i=0; i<pixelsInfo->numPixels; i++){
					pixelsRgb[i] = color;
				}
				setPixelsRgb(pixelsInfo, pixelsRgb);
				prevAreaIndex = currentAreaIndex;
			}

			prevRpm = currentRpm;
			break;
		}
	}
}

void displayStationaryPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, StationaryPixelPatternType pixelPattern){
	switch(pixelPattern){
		case STATIONARY_PATTERN1:{
			if(currentTime < nextTimeAction){
				break;
			}

			rotatingHue = fmodf((rotatingHue + 5), 360.0f);

			Rgb color = hsvToRgb((Hsv){rotatingHue, 1, 0.1f});
			for(int i=0; i<pixelsInfo->numPixels; i++){
				pixelsRgb[i] = color;
			}
			setPixelsRgb(pixelsInfo, pixelsRgb);

			nextTimeAction = currentTime + betweenTime;
			break;
		}

		case STATIONARY_PATTERN2:{

			break;
		}

		case STATIONARY_PATTERN3:{

			break;
		}
	}
}
