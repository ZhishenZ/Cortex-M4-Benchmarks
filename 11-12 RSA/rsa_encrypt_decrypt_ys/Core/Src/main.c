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
/**
 *
 *  Yuwen's home-made RSA-Encrypt-Decrypt Benchmark, part of the code refers:
 *  https://codeantenna.com/a/P1isIFfGFz
 *
 *  I have tried to keep the program as simple as possible, no malloc, no external library,
 *  extremly easy to be tested in embedded systems like STM32.
 *
 *
 * */
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
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if(INPUTDATAINFLASH)
static const
#else
#ifdef INPUTDATAINCCMRAM
__attribute__((section(".ccmram")))
#endif
#endif
char plaintext[SIZE]
#if(SIZE==100)
="To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
#else
#if(SIZE==200)
="To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
#else
#if(SIZE==300)
="To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
#else
#if(SIZE==400)
="To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
#else
#if(SIZE==500)
="To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
"To be, or not to be, that is the question: Whether tis nobler in the mind to suffer The slings and a"
#endif
#endif
#endif
#endif
#endif
;
char de_plaintext_char[SIZE];
int length; //plaintext length
uint64_t c[SIZE];  //crypted text

//struct key_info_struct{
//	uint64_t e;               /*                         exponent       */
//	uint64_t p;               /*                         prime number 1 */
//	uint64_t q;               /*                         prime number 2 */
//	uint64_t n;               /* p*q                                    */
//	uint64_t phi_n;           /* (p-1)*(q-1)             euler_function */
//	uint64_t d;               /* calculate_d(e, phi_n)   private_key    */
//};

/*
struct key_info_struct key_info = {
	.e = 7,
	.p = 17,
	.q = 13,
	.n = 221,
	.phi_n = 192,
	.d = 55
};*/

#if(DATAINFLASH)
const uint64_t ro_data_src[3] = {7, 221, 55};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) uint64_t ro_data_src[3] = {7, 221, 55};
#else
uint64_t ro_data_src[3] = {7, 221, 55};
#endif
#endif




/*#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
uint64_t calculate_d(uint64_t e, uint64_t phi){
	uint64_t k=1;
	while(((k * phi+1) % e) != 0){
		k++;
	}
	return (k*phi+1)/e;
}*/

/* encrypt function */
#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
void encrypt(){
	/* encrypt each plaintext element, c[] is the encripted array*/
	int temp_crypted_element=1;
	for(uint16_t i=0;i<SIZE;i++){
	   for(uint16_t j=0;j</*key_info.e*/ro_data[0];j++){
		   temp_crypted_element= temp_crypted_element * plaintext[i] % ro_data[1];
	   }
	   c[i]=temp_crypted_element;
	   temp_crypted_element=1;
	}

}


/* decrypt function*/
#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
void decrypto(){

	uint64_t temp_var=1;

	for(uint16_t i=0;i<SIZE;i++){
	   for(uint16_t j=0; j< ro_data[2]; j++){
		temp_var=temp_var*c[i] % ro_data[1];
	   }
	   de_plaintext_char[i]=temp_var;
	   temp_var=1;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	ro_data = ro_data_src;
	uint8_t gu8_MSG[60] = {'\0'};
	int cpu_frequency=24;				/* in MHz */
	volatile uint32_t time1, time2, time3, time4;

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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	time1 = TIM2->CNT;
	encrypt();
	//
	time2 = TIM2->CNT;
	HAL_Delay(1000);
	time3 = TIM2->CNT;
	decrypto();
	//HAL_Delay(1000);
	time4 = TIM2->CNT;
	encrypt_time =(int)((double)(time2-time1)/cpu_frequency*1000); /* in ns */
	decrypt_time =(int)((double)(time4-time3)/cpu_frequency*1000); /* in ns */
	sprintf(gu8_MSG, "\n\rEncryption time: %ld\n\rDecryption time: %ld\n\r",
			encrypt_time,decrypt_time);
	HAL_UART_Transmit(&huart1, gu8_MSG, sizeof(gu8_MSG), 0xFFFF);

	/* check if message is correct en-/decrypted */
	for(int i=0; i<SIZE; i++)
		if(de_plaintext_char[i]!=plaintext[i])
			HAL_UART_Transmit(&huart1, "en-/decryption error!\n\r",
					sizeof(gu8_MSG), 0xFFFF);

	HAL_Delay(1000);
	Reset_Handler();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  huart1.Init.Parity = UART_PARITY_EVEN;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
