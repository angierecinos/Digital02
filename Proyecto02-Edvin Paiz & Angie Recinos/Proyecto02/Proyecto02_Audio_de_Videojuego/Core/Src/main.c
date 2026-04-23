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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pitchesPRE.h"
#include <stdio.h>
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM_FREQ 84000000
#define ARR_pwm 100

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
int level_intro[] = {
		NOTE_C4, NOTE_D4, NOTE_DS4, NOTE_C4,
		NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_FS4,
		NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_FS4, NOTE_G4
};

int level_intro_dur[] = {
		500, 250, 350, 400,
		100, 100, 100, 100,
		100, 100, 100, 100, 500
};

int hammer_time[] = {
		NOTE_AS4, 0, NOTE_AS4, 0, NOTE_AS4, 0, NOTE_AS4, 0, NOTE_AS4, 0,
		NOTE_D5, NOTE_AS4, NOTE_D5, NOTE_AS4, 0,

		NOTE_D5, 0, NOTE_D5, 0, NOTE_D5, 0, NOTE_D5, 0, NOTE_D5, 0,
		NOTE_F5, NOTE_D5, NOTE_F5, NOTE_D5, 0
};

int hammer_time_dur[] = {
		150, 5, 110, 5, 110, 5, 110, 5, 110, 5,
		125, 125, 125, 125, 5,

		150, 5, 110, 5, 110, 5, 110, 5, 110, 5,
		125, 125, 125, 125, 10
};

int level_one[] = {
		NOTE_AS3, NOTE_D4, NOTE_F4, NOTE_G4, NOTE_F4
};

int level_one_dur[] = {
		500, 400, 200, 200, 200
};

int level_two[] = {
		NOTE_D2, 0, NOTE_D2, 0, NOTE_D2, 0
};

int level_two_dur[] = {
		100, 50, 100, 50, 100, 100
};

int level_three[] = {
		NOTE_AS5, 0, NOTE_DS5, 0, NOTE_AS4, 0
};

int level_three_dur[] = {
		150, 100, 150, 100, 150, 100
};

int level_four[] = {
		NOTE_AS1, 0, NOTE_F1, 0, NOTE_F1, 0
};

int level_four_dur[] = {
		100, 50, 50, 25, 50, 25, 50
};

int game_win[] = {
		NOTE_E4, NOTE_GS4, NOTE_B4, NOTE_CS5, NOTE_C5, NOTE_CS5,
		NOTE_GS4, NOTE_G5, NOTE_F5, NOTE_D5, NOTE_C5, 0, NOTE_C5, NOTE_F5,
		NOTE_CS5, NOTE_D5, NOTE_AS4
};

int game_win_dur[] = {
		300, 300, 300, 150, 150, 300,
		100, 250, 100, 300, 100, 50, 100, 300,
		100, 100, 500
};

int level_stats[] = {
		NOTE_C4, NOTE_D4, NOTE_F4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_AS3, NOTE_AS4
};

int level_stats_dur[] = {
		500, 500, 500, 350, 350, 350, 1000, 900
};

int mario_dead[] = {
		NOTE_B5, NOTE_AS5, NOTE_B5, NOTE_AS5,
		NOTE_A5, NOTE_GS5, NOTE_A5, NOTE_GS5,
		NOTE_G5, NOTE_FS5, NOTE_G5, NOTE_FS5,
		NOTE_F5, NOTE_E5, NOTE_F5, NOTE_E5, 0,
		NOTE_D5, NOTE_F5, NOTE_A4, NOTE_AS4
};

int mario_dead_dur[] = {
		50, 50, 50, 50,
		50, 50, 50, 50,
		50, 50, 50, 50,
		50, 50, 50, 50, 50,
		100, 150, 150, 500
};

int level_win[] = {
		NOTE_E5, NOTE_F5, NOTE_G5, 0, NOTE_E5, 0,
		NOTE_E5, NOTE_F5, NOTE_G5, 0, NOTE_E5,
};

int level_win_dur[] = {
		100, 100, 100, 50, 150, 100,
		100, 100, 100, 50, 150, 100,
};


uint8_t menu = 1;
volatile uint8_t intro = 0;
volatile uint8_t ctrl_state1;
volatile uint8_t ctrl_cmd2;
 uint8_t rx_data1;
 uint8_t rx_data2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
int presForFrequency(int frequency);
void playTone(int *tone, int *duration, int *pause, int size);
void playTonePRE(int *tone, int *duration, int *pause, int size);
void noTone(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int presForFrequency(int frequency)
{
	if (frequency == 0)
	{
		return 0;
	}
	return ((TIM_FREQ / (ARR_pwm * frequency)) -1 );
}

void playTone(int *tone, int *duration, int *pause, int size)
{
	for (int i = 0; i < size; i++)
	{
		int prescaler = presForFrequency(tone[i]); // Calcular el prescaler
		int dur = duration[i]; // Obtener la duración
		int pauseBetweenTones = 0;
		if (pause != NULL)
		{
			pauseBetweenTones = pause[i] - duration[i];
		}

		__HAL_TIM_SET_PRESCALER(&htim1, prescaler);
		HAL_Delay(dur); // Duración de la nota
		noTone(); // Pausa
		HAL_Delay(pauseBetweenTones); // Duración sin tono
	}
}

void playTonePRE(int *tone, int *duration, int *pause, int size)
{
    for (int i = 0; i < size; i++)
    {
        int prescaler = tone[i];
        int dur = duration[i];
        int pauseBetweenTones = 0;

        if (pause != NULL)
        {
            pauseBetweenTones = pause[i] - duration[i];
            if (pauseBetweenTones < 0)
            {
                pauseBetweenTones = 0;
            }
        }

        __HAL_TIM_SET_PRESCALER(&htim1, prescaler);
        __HAL_TIM_SET_COUNTER(&htim1, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 50);  // activar PWM

        HAL_Delay(dur);

        noTone();  // apagar PWM
        HAL_Delay(pauseBetweenTones);
    }
}

void noTone(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
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
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_UART_Receive_IT(&huart3, &rx_data1, 1);
  HAL_UART_Receive_IT(&huart2, &rx_data2, 1);
  noTone();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  switch (ctrl_state1)
	  	  {
	  	  case '1':
	  		  if (intro == 1)
	  		  {
	  			  playTonePRE(level_intro, level_intro_dur, NULL, (sizeof(level_intro)/sizeof(level_intro[0])));
	  			  noTone();
	  			  intro = 0;
	  		  }
	  		  playTonePRE(level_one, level_one_dur, NULL, (sizeof(level_one)/sizeof(level_one[0])));
	  		  break;

	  	  case '2':
	  		  if (intro == 1)
	  		  {
	  			  playTonePRE(level_intro, level_intro_dur, NULL, (sizeof(level_intro)/sizeof(level_intro[0])));
	  			  noTone();
	  			  intro = 0;
	  		  }
	  		  playTonePRE(level_two, level_two_dur, NULL, (sizeof(level_two)/sizeof(level_two[0])));
	  		  break;

	  	  case '3':
	  		  if (intro == 1)
	  		  {
	  			  playTonePRE(level_intro, level_intro_dur, NULL, (sizeof(level_intro)/sizeof(level_intro[0])));
	  			  noTone();
	  			  intro = 0;
	  		  }
	  		  playTonePRE(level_three, level_three_dur, NULL, (sizeof(level_three)/sizeof(level_three[0])));
	  		  break;

	  	  case '4':
	  		  if (intro == 1)
	  		  {
	  			  playTonePRE(level_intro, level_intro_dur, NULL, (sizeof(level_intro)/sizeof(level_intro[0])));
	  			  noTone();
	  			  intro = 0;
	  		  }
	  		  playTonePRE(level_four, level_four_dur, NULL, (sizeof(level_four)/sizeof(level_four[0])));
	  		  break;

	  	  case 'M':
	  		  playTonePRE(level_stats, level_stats_dur, NULL, (sizeof(level_stats)/sizeof(level_stats[0])));
	  		break;

	  	  case 'D':
	  		  playTonePRE(mario_dead, mario_dead_dur, NULL, (sizeof(mario_dead)/sizeof(mario_dead[0])));
	  		break;

	  	  case 'V':
	  		playTonePRE(game_win, game_win_dur, NULL, (sizeof(game_win)/sizeof(game_win[0])));
	  		break;
	  	  }

	  if (ctrl_cmd2 == 's'){
		  playTonePRE(game_win, game_win_dur, NULL, (sizeof(game_win)/sizeof(game_win[0])));
	  }

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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_SS_Pin */
  GPIO_InitStruct.Pin = SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SD_SS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) // Control 1
	    {
	    	ctrl_state1 = rx_data1;
	    	intro = 1;

	        HAL_UART_Transmit(&huart2, &ctrl_state1, 1, 100);

	        HAL_UART_Receive_IT(&huart3, &rx_data1, 1);
	    }

    if (huart->Instance == USART2) // Terminal para pruebas
    {
    	ctrl_cmd2 = rx_data2;

        HAL_UART_Transmit(&huart2, &rx_data2, 1, 100);

        HAL_UART_Receive_IT(&huart2, &rx_data2, 1);
    }
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
