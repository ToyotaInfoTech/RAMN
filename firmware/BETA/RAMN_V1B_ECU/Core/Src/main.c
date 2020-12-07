/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 * <h2><center>&copy; Copyright (c) 2020 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
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
#include "cmsis_os.h"

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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan1;

RNG_HandleTypeDef hrng;

SPI_HandleTypeDef hspi2;

/* Definitions for receiveCANTask */
osThreadId_t receiveCANTaskHandle;
const osThreadAttr_t receiveCANTask_attributes = {
  .name = "receiveCANTask",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 1024 * 4
};
/* Definitions for send_can */
osThreadId_t send_canHandle;
const osThreadAttr_t send_can_attributes = {
  .name = "send_can",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 256 * 4
};
/* Definitions for periodic_normal */
osThreadId_t periodic_normalHandle;
const osThreadAttr_t periodic_normal_attributes = {
  .name = "periodic_normal",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for periodic_low */
osThreadId_t periodic_lowHandle;
const osThreadAttr_t periodic_low_attributes = {
  .name = "periodic_low",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};
/* Definitions for periodic_belown */
osThreadId_t periodic_belownHandle;
const osThreadAttr_t periodic_belown_attributes = {
  .name = "periodic_belown",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 256 * 4
};
/* Definitions for periodic_high */
osThreadId_t periodic_highHandle;
const osThreadAttr_t periodic_high_attributes = {
  .name = "periodic_high",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 256 * 4
};
/* USER CODE BEGIN PV */
CAN_FilterTypeDef  sFilterConfig;
CAN_RxHeaderTypeDef   RxHeader;
uint8_t               RxData[8];
uint32_t              TxMailbox;
typedef struct{
	CAN_TxHeaderTypeDef TxHeader;
	uint8_t data[8];
	uint8_t requireSend;
	uint16_t counter;
	uint32_t random;
} period_msg_t;

#define MAXIMUM_TX_MSG_COUNT 16
//declare space in memory to save messages
period_msg_t msg1;
period_msg_t msg2;
period_msg_t msg3;
period_msg_t msg4;
period_msg_t msg5;
period_msg_t msg6;
period_msg_t msg7;
period_msg_t msg8;
period_msg_t msg9;
period_msg_t msg10;
period_msg_t msg11;
period_msg_t msg12;
period_msg_t msg13;
period_msg_t msg14;
period_msg_t msg15;
period_msg_t msg16;
const period_msg_t* tx_msgs[MAXIMUM_TX_MSG_COUNT] = {&msg1, &msg2, &msg3, &msg4, &msg5, &msg6, &msg7, &msg8, &msg9, &msg10, &msg11, &msg12, &msg13, &msg14, &msg15, &msg16 };

#define ADD_RANDOM_DATA
#ifdef ADD_RANDOM_DATA
uint16_t rng_counter = 0;
#endif

//#define DOMAIN_POWERTRAIN
#define DOMAIN_CHASSIS
//#define DOMAIN_BODY


#if defined(DOMAIN_POWERTRAIN) || defined(DOMAIN_CHASSIS) || defined(DOMAIN_BODY)
TickType_t high_period = 10;
TickType_t normal_period = 50;
TickType_t belownormal_period = 100;
TickType_t low_period = 500;
#endif

#ifdef DOMAIN_POWERTRAIN
const uint16_t canids[MAXIMUM_TX_MSG_COUNT] = {0x24, 0x39, 0x77, 0x83, 0x146, 0x15A, 0x16F, 0x183, 0x18D, 0x19A, 0x3D4, 0x482};
const period_msg_t* tx_high_msgs[] = {&msg1,&msg2,&msg3,&msg4 };
const period_msg_t* tx_normal_msgs[] = {&msg5,&msg6,&msg7,&msg8,&msg9,&msg10};
const period_msg_t* tx_belownormal_msgs[] = {};
const period_msg_t* tx_low_msgs[] = {&msg11,&msg12};

#define BRAKE msg1
#define ACCEL msg2
#define SHIFT msg3
#define TURNINDICATOR msg4
#define ABS msg6
#define ENGINEMALFUNC msg9
uint8_t brake_command[2] = {0,0};
uint8_t accel_command[2] = {0,0};
uint8_t shift_command[1] = {0};
uint8_t current_shift = 0;
uint8_t turn_signal = 0;
uint8_t center_button_state = 0;
uint8_t previous_adc = 0xF;
#endif

#ifdef DOMAIN_CHASSIS
const uint16_t canids[MAXIMUM_TX_MSG_COUNT] = {0x62, 0x98, 0x198, 0x1A7, 0x1B1, 0x1D3, 0x25C, 0x271, 0x29C, 0x2B1, 0x3DE};
const period_msg_t* tx_high_msgs[] = {&msg1,&msg2};
const period_msg_t* tx_normal_msgs[] = {&msg3,&msg4,&msg5,&msg6};
const period_msg_t* tx_belownormal_msgs[] = {&msg7,&msg8,&msg9,&msg10};
const period_msg_t* tx_low_msgs[] = {&msg11};

#define STEERING msg1
#define HORN msg2
#define STEERINGMALFUNC msg3
#define LAMP msg4
#define SIDEBRAKE msg6
uint8_t steering_command[2] = {0x08,0};
uint8_t sidebrake_command[1] = {0};
#endif


#ifdef DOMAIN_BODY
const uint16_t canids[MAXIMUM_TX_MSG_COUNT] = {0x8D, 0xb4, 0x1b8, 0x1bb, 0x266, 0x27b, 0x286, 0x290, 0x2a6, 0x2bb, 0x420, 0x457, 0x461, 0x46c, 0x477};
const period_msg_t* tx_high_msgs[] = {&msg1, &msg2};
const period_msg_t* tx_normal_msgs[] = {&msg3,&msg4};
const period_msg_t* tx_belownormal_msgs[] = {&msg5,&msg6,&msg7,&msg7,&msg9,&msg10};
const period_msg_t* tx_low_msgs[] = {&msg11,&msg12,&msg13,&msg14,&msg15};

#define TI_STATE msg1
#define HL_STATE msg4
#define ENGINESTATE msg3
#define LOCKSTATE msg7

#define LED_LEFT_TURN 	(1 << 0)
#define LED_HIGH_BEAM 	(1 << 1)
#define LED_WARNING 	(1 << 2)
#define LED_ENGINE 		(1 << 3)
#define LED_TAIL 		(1 << 4)
#define LED_SIDEBRAKE 	(1 << 5)
#define LED_RIGHT_TURN 	(1 << 6)
#define LED_LOW_BEAM 	(1 << 7)

//Key positions: 00 -> right //10 -> center  11 -> left
#define KEY_POS_LEFT 3
#define KEY_POS_CENTER 2
#define KEY_POS_RIGHT 0

uint8_t lockstate = 0;
uint8_t ledstate = 0x00;

#endif

#define TX_HIGH_MSG_COUNT (sizeof(tx_high_msgs)/sizeof(period_msg_t*))
#define TX_NORMAL_MSG_COUNT (sizeof(tx_normal_msgs)/sizeof(period_msg_t*))
#define TX_BELOWNORMAL_MSG_COUNT (sizeof(tx_belownormal_msgs)/sizeof(period_msg_t*))
#define TX_LOW_MSG_COUNT (sizeof(tx_low_msgs)/sizeof(period_msg_t*))
#define TX_MSG_COUNT (TX_HIGH_MSG_COUNT + TX_NORMAL_MSG_COUNT + TX_BELOWNORMAL_MSG_COUNT + TX_LOW_MSG_COUNT)

uint16_t ADCValues[3] = {0,0,0};

uint8_t newBaud = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_SPI2_Init(void);
static void MX_RNG_Init(void);
static void MX_ADC1_Init(void);
void receiveCANFunction(void *argument);
void send_can_func(void *argument);
void periodic_normal_func(void *argument);
void periodic_low_func(void *argument);
void periodic_belownormal_func(void *argument);
void periodic_high_func(void *argument);

/* USER CODE BEGIN PFP */

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
	for(uint8_t i = 0; i < TX_MSG_COUNT;i++)
	{
		period_msg_t* msg = (period_msg_t*)tx_msgs[i];
		msg->TxHeader.StdId = canids[i];
		msg->TxHeader.ExtId = 0x00;
		msg->TxHeader.RTR = CAN_RTR_DATA;
		msg->TxHeader.IDE = CAN_ID_STD;
		msg->TxHeader.DLC = 8;
#ifdef ADD_RANDOM_DATA
		msg->TxHeader.DLC = 8;
#endif
		msg->TxHeader.TransmitGlobalTime = DISABLE;
		msg->counter = 0;
		msg->random = 0;
	}
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_SPI2_Init();
  MX_RNG_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

	// Pull STBY line of CAN transceiver low (high = transceiver OFF)
	HAL_GPIO_WritePin(CAN_STB_GPIO_Port, CAN_STB_Pin, GPIO_PIN_RESET);


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of receiveCANTask */
  receiveCANTaskHandle = osThreadNew(receiveCANFunction, NULL, &receiveCANTask_attributes);

  /* creation of send_can */
  send_canHandle = osThreadNew(send_can_func, NULL, &send_can_attributes);

  /* creation of periodic_normal */
  periodic_normalHandle = osThreadNew(periodic_normal_func, NULL, &periodic_normal_attributes);

  /* creation of periodic_low */
  periodic_lowHandle = osThreadNew(periodic_low_func, NULL, &periodic_low_attributes);

  /* creation of periodic_belown */
  periodic_belownHandle = osThreadNew(periodic_belownormal_func, NULL, &periodic_belown_attributes);

  /* creation of periodic_high */
  periodic_highHandle = osThreadNew(periodic_high_func, NULL, &periodic_high_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
#ifdef ADD_RANDOM_DATA
	//Refresh  RNG pool
	rng_counter = 0;
	if (HAL_RNG_GenerateRandomNumber_IT(&hrng) != HAL_OK)
	{
		/* RNG peripheral start-up error */
		Error_Handler();
	}
#endif
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
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

  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RNG|RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 12;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK|RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV256;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 8;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_8TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
	/*##-2- Configure the CAN Filter ###########################################*/
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
	{
		/* Filter configuration Error */
		Error_Handler();
	}

	/*##-3- Start the CAN peripheral ###########################################*/
	if (HAL_CAN_Start(&hcan1) != HAL_OK)
	{
		/* Start Error */
		Error_Handler();
	}
	/*##-4- Activate CAN RX notification #######################################*/
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}

	//Activate TX Notification
	if (HAL_CAN_ActivateNotification(&hcan1,  CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}

	//Activate Error Notification
	if (HAL_CAN_ActivateNotification(&hcan1,  CAN_IT_ERROR) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CAN_STB_GPIO_Port, CAN_STB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : POS1_Pin POS2_Pin */
  GPIO_InitStruct.Pin = POS1_Pin|POS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : POS3_Pin SW1_Pin SW2_Pin */
  GPIO_InitStruct.Pin = POS3_Pin|SW1_Pin|SW2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : CAN_STB_Pin */
  GPIO_InitStruct.Pin = CAN_STB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CAN_STB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SELF_BOOT0_Pin */
  GPIO_InitStruct.Pin = SELF_BOOT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SELF_BOOT0_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/**
 * @brief  Rx Fifo 0 message pending callback
 * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
 *         the configuration information for the specified CAN.
 * @retval None
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	/* Get RX message */
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		/* Reception Error */
		Error_Handler();
	}
	else
	{
		switch(RxHeader.StdId)
		{
#if defined(DOMAIN_POWERTRAIN)
		case 0x1A:
			//Brake command
			brake_command[0] = RxData[0];
			brake_command[1] = RxData[1];
			break;
		case 0x2F:
			accel_command[0] = RxData[0];
			accel_command[1] = RxData[1];
			break;
		case 0x6D:
			shift_command[0] = RxData[0];
			break;
#endif
#if defined(DOMAIN_CHASSIS)
		case 0x58:
			//Brake command
			steering_command[0] = RxData[0];
			steering_command[1] = RxData[1];
			break;
		case 0x1C9:
			sidebrake_command[0] = RxData[0];
			break;
#endif

#if defined(DOMAIN_BODY)
		case 0x24:
			if((uint16_t)((RxData[0]<<8)+RxData[1]) > 1) ledstate |= LED_SIDEBRAKE;
			else ledstate &= ~LED_SIDEBRAKE;
			break;
		case 0x83:
			TI_STATE.data[0] = RxData[0];
			if(RxData[0] & 0x01) ledstate |= LED_RIGHT_TURN;
			else ledstate &= ~LED_RIGHT_TURN;
			if(RxData[0] & 0x02) ledstate |= LED_LEFT_TURN;
			else ledstate &= ~LED_LEFT_TURN;
			break;
		case 0x18D:
			if(RxData[0] != 0) ledstate |= LED_ENGINE;
			else ledstate &= ~LED_ENGINE;
			break;
		case 0x198:
			if(RxData[0] != 0) ledstate |= LED_WARNING ;
			else ledstate &= ~LED_WARNING ;
			break;
		case 0x1A7:
			HL_STATE.data[0] = RxData[0];
			if(RxData[0] & 0x01) ledstate |= LED_TAIL;
			else ledstate &= ~LED_TAIL;
			if(RxData[0] & 0x02) ledstate |= LED_LOW_BEAM;
			else ledstate &= ~LED_LOW_BEAM;
			if(RxData[0] & 0x04) ledstate |= LED_HIGH_BEAM;
			else ledstate &= ~LED_HIGH_BEAM;
			break;
			//		case 0x1D3:
			//			if(RxData[0] != 0) ledstate |= LED_SIDEBRAKE  ;
			//			else ledstate &= ~LED_SIDEBRAKE  ;
			//			break;
#endif

			// ID 0x01 is reserved as message to change baudrate on the fly
		case 0x01:
			switch(RxData[0])
			{
			case 0:
				hcan1.Init.Prescaler = 400; // 10k
				break;
			case 1:
				hcan1.Init.Prescaler = 200; // 20k
				break;
			case 2:
				hcan1.Init.Prescaler = 80; // 50k
				break;
			case 3:
				hcan1.Init.Prescaler = 40; // 100k
				break;
			case 4:
				hcan1.Init.Prescaler = 32; // 125k
				break;
			case 5:
				hcan1.Init.Prescaler = 16; // 250k
				break;
			case 6:
				hcan1.Init.Prescaler = 8; // 500k
				break;
			case 7:
				hcan1.Init.Prescaler = 5; // 800k
				break;
			case 8:
				hcan1.Init.Prescaler = 4; // 1MHz
				break;
			default:
				//keep 128 divider (125k, same as STM32 CAN Bootloader) as default
				hcan1.Init.Prescaler = 128;
				break;
			}


			newBaud = 1;
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			vTaskNotifyGiveFromISR(receiveCANTaskHandle,&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

			break;
			// ID 0x02 is used to change the period of messages
			case 0x02:
				switch(RxData[0])
				{
				case 0:
					high_period = (TickType_t)((RxData[1]<<8) + RxData[2]);
					break;
				case 1:
					normal_period = (TickType_t)((RxData[1]<<8) + RxData[2]);
					break;
				case 2:
					belownormal_period = (TickType_t)((RxData[1]<<8) + RxData[2]);
					break;
				case 3:
					low_period   = (TickType_t)((RxData[1]<<8) + RxData[2]);
					break;
				}

				break;
		}

	}
}


void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(send_canHandle,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(send_canHandle,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(send_canHandle,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	uint32_t err = HAL_CAN_GetError(hcan);
	if ((err & (HAL_CAN_ERROR_TX_ALST0 | HAL_CAN_ERROR_TX_TERR0)) != 0)
	{
		//Error in Mailbox 0
		//HAL_CAN_AbortTxRequest(hcan,CAN_TX_MAILBOX0);
	}
	if ((err & (HAL_CAN_ERROR_TX_ALST1 | HAL_CAN_ERROR_TX_TERR1)) != 0)
	{
		//Error in Mailbox 1
		//HAL_CAN_AbortTxRequest(hcan,CAN_TX_MAILBOX1);
	}
	if ((err & (HAL_CAN_ERROR_TX_ALST2 | HAL_CAN_ERROR_TX_TERR2)) != 0)
	{
		//Error in Mailbox 2
		//HAL_CAN_AbortTxRequest(hcan,CAN_TX_MAILBOX2);
	}
	HAL_CAN_ResetError(hcan);
}

void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef* hrng, uint32_t random32bit)
{

	/* Straight random number retrieval */
	period_msg_t* msg = (period_msg_t*)tx_msgs[rng_counter];
	msg->random = random32bit;
	rng_counter++;

	/* HAL_RNG_IRQHandler() disables IT at each interruption,
     so, need to re-enable IT in callback to get several random numbers in a row */
	if (rng_counter < MAXIMUM_TX_MSG_COUNT)
	{
		HAL_RNG_GenerateRandomNumber_IT(hrng);
	}

}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_receiveCANFunction */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_receiveCANFunction */
void receiveCANFunction(void *argument)
{
  /* USER CODE BEGIN 5 */


	/* Infinite loop */
	for(;;)
	{

		if(newBaud != 0)
		{
			if (HAL_CAN_Init(&hcan1) != HAL_OK)
			{
				Error_Handler();
			}
			//Update CAN adapter with new baudrate settings
			if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
			{
				/* Filter configuration Error */
				Error_Handler();
			}

			/*##-3- Start the CAN peripheral ###########################################*/
			if (HAL_CAN_Start(&hcan1) != HAL_OK)
			{
				/* Start Error */
				Error_Handler();
			}

			/*##-4- Activate CAN RX notification #######################################*/
			if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
			{
				/* Notification Error */
				Error_Handler();
			}
			if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
			{
				/* Notification Error */
				Error_Handler();
			}
			newBaud = 0;
		}
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY  );


	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_send_can_func */
/**
 * @brief Function implementing the send_can thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_send_can_func */
void send_can_func(void *argument)
{
  /* USER CODE BEGIN send_can_func */

	for(;;)
	{
		for(uint8_t i = 0;i < TX_MSG_COUNT;i++)
		{
			period_msg_t* msg = (period_msg_t*)tx_msgs[i];
			if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0 )
			{
				if (msg->requireSend != 0)
				{

#if defined(ADD_RANDOM_DATA)
					msg->data[2] = (uint8_t)(msg->counter >> 8);
					msg->data[3] = (uint8_t)msg->counter;
					msg->data[4] = (uint8_t)(msg->random >> 24);
					msg->data[5] = (uint8_t)(msg->random >> 16);
					msg->data[6] = (uint8_t)(msg->random >> 8);
					msg->data[7] = (uint8_t)(msg->random);
#endif

msg->counter++;

if (HAL_CAN_AddTxMessage(&hcan1, &msg->TxHeader, msg->data, &TxMailbox) != HAL_OK)
{
	/* Transmission request Error */
	Error_Handler();
}
else
{

	msg->requireSend = 0;
}
				}
				else
				{
					continue;
				}
			}
			else
			{
				//No TX space, go nap nap
				ulTaskNotifyTake( pdTRUE, 1000 );
				//Restart from 0 when wake up
				i = -1;
			}
		}
		//If we reach this, it means all messages had a chance to be sent.
		//Go sleep sleep
		ulTaskNotifyTake( pdTRUE, 1000 );

	}

  /* USER CODE END send_can_func */
}

/* USER CODE BEGIN Header_periodic_normal_func */
/**
 * @brief Function implementing the periodic_normal thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_periodic_normal_func */
void periodic_normal_func(void *argument)
{
  /* USER CODE BEGIN periodic_normal_func */
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{

#if defined(DOMAIN_CHASSIS)
		uint8_t sidebrake_switch = 0;
		uint8_t horn_switch = 0;
		if (!HAL_GPIO_ReadPin(POS1_GPIO_Port, POS1_Pin)) {LAMP.data[0] = 7;}
		else if (!HAL_GPIO_ReadPin(POS2_GPIO_Port, POS2_Pin)) {LAMP.data[0] = 3;}
		else if (!HAL_GPIO_ReadPin(POS3_GPIO_Port, POS3_Pin)) {LAMP.data[0] = 1;}
		else LAMP.data[0] = 0;
		if(!HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)) sidebrake_switch = 0;
		else sidebrake_switch = 1;
		if(HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)) horn_switch = 0;
		else horn_switch = 1;

		SIDEBRAKE.data[0] = sidebrake_switch | sidebrake_command[0];
		HORN.data[0] = horn_switch;
#endif

#if defined(DOMAIN_BODY)
		uint8_t keystate = 0;
		if (HAL_GPIO_ReadPin(POS1_GPIO_Port, POS1_Pin)) {keystate |= 1;}
		if (HAL_GPIO_ReadPin(POS2_GPIO_Port, POS2_Pin)) {keystate |= 2;}
		if(keystate == KEY_POS_RIGHT) ENGINESTATE.data[0] = 0x01;
		else ENGINESTATE.data[0] = 0x00;
#endif

		for(uint8_t i = 0;i < TX_NORMAL_MSG_COUNT;i++)
		{
			period_msg_t* msg = (period_msg_t*)tx_normal_msgs[i];
			msg->requireSend = 1;
		}
		vTaskDelayUntil( &xLastWakeTime, normal_period );
	}
  /* USER CODE END periodic_normal_func */
}

/* USER CODE BEGIN Header_periodic_low_func */
/**
 * @brief Function implementing the periodic_low thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_periodic_low_func */
void periodic_low_func(void *argument)
{
  /* USER CODE BEGIN periodic_low_func */
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		for(uint8_t i = 0;i < TX_LOW_MSG_COUNT;i++)
		{
			period_msg_t* msg = (period_msg_t*)tx_low_msgs[i];
			msg->requireSend = 1;
		}
		//notify CAN sending thread that new messages have been added
		xTaskNotifyGive(send_canHandle);
		vTaskDelayUntil( &xLastWakeTime, low_period );
	}
  /* USER CODE END periodic_low_func */
}

/* USER CODE BEGIN Header_periodic_belownormal_func */
/**
 * @brief Function implementing the periodic_belown thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_periodic_belownormal_func */
void periodic_belownormal_func(void *argument)
{
  /* USER CODE BEGIN periodic_belownormal_func */
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
#if defined(DOMAIN_BODY)
		uint8_t keystate = 0;
		if (HAL_GPIO_ReadPin(POS1_GPIO_Port, POS1_Pin)) {keystate |= 1;}
		if (HAL_GPIO_ReadPin(POS2_GPIO_Port, POS2_Pin)) {keystate |= 2;}
		if(keystate == KEY_POS_LEFT) LOCKSTATE.data[0] = 0x01;
		else LOCKSTATE.data[0] = 0x00;
#endif
		for(uint8_t i = 0;i < TX_BELOWNORMAL_MSG_COUNT;i++)
		{
			period_msg_t* msg = (period_msg_t*)tx_belownormal_msgs[i];
			msg->requireSend = 1;

		}
		//notify CAN sending thread that new messages have been added
		xTaskNotifyGive(send_canHandle);
		vTaskDelayUntil( &xLastWakeTime, belownormal_period );
	}
  /* USER CODE END periodic_belownormal_func */
}

/* USER CODE BEGIN Header_periodic_high_func */
/**
 * @brief Function implementing the periodic_high thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_periodic_high_func */
void periodic_high_func(void *argument)
{
  /* USER CODE BEGIN periodic_high_func */
	uint16_t tmp = 0;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

#if defined(DOMAIN_POWERTRAIN)
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADCValues, 3);
#elif defined(DOMAIN_CHASSIS)
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADCValues, 1);

#elif defined(DOMAIN_BODY)
	//OE must be set to 0 to activate LED driver output
	HAL_GPIO_WritePin(OE_GPIO_Port, OE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LE_GPIO_Port, LE_Pin, GPIO_PIN_SET);
	//LE must be set to 1 to activate latching of output register

#endif
	for(;;)
	{
#if defined(DOMAIN_POWERTRAIN)

		if(((ADCValues[0]) > 10) || ((ADCValues[1]) > 10)) //if there is action on any of the pedals, override other commands
		{
			BRAKE.data[0] = (uint8_t)((ADCValues[0]>>8)&0xFF);
			BRAKE.data[1] = (uint8_t)ADCValues[0]&0xFF;
			ACCEL.data[0] = (uint8_t)((ADCValues[1]>>8)&0xFF);
			ACCEL.data[1] = (uint8_t)ADCValues[1]&0xFF;
			ENGINEMALFUNC.data[0] = 0x01; //note an engine malfunction
		}
		else
		{
			BRAKE.data[0] = brake_command[0];
			BRAKE.data[1] = brake_command[1];
			ACCEL.data[0] = accel_command[0];
			ACCEL.data[1] = accel_command[1];
			ENGINEMALFUNC.data[0] = 0x00;
		}

		//MSB OF ADC OF JOYSTICK: 0F -> released 0A -> CENTER 01 -> UP 04 -> DOWN 0C -> RIGHT //07 -> LEFT
		tmp = ADCValues[2] = (uint8_t)((ADCValues[2]>>8)&0xFF);
		if(tmp != previous_adc)
		{
			switch(tmp)
			{
			case 0x0F:
				//button is released:
				break;
			case 0x0A:
				//center button
				center_button_state ^= 0x01;
				break;
			case 0x1:
				//up button
				current_shift++;
				break;
			case 0x4:
				//down button
				current_shift--;
				break;
			case 0xc:
				//right button
				if(turn_signal == 2) turn_signal = 0;

				turn_signal ^= 1;
				break;
			case 0x7:
				//left button
				if(turn_signal == 1) turn_signal = 0;
				turn_signal ^= 2;
				break;
			}
		}
		previous_adc = tmp;

		SHIFT.data[0] = (uint8_t)current_shift;
		TURNINDICATOR.data[0] = (uint8_t)turn_signal;
		ABS.data[0] = (uint8_t)center_button_state;

#elif defined(DOMAIN_BODY)
		HAL_SPI_Transmit(&hspi2, (uint8_t *)&ledstate, 1, HAL_MAX_DELAY);
		//HAL_GPIO_WritePin(LE_GPIO_Port, LE_Pin, GPIO_PIN_RESET);
#elif defined(DOMAIN_CHASSIS)
		if((ADCValues[0] > 0x810) || (ADCValues[0] < 0x7F0))  //if there is action on any of the pedals, override other commands
		{
			STEERING.data[0] = (uint8_t)((ADCValues[0]>>8)&0xFF);
			STEERING.data[1] = (uint8_t)ADCValues[0]&0xFF;
			STEERINGMALFUNC.data[0] = 0x01; //note a steering malfunction
		}
		else
		{
			STEERING.data[0] = steering_command[0];
			STEERING.data[1] = steering_command[1];
			STEERINGMALFUNC.data[0] = 0x00;
		}
#endif

		for(uint8_t i = 0;i < TX_HIGH_MSG_COUNT;i++)
		{
			period_msg_t* msg = (period_msg_t*)tx_high_msgs[i];
			//msg->requireSend = 0;
			//msg->data[0] = 0x04;
			//msg->data[1] = 0x01;
			msg->requireSend = 1;
		}
		//notify CAN sending thread that new messages have been added
		xTaskNotifyGive(send_canHandle);

#ifdef ADD_RANDOM_DATA
		//Refresh  RNG pool
		rng_counter = 0;
		if (HAL_RNG_GenerateRandomNumber_IT(&hrng) != HAL_OK)
		{
			/* RNG peripheral start-up error */
			Error_Handler();
		}
#endif
		vTaskDelayUntil( &xLastWakeTime, high_period );
	}
  /* USER CODE END periodic_high_func */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
