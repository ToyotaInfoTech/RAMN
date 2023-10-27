/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 * <h2><center>&copy; Copyright (c) 2022 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
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
#include "queue.h"
#include "stm32l5xx_hal_rng.h" // contains RNG peripheral error codes
#include "ramn_debug.h"
#include "ramn_usb.h"
#include "ramn_spi.h"
#include "ramn_canfd.h"
#include "ramn_trng.h"
#include "ramn_ecucontrol.h"
#include "ramn_dbc.h"
#include "ramn_simulator.h"
#include "ramn_sensors.h"
#include "ramn_actuators.h"
#include "ramn_crc.h"
#if defined(ENABLE_DIAG)
#include "ramn_diag.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
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

CRC_HandleTypeDef hcrc;

FDCAN_HandleTypeDef hfdcan1;

IWDG_HandleTypeDef hiwdg;

RNG_HandleTypeDef hrng;

SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;

/* Definitions for RAMN_ReceiveCAN */
osThreadId_t RAMN_ReceiveCANHandle;
uint32_t RAMN_ReceiveCANFuncBuffer[ 128 ];
osStaticThreadDef_t RAMN_ReceiveCANFuncControlBlock;
const osThreadAttr_t RAMN_ReceiveCAN_attributes = {
  .name = "RAMN_ReceiveCAN",
  .stack_mem = &RAMN_ReceiveCANFuncBuffer[0],
  .stack_size = sizeof(RAMN_ReceiveCANFuncBuffer),
  .cb_mem = &RAMN_ReceiveCANFuncControlBlock,
  .cb_size = sizeof(RAMN_ReceiveCANFuncControlBlock),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RAMN_SendCAN */
osThreadId_t RAMN_SendCANHandle;
uint32_t RAMN_SendCANBuffer[ 128 ];
osStaticThreadDef_t RAMN_SendCANControlBlock;
const osThreadAttr_t RAMN_SendCAN_attributes = {
  .name = "RAMN_SendCAN",
  .stack_mem = &RAMN_SendCANBuffer[0],
  .stack_size = sizeof(RAMN_SendCANBuffer),
  .cb_mem = &RAMN_SendCANControlBlock,
  .cb_size = sizeof(RAMN_SendCANControlBlock),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for RAMN_Periodic */
osThreadId_t RAMN_PeriodicHandle;
uint32_t RAMN_PeriodicBuffer[ 128 ];
osStaticThreadDef_t RAMN_PeriodicControlBlock;
const osThreadAttr_t RAMN_Periodic_attributes = {
  .name = "RAMN_Periodic",
  .stack_mem = &RAMN_PeriodicBuffer[0],
  .stack_size = sizeof(RAMN_PeriodicBuffer),
  .cb_mem = &RAMN_PeriodicControlBlock,
  .cb_size = sizeof(RAMN_PeriodicControlBlock),
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for RAMN_ErrorTask */
osThreadId_t RAMN_ErrorTaskHandle;
uint32_t RAMN_ErrorTaskBuffer[ 128 ];
osStaticThreadDef_t RAMN_ErrorTaskControlBlock;
const osThreadAttr_t RAMN_ErrorTask_attributes = {
  .name = "RAMN_ErrorTask",
  .stack_mem = &RAMN_ErrorTaskBuffer[0],
  .stack_size = sizeof(RAMN_ErrorTaskBuffer),
  .cb_mem = &RAMN_ErrorTaskControlBlock,
  .cb_size = sizeof(RAMN_ErrorTaskControlBlock),
  .priority = (osPriority_t) osPriorityRealtime1,
};
/* Definitions for RAMN_DiagRX */
osThreadId_t RAMN_DiagRXHandle;
uint32_t RAMN_DiagRXBuffer[ 128 ];
osStaticThreadDef_t RAMN_DiagRXControlBlock;
const osThreadAttr_t RAMN_DiagRX_attributes = {
  .name = "RAMN_DiagRX",
  .stack_mem = &RAMN_DiagRXBuffer[0],
  .stack_size = sizeof(RAMN_DiagRXBuffer),
  .cb_mem = &RAMN_DiagRXControlBlock,
  .cb_size = sizeof(RAMN_DiagRXControlBlock),
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for RAMN_DiagTX */
osThreadId_t RAMN_DiagTXHandle;
uint32_t RAMN_DiagTXBuffer[ 128 ];
osStaticThreadDef_t RAMN_DiagTXControlBlock;
const osThreadAttr_t RAMN_DiagTX_attributes = {
  .name = "RAMN_DiagTX",
  .stack_mem = &RAMN_DiagTXBuffer[0],
  .stack_size = sizeof(RAMN_DiagTXBuffer),
  .cb_mem = &RAMN_DiagTXControlBlock,
  .cb_size = sizeof(RAMN_DiagTXControlBlock),
  .priority = (osPriority_t) osPriorityLow7,
};
/* USER CODE BEGIN PV */

#if defined(ENABLE_DIAG)
//Holds currently processed Diag Command from CAN
uint8_t diagRxbuf[0xFFF+2];
//Holds currently generated Diag Command Answer for CAN
uint8_t diagTxbuf[0xFFF] __attribute__ ((section (".buffers")));
#endif

//Buffers for UDS commands, only allocated if enabled
#if defined(ENABLE_UDS)
static StaticStreamBuffer_t UDS_ISOTP_RX_BUFFER_STRUCT;
static uint8_t UDS_ISOTP_RX_BUFFER[UDS_ISOTP_RX_BUFFER_SIZE];
static StaticStreamBuffer_t UDS_ISOTP_TX_BUFFER_STRUCT;
static uint8_t UDS_ISOTP_TX_BUFFER[UDS_ISOTP_RX_BUFFER_SIZE];
#endif

//Buffers for KWP commands, only allocated if enabled
#if defined(ENABLE_KWP)
static StaticStreamBuffer_t KWP_ISOTP_RX_BUFFER_STRUCT;
static uint8_t KWP_ISOTP_RX_BUFFER[KWP_ISOTP_RX_BUFFER_SIZE];
static StaticStreamBuffer_t KWP_ISOTP_TX_BUFFER_STRUCT;
static uint8_t KWP_ISOTP_TX_BUFFER[KWP_ISOTP_RX_BUFFER_SIZE];
#endif

//Buffers for XCP commands, only allocated if enabled
#if defined(ENABLE_XCP)
static StaticStreamBuffer_t XCP_RX_BUFFER_STRUCT;
static uint8_t XCP_RX_BUFFER[XCP_RX_BUFFER_SIZE];
static StaticStreamBuffer_t XCP_TX_BUFFER_STRUCT;
static uint8_t XCP_TX_BUFFER[XCP_RX_BUFFER_SIZE];
#endif

//Buffers for CAN Messages
static StaticStreamBuffer_t CAN_RX_BUFFER_STRUCT;
static uint8_t CAN_RX_BUFFER[CAN_RX_BUFFER_SIZE];
static StaticStreamBuffer_t CAN_TX_BUFFER_STRUCT;
static uint8_t CAN_TX_BUFFER[CAN_TX_BUFFER_SIZE];

//Buffers for Diag Messages, even unallocated one
StreamBufferHandle_t UdsRxDataStreamBufferHandle;
StreamBufferHandle_t UdsTxDataStreamBufferHandle;

StreamBufferHandle_t KwpRxDataStreamBufferHandle;
StreamBufferHandle_t KwpTxDataStreamBufferHandle;

StreamBufferHandle_t XcpRxDataStreamBufferHandle;
StreamBufferHandle_t XcpTxDataStreamBufferHandle;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ICACHE_Init(void);
static void MX_RNG_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_SPI2_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_CRC_Init(void);
void RAMN_ReceiveCANFunc(void *argument);
void RAMN_SendCANFunc(void *argument);
void RAMN_PeriodicTaskFunc(void *argument);
void RAMN_ErrorTaskFunc(void *argument);
void RAMN_DiagRXFunc(void *argument);
void RAMN_DiagTXFunc(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


#if defined(GENERATE_RUNTIME_STATS)
volatile unsigned long ulHighFrequencyTimerTicks;

void configureTimerForRunTimeStats(void) {
	ulHighFrequencyTimerTicks = 0;
	HAL_TIM_Base_Start_IT(&htim3);
}

unsigned long getRunTimeCounterValue(void) {
	return ulHighFrequencyTimerTicks;
}
#endif
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	// Ensure proper configuration of VTOR or program does not work correctly after live booting from DFU bootloader
	__disable_irq();
	SCB->VTOR = FLASH_BASE_NS;
	__DSB();
	__enable_irq();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ICACHE_Init();
  MX_RNG_Init();
  MX_FDCAN1_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

	//Assign the Stream buffer used by the USB receive callback function


#if defined(ENABLE_UDS)
	UdsRxDataStreamBufferHandle = xStreamBufferCreateStatic(UDS_ISOTP_RX_BUFFER_SIZE,sizeof(uint8_t),UDS_ISOTP_RX_BUFFER,&UDS_ISOTP_RX_BUFFER_STRUCT);
	UdsTxDataStreamBufferHandle = xStreamBufferCreateStatic(UDS_ISOTP_TX_BUFFER_SIZE,sizeof(uint8_t),UDS_ISOTP_TX_BUFFER,&UDS_ISOTP_TX_BUFFER_STRUCT);
#endif
#if defined(ENABLE_KWP)
	KwpRxDataStreamBufferHandle = xStreamBufferCreateStatic(KWP_ISOTP_RX_BUFFER_SIZE,sizeof(uint8_t),KWP_ISOTP_RX_BUFFER,&KWP_ISOTP_RX_BUFFER_STRUCT);
	KwpTxDataStreamBufferHandle = xStreamBufferCreateStatic(KWP_ISOTP_TX_BUFFER_SIZE,sizeof(uint8_t),KWP_ISOTP_TX_BUFFER,&KWP_ISOTP_TX_BUFFER_STRUCT);
#endif
#if defined(ENABLE_XCP)
	XcpRxDataStreamBufferHandle = xStreamBufferCreateStatic(XCP_RX_BUFFER_SIZE,sizeof(uint8_t),XCP_RX_BUFFER,&XCP_RX_BUFFER_STRUCT);
	XcpTxDataStreamBufferHandle = xStreamBufferCreateStatic(XCP_TX_BUFFER_SIZE,sizeof(uint8_t),XCP_TX_BUFFER,&XCP_TX_BUFFER_STRUCT);
#endif

	CANRxDataStreamBufferHandle = xStreamBufferCreateStatic(CAN_RX_BUFFER_SIZE,sizeof(uint8_t),CAN_RX_BUFFER,&CAN_RX_BUFFER_STRUCT);
	CANTxDataStreamBufferHandle = xStreamBufferCreateStatic(CAN_TX_BUFFER_SIZE,sizeof(uint8_t),CAN_TX_BUFFER,&CAN_TX_BUFFER_STRUCT);



#if defined(ENABLE_SCREEN) || defined(EXPANSION_BODY)
	RAMN_SPI_Init(&hspi2, &RAMN_PeriodicHandle);
#endif

#if defined(ENABLE_EEPROM_EMULATION)
	RAMN_EEPROM_Init();
#endif

#if defined(ENABLE_DIAG)
	RAMN_DIAG_Init(xTaskGetTickCount(),&RAMN_DiagRXHandle, &UdsRxDataStreamBufferHandle, &KwpRxDataStreamBufferHandle, &XcpRxDataStreamBufferHandle);
#endif

	RAMN_RNG_Init(&hrng);
	RAMN_CRC_Init(&hcrc);
	RAMN_FDCAN_Init(&hfdcan1,&RAMN_SendCANHandle,&RAMN_ErrorTaskHandle);

	//Fill Random numbers pool
	if (HAL_RNG_GenerateRandomNumber_IT(&hrng) != HAL_OK) Error_Handler();

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of RAMN_ReceiveCAN */
  RAMN_ReceiveCANHandle = osThreadNew(RAMN_ReceiveCANFunc, NULL, &RAMN_ReceiveCAN_attributes);

  /* creation of RAMN_SendCAN */
  RAMN_SendCANHandle = osThreadNew(RAMN_SendCANFunc, NULL, &RAMN_SendCAN_attributes);

  /* creation of RAMN_Periodic */
  RAMN_PeriodicHandle = osThreadNew(RAMN_PeriodicTaskFunc, NULL, &RAMN_Periodic_attributes);

  /* creation of RAMN_ErrorTask */
  RAMN_ErrorTaskHandle = osThreadNew(RAMN_ErrorTaskFunc, NULL, &RAMN_ErrorTask_attributes);

  /* creation of RAMN_DiagRX */
  RAMN_DiagRXHandle = osThreadNew(RAMN_DiagRXFunc, NULL, &RAMN_DiagRX_attributes);

  /* creation of RAMN_DiagTX */
  RAMN_DiagTXHandle = osThreadNew(RAMN_DiagTXFunc, NULL, &RAMN_DiagTX_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV8;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the common periph clock
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_FDCAN;
  PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLLSAI1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSAI1SOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 2;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 56;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV8;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_SAI1CLK|RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
#if defined(EXPANSION_CHASSIS) || defined(EXPANSION_POWERTRAIN) || defined(EXPANSION_BODY)
  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV256;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
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

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
#endif
  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 1;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 60;
  hfdcan1.Init.NominalTimeSeg2 = 19;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 15;
  hfdcan1.Init.DataTimeSeg2 = 2;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */
#ifdef WATCHDOG_ENABLE
  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 1000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */
#endif
  /* USER CODE END IWDG_Init 2 */

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
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
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
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
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
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA2_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel1_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ECUC_EN_Pin|ECUC_BOOT0_Pin|ECUD_BOOT0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ECUD_EN_Pin|TPM_nCS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_nCS_Pin|LCD_DC_Pin|ECUB_EN_Pin|FDCAN1_STB_Pin
                          |ECUB_BOOT0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ECUC_EN_Pin ECUC_BOOT0_Pin ECUD_BOOT0_Pin */
  GPIO_InitStruct.Pin = ECUC_EN_Pin|ECUC_BOOT0_Pin|ECUD_BOOT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ECUD_EN_Pin TPM_nCS_Pin */
  GPIO_InitStruct.Pin = ECUD_EN_Pin|TPM_nCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_nCS_Pin LCD_DC_Pin ECUB_EN_Pin FDCAN1_STB_Pin
                           ECUB_BOOT0_Pin */
  GPIO_InitStruct.Pin = LCD_nCS_Pin|LCD_DC_Pin|ECUB_EN_Pin|FDCAN1_STB_Pin
                          |ECUB_BOOT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_USB;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_RAMN_ReceiveCANFunc */
/**
 * @brief Function implementing the RAMN_ReceiveCAN thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_ReceiveCANFunc */
void RAMN_ReceiveCANFunc(void *argument)
{
  /* USER CODE BEGIN 5 */

		FDCAN_RxHeaderTypeDef CANRxHeader;
		uint8_t CANRxData[64];

		for(;;)
		{
			if (xStreamBufferReceive(CANRxDataStreamBufferHandle, (void *)&CANRxHeader,sizeof(CANRxHeader), portMAX_DELAY) == sizeof(CANRxHeader))
			{
				uint8_t payloadSize = DLCtoUINT8(CANRxHeader.DataLength);
				if (payloadSize > 0)
				{
					if (xStreamBufferReceive(CANRxDataStreamBufferHandle, (void *) CANRxData,payloadSize, portMAX_DELAY ) != payloadSize) Error_Handler();
				}

				if(CANRxHeader.RxFrameType == FDCAN_DATA_FRAME) {
					RAMN_DBC_ProcessCANMessage(CANRxHeader.Identifier,payloadSize,(RAMN_CANFrameData_t*)CANRxData);
	#if defined(ENABLE_DIAG)
					RAMN_DIAG_ProcessRxCANMessage(&CANRxHeader, CANRxData, xTaskGetTickCount());
	#endif
				}

			}
			else
			{
				Error_Handler();
			}

		}

  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_RAMN_SendCANFunc */
/**
 * @brief Function implementing the RAMN_SendCAN thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_SendCANFunc */
void RAMN_SendCANFunc(void *argument)
{
  /* USER CODE BEGIN RAMN_SendCANFunc */
	/* Infinite loop */
	FDCAN_TxHeaderTypeDef CANTxHeader;
	uint8_t CANTxData[64];
	uint8_t payloadSize;
	/* Infinite loop */
	for(;;)
	{
		uint32_t index = 0;
		while(index < sizeof(CANTxHeader))
		{
			index += xStreamBufferReceive(CANTxDataStreamBufferHandle, (void *)(&CANTxHeader+index),sizeof(CANTxHeader)-index, portMAX_DELAY);
		}
		payloadSize = DLCtoUINT8(CANTxHeader.DataLength);
		if (CANTxHeader.TxFrameType == FDCAN_REMOTE_FRAME) payloadSize = 0;  // no payload for remote requests
		if (payloadSize > 0 )
		{
			index = 0;
			while(index < payloadSize) index += xStreamBufferReceive(CANTxDataStreamBufferHandle, (void *)(CANTxData+index),payloadSize-index, portMAX_DELAY);
		}

		// Wait for TX space to be available in FIFO
		while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) < 1)
		{
			ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		}

		if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &CANTxHeader, CANTxData) != HAL_OK) Error_Handler();
		RAMN_FDCAN_Status.CANTXRequestCnt++;

	}
  /* USER CODE END RAMN_SendCANFunc */
}

/* USER CODE BEGIN Header_RAMN_PeriodicTaskFunc */
/**
 * @brief Function implementing the RAMN_Periodic thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_PeriodicTaskFunc */
void RAMN_PeriodicTaskFunc(void *argument)
{
  /* USER CODE BEGIN RAMN_PeriodicTaskFunc */
	RAMN_DBC_Init();
	RAMN_FDCAN_ResetPeripheral();

#if defined(EXPANSION_CHASSIS) || defined(EXPANSION_POWERTRAIN) || defined(EXPANSION_BODY)
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)RAMN_SENSORS_ADCValues, 3);
	RAMN_SENSORS_Init();
#endif
	RAMN_ACTUATORS_Init();
	RAMN_SIM_Init();

	/* Infinite loop */
	for(;;)
	{
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
#ifdef WATCHDOG_ENABLE
		if(HAL_IWDG_Refresh(&hiwdg) != HAL_OK) { Error_Handler();  }
#endif
		if (RAMN_DBC_RequestSilence == 0U)
		{
			RAMN_SENSORS_Update(xLastWakeTime);
			RAMN_SIM_UpdatePeriodic(xLastWakeTime);
			RAMN_ACTUATORS_ApplyControls(xLastWakeTime);
			RAMN_DBC_Send(xLastWakeTime);
		}
#ifdef ENABLE_SCREEN
		RAMN_SPI_UpdateScreen(xLastWakeTime);
#endif

#if defined(ENABLE_DIAG)
		RAMN_DIAG_Update(xLastWakeTime);
#endif
		vTaskDelayUntil(&xLastWakeTime, SIM_LOOP_CLOCK_MS);

#if defined (ENABLE_USB) && defined(ENABLE_USB_AUTODETECT)
		if (RAMN_USB_Config.serialOpened == False)
		{
			RAMN_USB_Config.simulatorActive = False;
			RAMN_DBC_RequestSilence = True;
		}
#endif
	}
  /* USER CODE END RAMN_PeriodicTaskFunc */
}

/* USER CODE BEGIN Header_RAMN_ErrorTaskFunc */
/**
 * @brief Function implementing the RAMN_ErrorTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_ErrorTaskFunc */
void RAMN_ErrorTaskFunc(void *argument)
{
  /* USER CODE BEGIN RAMN_ErrorTaskFunc */
	/* Infinite loop */

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//Read all data in a critical section for consistent readings

		//If a transmission failed, the "transmission complete" notification will not be sent. We may need to wake up the CAN send thread (?) TODO: check
		//Normally, we should only require it if protocolStatus.Activity == FDCAN_COM_STATE_TX, but it is safer to notify the thread whatever the error is.
		/* if (protocolStatus.Activity == FDCAN_COM_STATE_TX) */
		xTaskNotifyGive(RAMN_SendCANHandle);

	}
  /* USER CODE END RAMN_ErrorTaskFunc */
}

/* USER CODE BEGIN Header_RAMN_DiagRXFunc */
/**
 * @brief Function implementing the RAMN_DiagRX thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_DiagRXFunc */
void RAMN_DiagRXFunc(void *argument)
{
  /* USER CODE BEGIN RAMN_DiagRXFunc */
	/* Infinite loop */
#if !defined(ENABLE_DIAG)
	vTaskDelete(NULL);
#else
	uint16_t diagRxSize;
	uint16_t index;
	uint16_t diagTxSize;
	size_t xBytesSent;

	for(;;)
	{
#ifdef ENABLE_UDS
		//Check UDS
		if (xStreamBufferBytesAvailable(UdsRxDataStreamBufferHandle) >= sizeof(diagRxSize))
		{
			if (xStreamBufferReceive(UdsRxDataStreamBufferHandle, (void *)&diagRxSize,sizeof(diagRxSize), portMAX_DELAY) == sizeof(diagRxSize))
			{
				if (diagRxSize <= 0xFFF)
				{
					index = 0;
					while (index != diagRxSize)
					{
						index += xStreamBufferReceive(UdsRxDataStreamBufferHandle, (void *)diagRxbuf+index,diagRxSize-index, portMAX_DELAY);
					}


					RAMN_UDS_ProcessDiagPayload(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
					if (diagTxSize > 0U)
					{
						xBytesSent = xStreamBufferSend(UdsTxDataStreamBufferHandle, (void *) &diagTxSize, sizeof(diagTxSize), portMAX_DELAY );
						xBytesSent += xStreamBufferSend(UdsTxDataStreamBufferHandle, (void *) diagTxbuf, diagTxSize, portMAX_DELAY );
						if( xBytesSent != (diagTxSize + sizeof(diagTxSize) )) Error_Handler();
						xTaskNotifyGive(RAMN_DiagTXHandle);
					}
				}
			}
		}
#endif
#ifdef ENABLE_KWP
		//Check KWP
		if (xStreamBufferBytesAvailable(KwpRxDataStreamBufferHandle) >= sizeof(diagRxSize))
		{
			if (xStreamBufferReceive(KwpRxDataStreamBufferHandle, (void *)&diagRxSize,sizeof(diagRxSize), portMAX_DELAY) == sizeof(diagRxSize))
			{
				if (diagRxSize <= 0xFFF)
				{
					index = 0;
					while (index != diagRxSize)
					{
						index += xStreamBufferReceive(KwpRxDataStreamBufferHandle, (void *)diagRxbuf+index,diagRxSize-index, portMAX_DELAY);
					}


					RAMN_KWP_ProcessDiagPayload(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
					if (diagTxSize > 0U)
					{
						xBytesSent = xStreamBufferSend(KwpTxDataStreamBufferHandle, (void *) &diagTxSize, sizeof(diagTxSize), portMAX_DELAY );
						xBytesSent += xStreamBufferSend(KwpTxDataStreamBufferHandle, (void *) diagTxbuf, diagTxSize, portMAX_DELAY );
						if( xBytesSent != (diagTxSize + sizeof(diagTxSize) )) Error_Handler();
						xTaskNotifyGive(RAMN_DiagTXHandle);
					}
				}
			}
		}
#endif
#ifdef ENABLE_XCP
		//Check XCP
		if (xStreamBufferBytesAvailable(XcpRxDataStreamBufferHandle) >= sizeof(diagRxSize))
		{
			if (xStreamBufferReceive(XcpRxDataStreamBufferHandle, (void *)&diagRxSize,sizeof(diagRxSize), portMAX_DELAY) == sizeof(diagRxSize))
			{
				if (diagRxSize <= 0xFFF)
				{
					index = 0;
					while (index != diagRxSize)
					{
						index += xStreamBufferReceive(XcpRxDataStreamBufferHandle, (void *)diagRxbuf+index,diagRxSize-index, portMAX_DELAY);
					}


					RAMN_XCP_ProcessDiagPayload(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
					if (diagTxSize > 0U)
					{
						xBytesSent = xStreamBufferSend(XcpTxDataStreamBufferHandle, (void *) &diagTxSize, sizeof(diagTxSize), portMAX_DELAY );
						xBytesSent += xStreamBufferSend(XcpTxDataStreamBufferHandle, (void *) diagTxbuf, diagTxSize, portMAX_DELAY );
						if( xBytesSent != (diagTxSize + sizeof(diagTxSize) )) Error_Handler();
						xTaskNotifyGive(RAMN_DiagTXHandle);
					}
				}
			}
		}
#endif
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
	}
#endif
  /* USER CODE END RAMN_DiagRXFunc */
}

/* USER CODE BEGIN Header_RAMN_DiagTXFunc */
/**
 * @brief Function implementing the RAMN_DiagTX thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_DiagTXFunc */
void RAMN_DiagTXFunc(void *argument)
{
  /* USER CODE BEGIN RAMN_DiagTXFunc */
	/* Infinite loop */
#if !defined(ENABLE_DIAG)
	vTaskDelete(NULL);
#else
	uint16_t index = 0U;
	for(;;)
	{

#if defined(ENABLE_UDS)
		if (xStreamBufferBytesAvailable(UdsTxDataStreamBufferHandle) >= sizeof(RAMN_UDS_ISOTPHandler.txSize))
		{
			if (xStreamBufferReceive(UdsTxDataStreamBufferHandle, (void *)&(RAMN_UDS_ISOTPHandler.txSize),sizeof(RAMN_UDS_ISOTPHandler.txSize), portMAX_DELAY) == sizeof(RAMN_UDS_ISOTPHandler.txSize))
			{
				if (RAMN_UDS_ISOTPHandler.txSize <= 0xFFF) //TODO: empty buffer securely if overflow
				{
					index = 0U;
					while (index != RAMN_UDS_ISOTPHandler.txSize)
					{
						index += xStreamBufferReceive(UdsTxDataStreamBufferHandle, (void *)(RAMN_UDS_ISOTPHandler.txData+index),RAMN_UDS_ISOTPHandler.txSize-index, portMAX_DELAY);
					}

					if (RAMN_ISOTP_RequestTx(&RAMN_UDS_ISOTPHandler, xTaskGetTickCount()) == RAMN_OK)
					{
						while (RAMN_UDS_Continue_TX(xTaskGetTickCount()) != True)
						{
							osDelay(RAMN_DBC_RequestSilence ? RAMN_UDS_ISOTPHandler.targetST : SIM_LOOP_CLOCK_MS);
						}
					}

				}
			}
		}
#endif

#if defined(ENABLE_KWP)
		if (xStreamBufferBytesAvailable(KwpTxDataStreamBufferHandle) >= sizeof(RAMN_KWP_ISOTPHandler.txSize))
		{
			if (xStreamBufferReceive(KwpTxDataStreamBufferHandle, (void *)&(RAMN_KWP_ISOTPHandler.txSize),sizeof(RAMN_KWP_ISOTPHandler.txSize), portMAX_DELAY) == sizeof(RAMN_KWP_ISOTPHandler.txSize))
			{
				if (RAMN_KWP_ISOTPHandler.txSize <= 0xFFF) //TODO: empty buffer securely if overflow
				{
					index = 0U;
					while (index != RAMN_KWP_ISOTPHandler.txSize)
					{
						index += xStreamBufferReceive(KwpTxDataStreamBufferHandle, (void *)(RAMN_KWP_ISOTPHandler.txData+index),RAMN_KWP_ISOTPHandler.txSize-index, portMAX_DELAY);
					}

					if (RAMN_ISOTP_RequestTx(&RAMN_KWP_ISOTPHandler, xTaskGetTickCount()) == RAMN_OK)
					{
						while (RAMN_KWP_Continue_TX(xTaskGetTickCount()) != True)
						{
							osDelay(RAMN_DBC_RequestSilence ? RAMN_KWP_ISOTPHandler.targetST : SIM_LOOP_CLOCK_MS);
						}
					}

				}
			}
		}
#endif

#if defined(ENABLE_XCP)
		uint16_t diagTxSize;
		uint8_t  XCP_payload[8];
		if (xStreamBufferBytesAvailable(XcpTxDataStreamBufferHandle) >= sizeof(diagTxSize))
		{
			if (xStreamBufferReceive(XcpTxDataStreamBufferHandle, (void *)&(diagTxSize),sizeof(diagTxSize), portMAX_DELAY) == sizeof(diagTxSize))
			{
				if (diagTxSize <= 0x8) //TODO: empty buffer securely if overflow
				{
					index = 0U;
					while (index != diagTxSize)
					{
						index += xStreamBufferReceive(XcpTxDataStreamBufferHandle, (void *)(XCP_payload+index),diagTxSize-index, portMAX_DELAY);
					}

					while (RAMN_XCP_Continue_TX(xTaskGetTickCount(),XCP_payload,diagTxSize) != True)
					{
						osDelay(SIM_LOOP_CLOCK_MS);
					}
				}

			}

		}
#endif
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
	}
#endif
  /* USER CODE END RAMN_DiagTXFunc */
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
