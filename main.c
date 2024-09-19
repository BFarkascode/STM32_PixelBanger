/* USER CODE BEGIN Header */
/**
 *  Author: BalazsFarkas
 *  Project: STM32_PixelBanger
 *  Processor: STM32L053R8
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  File: main.c
 *  Hardware description/pin distribution: PA5 (in-built LED)
 *  Change history: N/A
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ClockDriver_STM32L0x3.h"
#include "BitBang_STM32L0x3.h"
#include "NeopixelEffects_STM32L0x3.h"

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

output_signal_HandleTypeDef output_signal;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//IMPORTANT!!!!
//		In case a different bit depth signal is to be generated, all related functions should be adjusted
//		This means the need to change matrix sizes and re-targeting functions to these new matrices
//		Impacted functions and matrices have the "_32bit" written into their names
//		Locally for neopixels, all 32 bit functions are named with the "_GRBW" tag. These are not compatible with GRB/RGB neopixels.

uint8_t Bitbang_signal_generated;

uint8_t pixels_in_strip;

uint8_t bit_depth;

uint8_t Bitbang_signal_source_matrix_32bit [8][4];							//change the number of rows of the matrix here to have a different strip length
																			//Note: strip length should be an even number!
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
//  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */

  SysClockConfig();														//custom clock definition
  TIM6Config();															//TIM6 setup

  //We check the number of pixels in the strip (the repeat signal)
  pixels_in_strip = sizeof(Bitbang_signal_source_matrix_32bit)/sizeof(Bitbang_signal_source_matrix_32bit[0]);
  bit_depth = (sizeof(Bitbang_signal_source_matrix_32bit[0])/sizeof(Bitbang_signal_source_matrix_32bit[0][0])) * 8;


  //Below we define our signal frequency
  //For Neopixel, it should be 800 kHz, period number the same as the number of LEDs in a pixel x8, and repeat number should be the number of pixels in the strip
  output_signal.TIM_clock_prescale = 4;									//we do a prescale of 4
  output_signal.TIM_freq_kHz = 32000;									//APB timer clock is 32 MHz (this is the value AFTER APB prescaling)
  	  	  	  	  	  	  	  	  	  	  	  	  	  	  				//Note: the struct does not change the values of the APB clock. That should be set separately.
  output_signal.duty_cycle_RES = 10;									//we have a resolution of 10
  output_signal.PWM_freq_kHz = (output_signal.TIM_freq_kHz / output_signal.TIM_clock_prescale) / output_signal.duty_cycle_RES;
  	  	  	  	  	  	  	  	  	  	  	  	  	  	  				//Note: neopixels need a 800 kHz signal

  //Below we define our signal profile
  output_signal.duty_cycle_HIGH = 6;									//HIGH will be defined as 10/6
  output_signal.duty_cycle_LOW = 3;										//LOW will be defined as 10/3
  output_signal.unique_period_no = (bit_depth * 2);						//in case of 32 bit depth, we generate a 64 bit signal - two times 32 bits
  output_signal.repeat_signal_no = (pixels_in_strip / 2);				//we repeat the signal 4 times - for 8 pixels

  //Below we configure the bitbang machine on TIM2 CH1 and DMA1 CH5
  //TIM2_CH1 is used for the PWM signal since it is connected to the inbuilt LED
  DMA_Init_for_TIM2_CH1();												//DMA init
  DMA_TIM2_CH1_IRQPriorEnable();										//IRQ activation
  TIM2_Config_for_CH1_PWM_DMA();										//calibrate the timer and the PWM
  Bitbang_TIM2CH1DMA1CH5_Config_32bit();								//we knot TIM2 CH1 and DMA1 CH5 together into the bitbang machine
  Bitbang_signal_generated = 0;													//we reset the signal generation flag

#ifdef start_signal_generation
  //generate the start matrix
  //Note: this loads the same value in all the places in the matrix, except the last row, which will be kept at 0
  Bitbang_GenerateStartMatrix_32bit(0, 0, 0, 1, pixels_in_strip);
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

#ifdef spin
	  //Spins the colour
	  NeopixelColourSpin_GRWB(pixels_in_strip, 1, 50, 20);

	  NeopixelPublish();
#endif

#ifdef roll
	  //Rolls the existing pixels
	  //Note: we don't modify the values in the matrix, just shuffle the rows around. Thus this effect needs a start value to function!
	  //GENERATE(!) a start value matrix for this effect to work
	  NeopixelPublish();

	  NeopixelRoll_32bit(pixels_in_strip, 500);
#endif

//#ifdef fill&drain
	  //this below will fill and then drain the strip with a pink colour
	  StripFill_GRWB(8,8,50,20,0,500);
	  StripDrain_GRWB(8,500);
//#endif

#ifdef wipe
	  StripWipe_GRWB(pixels_in_strip);
	  NeopixelPublish();
	  Delay_ms(500);													//add a delay to witness the wipe
#endif
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
