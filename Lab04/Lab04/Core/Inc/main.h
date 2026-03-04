/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define LED0_J1_Pin GPIO_PIN_0
#define LED0_J1_GPIO_Port GPIOC
#define LED1_J1_Pin GPIO_PIN_1
#define LED1_J1_GPIO_Port GPIOC
#define LED2_J1_Pin GPIO_PIN_2
#define LED2_J1_GPIO_Port GPIOC
#define LED3_J1_Pin GPIO_PIN_3
#define LED3_J1_GPIO_Port GPIOC
#define BTN1_Pin GPIO_PIN_0
#define BTN1_GPIO_Port GPIOA
#define BTN1_EXTI_IRQn EXTI0_IRQn
#define BTN2_Pin GPIO_PIN_1
#define BTN2_GPIO_Port GPIOA
#define BTN2_EXTI_IRQn EXTI1_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define BTN_START_Pin GPIO_PIN_4
#define BTN_START_GPIO_Port GPIOA
#define BTN_START_EXTI_IRQn EXTI4_IRQn
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define LED0_J2_Pin GPIO_PIN_5
#define LED0_J2_GPIO_Port GPIOC
#define seg_a_Pin GPIO_PIN_0
#define seg_a_GPIO_Port GPIOB
#define seg_b_Pin GPIO_PIN_1
#define seg_b_GPIO_Port GPIOB
#define seg_c_Pin GPIO_PIN_2
#define seg_c_GPIO_Port GPIOB
#define LED1_J2_Pin GPIO_PIN_6
#define LED1_J2_GPIO_Port GPIOC
#define LED2_J2_Pin GPIO_PIN_7
#define LED2_J2_GPIO_Port GPIOC
#define LED3_J2_Pin GPIO_PIN_8
#define LED3_J2_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define seg_d_Pin GPIO_PIN_4
#define seg_d_GPIO_Port GPIOB
#define seg_e_Pin GPIO_PIN_5
#define seg_e_GPIO_Port GPIOB
#define seg_f_Pin GPIO_PIN_6
#define seg_f_GPIO_Port GPIOB
#define seg_g_Pin GPIO_PIN_7
#define seg_g_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
