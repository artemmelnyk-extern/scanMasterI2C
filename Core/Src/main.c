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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// ONLY REAS REGISTERS
#define POWERMGT_REG_SENSE_CNT		(10)
// ONLY REAS REGISTERS
#define POWERMGT_REG_CTL_CNT		(8)


typedef struct powerMgmType {
	uint16_t sense[10];
	uint8_t control[8];
} tPowerMgmType;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POWERMGT_REG_SIZE		(2)
#define POWERMGT_REG_READ_CNT   (POWERMGT_REG_SENSE_CNT +POWERMGT_REG_CTL_CNT)

// power mgt - STMicro
#define POWERMGT_I2C_ADDR		(0x16<<1)
// RTC
//#define POWERMGT_I2C_ADDR		(0x51<<1)
// TODO
//#define POWERMGT_I2C_ADDR		(0x57<<1)
// IMU
//#define POWERMGT_I2C_ADDR		(0x69<<1)

#define POWERMGT_REG_TEMP		(0x08)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

tPowerMgmType regsSetPowerMan = {0};

const uint8_t hello[] = "\nhellorld";
const uint8_t timeStamp[] = "\n.";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/

void showPowerCtlRegs(void) {
	printf("\n");
	for(uint8_t count=0; count < POWERMGT_REG_SENSE_CNT; count++) {
		// Decode per Power Controller user guide V4.0 (integer math, no float printf).
		uint16_t raw = regsSetPowerMan.sense[count];
		if(count == POWERMGT_REG_TEMP) {
			// Vtemperature: signed 16-bit, value x100 (degrees C)
			int16_t t = (int16_t)raw;
			int16_t a = (t < 0) ? -t : t;
			printf("\nreg 0x%02X: 0x%04X  %s%d.%02d C", count, raw, (t < 0) ? "-" : "", a/100, a%100);
		} else if(count == 0x09) {
			// VRefInt: do not use
			printf("\nreg 0x%02X: 0x%04X  (do not use)", count, raw);
		} else {
			// Sense voltage: bits [b0..b11], value x100
			uint16_t v = raw & 0x0FFF;
			printf("\nreg 0x%02X: 0x%04X  %u.%02u V", count, raw, v/100, v%100);
		}
	}
	for(uint8_t count=0; count < POWERMGT_REG_CTL_CNT; count++) {
		printf("\nreg 0x%02X: 0x%02X", POWERMGT_REG_SENSE_CNT+count, regsSetPowerMan.control[count]);
	}
	printf("\n");
}

/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart2, (const uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
    return len;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	showPowerCtlRegs();
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  HAL_I2C_Master_Receive_DMA(&hi2c1, POWERMGT_I2C_ADDR, (uint8_t *)&regsSetPowerMan,
	                                               sizeof(regsSetPowerMan));
	  for(uint8_t iter = 0; iter <16; iter=iter+1)
	  {

//		  HAL_I2C_Mem_Read(&hi2c1, POWERMGT_I2C_ADDR, iter,
//				  POWERMGT_REG_SIZE, (uint8_t *)&regsSetPowerMan[iter], POWERMGT_REG_SIZE, HAL_MAX_DELAY);

//		  HAL_I2C_Master_Transmit(&hi2c1, POWERMGT_I2C_ADDR, &iter, sizeof(iter), HAL_MAX_DELAY);
//
//		  HAL_I2C_Master_Receive(&hi2c1, POWERMGT_I2C_ADDR, (uint8_t *)&regsSetPowerMan[iter], POWERMGT_REG_SIZE, HAL_MAX_DELAY);

	  }

//	  showPowerCtlRegs();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  printf("%s", timeStamp);
	  HAL_Delay(1000);
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
