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
#include "st7789.h"
#include "cat_images.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define YBUTTON_PIN_Pin GPIO_PIN_7
#define YBUTTON_PIN_GPIO_Port GPIOB

#define NBUTTON_PIN_Pin GPIO_PIN_9
#define NBUTTON_PIN_GPIO_Port GPIOB

#define VALENTINE 0
#define YES 1
#define ARE_YOU_SURE 2
#define SUPER_DUPER_SURE 3
#define PLEASE_SAY_YES 4
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
void ST7789_Draw_Image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
void ST7789_Draw_Image_Double(uint16_t x, uint16_t y, uint16_t small_w, uint16_t small_h, uint16_t *data);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void ST7789_Draw_Image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data)
{
    // 1. Set the boundary for the image
    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    ST7789_Select();

    #ifdef USE_DMA
        /* * ST7789_WriteData handles the chunking and DMA calls.
         * We pass the total byte size: (width * height * 2 bytes per pixel)
         */
        ST7789_WriteData((uint8_t *)data, w * h * 2);
    #else
        // Manual 8-bit SPI fallback
        for (uint32_t i = 0; i < (w * h); i++) {
            uint8_t color_bytes[] = {data[i] >> 8, data[i] & 0xFF};
            ST7789_WriteData(color_bytes, 2);
        }
    #endif

    ST7789_UnSelect();
}

/**
 * @brief Draws a small image scaled up by 2x (Pixel Doubling)
 * @param x, y: Starting coordinates on LCD
 * @param small_w, small_h: Dimensions of the tiny source image (e.g., 120x50)
 * @param data: Pointer to the small image array
 */
void ST7789_Draw_Image_Double(uint16_t x, uint16_t y, uint16_t small_w, uint16_t small_h, uint16_t *data)
{
    // Set the window to the LARGER size (double the width and height)
    ST7789_SetAddressWindow(x, y, x + (small_w * 2) - 1, y + (small_h * 2) - 1);

    ST7789_Select();
    ST7789_DC_Set(); // Data mode

    for (uint16_t row = 0; row < small_h; row++)
    {
        // We do everything in this loop twice to double the height
        for (int repeat_row = 0; repeat_row < 2; repeat_row++)
        {
            for (uint16_t col = 0; col < small_w; col++)
            {
                uint16_t color = data[row * small_w + col];
                // Byte swap for ST7789 (Big Endian)
                uint8_t color_bytes[] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };

                // Send the same pixel twice to double the width
                HAL_SPI_Transmit(&ST7789_SPI_PORT, color_bytes, 2, HAL_MAX_DELAY);
                HAL_SPI_Transmit(&ST7789_SPI_PORT, color_bytes, 2, HAL_MAX_DELAY);
            }
        }
    }

    ST7789_UnSelect();
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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
  ST7789_Init();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);

  // Background
  ST7789_Fill_Color(WHITEISH);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  /*
  #define VALENTINE 0
  #define YES 1
  #define ARE_YOU_SURE 2
  #define SUPER_SURE 3
  #define SUPER_DUPER_SURE 4
  #define PLEASE_SAY_YES 5
  */
  int16_t* c = NULL;
  uint8_t state = VALENTINE;
  while (1)
  {
	switch(state) {
	  case(VALENTINE):
		ST7789_WriteString(30, 10, "Will you be", Font_16x26, RED, WHITE);
		ST7789_WriteString(20, 40, "My Valentine?", Font_16x26, RED, WHITE);
		ST7789_WriteString(100, 70, "<3", Font_16x26, RED, WHITE);
		ST7789_WriteString(10, 150, "YES<3    NO</3", Font_16x26, RED, WHITE);

		c = cat1;
		ST7789_Draw_Image_Double(0, 180, 120, 50, (uint16_t *)c);
		HAL_Delay(50);
		c = cat2;
		ST7789_Draw_Image_Double(0, 180, 120, 50, (uint16_t *)c);

		if (HAL_GPIO_ReadPin(YBUTTON_PIN_GPIO_Port, YBUTTON_PIN_Pin) == 0) {
			state = YES;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
		} else if (HAL_GPIO_ReadPin(NBUTTON_PIN_GPIO_Port, NBUTTON_PIN_Pin) == 0) {
			state = ARE_YOU_SURE;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
		}
		break;

	  case(YES):
		ST7789_WriteString(60, 10, "YIPPEE!!", Font_16x26, RED, WHITE);
		ST7789_WriteString(40, 40, "I LOVE YOU", Font_16x26, RED, WHITE);
		ST7789_WriteString(10, 70, "SOSOSO MUCH!!!", Font_16x26, RED, WHITE);
		ST7789_WriteString(20, 100, "MWAMWAMWAMWA", Font_16x26, RED, WHITE);
		ST7789_WriteString(10, 130, "<3<3<3<3<3<3<3", Font_16x26, RED, WHITE);
		ST7789_WriteString(10, 160, "YAYAYAYAYYAYAY", Font_16x26, RED, WHITE);
		c = catheart;
		ST7789_Draw_Image_Double(20, 190, 50, 50, (uint16_t *)c);
		c = catroses;
		ST7789_Draw_Image_Double(120, 190, 50, 50, (uint16_t *)c);
		break;
	  case(ARE_YOU_SURE):
		ST7789_WriteString(10, 30, "Are you sure??", Font_16x26, RED, WHITE);
	  	c = catplead;
	  	ST7789_Draw_Image_Double(70, 95, 50, 50, (uint16_t *)c);
	  	ST7789_WriteString(20, 250, "YES!!    NO!!", Font_16x26, RED, WHITE);

		if (HAL_GPIO_ReadPin(YBUTTON_PIN_GPIO_Port, YBUTTON_PIN_Pin) == 0) {
			state = SUPER_DUPER_SURE;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
		} else if (HAL_GPIO_ReadPin(NBUTTON_PIN_GPIO_Port, NBUTTON_PIN_Pin) == 0) {
			state = VALENTINE;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
 		}
		break;

	  case(SUPER_DUPER_SURE):
		ST7789_WriteString(30, 30, "SUPER DUPER", Font_16x26, RED, WHITE);
	  	ST7789_WriteString(10, 60, "SURE??????????", Font_16x26, RED, WHITE);
	    c = catcheeks;
		ST7789_Draw_Image_Double(70, 115, 50, 50, (uint16_t *)c);
		ST7789_WriteString(20, 250, "YES!!    NO!!", Font_16x26, RED, WHITE);
		if (HAL_GPIO_ReadPin(YBUTTON_PIN_GPIO_Port, YBUTTON_PIN_Pin) == 0) {
			state = PLEASE_SAY_YES;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
		} else if (HAL_GPIO_ReadPin(NBUTTON_PIN_GPIO_Port, NBUTTON_PIN_Pin) == 0) {
			state = VALENTINE;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
		}
		break;

	  case(PLEASE_SAY_YES):
		ST7789_WriteString(10, 10, "PLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLSPLS", Font_16x26, RED, WHITE);
	  	ST7789_WriteString(20, 250, "YES!!    NO!!", Font_16x26, RED, WHITE);
		if (HAL_GPIO_ReadPin(YBUTTON_PIN_GPIO_Port, YBUTTON_PIN_Pin) == 0) {
			state = YES;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
		} else if (HAL_GPIO_ReadPin(NBUTTON_PIN_GPIO_Port, NBUTTON_PIN_Pin) == 0) {
			state = PLEASE_SAY_YES;
			ST7789_Fill_Color(WHITEISH);
			HAL_Delay(200);
		}
		break;
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2|CS_PIN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DC_PIN_Pin|RST_PIN_Pin|BL_PIN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 CS_PIN_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_2|CS_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : DC_PIN_Pin RST_PIN_Pin BL_PIN_Pin */
  GPIO_InitStruct.Pin = DC_PIN_Pin|RST_PIN_Pin|BL_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB7 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
