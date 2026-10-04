/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "liquidcrystal_i2c.h"
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


int adc_raw = 0;
float voltage = 0.0f;

#define SINE_LEN   48
#define ADC_MAX    3204.0f   // lower this to the highest ADC value you see in bright light
#define NUM_LEVELS 9
#define HYST       0.15f     // fraction of a band you must cross before switching
uint16_t sine_table[SINE_LEN];

// Level 1 (dark) to level 9 (bright): C4 D4 E4 F4 G4 A4 B4 C5 D5
const float level_freq[NUM_LEVELS] = {0, 294, 330, 349, 392, 440, 494, 523, 587};
uint8_t audio_on = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int _write(int,char *, int);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t Read_OPT101(void)
{
    uint32_t value = 0;

    HAL_ADC_Start(&hadc1); //initialize ADC

    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        value = HAL_ADC_GetValue(&hadc1); //gets reading from the pin the photodiode is connected to
    }

    HAL_ADC_Stop(&hadc1);
    return value;
}

void set_pitch(float freq)
{

    // pitch = 72 MHz / ((ARR + 1) * SINE_LEN)
    uint32_t arr = (uint32_t)(72000000.0f / (freq * SINE_LEN)) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim6, arr);
}

void audio_output(uint8_t enable) //parameter is either 0 or 1 to run the specific loop.
{
    if (enable) // 1
    {
        for (int i = 0; i < SINE_LEN; i++)
            sine_table[i] = (uint16_t)(2048.0f +
                1500.0f * sinf(2.0f * 3.14159265f * i / SINE_LEN)); //calculation of sine wave for the DAC to initialize

        HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1,
                          (uint32_t *)sine_table,
                          SINE_LEN,
                          DAC_ALIGN_12B_R);

        HAL_TIM_Base_Start(&htim6);
    }
    else // 0
    {
        HAL_TIM_Base_Stop(&htim6);
        HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
    }
}

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_DAC1_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HD44780_Init(2);
  audio_output(audio_on);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) //B1 button check
	    {
	        audio_on = !audio_on; //inverts audio_on current value
	        audio_output(audio_on); //Starts/Ends DAC to output audio
	        HAL_Delay(200);
	    }

	  adc_raw = Read_OPT101();



	  // Convert to voltage (Vref = 3.3V, 12-bit resolution)
	  voltage = (adc_raw * 3.3f) / 4095.0f;
	  printf( "ADC: %d  Voltage: %.3f V AUDIO : %d \r\n", adc_raw, voltage, audio_on);

	    char line2[17];

	    if (adc_raw < 356)
	    {
	        strcpy(line2, "Level 1        ");
	        set_pitch(level_freq[0]);
	    }
	    else if (adc_raw < 712)
	    {
	        strcpy(line2, "Level 2        ");

	        set_pitch(level_freq[1]);

	    }
	    else if (adc_raw < 1068)
	    {
	        strcpy(line2, "Level 3        ");

	        set_pitch(level_freq[2]);
	    }
	    else if (adc_raw < 1424)
	    {
	        strcpy(line2, "Level 4        ");

	        set_pitch(level_freq[3]);
	    }
	    else if (adc_raw < 1780)
	    {
	        strcpy(line2, "Level 5        ");

	        set_pitch(level_freq[4]);
	    }
	    else if (adc_raw < 2136)
	    {
	        strcpy(line2, "Level 6        ");

	        set_pitch(level_freq[5]);
	    }
	    else if (adc_raw < 2492)
	    {
	        strcpy(line2, "Level 7        ");

	        set_pitch(level_freq[6]);
	    }
	    else if (adc_raw < 2848)
	    {
	        strcpy(line2, "Level 8        ");

	        set_pitch(level_freq[7]);
	    }
	    else
	    {
	        strcpy(line2, "Level 9        ");

	        set_pitch(level_freq[8]);
	    }

	    char line1[17];
	    snprintf(line1, sizeof(line1), "V:%.2f          ", voltage);

	    HD44780_SetCursor(0,0);
	    HD44780_PrintStr(line1);
	    HD44780_SetCursor(0,1);
	    HD44780_PrintStr(line2);

	  HAL_Delay(500);  // Sample every 500 ms

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}
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
#ifdef USE_FULL_ASSERT
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
