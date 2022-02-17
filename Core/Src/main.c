/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f0xx_hal.h"

#define YEAR 365*3   // 365*3  天數限制
#define OneSec 1000 //系統秒數 預設1000 測試可以改為100
#define OneSetpTime 1000 //測試時可以改為1000
uint32_t writeFlashData;
uint32_t addr_timesCount = 0x08007000;
uint32_t addr_dayCount = 0x08007400;

int dayCount = 0;

int max_count = 3000;  // 開機次數限制
int buzz_flag = 0;
int buzz_count = 0;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//FLASH写入数据测试
void writeFlashTest(uint32_t addr, int data) {
	//1、解锁FLASH
	HAL_FLASH_Unlock();

	//2、擦除FLASH
	//初始化FLASH_EraseInitTypeDef
	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = addr;
	f.NbPages = 1;
	//设置PageError
	uint32_t PageError = 0;
	//调用擦除函数
	HAL_FLASHEx_Erase(&f, &PageError);

	//3、对FLASH烧写
	HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, data);
	HAL_Delay(1);
	//4、锁住FLASH
	HAL_FLASH_Lock();
}

//FLASH读取数据测试
int printFlashTest(uint32_t addr) {
	uint32_t temp = *(__IO uint32_t*) (addr);

	printf("addr:0x%x, data:0x%x\r\n", addr, temp);
	//writeFlashData = temp + 1;
	return temp;

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	//flash TEST
	//writeFlashData = 0;
    HAL_Delay(500);
	printf("\n--s--\n");
	int count = 0;
	count = printFlashTest(addr_timesCount);
	count++;
	writeFlashTest(addr_timesCount, count);
	//檢查是否超出次數，超出則改變TimerCount(PA5狀態)
	if (count >= max_count) {
		printf("\rcount>%d\r", max_count);
		HAL_GPIO_WritePin(TimesCount_GPIO_Port, TimesCount_Pin,
				GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(TimesCount_GPIO_Port, TimesCount_Pin,
				GPIO_PIN_SET);
	}

	dayCount = printFlashTest(addr_dayCount);
	printf("\n--e--\n");

	int timer = 0;
	int reset_count=0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_GPIO_TogglePin(BlinkLed_GPIO_Port, BlinkLed_Pin);
		HAL_Delay(OneSec);
		//清除PA1進行軟體計數Reset
		if (HAL_GPIO_ReadPin(RESET_GPIO_Port, RESET_Pin) == 0) {
			reset_count++;
			if(reset_count>=10){
				printf("\n--reset--\n");
				writeFlashTest(addr_timesCount, 0);
				writeFlashTest(addr_dayCount, 0);
				count = 0;
				dayCount=0;
				//reset_count=0;
			}
		}else{
			reset_count=0;
		}

		printFlashTest(addr_timesCount);
		//檢查是否超出次數，超出則改變TimerCount(PA5狀態)
		if(count >= max_count) {
			printf("\rcount>%d\r", max_count);
			HAL_GPIO_WritePin(TimesCount_GPIO_Port, TimesCount_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(TimesCount_OK_GPIO_Port, TimesCount_OK_Pin,GPIO_PIN_RESET);
		} else if(count >= 5){
			printf("\rcount>5\r");
			HAL_GPIO_WritePin(TimesCount_GPIO_Port, TimesCount_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(TimesCount_OK_GPIO_Port, TimesCount_OK_Pin,GPIO_PIN_SET);
		} else {

			HAL_GPIO_WritePin(TimesCount_GPIO_Port, TimesCount_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(TimesCount_OK_GPIO_Port, TimesCount_OK_Pin,GPIO_PIN_SET);
		}
		//計數天數
		/*
		 * 一天=86400秒
		 * 每過一天進行計數並保存至FLASH中
		 */
		if (timer > 86400) {	// 天
			timer = 0;
			dayCount = printFlashTest(addr_dayCount);
			dayCount++;
			writeFlashTest(addr_dayCount, dayCount);
			printf("\nTimerReset: %1d \t dayCount:%1d \n", timer, dayCount);
		} else {
			timer = timer + OneSetpTime;
			printf("\nTimer: %1d \n", timer);
		}
		/*
		 * 當FLASH中天數超出設定天數，則改變dayControl_Pin(PA6)
		 */
		if (dayCount >= YEAR) {
			HAL_GPIO_WritePin(dayControl_GPIO_Port, dayControl_Pin, GPIO_PIN_RESET);
			buzz_flag = 1;
		}else{
			HAL_GPIO_WritePin(dayControl_GPIO_Port, dayControl_Pin, GPIO_PIN_SET);
			buzz_flag = 0;
		}

		/*
		 * Buzz flag = 1 時 蜂鳴器鳴叫半秒
		 * timer%4 則 表示 每四秒進行一次
		 */
		if (buzz_flag == 1) {
			if (timer % 4 == 0) {
				printf("\nbuzz!!\n");
				HAL_GPIO_WritePin(Buzz_GPIO_Port, Buzz_Pin, GPIO_PIN_SET);
				HAL_Delay(OneSec/2);
				HAL_GPIO_WritePin(Buzz_GPIO_Port, Buzz_Pin, GPIO_PIN_RESET);
				buzz_flag = 0;
			}
		}
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
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
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
	while (1) {
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
