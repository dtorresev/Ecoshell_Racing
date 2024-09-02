/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>
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
ADC_HandleTypeDef hadc2;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
/* USER CODE BEGIN PV */
uint32_t ThrottleSignal,InvThrottleSignal,VoltageSignal,CurrentSignal,voltage,current,watts,VOCurrent,VOVoltage,HIGH_SIGNAL,signal;
volatile int HALLA_State,HALLB_State,HALLC_State,DMS_State,ed;
volatile int fase = 0,nuevaFase = 0;
// TIM2 CH1 INA CH2 SDA
// TIM3 CH1 INB CH2 SDB
// TIM4 CH1 INC CH4 SDC
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
static void SetMotorPhase(int NuevaFase);

void switchfase(){
	if (HALLA_State == 1 && HALLB_State == 0 && HALLC_State == 1) {
		nuevaFase = 2;} /*A-B*/
	else if (HALLA_State == 1 && HALLB_State == 1 && HALLC_State == 0) {
		nuevaFase = 4;}/*B-C*/
	else if (HALLA_State == 0 && HALLB_State == 1 && HALLC_State == 1) {
		nuevaFase = 6;} /*C-A*/
	else if (HALLA_State == 0 && HALLB_State == 1 && HALLC_State == 0) {
		nuevaFase = 5;} /*B-A*/
	else if (HALLA_State == 1 && HALLB_State == 0 && HALLC_State == 0) {
		nuevaFase = 3;} /*A-C*/
	else if (HALLA_State == 0 && HALLB_State == 0 && HALLC_State == 1) {
		nuevaFase = 1;} /*C-B*/
	if (DMS_State == 1){
		HIGH_SIGNAL = 280;
		while (HIGH_SIGNAL > 240){
			HIGH_SIGNAL--;
		}
	}
	if (DMS_State == 0){
		HIGH_SIGNAL = 0; /* nuevaFase = 0; */}
	SetMotorPhase(nuevaFase);
}

void SetMotorPhase(){
	switch(nuevaFase){
		case 0:

			TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM2->CCER &= ~(TIM_CCER_CC1P); // Limpiar el bit CC1P para asegurar la polaridad normal

			TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M);
			TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM3->CCER &= ~(TIM_CCER_CC1P);

			TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM4->CCER &= ~(TIM_CCER_CC1P);

			/* SDA OFF */ TIM2 -> CCR4 = 0;
			/* SDB OFF */ TIM3 -> CCR2 = 0;
			/* SDC OFF */ TIM4 -> CCR4 = 0;
			/* INA OFF */ TIM2 -> CCR1 = 0;
			/* INB OFF */ TIM3 -> CCR1 = 0;
			/* INC OFF */ TIM4 -> CCR1 = 0;
			break;
		/* Phase1 C-B */
		case 1:
			/* SDA OFF */	TIM2 -> CCR4 = 0;
			/* SDB ON */	TIM3 -> CCR2 = HIGH_SIGNAL;
			/* SDC ON */	TIM4 -> CCR4 = HIGH_SIGNAL;

			TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM4->CCER &= ~(TIM_CCER_CC1P);

			TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM2->CCER &= ~(TIM_CCER_CC1P); // Limpiar el bit CC1P para asegurar la polaridad normal

			/* INA OFF */	TIM2 -> CCR1 = 0;
			/* INC HIGH */	TIM4 -> CCR1 = HIGH_SIGNAL;

			/* Se invierte la polardad del canal 1 del TIM3 */
			TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM3->CCER |= TIM_CCER_CC1P;

			/* INB LOW */	TIM3 -> CCR1 = HIGH_SIGNAL;
			break;
		/* Phase2 A-B */
		case 2:

			/* SDA ON */	TIM2 -> CCR4 = HIGH_SIGNAL;
			/* SDB ON */	TIM3 -> CCR2 = HIGH_SIGNAL;
			/* SDC OFF */	TIM4 -> CCR4 = 0;

			TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM4->CCER &= ~(TIM_CCER_CC1P);

			TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM2->CCER &= ~(TIM_CCER_CC1P); // Limpiar el bit CC1P para asegurar la polaridad normal

			/* INC OFF */	TIM4 -> CCR1 = 0;
			/* INA HIGH */	TIM2 -> CCR1 = HIGH_SIGNAL;

			/* Se invierte la polardad del canal 1 del TIM3 */
			TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM3->CCER |= TIM_CCER_CC1P;

			/* INB LOW */	TIM3 -> CCR1 = HIGH_SIGNAL;
			break;
		/* Phase3 A-C */
		case 3:
			/* SDA ON */	TIM2 -> CCR4 = HIGH_SIGNAL;
			/* SDB OFF */	TIM3 -> CCR2 = 0;
			/* SDC ON */	TIM4 -> CCR4 = HIGH_SIGNAL;


			TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM2->CCER &= ~(TIM_CCER_CC1P);

			TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M);
			TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM3->CCER &= ~(TIM_CCER_CC1P);

			/* INA HIGH */	TIM2 -> CCR1 = HIGH_SIGNAL;
			/* INB OFF */	TIM3 -> CCR1 = 0;

			/* Se invierte la polardad del canal 1 del TIM4 */
			TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM4->CCER |= TIM_CCER_CC1P;

			/* INC LOW */	TIM4 -> CCR1 = HIGH_SIGNAL;

			break;
		/* Phase4 B-C */
		case 4:
			/* SDA OFF */	TIM2 -> CCR4 = 0;
			/* SDB ON */	TIM3 -> CCR2 = HIGH_SIGNAL;
			/* SDC ON */	TIM4 -> CCR4 = HIGH_SIGNAL;

			TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM2->CCER &= ~(TIM_CCER_CC1P);

			TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M);
			TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM3->CCER &= ~(TIM_CCER_CC1P);

			/* INA OFF */	TIM2 -> CCR1 = 0;
			/* INB HIGH */	TIM3 -> CCR1 = HIGH_SIGNAL;

			/* Se invierte la polardad del canal 1 del TIM4 */
			TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM4->CCER |= TIM_CCER_CC1P;

			/* INC LOW */	TIM4 -> CCR1 = HIGH_SIGNAL;
			break;
		/* Phase5 B-A */
		case 5:

			/* SDA ON */	TIM2 -> CCR4 = HIGH_SIGNAL;
			/* SDB ON */	TIM3 -> CCR2 = HIGH_SIGNAL;
			/* SDC OFF */	TIM4 -> CCR4 = 0;

			TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M);
			TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM3->CCER &= ~(TIM_CCER_CC1P);

			TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM4->CCER &= ~(TIM_CCER_CC1P);

			/* INB HIGH */	TIM3 -> CCR1 = HIGH_SIGNAL;
			/* INC OFF */	TIM4 -> CCR1 = 0;

			/* Se invierte la polardad del canal 1 del TIM2 */
			TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM2->CCER |= TIM_CCER_CC1P;

			/* INA LOW */	TIM2 -> CCR1 = HIGH_SIGNAL;
			break;
		/* Phase6 C-A */
		case 6:

			/* SDA ON */ 	TIM2 -> CCR4 = HIGH_SIGNAL;
			/* SDB OFF */ 	TIM3 -> CCR2 = 0;
			/* SDC ON */ 	TIM4 -> CCR4 = HIGH_SIGNAL;

			TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M);
			TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM3->CCER &= ~(TIM_CCER_CC1P);

			TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M); // Limpiar los bits OC1M para asegurar el modo de salida PWM1
			TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; // Establecer bits OC1M en 1 y 0 para PWM1
			TIM4->CCER &= ~(TIM_CCER_CC1P);

			/* INB OFF */	TIM3 -> CCR1 = 0;
			/* INC HIGH */	TIM4 -> CCR1 = HIGH_SIGNAL;

			/* Se invierte la polardad del canal 1 del TIM2 */
			TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
			TIM2->CCER |= TIM_CCER_CC1P;

			/* INA LOW */	TIM2 -> CCR1 = HIGH_SIGNAL;
			break;
		}
}

void StartMotor(uint32_t HIGH_SIGNAL){
		for (nuevaFase = 0;nuevaFase < 7; nuevaFase++){
			SetMotorPhase(nuevaFase);}
		if (nuevaFase > 6){ nuevaFase = 0;}
	}

void stopMotor(){
	for (HIGH_SIGNAL; HIGH_SIGNAL > 30; HIGH_SIGNAL -=20) { HAL_Delay(50); }
}

/* USER CODE END PFP */
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
/** * @brief  The application entry point. * @retval int*/
int main(void){
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
  MX_ADC2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  /*Iniciar PWMs de los 3 Timers en sus canales*/
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); /*INA*/
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); /*INB*/
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); /*INC*/

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4); /*SDA*/
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); /*SDB*/
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4); /*SDC*/

  /*Inicializar todos con un %DC de 0% -> Todo apagado*/
  TIM2 -> CCR4 = 0;
  TIM3 -> CCR2 = 0;
  TIM4 -> CCR4 = 0;
  TIM2 -> CCR1 = 0;
  TIM3 -> CCR1 = 0;
  TIM4 -> CCR1 = 0;
  nuevaFase = 0;
//  uint32_t tiempoInicial = HAL_GetTick();
  HIGH_SIGNAL = 0;
  HALLA_State = HAL_GPIO_ReadPin(GPIOA, HALL_A_Pin);
  HALLB_State = HAL_GPIO_ReadPin(GPIOA, HALL_B_Pin);
  HALLC_State = HAL_GPIO_ReadPin(GPIOA, HALL_C_Pin);
  switchfase();

  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1){
	DMS_State = HAL_GPIO_ReadPin(GPIOA, DMS_Pin);
	/*Duty Cyle recomendado en start: 50%-70%, en drive 30%-60% -- Según flux.ai*/
	/* Cambiar la fase solo si hay un cambio en la fase */
	HALLA_State = HAL_GPIO_ReadPin(GPIOA, HALL_A_Pin);
	HALLB_State = HAL_GPIO_ReadPin(GPIOA, HALL_B_Pin);
	HALLC_State = HAL_GPIO_ReadPin(GPIOA, HALL_C_Pin);
	switchfase();
	SetMotorPhase(nuevaFase);
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */
  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */
  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */
  /* USER CODE END ADC2_Init 2 */

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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */
  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 400;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */
  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */
  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 400;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */
  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */
  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 400;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */
  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : HALL_A_Pin HALL_B_Pin HALL_C_Pin */
  GPIO_InitStruct.Pin = HALL_A_Pin|HALL_B_Pin|HALL_C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : DMS_Pin */
  GPIO_InitStruct.Pin = DMS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(DMS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (GPIO_Pin == HALL_A_Pin || GPIO_Pin == HALL_B_Pin || GPIO_Pin == HALL_C_Pin  /*|| GPIO_Pin == DMS_Pin */){
	HALLA_State = HAL_GPIO_ReadPin(GPIOA, HALL_A_Pin);
	HALLB_State = HAL_GPIO_ReadPin(GPIOA, HALL_B_Pin);
	HALLC_State = HAL_GPIO_ReadPin(GPIOA, HALL_C_Pin);
	DMS_State = HAL_GPIO_ReadPin(GPIOA, DMS_Pin);
	switchfase();
	}
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
  while (1){}
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
