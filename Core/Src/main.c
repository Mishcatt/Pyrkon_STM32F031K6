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
#include "checkers.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define LEDS (144)
#define FRAMEBUFFER (1296)
#define LEDPACKET (FRAMEBUFFER + 64)
#define BUTTONHISTORY (19)
#define CAR_TROPHY_SIZE (30)
#define CAR_TRACK_SIZE (13)
#define CAR_TRACK_DELAY (15)

#define COLOR_BLACK {0, 0, 0}
#define COLOR_RED {64, 0, 0}
#define COLOR_ORANGE {64, 16, 0}
#define COLOR_YELLOW {64, 64, 0}
#define COLOR_GREEN {0, 32, 0}
#define COLOR_BLUE {0, 0, 64}
#define COLOR_PURPLE {32, 0, 32}
#define ALL_COLORS_COUNT (8)

enum carStates {
	CAR_NORMAL,
	CAR_EXPLODE,
	CAR_SHOOT,
	CAR_WINNER
};

enum buttons {
	BUTTON_NONE = 0b00000000,
	BUTTON_B = 0b00000001,
	BUTTON_A = 0b00000010,
	BUTTON_RIGHT = 0b00000100,
	BUTTON_LEFT = 0b00001000,
	BUTTON_DOWN = 0b00010000,
	BUTTON_UP = 0b00100000
};

enum myntGraphics {
	MYNT_GRAPHIC_APPLE,
	MYNT_GRAPHIC_OWO,
	MYNT_GRAPHIC_BLINK1,
	MYNT_GRAPHIC_BLINK2,
	MYNT_GRAPHIC_BLINK3,
	MYNT_GRAPHIC_UWU,
	MYNT_GRAPHIC_TRACK,
	MYNT_GRAPHIC_HAPPY,
	MYNT_GRAPHIC_OK,
	MYNT_GRAPHIC_NOK,
	MYNT_GRAPHIC_HEART,
	MYNT_GRAPHIC_BLUSH,
	MYNT_GRAPHIC_STARS,
	MYNT_GRAPHIC_BITMAP,
	MYNT_GRAPHIC_QUESTION,
	MYNT_GRAPHIC_TRIOPTIMUM,
	MYNT_GRAPHIC_P03,
	MYNT_GRAPHIC_CHECKERS
};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t framebuffer[LEDPACKET];
volatile uint8_t spiBusy = 0;
volatile uint8_t msCounter = 0;

const uint8_t colors[8][3] = {
		{0, 0, 0},			// black
		{128, 128, 128},	// white
		{128, 0, 0},		// red
		{0, 128, 0},		// green
		{0, 0, 128},		// blue
		{128, 0, 128},		// purple
		{0, 128, 128},		// cyan
		{128, 128, 0}		// yellow
};

const uint8_t lineNumber[16] = {
		1, 11, 20, 30, 39, 49, 58, 68, 77, 87, 96, 106, 115, 125, 134, 144
};

uint8_t buttonState[6];
uint8_t tempButton = 0;
uint8_t buttonHistory[BUTTONHISTORY];
uint8_t heldButton = 0;
uint32_t buttonTime = 0;

uint32_t adcBuffer[1] = {0};
uint32_t randomNumber = 0;
uint8_t randomBits = 0;

uint8_t brightness = 2;
uint8_t currentForegroundColor[3] = {0, 0, 0};
uint8_t currentBackgroundColor[3] = {0, 0, 0};
uint8_t allColorsBrightness[ALL_COLORS_COUNT][3] = {
		{0, 0, 0},			// black
		{128, 128, 128},	// white
		{128, 0, 0},		// red
		{0, 128, 0},		// green
		{0, 0, 128},		// blue
		{128, 0, 128},		// purple
		{0, 128, 128},		// cyan
		{128, 128, 0}		// yellow
};
uint8_t myntState = MYNT_BOOT;
uint8_t backgroundColor = 0;
uint8_t foregroundColor = FOREGROUND_WHITE;
uint8_t previousColor = 1;
uint8_t currentGraphic = MYNT_GRAPHIC_APPLE;
uint8_t currentBitmap = 1;
uint8_t glitchFlag = 0;
uint8_t lineCount = 0;
uint8_t lineNoise = 0;
uint8_t phaseGlitch = 0;

uint8_t carState = CAR_NORMAL;
uint8_t carAnim = 0;
uint8_t curbAnim = 0;
uint8_t carExplosionAnim = 0;
uint8_t carSpeed = 24;
int8_t carPlace[7] = {0, 24, 18, 12, 26, 22, 10};
uint8_t carSide[7] = {1, 0, 0, 1, 1, 2, 2};
uint8_t carColors[7] = {1, 2, 3, 4, 5, 6, 7};
int8_t curbAnimation = 0;
uint8_t carExplosionFrame = 0;
uint8_t carExplosionRepeat = 0;
uint8_t carExplosionDelay = 0;
uint8_t carCurrentTrack = 0;
uint8_t carTrackDelay = 0;
uint8_t carCheats = CAR_NORMAL;
uint8_t konamiCode = 0;

extern volatile uint8_t checkersState;
extern volatile uint8_t checkersBoard[8][8];
extern volatile uint8_t checkersMoveCount;
extern volatile uint8_t checkersMoveList[24][5];
extern volatile uint8_t checkersCaptureCount;
volatile uint8_t checkersCurrentMove = 0;

uint32_t blinkDelay = 0;
uint32_t blinkAnim = 0;
uint8_t blinkState = 0;

const uint8_t carSprites[][6] = {
		{8, 9, 12, 27, 28, 31},		// left car
		{6, 7, 14, 25, 26, 33},		// middle car
		{4, 5, 16, 23, 24, 35},		// right car
		{8, 9, 27, 28, 30, 32},		// left car explosion 1
		{6, 7, 25, 26, 32, 34},		// middle car explosion 1
		{4, 5, 23, 24, 34, 36},		// right car explosion 1
		{11, 12, 13, 31, 31, 31},	// left car explosion 2
		{13, 14, 15, 33, 33, 33},	// middle car explosion 2
		{15, 16, 17, 35, 35, 35},	// right car explosion 2
		{8, 9, 27, 28, 28, 28},		// left car explosion 3
		{6, 7, 25, 26, 26, 26},		// middle car explosion 3
		{4, 5, 23, 24, 24, 24},		// right car explosion 3
		{12, 12, 12, 12, 12, 12},	// left car explosion 4
		{14, 14, 14, 14, 14, 14},	// middle car explosion 4
		{16, 16, 16, 16, 16, 16},	// right car explosion 4
		{0, 0, 0, 0, 0, 0},			// left car explosion 5
		{0, 0, 0, 0, 0, 0},			// middle car explosion 5
		{0, 0, 0, 0, 0, 0}			// right car explosion 5
};

const uint8_t carTrophy[CAR_TROPHY_SIZE] = {
		128, 121, 120, 107, 108, 109, 110, 111,
		103, 102, 101, 100, 89, 90, 91, 83,
		82, 71, 64, 63, 52, 45, 44, 32,
		33, 34, 27, 26, 25, 24
};

const uint8_t carTrack[CAR_TRACK_SIZE] = {
		19, 21, 38, 39, 57, 59, 76, 77,
		95, 97, 114, 115, 133
};

const uint8_t checkersBoardPixels[8][8] = {
		{31, 200, 50, 200, 69, 200, 88, 200},
		{200, 46, 200, 65, 200, 84, 200, 103},
		{32, 200, 51, 200, 70, 200, 89, 200},
		{200, 45, 200, 64, 200, 83, 200, 102},
		{33, 200, 52, 200, 71, 200, 90, 200},
		{200, 44, 200, 63, 200, 82, 200, 101},
		{34, 200, 53, 200, 72, 200, 91, 200},
		{200, 43, 200, 62, 200, 81, 200, 100}
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

const uint8_t graphics[][18] = {
		{		// MYNT_GRAPHIC_APPLE
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b01010000, 0b00111100, 0b00011110, 0b00000011,
				0b10000011, 0b11000000, 0b01110000, 0b01111000,
				0b00011110, 0b00000101, 0b00000001, 0b00000000,
				0b00100000, 0b00000000
		},
		{		// MYNT_GRAPHIC_OWO
				0b00000000, 0b00000010, 0b10000001, 0b11100000,
				0b10101000, 0b00100100, 0b00000000, 0b00000000,
				0b00000110, 0b00110010, 0b10010100, 0b00000000,
				0b01010010, 0b10011000, 0b11000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_BLINK1
				0b00000000, 0b00000010, 0b10000001, 0b11100000,
				0b10101000, 0b00100100, 0b00000000, 0b00000000,
				0b00000110, 0b00110010, 0b10010100, 0b00000000,
				0b01110011, 0b10000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_BLINK2
				0b00000000, 0b00000010, 0b10000001, 0b11100000,
				0b10101000, 0b00100100, 0b00000000, 0b00000000,
				0b00000000, 0b00000011, 0b10011100, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_BLINK3
				0b00000000, 0b00000010, 0b10000001, 0b11100000,
				0b10101000, 0b00100100, 0b00000000, 0b00000000,
				0b00000110, 0b00110010, 0b10010100, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_UWU
				0b00000000, 0b00000010, 0b10000001, 0b11100000,
				0b10101000, 0b00100100, 0b00000000, 0b00000000,
				0b00000110, 0b00110010, 0b10010100, 0b00000000,
				0b01010010, 0b10000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_TRACK
				0b01100000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000011, 0b00000000
		},
		{		// MYNT_GRAPHIC_HAPPY
				0b00000000, 0b00000010, 0b10000001, 0b11100000,
				0b10101000, 0b00100100, 0b00000000, 0b00000000,
				0b00000000, 0b00000010, 0b10010100, 0b11000110,
				0b00100001, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_OK
				0b00000000, 0b00000000, 0b00000000, 0b01000000,
				0b01100000, 0b00011100, 0b00011110, 0b00000110,
				0b11000010, 0b01100001, 0b10000000, 0b00000110,
				0b01100000, 0b00000000, 0b01000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_NOK
				0b00000000, 0b00000000, 0b00000010, 0b00010001,
				0b10001100, 0b01100110, 0b00011011, 0b00000111,
				0b10000001, 0b11000000, 0b11110000, 0b01101100,
				0b00110011, 0b00011000, 0b11000100, 0b00100000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_HEART
				0b00000000, 0b00000001, 0b00000000, 0b11000000,
				0b01110000, 0b00111100, 0b00011111, 0b00001111,
				0b11000111, 0b11110011, 0b11111100, 0b11111110,
				0b01111111, 0b10011101, 0b11000110, 0b01100000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_BLUSH
				0b00000000, 0b00000010, 0b10000001, 0b11100000,
				0b10101000, 0b00100100, 0b00000000, 0b00000000,
				0b00000100, 0b00010001, 0b00001000, 0b01000100,
				0b00100001, 0b00010000, 0b01000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_STARS
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00010101, 0b00000000,
				0b00000000, 0b00000000, 0b00000001, 0b01010101,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_BITMAP
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_QUESTION
				0b00000000, 0b00000001, 0b00000000, 0b00000000,
				0b00000000, 0b00000000, 0b00000100, 0b00000000,
				0b00000000, 0b10000000, 0b01000000, 0b00001000,
				0b00000000, 0b00000101, 0b00000001, 0b10000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_TRIOPTIMUM
				0b00000000, 0b00000000, 0b00000000, 0b11000000,
				0b01010000, 0b00100100, 0b00011011, 0b00001111,
				0b11000111, 0b01110011, 0b11111100, 0b00010000,
				0b01001100, 0b10010111, 0b01000111, 0b11100000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_P03
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b01000000, 0b00111000, 0b00000000, 0b00000000,
				0b00000000, 0b00000001, 0b10011000, 0b00000000,
				0b00000000, 0b00000000, 0b00000000, 0b00000000,
				0b00000000, 0b00000000
		},
		{		// MYNT_GRAPHIC_CHECKERS
				0b00000000, 0b00011111, 0b11000000, 0b00000100,
				0b00001000, 0b00000000, 0b10000001, 0b00000000,
				0b00010000, 0b00100000, 0b00000010, 0b00000100,
				0b00000000, 0b01000000, 0b10000011, 0b11111000,
				0b00000000, 0b00000000
		}

};

const uint8_t bitmaps[2][144][3] = {
	{
		COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK,
		COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE,
		COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE,
		COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE,
		COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE,
		COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN,
		COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN,
		COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW,
		COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW,
		COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE,
		COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE, COLOR_ORANGE,
		COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED,
		COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED, COLOR_RED,
		COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK,
		COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK
	},
	{
		{0, 0, 0}, {1, 2, 0}, {1, 1, 0}, {2, 3, 0}, {13, 15, 1}, {52, 59, 11}, {45, 52, 6}, {35, 33, 10}, {4, 4, 1}, {1, 1, 0}, {1, 1, 0},
		{1, 1, 0}, {4, 4, 0}, {27, 27, 5}, {46, 52, 7}, {59, 65, 10}, {49, 57, 8}, {7, 9, 2}, {3, 4, 0}, {0, 0, 0},
		{0, 0, 0}, {0, 0, 0}, {6, 8, 0}, {38, 41, 9}, {59, 64, 13}, {56, 63, 12}, {42, 48, 5}, {6, 8, 0}, {1, 2, 0}, {0, 0, 0},
		{2, 2, 1}, {1, 2, 0}, {30, 36, 7}, {15, 15, 0}, {7, 7, 1}, {15, 15, 2}, {36, 30, 7}, {3, 4, 0}, {0, 0, 0},
		{0, 0, 0}, {0, 0, 0}, {6, 7, 1}, {46, 49, 14}, {59, 63, 11}, {56, 60, 6}, {36, 35, 7}, {12, 15, 2}, {4, 5, 2}, {0, 0, 0},
		{1, 2, 0}, {11, 16, 2}, {34, 37, 7}, {94, 71, 7}, {90, 71, 5}, {87, 66, 6}, {24, 28, 4}, {2, 3, 1}, {0, 0, 0},
		{0, 0, 0}, {1, 2, 0}, {14, 18, 5}, {59, 47, 6}, {99, 68, 18}, {97, 66, 18}, {76, 53, 9}, {45, 57, 7}, {4, 6, 0}, {0, 0, 0},
		{0, 0, 0}, {18, 24, 3}, {46, 48, 15}, {31, 31, 127}, {88, 71, 13}, {31, 31, 127}, {38, 43, 10}, {1, 1, 0}, {0, 0, 0},
		{0, 0, 0}, {0, 0, 0}, {10, 12, 3}, {54, 53, 11}, {99, 84, 16}, {90, 77, 15}, {59, 54, 11}, {50, 53, 22}, {0, 0, 0}, {0, 0, 0},
		{0, 0, 0}, {6, 7, 3}, {42, 47, 9}, {83, 74, 11}, {120, 101, 13}, {87, 76, 9}, {39, 43, 10}, {17, 16, 3}, {0, 1, 0},
		{0, 0, 0}, {19, 19, 4}, {32, 31, 4}, {52, 54, 9}, {103, 91, 10}, {108, 92, 11}, {47, 46, 8}, {40, 45, 9}, {5, 6, 1}, {0, 1, 0},
		{13, 16, 1}, {18, 19, 2}, {56, 61, 20}, {63, 59, 9}, {100, 87, 8}, {71, 66, 13}, {39, 44, 11}, {2, 2, 0}, {1, 0, 0},
		{0, 0, 0}, {0, 0, 0}, {18, 19, 11}, {48, 54, 15}, {64, 69, 11}, {57, 57, 7}, {45, 48, 14}, {17, 20, 7}, {6, 7, 0}, {37, 43, 2},
		{21, 24, 4}, {17, 18, 7}, {21, 25, 5}, {53, 59, 13}, {59, 68, 12}, {64, 70, 11}, {42, 43, 12}, {10, 11, 3}, {0, 0, 0},
		{0, 0, 0}, {4, 4, 1}, {31, 30, 10}, {42, 46, 12}, {48, 55, 10}, {51, 58, 19}, {24, 27, 12}, {6, 7, 2}, {1, 1, 0}, {0, 0, 0}
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
static void MX_ADC_Init(void);
/* USER CODE BEGIN PFP */

void setPixelColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b);
uint32_t readPixelColor(uint16_t p);
void addPixelColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
void SPI_SetOpenDrain();

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
  MX_ADC_Init();
  /* USER CODE BEGIN 2 */

  for (uint16_t i=0; i<FRAMEBUFFER; i+=9) {
	  framebuffer[i+0] = 0b00100100; // G
	  framebuffer[i+1] = 0b10010010;
	  framebuffer[i+2] = 0b01001001;
	  framebuffer[i+3] = 0b00100100; // R
	  framebuffer[i+4] = 0b10010010;
	  framebuffer[i+5] = 0b01001001;
	  framebuffer[i+6] = 0b00100100; // B
	  framebuffer[i+7] = 0b10010010;
	  framebuffer[i+8] = 0b01001001;
  }

  HAL_TIM_Base_Start_IT(&htim14);
  HAL_ADC_Start_DMA(&hadc, adcBuffer, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if (HAL_GPIO_ReadPin(PadUp_GPIO_Port, PadUp_Pin) == GPIO_PIN_RESET) {
		  if (buttonState[0] < 255) buttonState[0]++;
	  }	else {
		  if (buttonState[0] > 0) buttonState[0]--;
	  }
	  if (HAL_GPIO_ReadPin(PadDown_GPIO_Port, PadDown_Pin) == GPIO_PIN_RESET) {
		  if (buttonState[1] < 255) buttonState[1]++;
	  }	else {
		  if (buttonState[1] > 0) buttonState[1]--;
	  }
	  if (HAL_GPIO_ReadPin(PadLeft_GPIO_Port, PadLeft_Pin) == GPIO_PIN_RESET) {
		  if (buttonState[2] < 255) buttonState[2]++;
	  }	else {
		  if (buttonState[2] > 0) buttonState[2]--;
	  }
	  if (HAL_GPIO_ReadPin(PadRight_GPIO_Port, PadRight_Pin) == GPIO_PIN_RESET) {
		  if (buttonState[3] < 255) buttonState[3]++;
	  }	else {
		  if (buttonState[3] > 0) buttonState[3]--;
	  }
	  if (HAL_GPIO_ReadPin(PadA_GPIO_Port, PadA_Pin) == GPIO_PIN_RESET) {
		  if (buttonState[4] < 255) buttonState[4]++;
	  }	else {
		  if (buttonState[4] > 0) buttonState[4]--;
	  }
	  if (HAL_GPIO_ReadPin(PadB_GPIO_Port, PadB_Pin) == GPIO_PIN_RESET) {
		  if (buttonState[5] < 255) buttonState[5]++;
	  }	else {
		  if (buttonState[5] > 0) buttonState[5]--;
	  }

	  tempButton = BUTTON_NONE;
	  if (buttonState[0] > 127) { // Up
		  tempButton |= BUTTON_UP;
	  }
	  if (buttonState[1] > 127) { // Down
		  tempButton |= BUTTON_DOWN;
	  }
	  if (buttonState[2] > 127) { // Left
		  tempButton |= BUTTON_LEFT;
	  }
	  if (buttonState[3] > 127) { // Right
		  tempButton |= BUTTON_RIGHT;
	  }
	  if (buttonState[4] > 127) { // A
		  tempButton |= BUTTON_A;
	  }
	  if (buttonState[5] > 127) { // B
		  tempButton |= BUTTON_B;
	  }
	  if (buttonHistory[0] != tempButton) {
		  for (uint8_t i=(BUTTONHISTORY-1); i>0; i--) {
			  buttonHistory[i] = buttonHistory[i-1];
		  }
		  buttonHistory[0] = tempButton;
	  }
	  if (buttonHistory[18] == BUTTON_UP && buttonHistory[17] == BUTTON_NONE &&
		  buttonHistory[16] == BUTTON_UP && buttonHistory[15] == BUTTON_NONE &&
		  buttonHistory[14] == BUTTON_DOWN && buttonHistory[13] == BUTTON_NONE &&
		  buttonHistory[12] == BUTTON_DOWN && buttonHistory[11] == BUTTON_NONE &&
		  buttonHistory[10] == BUTTON_LEFT && buttonHistory[9] == BUTTON_NONE &&
		  buttonHistory[8] == BUTTON_RIGHT && buttonHistory[7] == BUTTON_NONE &&
		  buttonHistory[6] == BUTTON_LEFT && buttonHistory[5] == BUTTON_NONE &&
		  buttonHistory[4] == BUTTON_RIGHT && buttonHistory[3] == BUTTON_NONE &&
		  buttonHistory[2] == BUTTON_B && buttonHistory[1] == BUTTON_NONE &&
		  buttonHistory[0] == BUTTON_A) {
		  konamiCode ^= 1;
		  buttonHistory[18] = BUTTON_NONE;
		  //currentGraphic = 6;
	  }

	  if (msCounter) {
		  msCounter--;
		  if (currentGraphic == MYNT_GRAPHIC_TRACK) {
			  carAnim++;
			  if (carExplosionFrame > 0) {
				  carExplosionAnim++;
			  }
			  if (carState != CAR_EXPLODE) {
				  curbAnim++;
			  }
		  } else {
			  blinkAnim++;
		  }
		  if (heldButton == tempButton) {
			  buttonTime++;
		  } else {
			  buttonTime = 0;
			  heldButton = tempButton;
			  if (myntState == MYNT_RACE) {
				  if (tempButton & BUTTON_A) {
					  carSpeed = 16 - carCurrentTrack;
				  } else {
					  carSpeed = 24 - carCurrentTrack;
				  }
				  if ((tempButton & BUTTON_B) && (carState == CAR_WINNER)) {
					  carState = CAR_NORMAL;
					  konamiCode = 0;
					  carCurrentTrack = 0;
					  carTrackDelay = 0;
				  }
				  if (carState != CAR_EXPLODE) {
					  if (tempButton & BUTTON_LEFT) {
						  if (carSide[0] > 0) carSide[0]--;
					  }
					  if (tempButton & BUTTON_RIGHT) {
						  if (carSide[0] < 2) carSide[0]++;
					  }
				  }
			  }
			  else if (myntState == MYNT_CHECKERS) {
				  if (tempButton & BUTTON_LEFT) {
					  if (checkersCurrentMove > 0) {
						  checkersCurrentMove--;
					  } else {
						  if (checkersCaptureCount > 0) {
							  checkersCurrentMove = checkersCaptureCount - 1;
						  } else {
							  checkersCurrentMove = checkersMoveCount - 1;
						  }
					  }
				  }
				  if (tempButton & BUTTON_RIGHT) {
					  if (checkersCurrentMove < checkersMoveCount) {
						  checkersCurrentMove++;
					  } else {
						  checkersCurrentMove = 0;
					  }
				  }
				  if (tempButton & BUTTON_A) {
					  uint8_t x = checkersMoveList[checkersCurrentMove][0];
					  uint8_t y = checkersMoveList[checkersCurrentMove][1];
					  uint8_t nx = checkersMoveList[checkersCurrentMove][2];
					  uint8_t ny = checkersMoveList[checkersCurrentMove][3];
					  uint8_t nnx = 0;
					  uint8_t nny = 0;
					  uint8_t queen = checkersBoard[x][y] & CHECKERS_QUEEN_FLAG;
					  checkersBoard[x][y] = CHECKERS_BOARD_EMPTY;
					  if (nx > x) {
						  if ((nx - x) > 1) {
							  nnx = x + 1;
						  }
					  } else {
						  if ((x - nx) > 1) {
							  nnx = nx + 1;
						  }
					  }
					  if (ny > y) {
						  if ((ny - y) > 1) {
							  nny = y + 1;
							  checkersBoard[nnx][nny] = CHECKERS_BOARD_EMPTY;
						  }
					  } else {
						  if ((y - ny) > 1) {
							  nny = ny + 1;
							  checkersBoard[nnx][nny] = CHECKERS_BOARD_EMPTY;
						  }
					  }
					  if (checkersState == CHECKERS_STATE_PLAY) {
						  checkersBoard[nx][ny] = CHECKERS_BOARD_WHITE | queen;
						  if (ny == 7) checkersBoard[nx][ny] |= CHECKERS_QUEEN_FLAG;
						  checkersState = CHECKERS_STATE_RESPONSE;
					  }
					  else if (checkersState == CHECKERS_STATE_RESPONSE) {
						  checkersBoard[nx][ny] = CHECKERS_BOARD_BLACK | queen;
						  if (ny == 0) checkersBoard[nx][ny] |= CHECKERS_QUEEN_FLAG;
						  checkersState = CHECKERS_STATE_PLAY;
					  }
					  else {
						  checkersState = CHECKERS_STATE_START;
					  }
				  }
			  }
			  else {
				  if (tempButton == (BUTTON_A | BUTTON_UP)) {
					  currentGraphic = MYNT_GRAPHIC_OK;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_GREEN;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == (BUTTON_A | BUTTON_DOWN)) {
					  currentGraphic = MYNT_GRAPHIC_NOK;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_RED;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == (BUTTON_A | BUTTON_LEFT)) {
					  currentGraphic = MYNT_GRAPHIC_HEART;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_PURPLE;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == (BUTTON_A | BUTTON_RIGHT)) {
					  currentGraphic = MYNT_GRAPHIC_QUESTION;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_YELLOW;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == (BUTTON_A | BUTTON_B | BUTTON_LEFT)) {
					  currentGraphic = MYNT_GRAPHIC_P03;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_CYAN;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == (BUTTON_A | BUTTON_B | BUTTON_UP)) {
					  currentGraphic = MYNT_GRAPHIC_TRIOPTIMUM;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_GREEN;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == (BUTTON_A | BUTTON_B | BUTTON_DOWN)) {
					  currentGraphic = MYNT_GRAPHIC_APPLE;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_WHITE;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == (BUTTON_A | BUTTON_B | BUTTON_RIGHT)) {
					  currentGraphic = MYNT_GRAPHIC_BITMAP;
					  if (myntState != MYNT_ALERT) previousColor = foregroundColor;
					  foregroundColor = FOREGROUND_WHITE;
					  myntState = MYNT_ALERT;
				  }
				  if (tempButton == BUTTON_LEFT) {
					  currentGraphic = MYNT_GRAPHIC_OWO;
					  blinkDelay = 0;
					  blinkAnim = 0;
					  if (myntState == MYNT_ALERT) foregroundColor = previousColor;
					  myntState = MYNT_BLINK;
				  }
				  if (tempButton == BUTTON_RIGHT) {
					  currentGraphic = MYNT_GRAPHIC_BLUSH;
					  if (myntState == MYNT_ALERT) foregroundColor = previousColor;
					  myntState = MYNT_STATIC;
				  }
				  if (tempButton == BUTTON_UP) {
					  currentGraphic = MYNT_GRAPHIC_HAPPY;
					  if (myntState == MYNT_ALERT) foregroundColor = previousColor;
					  myntState = MYNT_STATIC;
				  }
				  if (tempButton == BUTTON_DOWN) {
					  currentGraphic = MYNT_GRAPHIC_UWU;
					  if (myntState == MYNT_ALERT) foregroundColor = previousColor;
					  myntState = MYNT_STATIC;
				  }
			  }
			  if (tempButton == (BUTTON_B | BUTTON_RIGHT)) {
				  foregroundColor++;
				  if (foregroundColor > 7) foregroundColor = 1;
			  }
			  if (tempButton == (BUTTON_B | BUTTON_LEFT)) {
				  glitchFlag ^= 1;
			  }
			  if (tempButton == (BUTTON_B | BUTTON_UP)) {
				  if (brightness > 0) brightness--;
			  }
			  if (tempButton == (BUTTON_B | BUTTON_DOWN)) {
				  if (brightness < 7) brightness++;
			  }


		  }
		  if (buttonTime > 100) {
			  if (tempButton == (BUTTON_UP | BUTTON_DOWN |BUTTON_LEFT | BUTTON_RIGHT | BUTTON_B)) {
				  currentGraphic = MYNT_GRAPHIC_APPLE;
				  myntState = MYNT_STATIC;
			  }
			  if (tempButton == (BUTTON_UP | BUTTON_DOWN | BUTTON_B)) {
				  currentGraphic = MYNT_GRAPHIC_OWO;
				  myntState = MYNT_BLINK;
			  }
			  if (tempButton == (BUTTON_LEFT | BUTTON_RIGHT | BUTTON_B)) {
				  currentGraphic = MYNT_GRAPHIC_TRACK;
				  foregroundColor = FOREGROUND_WHITE;
				  myntState = MYNT_RACE;
				  carState = CAR_NORMAL;
				  konamiCode = 0;
				  carExplosionDelay = 0;
				  carExplosionFrame = 0;
				  carExplosionRepeat = 0;
				  carSide[0] = 1;
				  carPlace[1] = 24;
				  carPlace[2] = 18;
				  carPlace[3] = 12;
				  carPlace[4] = 26;
				  carPlace[5] = 22;
				  carPlace[6] = 10;
				  carCurrentTrack = 0;
				  carTrackDelay = 0;
				  carColors[0] = 1;
				  carSpeed = 24;
				  curbAnimation = 0;
			  }
			  if (tempButton == (BUTTON_UP | BUTTON_LEFT | BUTTON_B)) {
				  currentGraphic = MYNT_GRAPHIC_CHECKERS;
				  myntState = MYNT_CHECKERS;
				  foregroundColor = FOREGROUND_WHITE;
				  checkersState = CHECKERS_STATE_START;
			  }
		  }
	  }

	  if (spiBusy == 0) {
		  for (uint8_t i=0; i<8; i++) {
			  allColorsBrightness[i][0] = colors[i][0] >> brightness;
			  allColorsBrightness[i][1] = colors[i][1] >> brightness;
			  allColorsBrightness[i][2] = colors[i][2] >> brightness;
		  }

		  currentForegroundColor[0] = allColorsBrightness[foregroundColor][0];
		  currentForegroundColor[1] = allColorsBrightness[foregroundColor][1];
		  currentForegroundColor[2] = allColorsBrightness[foregroundColor][2];
		  currentBackgroundColor[0] = allColorsBrightness[backgroundColor][0];
		  currentBackgroundColor[1] = allColorsBrightness[backgroundColor][1];
		  currentBackgroundColor[2] = allColorsBrightness[backgroundColor][2];

		  if (currentGraphic == MYNT_GRAPHIC_BITMAP) {
			  for (uint8_t i=0; i<LEDS; i++) { // Rysuj piksele z wybranej bitmapy
				  setPixelColor(i+(phaseGlitch), bitmaps[currentBitmap][i][0]>>brightness, bitmaps[currentBitmap][i][1]>>brightness, bitmaps[currentBitmap][i][2]>>brightness);
			  }
		  } else {
			  for (uint8_t i=0; i<LEDS; i++) { // Rysuj piksele z wybranej grafiki
				  if (graphics[currentGraphic][i/8] & (0x80 >> i%8)) {
					  setPixelColor(i+(phaseGlitch), currentForegroundColor[0], currentForegroundColor[1], currentForegroundColor[2]);
				  } else {
					  setPixelColor(i+(phaseGlitch), currentBackgroundColor[0], currentBackgroundColor[1], currentBackgroundColor[2]);
					  //setPixelColor(i, colors[blinkAnim%8][0], colors[blinkAnim%8][1], colors[blinkAnim%8][2]);
				  }
			  }
		  }

		  if (myntState == MYNT_RACE) { // Dodatkowe grafiki samochodów
			  if (curbAnim >= (carSpeed>>(1+konamiCode))) {
				  curbAnim -= (carSpeed>>(1+konamiCode));
				  curbAnimation--;
				  if (curbAnimation < 0) curbAnimation = 3;
			  }

			  if (carState == CAR_NORMAL) {
				  for (uint8_t car=1; car<7; car++) {
					  if (carSide[car] == carSide[0]) {
						  if (carPlace[car] < 2 && carPlace[car] > -2) {
							  if (konamiCode == 0) {
								  carState = CAR_EXPLODE;
								  carColors[0] = 2;
							  }
							  if (carExplosionFrame == 0) carExplosionFrame = 3;
						  }
					  }
				  }
			  }

			  if (carExplosionFrame > 0) {
				  if (carExplosionAnim > 4) {
					  carExplosionAnim -= 5;
					  carExplosionFrame += 3;
					  if (carExplosionRepeat < 5 && carExplosionFrame > 9) {
						  carExplosionRepeat++;
						  carExplosionFrame = 3;
					  }
					  if (carExplosionFrame > 15) {
						  if (carState == CAR_EXPLODE) {
							  if (carExplosionDelay < 10) {
								  carExplosionDelay++;
								  carExplosionFrame = 15;
							  } else {
								  carExplosionDelay = 0;
								  carExplosionFrame = 0;
								  carExplosionRepeat = 0;
								  carSide[0] = 1;
								  carPlace[1] = 24;
								  carPlace[2] = 18;
								  carPlace[3] = 12;
								  carPlace[4] = 26;
								  carPlace[5] = 22;
								  carPlace[6] = 10;
								  carState = CAR_NORMAL;
								  carCurrentTrack = 0;
								  carTrackDelay = 0;
								  carColors[0] = 1;
								  carSpeed = 24;
								  curbAnimation = 0;
							  }
						  } else {
							  carExplosionFrame = 0;
							  carExplosionRepeat = 0;
						  }
					  }
				  }
			  }

			  if (carAnim >= (carSpeed>>konamiCode)) {
				  carAnim -= (carSpeed>>konamiCode);

				  if (carState == CAR_NORMAL) carTrackDelay++;
				  if (carTrackDelay >= CAR_TRACK_DELAY) {
					  carTrackDelay -= CAR_TRACK_DELAY;
					  if (carCurrentTrack < (CAR_TRACK_SIZE-1)) {
						  carCurrentTrack++;
					  } else {
						  carExplosionDelay = 0;
						  carExplosionFrame = 0;
						  carExplosionRepeat = 0;
						  carSide[0] = 1;
						  carPlace[1] = 24;
						  carPlace[2] = 18;
						  carPlace[3] = 12;
						  carPlace[4] = 26;
						  carPlace[5] = 22;
						  carPlace[6] = 10;
						  carCurrentTrack = 0;
						  carTrackDelay = 0;
						  carColors[0] = 1;
						  carSpeed = 24;
						  curbAnimation = 0;
						  carCurrentTrack = CAR_TRACK_SIZE;
						  carState = CAR_WINNER;
					  }
				  }


				  if (carState == CAR_NORMAL) {
					  for (uint8_t car=1; car<7; car++) {
						  carPlace[car]--;
						  if (carPlace[car] < -2) {
							  carColors[car] = 2+(adcBuffer[0]%6);
							  carPlace[car] = 10+(adcBuffer[0]&0x7)+carSide[car];
							  for (uint8_t i=1; i<7; i++) {
								  if (i == car) continue;
								  if ( (carPlace[car] <= (carPlace[i]+2)) && (carPlace[car] >= (carPlace[i]-2)) ) {
									  carPlace[car] = (carPlace[i]+3) + (adcBuffer[0]&0x3);
								  }
							  }
							  if ((car&1) > 0) { // odd car
								  if ( (carPlace[car] <= (carPlace[car+1]+2)) && (carPlace[car] >= (carPlace[car+1]-2)) ) {
									  carPlace[car] = (carPlace[car+1]+3) + (adcBuffer[0]&0x3);
								  }
							  } else { // even car
								  if ( (carPlace[car] <= (carPlace[car-1]+2)) && (carPlace[car] >= (carPlace[car-1]-2)) ) {
									  carPlace[car] = (carPlace[car-1]+3) + (adcBuffer[0]&0x3);
								  }
							  }
							  uint8_t carParallelCount = 0;
							  for (uint8_t i=1; i<7; i++) {
								  if (i == car) continue;
								  if ( (carPlace[car] <= (carPlace[i]+6)) && (carPlace[car] >= (carPlace[i]-6)) ) {
									  carParallelCount++;
								  }
							  }
							  while (carParallelCount > 1) {
								  carParallelCount = 0;
								  carPlace[car] += 1;
								  for (uint8_t i=1; i<7; i++) {
									  if (i == car) continue;
									  if ( (carPlace[car] <= (carPlace[i]+5)) && (carPlace[car] >= (carPlace[i]-5)) ) {
										  carParallelCount++;
									  }
								  }
							  }
							  //while(1) {
							  //  uint8_t parallelCars = 0;
							  //  if (carPlace[1] - carPlace[2] == 0) parallelCars++;
							  //}
						  }
					  }
				  }
			  }
			  for (uint8_t i=0; i<carCurrentTrack; i++) {
				  setPixelColor(carTrack[i]+phaseGlitch, currentForegroundColor[0], currentForegroundColor[1], currentForegroundColor[2]);
			  }
			  if (carState != CAR_WINNER) {
				  for (uint8_t car=1; car<7; car++) { // enemy cars
					  for (uint8_t i=0; i<6; i++) {
						  setPixelColor((carSprites[carSide[car]][i]+(19*carPlace[car]))+phaseGlitch, allColorsBrightness[carColors[car]][0], allColorsBrightness[carColors[car]][1], allColorsBrightness[carColors[car]][2]);
					  }
				  }
				  if (carExplosionFrame > 0) { // clear player car space
					  for (uint8_t i=0; i<6; i++) {
						  setPixelColor((carSprites[carSide[0]][i]+(19*carPlace[0]))+phaseGlitch, currentBackgroundColor[0], currentBackgroundColor[1], currentBackgroundColor[2]);
					  }
				  }
				  for (uint8_t i=0; i<6; i++) { // player car
					  setPixelColor((carSprites[carSide[0]+carExplosionFrame][i]+(19*carPlace[0]))+phaseGlitch, allColorsBrightness[carColors[0]+konamiCode][0], allColorsBrightness[carColors[0]+konamiCode][1], allColorsBrightness[carColors[0]+konamiCode][2]);
				  }
				  for (uint8_t p=0; p<2; p++) {
					  setPixelColor(3+(19*curbAnimation)+(19*p*4)+phaseGlitch, currentForegroundColor[0], currentForegroundColor[1], currentForegroundColor[2]);
					  setPixelColor(10+(19*curbAnimation)+(19*p*4)+phaseGlitch, currentForegroundColor[0], currentForegroundColor[1], currentForegroundColor[2]);
				  }
			  } else {
				  for (uint8_t i=0; i<CAR_TROPHY_SIZE; i++) {
					  setPixelColor(carTrophy[i]+phaseGlitch, allColorsBrightness[7-konamiCode][0], allColorsBrightness[7-konamiCode][1], allColorsBrightness[7-konamiCode][2]);
				  }
			  }
		  }
		  else if (myntState == MYNT_CHECKERS) {
			  checkersMoveCount = 0;
			  checkersCaptureCount = 0;
			  switch (checkersState) {
				  case CHECKERS_STATE_START: {
					  for (uint8_t x = 0; x < 8; x++) {
						  for (uint8_t y = 0; y < 8; y++) {
							  if (x%2 == 1) {
								  if (y%2 == 1) {
									  if (y < 3) checkersBoard[x][y] = CHECKERS_BOARD_WHITE;
									  else if (y >= 5) checkersBoard[x][y] = CHECKERS_BOARD_BLACK;
									  else checkersBoard[x][y] = CHECKERS_BOARD_EMPTY;
								  } else {
									  checkersBoard[x][y] = CHECKERS_BOARD_ILLEGAL;
								  }
							  } else {
								  if (y%2 == 1) {
									  checkersBoard[x][y] = CHECKERS_BOARD_ILLEGAL;
								  } else {
									  if (y < 3) checkersBoard[x][y] = CHECKERS_BOARD_WHITE;
									  else if (y >= 5) checkersBoard[x][y] = CHECKERS_BOARD_BLACK;
									  else checkersBoard[x][y] = CHECKERS_BOARD_EMPTY;
								  }
							  }
							  uint8_t pieceColor = CHECKERS_COLOR_EMPTY;
							  switch (checkersBoard[x][y]) {
								  case CHECKERS_BOARD_WHITE: {
									  pieceColor = CHECKERS_COLOR_WHITE;
									  break;
								  }
								  case CHECKERS_BOARD_BLACK: {
									  pieceColor = CHECKERS_COLOR_BLACK;
									  break;
								  }
							  }
							  setPixelColorNumber(checkersBoardPixels[x][y], pieceColor);
						  }
					  }
					  checkersState = CHECKERS_STATE_PLAY;
					  break;
				  }
				  case CHECKERS_STATE_PLAY: {
					  for (uint8_t x = 0; x < 8; x++) {
						  for (uint8_t y = 0; y < 8; y++) {
							  uint8_t pieceColor = CHECKERS_COLOR_EMPTY;
							  switch (checkersBoard[x][y] & 0b1111) {
								  case CHECKERS_BOARD_WHITE: {
									  pieceColor = CHECKERS_COLOR_WHITE;
									  checkersCheckPossibleMoves(x, y, CHECKERS_BOARD_BLACK);
								  	  break;
								  }
								  case CHECKERS_BOARD_BLACK: {
									  pieceColor = CHECKERS_COLOR_BLACK;
									  break;
								  }
						      }
						      setPixelColorNumber(checkersBoardPixels[x][y], pieceColor);
					      }
					  }
					  if (checkersMoveCount > 0) {
						  if (checkersCurrentMove >= checkersMoveCount) {
							  checkersCurrentMove = 0;
						  }
						  uint8_t checkersTempMove = checkersCurrentMove;
						  if (checkersCaptureCount > 0) {
//							  if (checkersCurrentMove >= checkersCaptureCount) {
//								  checkersCurrentMove = 0;
//							  }
//							  uint8_t checkersTempCapture = 0;
							  uint8_t i = checkersCurrentMove;
							  while(1) {
								  i = i % checkersMoveCount;
								  if (checkersMoveList[i][4] == 0) {
									  i++;
									  continue;
								  }

//								  if (checkersCurrentMove > checkersTempCapture) {
//									  checkersTempCapture++;
//								  } else {
									  checkersTempMove = i;
									  checkersCurrentMove = i;
									  break;
//								  }
							  }
						  }
						  uint8_t x = checkersMoveList[checkersTempMove][0];
						  uint8_t y = checkersMoveList[checkersTempMove][1];
						  uint8_t nx = checkersMoveList[checkersTempMove][2];
						  uint8_t ny = checkersMoveList[checkersTempMove][3];
						  setPixelColorNumber(checkersBoardPixels[x][y], CHECKERS_COLOR_SELECTED);
						  setPixelColorNumber(checkersBoardPixels[nx][ny], CHECKERS_COLOR_CHOICE);
					  } else {
						  checkersState = CHECKERS_STATE_LOSER;
					  }
					  break;
				  }
				  case CHECKERS_STATE_RESPONSE: {
					  for (uint8_t x = 0; x < 8; x++) {
						  for (uint8_t y = 0; y < 8; y++) {
							  uint8_t pieceColor = CHECKERS_COLOR_EMPTY;
							  switch (checkersBoard[x][y] & 0b1111) {
								  case CHECKERS_BOARD_BLACK: {
									  pieceColor = CHECKERS_COLOR_BLACK;
									  checkersCheckPossibleMoves(x, y, CHECKERS_BOARD_WHITE);
									  break;
								  }
								  case CHECKERS_BOARD_WHITE: {
									  pieceColor = CHECKERS_COLOR_WHITE;
									  break;
								  }
							  }
							  setPixelColorNumber(checkersBoardPixels[x][y], pieceColor);
						  }
					  }
					  if (checkersMoveCount > 0) {
						  checkersCurrentMove = adcBuffer[0]%checkersMoveCount;
						  uint8_t checkersTempMove = checkersCurrentMove;
						  if (checkersCaptureCount > 0) {
//							  checkersCurrentMove = adcBuffer[0]%checkersCaptureCount;
//							  uint8_t checkersTempCapture = 0;
							  uint8_t i = checkersCurrentMove;
							  while(1) {
								  i = i % checkersMoveCount;
								  if (checkersMoveList[i][4] == 0) {
									  i++;
									  continue;
								  }
//								  if (checkersCurrentMove > checkersTempCapture) {
//									  checkersTempCapture++;
//								  } else {
									  checkersTempMove = i;
									  checkersCurrentMove = i;
									  break;
//								  }
							  }
						  }
						  uint8_t x = checkersMoveList[checkersTempMove][0];
						  uint8_t y = checkersMoveList[checkersTempMove][1];
						  uint8_t nx = checkersMoveList[checkersTempMove][2];
						  uint8_t ny = checkersMoveList[checkersTempMove][3];
						  setPixelColorNumber(checkersBoardPixels[x][y], CHECKERS_COLOR_SELECTED);
						  setPixelColorNumber(checkersBoardPixels[nx][ny], CHECKERS_COLOR_CHOICE);
					  } else {
						  checkersState = CHECKERS_STATE_WINNER;
					  }
					  break;
				  }
				  case CHECKERS_STATE_LOSER: {

					  break;
				  }
				  case CHECKERS_STATE_WINNER: {

					  break;
				  }
			  }
		  } else { // klatki animacji
			  if (myntState == MYNT_BLINK) {
				  switch (blinkState) {
				  case 0:
					  if (blinkAnim > blinkDelay) {
						  currentGraphic = MYNT_GRAPHIC_BLINK1;
						  blinkState = 1;
						  blinkAnim -= blinkDelay;
						  blinkDelay = 8;
					  }
					  break;
				  case 1:
					  if (blinkAnim > blinkDelay) {
						  currentGraphic = MYNT_GRAPHIC_BLINK2;
						  blinkState = 2;
						  blinkAnim -= blinkDelay;
						  blinkDelay = 16;
					  }
					  break;
				  case 2:
					  if (blinkAnim > blinkDelay) {
						  currentGraphic = MYNT_GRAPHIC_BLINK3;
						  blinkState = 3;
						  blinkAnim -= blinkDelay;
						  blinkDelay = 8;
					  }
				  case 3:
					  if (blinkAnim > blinkDelay) {
						  currentGraphic = MYNT_GRAPHIC_UWU;
						  blinkState = 4;
						  blinkAnim -= blinkDelay;
						  blinkDelay = 8;
					  }
				  case 4:
					  if (blinkAnim > blinkDelay) {
						  currentGraphic = MYNT_GRAPHIC_OWO;
						  blinkState = 0;
						  blinkAnim -= blinkDelay;
						  blinkDelay = (randomNumber & 0x3FF);
					  }
					  break;
				  default:
					  break;
				  }
			  }
		  }

		  if (glitchFlag > 0) {
			  // efekt szumu
			  uint8_t staticNoise = adcBuffer[0]&0x1;
			  for (uint8_t i=0; i<LEDS; i += (adcBuffer[0]&0x3F)) {
				  addPixelColor(i, staticNoise, staticNoise, staticNoise);
			  }

			  // efekt linii
			  if (phaseGlitch > 0) phaseGlitch = 0;

			  lineNoise += adcBuffer[0]&1;
			  lineCount++;
			  if (lineCount >= 32) {
				  //phaseGlitch = 0;
				  if (lineNoise >= 16) {
					  lineNoise -= 16;
					  //phaseGlitch = 1;
					  for (uint8_t i=lineNumber[lineNoise]; i<lineNumber[lineNoise+1]; i += 1) {
						  addPixelColor(i, 15, 15, 15);
					  }
				  }
				  lineCount = 0;
				  lineNoise = 0;
			  }
		  } else {
			  lineCount = 0;
			  lineNoise = 0;
			  phaseGlitch = 0;
		  }


		  /*if (tempButton & BUTTON_UP) setPixelColor(1, 63, 0, 0);
		  if (tempButton & BUTTON_DOWN) setPixelColor(20, 63, 0, 0);
		  if (tempButton & BUTTON_LEFT) setPixelColor(39, 63, 0, 0);
		  if (tempButton & BUTTON_RIGHT) setPixelColor(58, 63, 0, 0);
		  if (tempButton & BUTTON_A) setPixelColor(77, 63, 0, 0);
		  if (tempButton & BUTTON_B) setPixelColor(96, 63, 0, 0);*/

		  spiBusy = 1;
		  HAL_SPI_Transmit_DMA(&hspi1, framebuffer, LEDPACKET);
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
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

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
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
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
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
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

  /*Configure GPIO pins : PadA_Pin PadB_Pin */
  GPIO_InitStruct.Pin = PadA_Pin|PadB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PadUp_Pin PadRight_Pin PadLeft_Pin */
  GPIO_InitStruct.Pin = PadUp_Pin|PadRight_Pin|PadLeft_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PadDown_Pin */
  GPIO_InitStruct.Pin = PadDown_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PadDown_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void setPixelColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b) {
	if (p > LEDS) return;

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
	framebuffer[i+0] = 0b00100100 | ((eg>>16)&0xFF);
	framebuffer[i+1] = 0b10010010 | ((eg>>8)&0xFF);
	framebuffer[i+2] = 0b01001001 | ((eg)&0xFF);
	framebuffer[i+3] = 0b00100100 | ((er>>16)&0xFF);
	framebuffer[i+4] = 0b10010010 | ((er>>8)&0xFF);
	framebuffer[i+5] = 0b01001001 | ((er)&0xFF);
	framebuffer[i+6] = 0b00100100 | ((eb>>16)&0xFF);
	framebuffer[i+7] = 0b10010010 | ((eb>>8)&0xFF);
	framebuffer[i+8] = 0b01001001 | ((eb)&0xFF);
}

void setPixelColorNumber(uint16_t p, uint8_t n) {
	if (n >= ALL_COLORS_COUNT) return;
	setPixelColor(p, allColorsBrightness[n][0], allColorsBrightness[n][1], allColorsBrightness[n][2]);
}

/*
 * Returns 0x0RGB, 0x1000 if no pixel
 */
uint32_t readPixelColor(uint16_t p) {
	if (p > LEDS) return 0x1000;

	uint16_t i = p*9;
	uint32_t rgb = 0;
	//                                          00000000RRRRRRRRGGGGGGGGBBBBBBBB
	if (framebuffer[i+0] & 0b01000000) rgb |= 0b00000000000000001000000000000000;
	if (framebuffer[i+0] & 0b00001000) rgb |= 0b00000000000000000100000000000000;
	if (framebuffer[i+0] & 0b00000001) rgb |= 0b00000000000000000010000000000000;
	if (framebuffer[i+1] & 0b00100000) rgb |= 0b00000000000000000001000000000000;
	if (framebuffer[i+1] & 0b00000100) rgb |= 0b00000000000000000000100000000000;
	if (framebuffer[i+2] & 0b10000000) rgb |= 0b00000000000000000000010000000000;
	if (framebuffer[i+2] & 0b00010000) rgb |= 0b00000000000000000000001000000000;
	if (framebuffer[i+2] & 0b00000010) rgb |= 0b00000000000000000000000100000000;
	if (framebuffer[i+3] & 0b01000000) rgb |= 0b00000000100000000000000000000000;
	if (framebuffer[i+3] & 0b00001000) rgb |= 0b00000000010000000000000000000000;
	if (framebuffer[i+3] & 0b00000001) rgb |= 0b00000000001000000000000000000000;
	if (framebuffer[i+4] & 0b00100000) rgb |= 0b00000000000100000000000000000000;
	if (framebuffer[i+4] & 0b00000100) rgb |= 0b00000000000010000000000000000000;
	if (framebuffer[i+5] & 0b10000000) rgb |= 0b00000000000001000000000000000000;
	if (framebuffer[i+5] & 0b00010000) rgb |= 0b00000000000000100000000000000000;
	if (framebuffer[i+5] & 0b00000010) rgb |= 0b00000000000000010000000000000000;
	if (framebuffer[i+6] & 0b01000000) rgb |= 0b00000000000000000000000010000000;
	if (framebuffer[i+6] & 0b00001000) rgb |= 0b00000000000000000000000001000000;
	if (framebuffer[i+6] & 0b00000001) rgb |= 0b00000000000000000000000000100000;
	if (framebuffer[i+7] & 0b00100000) rgb |= 0b00000000000000000000000000010000;
	if (framebuffer[i+7] & 0b00000100) rgb |= 0b00000000000000000000000000001000;
	if (framebuffer[i+8] & 0b10000000) rgb |= 0b00000000000000000000000000000100;
	if (framebuffer[i+8] & 0b00010000) rgb |= 0b00000000000000000000000000000010;
	if (framebuffer[i+8] & 0b00000010) rgb |= 0b00000000000000000000000000000001;

	return rgb;
}

void addPixelColor(uint16_t p, uint8_t ri, uint8_t gi, uint8_t bi) {
	if (p > LEDS) return;

	uint16_t i = p*9;
	uint16_t r = 0;
	uint16_t g = 0;
	uint16_t b = 0;
	if (framebuffer[i+0] & 0b01000000) g |= 0b10000000;
	if (framebuffer[i+0] & 0b00001000) g |= 0b01000000;
	if (framebuffer[i+0] & 0b00000001) g |= 0b00100000;
	if (framebuffer[i+1] & 0b00100000) g |= 0b00010000;
	if (framebuffer[i+1] & 0b00000100) g |= 0b00001000;
	if (framebuffer[i+2] & 0b10000000) g |= 0b00000100;
	if (framebuffer[i+2] & 0b00010000) g |= 0b00000010;
	if (framebuffer[i+2] & 0b00000010) g |= 0b00000001;
	if (framebuffer[i+3] & 0b01000000) r |= 0b10000000;
	if (framebuffer[i+3] & 0b00001000) r |= 0b01000000;
	if (framebuffer[i+3] & 0b00000001) r |= 0b00100000;
	if (framebuffer[i+4] & 0b00100000) r |= 0b00010000;
	if (framebuffer[i+4] & 0b00000100) r |= 0b00001000;
	if (framebuffer[i+5] & 0b10000000) r |= 0b00000100;
	if (framebuffer[i+5] & 0b00010000) r |= 0b00000010;
	if (framebuffer[i+5] & 0b00000010) r |= 0b00000001;
	if (framebuffer[i+6] & 0b01000000) b |= 0b10000000;
	if (framebuffer[i+6] & 0b00001000) b |= 0b01000000;
	if (framebuffer[i+6] & 0b00000001) b |= 0b00100000;
	if (framebuffer[i+7] & 0b00100000) b |= 0b00010000;
	if (framebuffer[i+7] & 0b00000100) b |= 0b00001000;
	if (framebuffer[i+8] & 0b10000000) b |= 0b00000100;
	if (framebuffer[i+8] & 0b00010000) b |= 0b00000010;
	if (framebuffer[i+8] & 0b00000010) b |= 0b00000001;

	r += ri;
	if (r > 255) r = 255;
	g += gi;
	if (g > 255) g = 255;
	b += bi;
	if (b > 255) b = 255;

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

	framebuffer[i+0] = 0b00100100 | ((eg>>16)&0xFF);
	framebuffer[i+1] = 0b10010010 | ((eg>>8)&0xFF);
	framebuffer[i+2] = 0b01001001 | ((eg)&0xFF);
	framebuffer[i+3] = 0b00100100 | ((er>>16)&0xFF);
	framebuffer[i+4] = 0b10010010 | ((er>>8)&0xFF);
	framebuffer[i+5] = 0b01001001 | ((er)&0xFF);
	framebuffer[i+6] = 0b00100100 | ((eb>>16)&0xFF);
	framebuffer[i+7] = 0b10010010 | ((eb>>8)&0xFF);
	framebuffer[i+8] = 0b01001001 | ((eb)&0xFF);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	spiBusy = 0;
	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	msCounter++;
	//if (periodCounter > 100) periodCounter = 0;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	static uint32_t randomNumberBuffer = 0;
	static uint8_t randomShift = 0;
	static uint8_t randomBitsBuffer = 0;

	if (adcBuffer[0] & 1) {
		randomNumberBuffer |= 1 << randomShift;
		randomBitsBuffer++;
	}
	randomShift++;

	if (randomShift > 31) {
		randomNumber = randomNumberBuffer;
		randomBits = randomBitsBuffer;
		randomNumberBuffer = 0;
		randomShift = 0;
		randomBitsBuffer = 0;
	}
}

void SPI_SetOpenDrain() { // Added to stm32f0xx_hal_msp.c
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SPI1_MOSI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	HAL_GPIO_Init(SPI1_MOSI_GPIO_Port, &GPIO_InitStruct);
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
