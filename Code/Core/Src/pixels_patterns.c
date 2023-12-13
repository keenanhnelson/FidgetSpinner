#include <kinematics.h>
#include "pixels_patterns.h"
#include "tim.h"
#include <math.h>
#include "user_interface.h"

//General variables
static int16_t currentCnt;
static int16_t prevCnt = -1;
static uint32_t currentTime = 0;
static float currentRpm = 0;
static float prevRpm = 0;
static float brightness = -1;

void displayPixelPattern(PixelsInfo *pixelInfo, Rgb *pixelsRgb, MenuInfo *menuInfo){
	currentRpm = fabsf(getRpm());
	currentTime = HAL_GetTick();

	brightness = menuInfo->brightnessToValue[menuInfo->itemValues[Brightness]];//Map menu brightness to a range between 0 and 1

	if(currentRpm > 60){
		//Display moving pattern selected in menu
		MovingPixelPatternType movingPattern = menuInfo->itemValues[PatternMoving];
		displayMovingPixelPattern(pixelInfo, pixelsRgb, movingPattern);
	}
	else{
		//Display stationary pattern
		StationaryPixelPatternType stationaryPattern = menuInfo->itemValues[PatternStationary];
		displayStationaryPixelPattern(pixelInfo, pixelsRgb, stationaryPattern);
	}
}

void displayMovingPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, MovingPixelPatternType pixelPattern){
	switch(pixelPattern){
		case PIXEL_PATTERN1:{
			static int currentLedIndex = -1;
			static int prevLedIndex = -1;
			static int16_t diffCnt;
			static Hsv colors[] = {{45, 1, 0.1f}, {135, 1, 0.1f}, {225, 1, 0.1f}, {315, 1, 0.1f}, };
			static int numColors = sizeof(colors)/sizeof(colors[0]);
			static int colorIndex = 0;

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
				colors[colorIndex].v = brightness;//Update brightness with menu selected value
				pixelsRgb[currentLedIndex] = hsvToRgb(colors[colorIndex]);
				prevCnt = currentCnt;
				prevLedIndex = currentLedIndex;
				setPixelsRgb(pixelsInfo, pixelsRgb);
			}
			break;
		}

		case PIXEL_PATTERN2:{
			static Hsv colors[] = {{0, 1, 0.1f}, {45, 1, 0.1f}, {90, 1, 0.1f}, {135, 1, 0.1f}, {180, 1, 0.1f}, {225, 1, 0.1f}, };
			static int numColors = sizeof(colors)/sizeof(colors[0]);
			static int16_t diffCnt;
			static int colorIndex = 0;

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
				colors[colorIndex].v = brightness;//Update brightness with menu selected value
				Rgb color = hsvToRgb(colors[colorIndex]);
				for(int i=0; i<pixelsInfo->numPixels; i++){
					pixelsRgb[i] = color;
				}
				setPixelsRgb(pixelsInfo, pixelsRgb);
				prevCnt = currentCnt;
			}
			break;
		}

		case PIXEL_PATTERN3:{
			static float lowToHighRpmBoundaries[] = {100, 200, 500};//Threshold to pass whem coming from low rpm to high rpm before changing color
			static float highToLowRpmBoundaries[] = {100, 200, 500};//Threshold to pass when coming from high rpm to low rpm before changing color
			static int numBoundaries = sizeof(lowToHighRpmBoundaries)/sizeof(lowToHighRpmBoundaries[0]);
			static Hsv areaColors[] = {{0, 1, 0.1f}, {90, 1, 0.1f}, {180, 1, 0.1f}, {270, 1, 0.1f},};
			//static int numAreas = sizeof(areaColors)/sizeof(areaColors[0]);
			static int currentAreaIndex = 0;
			static int prevAreaIndex = sizeof(areaColors)/sizeof(areaColors[0]) - 1;

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
				areaColors[currentAreaIndex].v = brightness;//Update brightness with menu selected value
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

		case PIXEL_PATTERN4:{
			static const float minRpm = 600;
			static const float maxRpm = 1000;
			static const float minHue = 0;
			static const float maxHue = 320;
			static const float m = (maxHue-minHue)/(maxRpm-minRpm);
			static const float b = minHue - m*minRpm;

			//Make sure to wait some time for pixel data to write to the leds
			if(getIsSendingPixelData()){
				break;
			}

			float rpm = fabsf(getRpm());
			if(rpm < minRpm){rpm = minRpm;}
			if(rpm > maxRpm){rpm = maxRpm;}
			float rpmToHue = rpm * m + b;
			Hsv hsv = {rpmToHue, 1, brightness};
			Rgb color = hsvToRgb(hsv);
			for(int i=0; i<pixelsInfo->numPixels; i++){
				pixelsRgb[i] = color;
			}
			setPixelsRgb(pixelsInfo, pixelsRgb);
			break;
		}
	}
}

void displayStationaryPixelPattern(PixelsInfo *pixelsInfo, Rgb *pixelsRgb, StationaryPixelPatternType pixelPattern){
	switch(pixelPattern){
		case STATIONARY_PATTERN1:{
			static uint32_t nextTimeAction = 0;
			static uint32_t betweenTime = 50;
			static float rotatingHue[10] = {0, 20, 40, 80, 100, 120, 140, 160, 180, 200};//0-360

			if(currentTime < nextTimeAction){
				break;
			}

			for(int i=0; i<pixelsInfo->numPixels; i++){
				rotatingHue[i] = fmodf((rotatingHue[i] + 5), 360.0f);
				Rgb color = hsvToRgb((Hsv){rotatingHue[i], 1, brightness});
				pixelsRgb[i] = color;
			}
			setPixelsRgb(pixelsInfo, pixelsRgb);

			nextTimeAction = currentTime + betweenTime;
			break;
		}

		case STATIONARY_PATTERN2:{
			static uint32_t nextTimeAction = 0;
			static uint32_t betweenTime = 50;
			static float rotatingHue = 0;//0-360

			if(currentTime < nextTimeAction){
				break;
			}

			rotatingHue = fmodf((rotatingHue + 5), 360.0f);

			Rgb color = hsvToRgb((Hsv){rotatingHue, 1, brightness});
			for(int i=0; i<pixelsInfo->numPixels; i++){
				pixelsRgb[i] = color;
			}
			setPixelsRgb(pixelsInfo, pixelsRgb);

			nextTimeAction = currentTime + betweenTime;
			break;
		}

		case STATIONARY_PATTERN3:{

			break;
		}
	}
}
