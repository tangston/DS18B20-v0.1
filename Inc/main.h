/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

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
#define KEY_GPIO_Pin0_Pin GPIO_PIN_0
#define KEY_GPIO_Pin0_GPIO_Port GPIOC
#define KEY_GPIO_Pin1_Pin GPIO_PIN_1
#define KEY_GPIO_Pin1_GPIO_Port GPIOC
#define KEY_GPIO_Pin2_Pin GPIO_PIN_2
#define KEY_GPIO_Pin2_GPIO_Port GPIOC
#define DS18B20_Pin GPIO_PIN_1
#define DS18B20_GPIO_Port GPIOA
#define LED_0_GPIO_Pin_Pin GPIO_PIN_8
#define LED_0_GPIO_Pin_GPIO_Port GPIOE
#define LED_1_GPIO_Pin_Pin GPIO_PIN_9
#define LED_1_GPIO_Pin_GPIO_Port GPIOE
#define LED_2_GPIO_Pin_Pin GPIO_PIN_10
#define LED_2_GPIO_Pin_GPIO_Port GPIOE
#define LED_3_GPIO_Pin_Pin GPIO_PIN_11
#define LED_3_GPIO_Pin_GPIO_Port GPIOE
#define LED_4_GPIO_Pin_Pin GPIO_PIN_12
#define LED_4_GPIO_Pin_GPIO_Port GPIOE
#define LED_5_GPIO_Pin_Pin GPIO_PIN_13
#define LED_5_GPIO_Pin_GPIO_Port GPIOE
#define LED_6_GPIO_Pin_Pin GPIO_PIN_14
#define LED_6_GPIO_Pin_GPIO_Port GPIOE
#define LED_7_GPIO_Pin_Pin GPIO_PIN_15
#define LED_7_GPIO_Pin_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
