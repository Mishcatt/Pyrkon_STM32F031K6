/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LEDS 144
#define FRAMEBUFFER 1296

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t framebuffer[1][FRAMEBUFFER];
volatile uint8_t currentFramebuffer = 0;
volatile uint8_t spiBusy = 0;
volatile uint8_t periodCounter = 99;

const uint8_t colors[5][3] = {
		{0, 0, 0},			// black
		{255, 255, 255},	// white
		{255, 0, 0},		// red
		{0, 255, 0},		// green
		{0, 0, 255}			// blue
};

uint8_t backgroundColor = 0;
uint8_t foregroundColor = 1;
uint8_t currentGraphic = 0;

int8_t carPlace[4] = {6, 3, 1, -1};
uint8_t carSide[4] = {0, 1, 0, 1};

const uint8_t carSprites[2][6] = {
		{8, 9, 12, 27, 28, 31},		// left car
		{6, 7, 14, 25, 26, 33}		// right car
};

const uint8_t digits[10][7] = {
		{78, 94, 95,  0, 113, 114, 116}, 	// 0
		{78,  0,  0, 97,   0,   0, 116}, 	// 1
		{78, 94,  0, 97,   0, 114, 116},	// 2
		{78,  0, 95, 97,   0, 114, 116},	// 3
		{78,  0,  0, 97, 113,   0, 116},	// 4
		{78,  0, 95, 97, 113,   0, 116},	// 5
		{78, 94, 95, 97, 113,   0, 116},	// 6
		{ 0,  0, 95,  0, 113, 114, 116},	// 7
		{78, 94, 95, 97, 113, 114, 116},	// 8
		{78,  0, 95, 97, 113, 114, 116}		// 9
};

const uint8_t graphics[5][18] = {
		{		// Appul
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b01010000, 0b00111100, 0b00011110, 0b00000011,
				0b10000011, 0b11000000, 0b01110000, 0b01111000,
				0b00011110, 0b00000101, 0b00000001, 0b00000000,
				0b00100000, 0b00000000
		},
		{		// OwO
				0b00000000, 0b00000011, 0b10000001, 0b11100000,
				0b10001000, 0b00100100, 0b00000000, 0b00000000,
				0b00000110, 0b00110010, 0b10010100, 0b00000000,
				0b01010010, 0b10011000, 0b11000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// dwd
				0b00000000, 0b00000011, 0b10000001, 0b11100000,
				0b10001000, 0b00100100, 0b00000000, 0b00000000,
				0b00000110, 0b00110010, 0b10010100, 0b00000000,
				0b01110011, 0b10000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// uwu
				0b00000000, 0b00000011, 0b10000001, 0b11100000,
				0b10001000, 0b00100100, 0b00000000, 0b00000000,
				0b00000110, 0b00110010, 0b10010100, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// track
				0b00011100, 0b00100000, 0b00000000, 0b10000100,
				0b00000000, 0b00010000, 0b10000000, 0b00000010,
				0b00010000, 0b00000000, 0b01000010, 0b00000000,
				0b00001000, 0b01000000, 0b00000001, 0b00001000,
				0b00000000, 0b11100001
		}

};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */

void setPixelColor(uint8_t p, uint8_t r, uint8_t g, uint8_t b);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */

  for (uint16_t i=0; i<FRAMEBUFFER; i+=9) {
	  framebuffer[currentFramebuffer][i+0] = 0b10010010; // G
	  framebuffer[currentFramebuffer][i+1] = 0b01001001;
	  framebuffer[currentFramebuffer][i+2] = 0b00100100;
	  framebuffer[currentFramebuffer][i+3] = 0b10010010; // R
	  framebuffer[currentFramebuffer][i+4] = 0b01001001;
	  framebuffer[currentFramebuffer][i+5] = 0b00100100;
	  framebuffer[currentFramebuffer][i+6] = 0b10010010; // B
	  framebuffer[currentFramebuffer][i+7] = 0b01001001;
	  framebuffer[currentFramebuffer][i+8] = 0b00100100;
  }

  HAL_TIM_Base_Start_IT(&htim14);

  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (spiBusy == 0 && periodCounter == 0) {
		  periodCounter = 9;
		  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		  //HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		  spiBusy = 1;

		  for (uint8_t i=0; i<LEDS; i++) {
			  if (graphics[currentGraphic][i/8] & (0x80 >> i%8)) {
				  setPixelColor(i, colors[foregroundColor][0], colors[foregroundColor][1], colors[foregroundColor][2]);
			  } else {
				  setPixelColor(i, colors[backgroundColor][0], colors[backgroundColor][1], colors[backgroundColor][2]);
			  }
		  }

		  if (currentGraphic == 4) {
			  for (uint8_t car=0; car<4; car++) {
				  for (uint8_t i=0; i<6; i++) {
					  if ((carSprites[carSide[car]][i]+(13*carPlace[car]) > 0) && (carSprites[carSide[car]][i]+(13*carPlace[car]) < 144)) {
						  setPixelColor((carSprites[carSide[car]][i]+(13*carPlace[car])), 255, 255, 255);
					  }
				  }
			  }
		  }

		  HAL_SPI_Transmit_DMA(&hspi1, framebuffer[currentFramebuffer], FRAMEBUFFER);
		  //HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	  }
	  //if (spiBusy == 3) HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL5;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 200;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 1000;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PadUp_Pin PadDown_Pin */
  GPIO_InitStruct.Pin = PadUp_Pin|PadDown_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PadRight_Pin PadLeft_Pin PadB_Pin PadA_Pin */
  GPIO_InitStruct.Pin = PadRight_Pin|PadLeft_Pin|PadB_Pin|PadA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

inline void setPixelColor(uint8_t p, uint8_t r, uint8_t g, uint8_t b) {

	uint32_t er = 0;
	er |= ((r&0b10000000)<<15);
	er |= ((r&0b01000000)<<13);
	er |= ((r&0b00100000)<<11);
	er |= ((r&0b00010000)<<9);
	er |= ((r&0b00001000)<<7);
	er |= ((r&0b00000100)<<5);
	er |= ((r&0b00000010)<<3);
	er |= ((r&0b00000001)<<1);

	uint32_t eg = 0;
	eg |= ((g&0b10000000)<<15);
	eg |= ((g&0b01000000)<<13);
	eg |= ((g&0b00100000)<<11);
	eg |= ((g&0b00010000)<<9);
	eg |= ((g&0b00001000)<<7);
	eg |= ((g&0b00000100)<<5);
	eg |= ((g&0b00000010)<<3);
	eg |= ((g&0b00000001)<<1);

	uint32_t eb = 0;
	eb |= ((b&0b10000000)<<15);
	eb |= ((b&0b01000000)<<13);
	eb |= ((b&0b00100000)<<11);
	eb |= ((b&0b00010000)<<9);
	eb |= ((b&0b00001000)<<7);
	eb |= ((b&0b00000100)<<5);
	eb |= ((b&0b00000010)<<3);
	eb |= ((b&0b00000001)<<1);

	uint16_t i = p*9;
	framebuffer[currentFramebuffer][i+0] = 0b10010010 | (eg>>16);
	framebuffer[currentFramebuffer][i+1] = 0b01001001 | (eg>>8);
	framebuffer[currentFramebuffer][i+2] = 0b00100100 | (eg);
	framebuffer[currentFramebuffer][i+3] = 0b10010010 | (er>>16);
	framebuffer[currentFramebuffer][i+4] = 0b01001001 | (er>>8);
	framebuffer[currentFramebuffer][i+5] = 0b00100100 | (er);
	framebuffer[currentFramebuffer][i+6] = 0b10010010 | (eb>>16);
	framebuffer[currentFramebuffer][i+7] = 0b01001001 | (eb>>8);
	framebuffer[currentFramebuffer][i+8] = 0b00100100 | (eb);

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	spiBusy = 0;
	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (periodCounter) periodCounter--;
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
