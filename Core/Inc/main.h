/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
enum myntColors {
	FOREGROUND_BLACK,
	FOREGROUND_WHITE,
	FOREGROUND_RED,
	FOREGROUND_GREEN,
	FOREGROUND_BLUE,
	FOREGROUND_PURPLE,
	FOREGROUND_CYAN,
	FOREGROUND_YELLOW,
	FOREGROUND_RAINBOW = 255
};

enum myntStates {
	MYNT_BOOT,
	MYNT_STATIC,
	MYNT_BLINK,
	MYNT_ALERT,
	MYNT_RACE,
	MYNT_CHECKERS
};


/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void setPixelColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b);
void setPixelColorNumber(uint16_t p, uint8_t n);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PadA_Pin GPIO_PIN_0
#define PadA_GPIO_Port GPIOF
#define PadB_Pin GPIO_PIN_1
#define PadB_GPIO_Port GPIOF
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define PadUp_Pin GPIO_PIN_0
#define PadUp_GPIO_Port GPIOB
#define PadRight_Pin GPIO_PIN_1
#define PadRight_GPIO_Port GPIOB
#define PadDown_Pin GPIO_PIN_12
#define PadDown_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_15
#define VCP_RX_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_3
#define LED_GPIO_Port GPIOB
#define SPI1_MOSI_Pin GPIO_PIN_5
#define SPI1_MOSI_GPIO_Port GPIOB
#define PadLeft_Pin GPIO_PIN_6
#define PadLeft_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
