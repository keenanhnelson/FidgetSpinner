/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//SPI to WS2812B
//5MHz spi clock
//200ns resolution for each spi bit
//6 spi bits to 1 ws2812b bit
//144 spi bits for 24 ws2812b bits
//18 spi bytes for 1 ws2812b pixel
//Low bit requires 400ns on and 800ns off. Which is about 2 spi high and 4 spi low
//High bit requires 800ns on and 400ns off. Which is about 4 spi high and 2 spi low

//rgb points to an array of desired rgb values that should equal numPixels
//pixelData need to point to an amount of memory equal to NumSpiBytesPerPixel*numPixels
#define NUM_SPI_BYTES_PER_PIXEL 18//Also change numOfHighSpiBits NUM_SPI_BITS_PER_PIXEL
#define NUM_SPI_BITS_PER_PIXEL_BIT 6//Also change numOfHighSpiBits NUM_SPI_BYTES_PER_PIXEL

void rgbToPixel_ws2812b(uint32_t *rgb, uint8_t *pixelData, int32_t numPixels){
    uint32_t pixelDataBitIndex = 0;
    //Set the first byte to all zeros so that the first bit isn't high for so long. Only need on st microcontrollers
    pixelData[0] = 0;
    pixelDataBitIndex += 8;
    for(int k=0; k<numPixels; k++){
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
            uint8_t numOfHighSpiBits = colorBit*2 + 2;//Equation to map 0->2 and 1->4
            for(int j=0; j<NUM_SPI_BITS_PER_PIXEL_BIT; j++){//Number of spi bits in one smart pixel bit
                byteIndex = pixelDataBitIndex / 8;//Determines current spi byte
                subBitIndex = 7 - (pixelDataBitIndex % 8);//Determines which bit needs to be change in the current spi byte
                if(j<numOfHighSpiBits){//how many spi bits is the signal high for
                    pixelData[byteIndex] |= 1 << subBitIndex;//logic high
                }else{
                    pixelData[byteIndex] &= ~(1 << subBitIndex);//logic low
                }
                pixelDataBitIndex++;
            }
        }
    }

    //Transmit all the data needed to light up all of the smart pixels
    HAL_SPI_Transmit(&hspi1, pixelData, NUM_SPI_BYTES_PER_PIXEL*numPixels+1, 1000);

}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  //Allocate space for pixels
  #define NUM_OF_PIXELS 4
#define SCRATCH_PAD_SIZE NUM_SPI_BYTES_PER_PIXEL * NUM_OF_PIXELS + 1//One for extra byte so the first high bit is high for so long
  uint32_t pixelsRgb[NUM_OF_PIXELS] = {0x0f0000, 0x000f00, 0x0f0000, 0x0f000f};
  uint8_t pixelDataScratchPad[SCRATCH_PAD_SIZE];
  rgbToPixel_ws2812b(pixelsRgb, pixelDataScratchPad, NUM_OF_PIXELS);
//  rgbToPixel_ws2812b(pixelsRgb, pixelDataScratchPad, NUM_OF_PIXELS);
//  rgbToPixel_ws2812b(pixelsRgb, pixelDataScratchPad, NUM_OF_PIXELS);
//  rgbToPixel_ws2812b(pixelsRgb, pixelDataScratchPad, NUM_OF_PIXELS);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == 0){
		  pixelsRgb[0] = 0x030000;
		  pixelsRgb[1] = 0x030000;
		  pixelsRgb[2] = 0x030000;
		  pixelsRgb[3] = 0x030000;
//		  pixelsRgb[0] = 0x00BFFF;
//		  pixelsRgb[1] = 0xB0E0E6;
//		  pixelsRgb[2] = 0x00BFFF;
//		  pixelsRgb[3] = 0xB0E0E6;
		  rgbToPixel_ws2812b(pixelsRgb, pixelDataScratchPad, NUM_OF_PIXELS);
	  }else{
		  pixelsRgb[0] = 0x000300;
		  pixelsRgb[1] = 0x000300;
		  pixelsRgb[2] = 0x000300;
		  pixelsRgb[3] = 0x000300;
//		  pixelsRgb[0] = 0xFFA500;
//		  pixelsRgb[1] = 0x800080;
//		  pixelsRgb[2] = 0xFFA500;
//		  pixelsRgb[3] = 0x800080;
		  rgbToPixel_ws2812b(pixelsRgb, pixelDataScratchPad, NUM_OF_PIXELS);
	  }
//	  HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
