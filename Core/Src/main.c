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
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// ONLY REAS REGISTERS
#define POWERMGT_REG_SENSE_CNT		(10)
// ONLY REAS REGISTERS
#define POWERMGT_REG_CTL_CNT		(5) // 0x0A..0x0D,0x0F 5 control registers (read/write)

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

tPowerMgmType regsSetPowerMan = { 0 };

const uint8_t hello[] = "\nhellorld";
const uint8_t timeStamp[] = "\n.";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/

void showPowerCtlRegs(void) {
	// Register names per Power Controller user guide V4.0
	static const char *senseName[10] = { 
		"Sense_CM4", "Sense_Lidar1", "Sense_Lidar2", "Sense_CanBus", "Sense_Accessory",
		"Sense_5V", "Sense_Batt", "Sense_MainPwr", "Temp", "VRefInt" 
	};
	static const char *ctlName[6] = { 
		"Enable_Lidar1", "Enable_Lidar2", "Enable_CanBus", "Enable_CM4", "Switch", "Voltage_Ctrl"
	};
	static const char *state[5] = { "", "OFF", "ON", "Starting", "Shutting down" };

	printf("\n=== Power Management Registers ===\n");
	printf("UID: %08lX-%08lX-%08lX\n", HAL_GetUIDw2(), HAL_GetUIDw1(), HAL_GetUIDw0());

	// Sense registers (0x00-0x09)
	printf("\n[Sense Registers]\n");
	for (uint8_t i = 0; i < 10; i++) {
		uint16_t raw = regsSetPowerMan.sense[i];
		if (i == 0x08) {
			// Temperature: signed, value x100
			int16_t t = (int16_t) raw;
			int16_t a = (t < 0) ? -t : t;
			printf("0x%02X %-18s = %s%d.%02d °C\n", i, senseName[i], (t < 0) ? "-" : "", a / 100, a % 100);
		} else if (i == 0x09) {
			printf("0x%02X %-18s = 0x%04X (do not use)\n", i, senseName[i], raw);
		} else {
			// Voltage: value x100
			printf("0x%02X %-18s = %u.%02u V\n", i, senseName[i], raw / 100, raw % 100);
		}
	}

	// Control registers (0x0A-0x0F)
	printf("\n[Control Registers]\n");
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t val = regsSetPowerMan.control[i];
		printf("0x%02X %-18s = 0x%02X (%s)\n", 0x0A + i, ctlName[i], val, (val < 5) ? state[val] : "?");
	}

	// Register 0x0E: Switch
	{
		uint8_t val = regsSetPowerMan.control[4];
		bool P_bp = (val & 0x01) != 0;
		bool M_bp = (val & 0x10) != 0;
		printf("0x0E %-18s = 0x%02X (P_bp=%d, M_bp=%d)\n", ctlName[4], val, P_bp, M_bp);
	}

	// Register 0x0F: Voltage Control
	{
		uint8_t val = regsSetPowerMan.control[5];
		printf("0x0F %-18s = 0x%02X (%s)\n", ctlName[5], val, (val < 5) ? state[val] : "?");
	}

	printf("\n");
}

/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
	// Translate LF -> CRLF so a raw serial terminal returns to column 0
	// (otherwise lines "staircase" diagonally across the screen).
	for (int i = 0; i < len; i++) {
		if (ptr[i] == '\n') {
			uint8_t cr = '\r';
			HAL_UART_Transmit(&huart2, &cr, 1, HAL_MAX_DELAY);
		}
		HAL_UART_Transmit(&huart2, (uint8_t*) &ptr[i], 1, HAL_MAX_DELAY);
	}
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
int main(void) {

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
	while (1) {

		// Clear the buffer before each read so a failed/absent slave shows zeros,
		// not the previous (stale) values.
		memset(&regsSetPowerMan, 0, sizeof(regsSetPowerMan));
		// Presence check: send the address only and look for an ACK. If the slave
		// truly NACKs (powered off and not parasitically driven), report it and
		// skip the read so no stale block is reprinted.
		if (HAL_I2C_IsDeviceReady(&hi2c1, POWERMGT_I2C_ADDR, 2, 10) != HAL_OK) {
			printf("\nI2C 0x%02X not responding", POWERMGT_I2C_ADDR >> 1);
		} else {
			HAL_I2C_Master_Receive_DMA(&hi2c1, POWERMGT_I2C_ADDR, (uint8_t*) &regsSetPowerMan, sizeof(regsSetPowerMan));
		}
//	  for(uint8_t iter = 0; iter <16; iter=iter+1)
//	  {

//		  HAL_I2C_Mem_Read(&hi2c1, POWERMGT_I2C_ADDR, iter,
//				  POWERMGT_REG_SIZE, (uint8_t *)&regsSetPowerMan[iter], POWERMGT_REG_SIZE, HAL_MAX_DELAY);

//		  HAL_I2C_Master_Transmit(&hi2c1, POWERMGT_I2C_ADDR, &iter, sizeof(iter), HAL_MAX_DELAY);
//
//		  HAL_I2C_Master_Receive(&hi2c1, POWERMGT_I2C_ADDR, (uint8_t *)&regsSetPowerMan[iter], POWERMGT_REG_SIZE, HAL_MAX_DELAY);

//	  }

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
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
