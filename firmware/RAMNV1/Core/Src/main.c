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
#include "usb_device.h"

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
#if defined(ENABLE_SCREEN)
#include "ramn_screen.h"
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

/* Definitions for RAMN_ReceiveUSB */
osThreadId_t RAMN_ReceiveUSBHandle;
uint32_t RAMN_ReceiveUSBFuncBuffer[ 256 ];
osStaticThreadDef_t RAMN_ReceiveUSBFuncControlBlock;
const osThreadAttr_t RAMN_ReceiveUSB_attributes = {
		.name = "RAMN_ReceiveUSB",
		.stack_mem = &RAMN_ReceiveUSBFuncBuffer[0],
		.stack_size = sizeof(RAMN_ReceiveUSBFuncBuffer),
		.cb_mem = &RAMN_ReceiveUSBFuncControlBlock,
		.cb_size = sizeof(RAMN_ReceiveUSBFuncControlBlock),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RAMN_ReceiveCAN */
osThreadId_t RAMN_ReceiveCANHandle;
uint32_t RAMN_ReceiveCANBuffer[ 256 ];
osStaticThreadDef_t RAMN_ReceiveCANControlBlock;
const osThreadAttr_t RAMN_ReceiveCAN_attributes = {
		.name = "RAMN_ReceiveCAN",
		.stack_mem = &RAMN_ReceiveCANBuffer[0],
		.stack_size = sizeof(RAMN_ReceiveCANBuffer),
		.cb_mem = &RAMN_ReceiveCANControlBlock,
		.cb_size = sizeof(RAMN_ReceiveCANControlBlock),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RAMN_SendCAN */
osThreadId_t RAMN_SendCANHandle;
uint32_t RAMN_SendCANBuffer[ 256 ];
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
uint32_t RAMN_PeriodicBuffer[ 512 ];
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
uint32_t RAMN_ErrorTaskBuffer[ 256 ];
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
uint32_t RAMN_DiagRXBuffer[ 1024 ];
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
uint32_t RAMN_DiagTXBuffer[ 512 ];
osStaticThreadDef_t RAMN_DiagTXControlBlock;
const osThreadAttr_t RAMN_DiagTX_attributes = {
		.name = "RAMN_DiagTX",
		.stack_mem = &RAMN_DiagTXBuffer[0],
		.stack_size = sizeof(RAMN_DiagTXBuffer),
		.cb_mem = &RAMN_DiagTXControlBlock,
		.cb_size = sizeof(RAMN_DiagTXControlBlock),
		.priority = (osPriority_t) osPriorityLow7,
};
/* Definitions for RAMN_SendUSB */
osThreadId_t RAMN_SendUSBHandle;
uint32_t RAMN_SendUSBBuffer[ 256 ];
osStaticThreadDef_t RAMN_SendUSBControlBlock;
const osThreadAttr_t RAMN_SendUSB_attributes = {
		.name = "RAMN_SendUSB",
		.stack_mem = &RAMN_SendUSBBuffer[0],
		.stack_size = sizeof(RAMN_SendUSBBuffer),
		.cb_mem = &RAMN_SendUSBControlBlock,
		.cb_size = sizeof(RAMN_SendUSBControlBlock),
		.priority = (osPriority_t) osPriorityRealtime,
};
/* USER CODE BEGIN PV */

#if defined(ENABLE_USB)
static StaticStreamBuffer_t USB_RX_BUFFER_STRUCT;
static uint8_t USB_RX_BUFFER[USB_RX_BUFFER_SIZE];
StreamBufferHandle_t USBD_RxStreamBufferHandle;

static StaticStreamBuffer_t USB_TX_BUFFER_STRUCT;
static uint8_t USB_TX_BUFFER[USB_TX_BUFFER_SIZE];
StreamBufferHandle_t USBD_TxStreamBufferHandle;

//Holds currently processed USB RX Buffer
uint8_t USBRxBuffer[USB_COMMAND_BUFFER_SIZE];
//Holds currently generated slcan command (Used by CAN receiving thread)
uint8_t slCAN_USBTxBuffer[0x200];
//Holds USB DATA currently being sent over USB. TODO: remove ?
uint8_t USBIntermediateTxBuffer[APP_TX_DATA_SIZE];
#endif

#if defined(ENABLE_DIAG)
//Holds currently processed Diag Command from CAN
uint8_t diagRxbuf[0xFFF+2];
//Holds currently generated Diag Command Answer for CAN
uint8_t diagTxbuf[0xFFF];

#if defined(ENABLE_USB)
//Holds currently processed Diag Command from USB
uint8_t diagRxUSBbuf[0xFFF+2];
//Holds currently processed Diag Command Answer from USB
uint8_t diagTxUSBbuf[0xFFF+2];


#ifdef START_IN_CLI_MODE
uint8_t USB_CLI_ENABLE = 1U;
#else
uint8_t USB_CLI_ENABLE = 0U;
#endif
#define LOCAL_USB_COMMAND_BUFFER_SIZE  0x200
uint8_t USBCommandBuffer[LOCAL_USB_COMMAND_BUFFER_SIZE];


#endif

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
void RAMN_ReceiveUSBFunc(void *argument);
void RAMN_ReceiveCANFunc(void *argument);
void RAMN_SendCANFunc(void *argument);
void RAMN_PeriodicTaskFunc(void *argument);
void RAMN_ErrorTaskFunc(void *argument);
void RAMN_DiagRXFunc(void *argument);
void RAMN_DiagTXFunc(void *argument);
void RAMN_SendUSBFunc(void *argument);

/* USER CODE BEGIN PFP */
#ifdef ENABLE_USB
static uint16_t reportFIFOStatus_USB(uint8_t* usbSendBuffer);
#endif
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef ENABLE_USB
//Reports the status of each Stream Buffer over USB
static uint16_t reportFIFOStatus_USB(uint8_t* usbSendBuffer)
{
	uint16_t index = 0U;

	usbSendBuffer[index++] = 'q';

	//Send RX FIFO Fill Level
	index += uint32toASCII(HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1,FDCAN_RX_FIFO0),&usbSendBuffer[index]);

	//Send TX FIFO Free Level
	index += uint32toASCII(HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1),&usbSendBuffer[index]);

	//send CAN RX Stream Buffer levels
	index += uint32toASCII(xStreamBufferSpacesAvailable(CANRxDataStreamBufferHandle),&usbSendBuffer[index]);
	index += uint32toASCII(xStreamBufferBytesAvailable(CANRxDataStreamBufferHandle),&usbSendBuffer[index]);

	//send CAN TX Stream Buffer levels
	index += uint32toASCII(xStreamBufferSpacesAvailable(CANTxDataStreamBufferHandle),&usbSendBuffer[index]);
	index += uint32toASCII(xStreamBufferBytesAvailable(CANTxDataStreamBufferHandle),&usbSendBuffer[index]);

	usbSendBuffer[index++] = '\r';
	return index;
}
#endif

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

#if defined(TARGET_ECUA)
	//Check that ECU A BOOT option bytes are properly configured to hardware BOOT0, which is set pulled-up by default.
	RAMN_FLASH_ConfigureOptionBytesApplicationMode(); //TODO: remove ?
#endif
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
#if defined(ENABLE_USB)
	USBD_RxStreamBufferHandle   = xStreamBufferCreateStatic(USB_RX_BUFFER_SIZE,sizeof(uint8_t),USB_RX_BUFFER,&USB_RX_BUFFER_STRUCT);
	USBD_TxStreamBufferHandle   = xStreamBufferCreateStatic(USB_TX_BUFFER_SIZE,sizeof(uint8_t),USB_TX_BUFFER,&USB_TX_BUFFER_STRUCT);
#endif

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

#if defined(TARGET_ECUA)
	RAMN_ECU_SetEnableAll(0U);
	RAMN_ECU_SetBoot0All(0U);
#endif

#if defined(EXPANSION_BODY)
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

	//Automatically add a DTC if none is stored in memory
	uint32_t dtc_number = 0;
	if (RAMN_DTC_GetNumberOfDTC(&dtc_number) == RAMN_OK)
	{
		if (dtc_number == 0)
		{
			//no DTC, add one per ECU
#ifdef TARGET_ECUA
			uint32_t dtc_val = 0b11 << 30; //"11" for network ("U")
			dtc_val |= 0x0029 << 16; //Bus A Performance, FTB 0
#elif defined(TARGET_ECUB)
			uint32_t dtc_val = 0b01 << 30;//"01" for chassis ("C")
			dtc_val |= 0x0563 << 16; //Calibration ROM Checksum Error, FTB 0
#elif defined(TARGET_ECUC)
			uint32_t dtc_val = 0b00 << 30;//"00" for powertrain ("P")
			dtc_val |= 0x0172 << 16; //System too Rich, FTB 0
#elif defined(TARGET_ECUD)
			uint32_t dtc_val = 0b10 << 30;//"10" for body ("B")
			dtc_val |= 0x0091 << 16; //Active switch wrong state, FTB 0
#endif

			dtc_val |= 1 << 2; //mark DTC as pending.
			RAMN_DTC_AddNew(dtc_val);

		}
	}
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
	/* creation of RAMN_ReceiveUSB */
	RAMN_ReceiveUSBHandle = osThreadNew(RAMN_ReceiveUSBFunc, NULL, &RAMN_ReceiveUSB_attributes);

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

	/* creation of RAMN_SendUSB */
	RAMN_SendUSBHandle = osThreadNew(RAMN_SendUSBFunc, NULL, &RAMN_SendUSB_attributes);

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
	RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

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

	/** Enable the SYSCFG APB clock
	 */
	__HAL_RCC_CRS_CLK_ENABLE();

	/** Configures CRS
	 */
	RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
	RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
	RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
	RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
	RCC_CRSInitStruct.ErrorLimitValue = 34;
	RCC_CRSInitStruct.HSI48CalibrationValue = 32;

	HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
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
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

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

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static int countElements(char* buffer, int length) {
	int count = 0;
	int inWord = 0;

	for (int i = 0; i < length; i++) {
		if (buffer[i] == ' ') {
			inWord = 0;  // Not in a word
		} else if (inWord == 0) {
			inWord = 1;  // Start of a new word
			count++;
		}
	}
	return count;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_RAMN_ReceiveUSBFunc */
/**
 * @brief  Function implementing the RAMN_ReceiveUSB thread.
 * @param  argument: Not used
 * @retval None
 */
//TODO: better parameters sanity check
/* USER CODE END Header_RAMN_ReceiveUSBFunc */
void RAMN_ReceiveUSBFunc(void *argument)
{
	/* init code for USB_Device */
	MX_USB_Device_Init();
	/* USER CODE BEGIN 5 */
#if !defined(ENABLE_USB)
	vTaskDelete(NULL);
#else
	FDCAN_TxHeaderTypeDef CANTxHeader;
	uint8_t CANTxData[64];
	FDCAN_ProtocolStatusTypeDef protocolStatus;
	FDCAN_ErrorCountersTypeDef errorCount;

	for(;;)
	{
		uint16_t commandLength;
		size_t xBytesReceived;
		uint8_t dlc;
		uint8_t offset = 0U; //TODO: remove use of offset by using pointers and dedicated functions
		uint8_t smallResponseBuffer[50U]; //buffer for small responses

		xBytesReceived = xStreamBufferReceive(USBD_RxStreamBufferHandle, (void *)&commandLength, 2U, portMAX_DELAY );
		if (xBytesReceived != 2U) Error_Handler();

		xBytesReceived = xStreamBufferReceive(USBD_RxStreamBufferHandle, (void*)USBRxBuffer, commandLength,portMAX_DELAY);
		if (xBytesReceived != commandLength) Error_Handler();

		if (USB_CLI_ENABLE != 0U)
		{
			// command is in USBRxBuffer, length is in commandLength. There is no endline in buffer.
			//zero terminate the USB command buffer

			USBRxBuffer[commandLength] = '\0';

			if (commandLength >= LOCAL_USB_COMMAND_BUFFER_SIZE)
			{
				RAMN_USB_SendStringFromTask("Command too long.\r");
			}
			else
			{
				//must remove backspace
				uint32_t processedLength = 0;
				USBCommandBuffer[0] = '\0';

				for (uint32_t k = 0; k < commandLength; k++)
				{
					if (USBRxBuffer[k] == '\b')
					{
						if (processedLength > 0)
						{
							processedLength -= 1; //remove previous character
							USBCommandBuffer[processedLength] = '\0'; //replace previous character by 0
						}
					}
					else if (USBRxBuffer[k] != '\n')
					{

						USBCommandBuffer[processedLength] = USBRxBuffer[k];
						processedLength += 1;
					}
				}
				commandLength = processedLength;
				USBCommandBuffer[commandLength] = '\0';

				uint32_t elementCount = countElements(USBCommandBuffer, commandLength);

				if (elementCount == 0U && commandLength == 0U)
				{
					RAMN_USB_SendStringFromTask("\r>\r");
				}
				else
				{
					char *token;

					token = strtok(USBCommandBuffer, " ");

					if (token == NULL) {
						RAMN_USB_SendStringFromTask("No command found. Type \"help\" for help.\r");
					}
					else
					{
						// Compare the command to a set of possible commands
						if (strcmp(token, "help") == 0 || strcmp(token, "Help") == 0 || strcmp(token, "HELP") == 0 || strcmp(token, "man") == 0) {

							if (elementCount >= 2)
							{
								token = strtok(NULL, " ");
								if (strcmp(token, "theme") == 0) {
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
									RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Command: theme\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Description:\r");
									RAMN_USB_SendStringFromTask("    Set the color theme for the device LCD screen.\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Usage:\r");
									RAMN_USB_SendStringFromTask("    theme <theme_number>\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Options:\r");
									RAMN_USB_SendStringFromTask("    <theme_number>   The theme number to be set, ranging from 1 to 7.\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Examples:\r");
									RAMN_USB_SendStringFromTask("    - To set the device LCD screen theme to theme number 3, enter:\r");
									RAMN_USB_SendStringFromTask("      theme 3\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
								}
								else if (strcmp(token, "enable") == 0) {
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
									RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Command: enable\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Description:\r");
									RAMN_USB_SendStringFromTask("    Enable the power supply of another ECU (B, C, or D).\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Usage:\r");
									RAMN_USB_SendStringFromTask("    enable <ECU>\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Options:\r");
									RAMN_USB_SendStringFromTask("    <ECU>   The ECU to be enabled, identified by a letter (B, C, or D).\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Examples:\r");
									RAMN_USB_SendStringFromTask("    - To enable ECU B, use:\r");
									RAMN_USB_SendStringFromTask("      enable B\r");
									RAMN_USB_SendStringFromTask("    - To enable ECU C, use:\r");
									RAMN_USB_SendStringFromTask("      enable C\r");
									RAMN_USB_SendStringFromTask("    - To enable ECU D, use:\r");
									RAMN_USB_SendStringFromTask("      enable D\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
								}
								else if (strcmp(token, "disable") == 0) {
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
									RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Command: disable\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Description:\r");
									RAMN_USB_SendStringFromTask("    Disable the power supply of another ECU (B, C, or D).\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Usage:\r");
									RAMN_USB_SendStringFromTask("    disable <ECU>\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Options:\r");
									RAMN_USB_SendStringFromTask("    <ECU>   The ECU to be disabled, identified by a letter (B, C, or D).\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("Examples:\r");
									RAMN_USB_SendStringFromTask("    - To disable ECU B, use:\r");
									RAMN_USB_SendStringFromTask("      disable B\r");
									RAMN_USB_SendStringFromTask("    - To disable ECU C, use:\r");
									RAMN_USB_SendStringFromTask("      disable C\r");
									RAMN_USB_SendStringFromTask("    - To disable ECU D, use:\r");
									RAMN_USB_SendStringFromTask("      disable D\r");
									RAMN_USB_SendStringFromTask("\r");
									RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
								}
								else
								{
									RAMN_USB_SendStringFromTask("No help page found for this command.\r");
								}
								RAMN_USB_SendStringFromTask("End of command help page. Type \"help\" without argument for the general help page.\r");
							}
							else
							{
								RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
								RAMN_USB_SendStringFromTask("                      RAMN Command Help\r");
								RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
								RAMN_USB_SendStringFromTask("\r");
								RAMN_USB_SendStringFromTask("This interface allows you to interact with various RAMN functions and control the device's operations through a set of commands.\rType 'b' to go back to slcan mode.");
								RAMN_USB_SendStringFromTask("\r");
								RAMN_USB_SendStringFromTask("Public Commands:\r");
								RAMN_USB_SendStringFromTask("    - clear: Clears your serial terminal.\r");
								RAMN_USB_SendStringFromTask("    - disable: Disable the power supply for another ECU. Usage: disable <ECU>.\r");
								RAMN_USB_SendStringFromTask("    - enable: Enable the power supply for another ECU. Usage: enable <ECU>.\r");
								RAMN_USB_SendStringFromTask("    - exit: Exit this debug interface and revert to slcan mode. Usage: exit.\r");
								RAMN_USB_SendStringFromTask("    - help: Display general help, or help for a specific command when available. Usage: help <command>.\r");
								RAMN_USB_SendStringFromTask("    - b: Alias for the \"exit\" command.\r");
								RAMN_USB_SendStringFromTask("    - quit: Alias for the \"exit\" command.\r");
								RAMN_USB_SendStringFromTask("    - reset: Reset the device. Usage: reset.\r");
								RAMN_USB_SendStringFromTask("    - slcan: Alias for the \"exit\" command.\r");
								RAMN_USB_SendStringFromTask("    - theme: Set the color theme for ECU A's LCD screen. Usage: theme <theme number>.\r");
								RAMN_USB_SendStringFromTask("    - play: Play a game on ECU A's LCD screen. Usage: play <game number>.\r");
								RAMN_USB_SendStringFromTask("    - stop: Stop any ongoing game. Usage: stop.\r");
								RAMN_USB_SendStringFromTask("\r");
								RAMN_USB_SendStringFromTask("Commands are case sensitive.\r");
								RAMN_USB_SendStringFromTask("-------------------------------------------------------------\r");
							}
						}
						else if ( strcmp(token, "disable") == 0) {
							if (elementCount != 2)
							{
								RAMN_USB_SendStringFromTask("Invalid number of arguments. Type \"help disable\" for help using this command.\r");
							}
							else
							{
								token = strtok(NULL, " ");
								if (strcmp(token, "A") == 0) {
									RAMN_USB_SendStringFromTask("Cannot disable ECU A. Type \"reset\" if reset it.\r");
								} else if (strcmp(token, "B") == 0)
								{
									RAMN_USB_SendStringFromTask("Setting ECU B power supply to OFF.\r");
									RAMN_ECU_SetEnable('B',GPIO_PIN_RESET);
								} else if (strcmp(token, "C") == 0)
								{
									RAMN_USB_SendStringFromTask("Setting ECU C power supply to OFF.\r");
									RAMN_ECU_SetEnable('C',GPIO_PIN_RESET);
								}
								else if (strcmp(token, "D") == 0)
								{
									RAMN_USB_SendStringFromTask("Setting ECU D power supply to OFF.\r");
									RAMN_ECU_SetEnable('D',GPIO_PIN_RESET);
								}
								else
								{
									RAMN_USB_SendStringFromTask("Invalid ECU. Must be B, C, or D.\r");

								}
							}
						}
						else if ( strcmp(token, "enable") == 0) {
							if (elementCount != 2)
							{
								RAMN_USB_SendStringFromTask("Invalid number of arguments. Type \"help enable\" for help using this command.\r");
							}
							else
							{
								token = strtok(NULL, " ");
								if (strcmp(token, "A") == 0) {
									RAMN_USB_SendStringFromTask("ECU A is always enabled.\r");
								}
								else if (strcmp(token, "B") == 0)
								{
									RAMN_USB_SendStringFromTask("Setting ECU B power supply to ON.\r");
									RAMN_ECU_SetEnable('B',GPIO_PIN_SET);
								} else if (strcmp(token, "C") == 0)
								{
									RAMN_USB_SendStringFromTask("Setting ECU C power supply to ON.\r");
									RAMN_ECU_SetEnable('C',GPIO_PIN_SET);
								}
								else if (strcmp(token, "D") == 0)
								{
									RAMN_USB_SendStringFromTask("Setting ECU D power supply to ON.\r");
									RAMN_ECU_SetEnable('D',GPIO_PIN_SET);
								}
								else
								{
									RAMN_USB_SendStringFromTask("Invalid ECU. Must be B, C, or D.\r");
								}
							}

						}
						else if ( strcmp(token, "theme") == 0) {
							if (elementCount != 2)
							{
								RAMN_USB_SendStringFromTask("Invalid number of arguments. Type \"help theme\" for help using this command.\r");
							}
							else
							{
								token = strtok(NULL, " ");
								if (strcmp(token, "1") == 0) {  RAMN_SCREEN_UpdateTheme(1); RAMN_USB_SendStringFromTask("Updated.\r");
								} else if (strcmp(token, "2") == 0) {  RAMN_SCREEN_UpdateTheme(2); RAMN_USB_SendStringFromTask("Updated.\r");
								} else if (strcmp(token, "3") == 0) {  RAMN_SCREEN_UpdateTheme(3); RAMN_USB_SendStringFromTask("Updated.\r");
								} else if (strcmp(token, "4") == 0) {  RAMN_SCREEN_UpdateTheme(4); RAMN_USB_SendStringFromTask("Updated.\r");
								} else if (strcmp(token, "5") == 0) {  RAMN_SCREEN_UpdateTheme(5); RAMN_USB_SendStringFromTask("Updated.\r");
								} else if (strcmp(token, "6") == 0) {  RAMN_SCREEN_UpdateTheme(6); RAMN_USB_SendStringFromTask("Updated.\r");
								} else if (strcmp(token, "7") == 0) {  RAMN_SCREEN_UpdateTheme(7); RAMN_USB_SendStringFromTask("Updated.\r");

								}
								else
								{
									RAMN_USB_SendStringFromTask("Theme number not found. Try between 1 and 7.\r");
								}
							}
						}
						else if (strcmp(token, "b") == 0  || strcmp(token, "exit") == 0 || strcmp(token, "quit") == 0 || strcmp(token, "slcan") == 0) {
							USB_CLI_ENABLE = 0U;
						}

						else if (strcmp(token, "reset") == 0) {

#ifdef START_IN_CLI_MODE
							RAMN_USB_SendStringFromTask("Resetting...\r\r>");
#else
							RAMN_USB_SendStringFromTask("Resetting. Remember to first send the \"#\" command to reenter this interface.\r");
#endif
							osDelay(200);
							HAL_NVIC_SystemReset();
						}
						else if (strcmp(token, "clear") == 0) {
							RAMN_USB_SendStringFromTask("\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r");
						}
						else if (strcmp(token, "play") == 0)
						{
							if (elementCount != 2)
							{
								RAMN_USB_SendStringFromTask("Invalid number of arguments.\r");
							}
							else
							{
								token = strtok(NULL, " ");
								if (strcmp(token, "1") == 0) {  RAMN_SCREEN_StartGameFromIndex(1); RAMN_USB_SendStringFromTask("Starting game 1.\r");
								} else if (strcmp(token, "2") == 0) {  RAMN_SCREEN_StartGameFromIndex(2); RAMN_USB_SendStringFromTask("Starting game 2.\r");
								} else if (strcmp(token, "3") == 0) {  RAMN_SCREEN_StartGameFromIndex(3); RAMN_USB_SendStringFromTask("Starting game 3.\r");
								}
								else
								{
									RAMN_USB_SendStringFromTask("Game number not found. Try between 1 and 3.\r");
								}
							}

						}
						else if (strcmp(token, "stop") == 0)
						{
							if (RAMN_CHIP8_IsGameActive() != 0U)
							{
								RAMN_USB_SendStringFromTask("Stopping game.\r");
								RAMN_CHIP8_StopGame(1);
							}
							else
							{
								RAMN_USB_SendStringFromTask("No ongoing game.\r");
							}
						}
						else {
							// Handle unknown commands
							RAMN_USB_SendStringFromTask("Unknown command: ");
							RAMN_USB_SendStringFromTask(token);
							RAMN_USB_SendStringFromTask("\rType \"help\" for help. Remember the interface is case sensitive.\r");
						}

					}
				}

			}
			if (USB_CLI_ENABLE != 0U)  RAMN_USB_SendStringFromTask("\r>");

		}
		else
		{

			if ((USBRxBuffer[0] == '0') || (USBRxBuffer[0] == '1'))
			{
				//Prefix to announce FD CAN Frames
				CANTxHeader.FDFormat = FDCAN_FD_CAN;
				if (USBRxBuffer[1] == '0') CANTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
				else CANTxHeader.BitRateSwitch = FDCAN_BRS_ON;
				offset = 1;
			}
			else
			{
				CANTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
				CANTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
				offset = 0;
			}
			//Parse the 'i' suffix if it was added to alter the Error State Indicator
			if (RAMN_USB_Config.addESIFlag == True)
			{
				if(USBRxBuffer[commandLength-1] == 'i')
				{
					CANTxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
					commandLength--;
				}
				else
				{
					CANTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
				}
			}
			else
			{
				CANTxHeader.ErrorStateIndicator = RAMN_FDCAN_Status.ErrorStateIndicator;
			}
			CANTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
			CANTxHeader.MessageMarker = 0U;

			//Sending and Receiving are the most likely commands, so check for them first

			//Most common commands are treated first
			if(USBRxBuffer[0U] == 'u')
			{
				//format is u<brake12><accel12><rpm12><steer12><shift8><horn8><handbrake8>
				//eg "u{:03x}{:03x}{:03x}{:03x}{:02x}{:02x}{:02x}\r"
				RAMN_DBC_ProcessUSBBuffer(USBRxBuffer);
			}
			else if ( (USBRxBuffer[0U+offset] == 't') || (USBRxBuffer[0U+offset] == 'r') )
			{
				//'t' : Transmit Standard ID DATA
				//'r' : Transmit Standard ID RTR
				CANTxHeader.IdType = FDCAN_STANDARD_ID;
				CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
				//CANTxHeader.Identifier = (ascii_hashmap[commandBuffer[1+offset]] << 8) + (ascii_hashmap[commandBuffer[2+offset]] << 4) + (ascii_hashmap[commandBuffer[3+offset]]);
				CANTxHeader.Identifier = ASCIItoUint12(&USBRxBuffer[1U+offset]);

				dlc = ASCIItoUint4(&USBRxBuffer[4U+offset]);
				CANTxHeader.DataLength = ((dlc));

				if (USBRxBuffer[0+offset] == 'r')
				{
					CANTxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
					//for remote frames, no need to copy CANTxData.
					dlc = 0U;
				}
				offset += 5U;
				uint8_t i = 0U;

				while (offset < (commandLength-1) )
				{
					CANTxData[i++] = ASCIItoUint8(&USBRxBuffer[offset]);
					offset += 2U;
				}
				/*
			for(uint8_t i=0; i < dlc; i++)
			{
				CANTxData[i] = ASCIItoUint8(&commandBuffer[5+(i*2)+offset]);
			}*/


				while (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER)
				{
					//Buffer is Full, Try later
					osDelay(10U);
				}

#if defined(CAN_ECHO)
				RAMN_USB_SendFromTask(USBRxBuffer,commandLength);
#endif

#if defined(PROCESS_SLCAN_BY_DBC)
				RAMN_DBC_ProcessCANMessage(CANTxHeader.Identifier,dlc,(RAMN_CANFrameData_t*)CANTxData);
#endif


			}
			else if ( (USBRxBuffer[0U+offset] == 'R') || (USBRxBuffer[0U+offset] == 'T') )
			{
				//'T' : Transmit Extended ID DATA
				//'R' : Transmit Extended ID RTR
				CANTxHeader.IdType = FDCAN_EXTENDED_ID;
				CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
				CANTxHeader.Identifier =  ASCIItoUint32(&USBRxBuffer[1U+offset]);
				dlc = ASCIItoUint4(&USBRxBuffer[9U+offset]);
				CANTxHeader.DataLength = ((dlc));

				if (USBRxBuffer[0+offset] == 'R')
				{
					CANTxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
					//for remote frames, no need to copy payload
					dlc = 0;
				}

				offset += 10U;
				uint8_t i = 0U;
				while (offset < (commandLength-1) )
				{
					CANTxData[i++] = ASCIItoUint8(&USBRxBuffer[offset]);
					offset += 2U;
				}

				while (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER) osDelay(20U);

#if defined(CAN_ECHO)
				RAMN_USB_SendFromTask(USBRxBuffer,commandLength);
#endif

#if defined(PROCESS_SLCAN_BY_DBC)
				RAMN_DBC_ProcessCANMessage(CANTxHeader.Identifier,dlc,(RAMN_CANFrameData_t*)CANTxData);
#endif
			}
			else
			{
				switch(USBRxBuffer[0]){
				case 'O': //Open the channel
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
					RAMN_USB_Config.slcanOpened = True;
					RAMN_FDCAN_ResetPeripheral();
					break;
				case 'C': //Close the channel
					RAMN_USB_Config.slcanOpened = False;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'L': //Open in listening mode
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					hfdcan1.Init.Mode = FDCAN_MODE_BUS_MONITORING;
					RAMN_USB_Config.slcanOpened = True;
					RAMN_FDCAN_ResetPeripheral();
					break;
				case 'V': //Return sw version
					RAMN_USB_SendFromTask((uint8_t*)"V1 SLCAN RAMN (",15U);
					RAMN_USB_SendFromTask((uint8_t*)__DATE__,sizeof(__DATE__));
					RAMN_USB_SendFromTask((uint8_t*)" ",1U);
					RAMN_USB_SendFromTask((uint8_t*)__TIME__,sizeof(__TIME__));
					RAMN_USB_SendFromTask((uint8_t*)")\r",2U);
					break;
				case 'N': //Return serial number
					smallResponseBuffer[0] = 'N';
					for(uint8_t k = 0; k <12U; k++)
					{
						uint8toASCII(*((uint8_t*)(DEVICE_HARDWARE_ID_ADDRESS+k)),&smallResponseBuffer[1U+2*k]);
					}
					RAMN_USB_SendFromTask(smallResponseBuffer, 25U);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'S': //Set baudrate
					RAMN_FDCAN_UpdateBaudrate(USBRxBuffer[1U]);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 's':
					hfdcan1.Init.NominalTimeSeg1 = ASCIItoUint8(&USBRxBuffer[1U]);
					hfdcan1.Init.NominalTimeSeg2 = ASCIItoUint8(&USBRxBuffer[3U]);
					RAMN_FDCAN_ResetPeripheral();
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'F':
					smallResponseBuffer[0U] = USBRxBuffer[0U];
					uint8toASCII(RAMN_FDCAN_Status.slcan_flags,&smallResponseBuffer[1U]);
					smallResponseBuffer[3U] = '\r';
					RAMN_USB_SendFromTask(smallResponseBuffer,4U);
					break;
				case 'W': //Set filter mode
					if (USBRxBuffer[1] == '0')
					{
						RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_RANGE;
						RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_RANGE;
					}
					else if (USBRxBuffer[1] == '1')
					{
						RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_DUAL;
						RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_DUAL;
					}
					else if (USBRxBuffer[1] == '3')
					{
						RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_RANGE_NO_EIDM;
						RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_RANGE_NO_EIDM;
					}
					else
					{
						RAMN_FDCAN_Status.sFilterStdConfig.FilterType = FDCAN_FILTER_MASK; //Single Filter mask (Classic filter)
						RAMN_FDCAN_Status.sFilterExtConfig.FilterType = FDCAN_FILTER_MASK; //Single Filter mask (Classic filter)
					}
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'M': //Set "Acceptance Code" Register (filter)
					if (commandLength == 4U)
					{
						RAMN_FDCAN_Status.sFilterStdConfig.FilterID1 = ASCIItoUint12(&USBRxBuffer[1])&0x7FF;
						RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					}
					else if (commandLength == 9U)
					{
						RAMN_FDCAN_Status.sFilterExtConfig.FilterID1 = ASCIItoUint32(&USBRxBuffer[1])&0x7FFFFFFF;
						RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					}
					else
					{
						RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
					}
					break;
				case 'm': //Set "Acceptance Mask" Register (mask)
					if (commandLength == 4U)
					{
						RAMN_FDCAN_Status.sFilterStdConfig.FilterID2 = ASCIItoUint12(&USBRxBuffer[1])&0x7FF;
						RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					}
					else if (commandLength == 9U)
					{
						RAMN_FDCAN_Status.sFilterExtConfig.FilterID2 = ASCIItoUint32(&USBRxBuffer[1])&0x7FFFFFFF;
						RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					}
					else
					{
						RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
					}
					break;
				case 'Z': //Enable time stamp
					if (USBRxBuffer[1U] == '0') RAMN_USB_Config.slcan_enableTimestamp = False;
					else RAMN_USB_Config.slcan_enableTimestamp = True;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;

					/* BELOW ARE RAMN SPECIFIC COMMANDS */
				case 'w': //Update CAN controller settings
					RAMN_FDCAN_ResetPeripheral();
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'i': //Enable/Disable ISO mode
					if (USBRxBuffer[1U] == '0') HAL_FDCAN_DisableISOMode(&hfdcan1);
					else HAL_FDCAN_EnableISOMode(&hfdcan1);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
					//				case 'e': //Enable/Disable edge filtering
					//					if (USBRxBuffer[1U] == '0') HAL_FDCAN_DisableEdgeFiltering(&hfdcan1);
					//					else HAL_FDCAN_EnableEdgeFiltering(&hfdcan1);
					//					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					//					break;
					//				case 'g': //Enable/Disable TX Compensation
					//					if (USBRxBuffer[1U] == '0') HAL_FDCAN_DisableTxDelayCompensation(&hfdcan1);
					//					else HAL_FDCAN_EnableTxDelayCompensation(&hfdcan1);
					//					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					//					break;
				case 'G': // Sets Nominal and Data SJW
					hfdcan1.Init.NominalSyncJumpWidth = ASCIItoUint8(&USBRxBuffer[1]);
					if(commandLength > 3)
					{
						hfdcan1.Init.DataSyncJumpWidth = ASCIItoUint8(&USBRxBuffer[3]);
					}
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'a': //Enable/Disable TX auto retransmission
					if (USBRxBuffer[1U] == '0') hfdcan1.Init.AutoRetransmission = DISABLE;
					else hfdcan1.Init.AutoRetransmission = ENABLE;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'f': //Select Frame mode
					if (USBRxBuffer[1U] == '0') hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
					else if (USBRxBuffer[1U] == '1') hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
					else hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'v': //Add a "i" to frames with the ESI flag set
					if (USBRxBuffer[1U] == '0') RAMN_USB_Config.addESIFlag = False;
					else RAMN_USB_Config.addESIFlag = True;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case '@': //Select auto report of errors
					if (USBRxBuffer[1U] == '0') RAMN_USB_Config.autoreportErrors = False;
					else if (USBRxBuffer[1U] == '1') RAMN_USB_Config.autoreportErrors = True;
					break;
				case '#': //Enable CLI
					USB_CLI_ENABLE = 1U;
					RAMN_USB_SendStringFromTask("Welcome to RAMN CLI. Type 'help' for help.\r>");
					break;
				case 'd': //Enable/Disable debug reports
					if (USBRxBuffer[1U] == '0') RAMN_DEBUG_SetStatus(False);
					else if (USBRxBuffer[1U] == '1')RAMN_DEBUG_SetStatus(True);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'k': //Configure "Nominal (arbitration) phase" bit rate
					hfdcan1.Init.NominalPrescaler = ASCIItoUint16(&USBRxBuffer[1]); //16-bit prescaler
					hfdcan1.Init.NominalTimeSeg1 = ASCIItoUint8(&USBRxBuffer[5]);
					hfdcan1.Init.NominalTimeSeg2 = ASCIItoUint8(&USBRxBuffer[7]);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'K': //Configure "Data phase" bit rate
					hfdcan1.Init.DataPrescaler = ASCIItoUint8(&USBRxBuffer[1]); //8-bit prescaler
					hfdcan1.Init.DataTimeSeg1 = ASCIItoUint8(&USBRxBuffer[3]);
					hfdcan1.Init.DataTimeSeg2 = ASCIItoUint8(&USBRxBuffer[5]);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'j': //Return Random byte
					smallResponseBuffer[0U] = USBRxBuffer[0U];
					uint8toASCII(RAMN_RNG_Pop8()&0xFF,&smallResponseBuffer[1U]);
					smallResponseBuffer[3U] = '\r';
					RAMN_USB_SendFromTask(smallResponseBuffer,4U);
					break;
				case 'J': //Return Random Integer
					smallResponseBuffer[0U] = USBRxBuffer[0U];
					uint32toASCII(RAMN_RNG_Pop32(),&smallResponseBuffer[1]);
					smallResponseBuffer[9U] = '\r';
					RAMN_USB_SendFromTask(smallResponseBuffer,10U);
					break;
				case '?':
				case 'h':
				case 'H':
					RAMN_USB_SendFromTask((uint8_t*)"https://ramn.rtfd.io/\r",22U);
					break;
				case 'l': //Open in restricted mode
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					hfdcan1.Init.Mode = FDCAN_MODE_RESTRICTED_OPERATION;
					RAMN_USB_Config.slcanOpened = True;
					RAMN_FDCAN_ResetPeripheral();
					break;
				case 'E'://Full Error and protocol flags
					HAL_FDCAN_GetProtocolStatus(&hfdcan1,&protocolStatus);
					HAL_FDCAN_GetErrorCounters(&hfdcan1, &errorCount);
					RAMN_DEBUG_DumpCANErrorRegisters(&errorCount, &protocolStatus);
					break;
				case 'q': // get status of FIFOs
					offset = reportFIFOStatus_USB(smallResponseBuffer);
					RAMN_USB_SendFromTask(smallResponseBuffer,offset);
					break;
				case 'I'://Send GW Stats information
					RAMN_DEBUG_ReportCANStats(&RAMN_FDCAN_Status);
					break;
				case 'D'://Restart in DFU Mode
					if((USBRxBuffer[1U] == 'z') && (USBRxBuffer[2U] == 'Z') && commandLength == 3U)
						RAMN_FLASH_ConfigureOptionBytesBootloaderMode();
					//Board should reset automatically, if we reach here there was an error
					RAMN_USB_SendFromTask((uint8_t*)"\a",1);
					break;
				case 'p'://Program ECU over CAN
					//Turn off all ECUs
					if( (USBRxBuffer[1U] != 'B') && (USBRxBuffer[1U] != 'C') && (USBRxBuffer[1U] != 'D'))
					{
						RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
						break;
					}
					RAMN_USB_Config.simulatorActive = False; //Turn off simulator
					RAMN_USB_Config.slcanOpened = False; //Turn off slcan to prevent forwarding bootloader test commands
					CANTxHeader.BitRateSwitch = FDCAN_BRS_ON;
					CANTxHeader.DataLength = 0U;
					CANTxHeader.FDFormat = FDCAN_FD_CAN;
					CANTxHeader.IdType = FDCAN_STANDARD_ID;
					CANTxHeader.Identifier = 0x181;
#define BOOTLOADER_MAX_ATTEMPTS 20
					for(uint8_t j = 0; j < BOOTLOADER_MAX_ATTEMPTS; j++)
					{
						RAMN_ECU_SetEnableAll(0U);
						RAMN_FDCAN_SetupForSTBootloader();
						RAMN_FDCAN_ResetPeripheral();
						osDelay(100); // wait for power supply disable to be effective
						RAMN_ECU_SetBoot0(USBRxBuffer[1U],GPIO_PIN_SET);
						RAMN_ECU_SetEnable(USBRxBuffer[1U],GPIO_PIN_SET);
						osDelay(20 + j*20); //add increasingly long delay if bootloader transition failed
						while (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_TRY_LATER)
						{
							//Buffer is Full, Try later
							osDelay(10U);
						}
						osDelay(50U); //leave time to send message and receive responses
						if (RAMN_FDCAN_Status.CANRXCnt > 0)
						{
							//received an answer, assume it is from bootloader
							break;
						}
					}
					if (RAMN_FDCAN_Status.CANRXCnt == 0)
					{
						RAMN_USB_SendFromTask((uint8_t*)"\a",1U); //all bootloader transitions failed
					}
					else
					{
						RAMN_USB_Config.slcanOpened = True; //make sure slcan is opened for following commands
						RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					}

					break;
				case 'Y': //Set ENABLE of all ECUs
					if (USBRxBuffer[1U] == '0') RAMN_ECU_SetEnableAll(GPIO_PIN_RESET);
					else RAMN_ECU_SetEnableAll(GPIO_PIN_SET);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'y': //Set ENABLE of all ECUs
					if (USBRxBuffer[2U] == '0') RAMN_ECU_SetEnable(USBRxBuffer[1U],GPIO_PIN_RESET);
					else RAMN_ECU_SetEnable(USBRxBuffer[1U],GPIO_PIN_SET);
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					break;
				case 'n'://Reset whole board (used to leave programming mode)
					RAMN_USB_SendFromTask((uint8_t*)"\r",1U);
					RAMN_DEBUG_Log("d Resetting\r");
					RAMN_ECU_SetEnableAll(GPIO_PIN_RESET);
					RAMN_ECU_SetBoot0All(GPIO_PIN_RESET);
					osDelay(100);
					HAL_NVIC_SystemReset();
					break; //Should not reach here
				case 'c': //Connection to computer
					if (USBRxBuffer[1U] == '0')
					{
						RAMN_DBC_RequestSilence = True;
						RAMN_USB_Config.simulatorActive = False;
					}
					else
					{
						hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
						RAMN_USB_Config.slcanOpened = False; //close slcan mode by default
						RAMN_FDCAN_ResetPeripheral(); //reset in case settings have changed or port has never been opened
						RAMN_USB_Config.simulatorActive = True;
						RAMN_DBC_RequestSilence = False;
					}
					break;
					//				case 'x': //Get Microcontroller Unique ID Address
					//					smallResponseBuffer[0U] = USBRxBuffer[0U];
					//					uint32toASCII((uint32_t)*(HARDWARE_UNIQUE_ID_ADDRESS),&smallResponseBuffer[1]);
					//					smallResponseBuffer[9U] = '\r';
					//					RAMN_USB_SendFromTask(smallResponseBuffer,10U);
					//					break;
#ifdef ENABLE_UDS
				case '%': //Diagnostic Message
					if (commandLength >= 4)
					{
						uint16_t reqSize = (commandLength-4)/2; //Remove command byte and payload size
						if (reqSize == ASCIItoUint12(&USBRxBuffer[1]))
						{
							uint16_t ansSize;
							ASCIItoRaw(diagRxUSBbuf,&USBRxBuffer[4],reqSize);
							RAMN_UDS_ProcessDiagPayload(xTaskGetTickCount(), diagRxUSBbuf, reqSize, diagTxUSBbuf, &ansSize);
							//We do not need the USB RX buffer anymore, so we use it so save the answer
							uint12toASCII(ansSize, &USBRxBuffer[1]);
							rawtoASCII(&USBRxBuffer[4],diagTxUSBbuf,ansSize);
							USBRxBuffer[ansSize*2+4] = '\r';
							//add 1 for %, 1 for \r
							RAMN_USB_SendFromTask(USBRxBuffer,(ansSize*2)+5);
							RAMN_UDS_PerformPostAnswerActions(xTaskGetTickCount(), diagRxUSBbuf, reqSize, diagTxUSBbuf, &ansSize);
						}
						else RAMN_USB_SendFromTask((uint8_t*)"\a",1);
					}
					else RAMN_USB_SendFromTask((uint8_t*)"\a",1);
#endif
					break;
				case 'b': //disable CLI
					USB_CLI_ENABLE = 0U;
					RAMN_USB_SendFromTask((uint8_t*)"\r",1);
					break;
				case 'P':
				case 'A':
				case 'X':
				case 'U':
				case 'Q':
				default:
					RAMN_USB_SendFromTask((uint8_t*)"\a",1);
					break;
				}
			}
		}

	}
#endif
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_RAMN_ReceiveCANFunc */
/**
 * @brief Function implementing the RAMN_ReceiveCAN thread.
 * @param argument: Not used
 * @retval None
 */
//Header for RTR answers
FDCAN_TxHeaderTypeDef RTRTxHeader;
uint8_t RTRTxData[8];
/* USER CODE END Header_RAMN_ReceiveCANFunc */
void RAMN_ReceiveCANFunc(void *argument)
{
	/* USER CODE BEGIN RAMN_ReceiveCANFunc */
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

#ifdef RTR_DEMO_ID
			else if (CANRxHeader.RxFrameType == FDCAN_REMOTE_FRAME)
			{

				if (CANRxHeader.Identifier == RTR_DEMO_ID)
				{
					RTRTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
					RTRTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
					RTRTxHeader.TxFrameType = FDCAN_DATA_FRAME;
					RTRTxHeader.IdType = CANRxHeader.IdType;
					RTRTxHeader.Identifier = CANRxHeader.Identifier;
					RTRTxHeader.DataLength = CANRxHeader.DataLength;
					if (RTRTxHeader.DataLength > 8U) RTRTxHeader.DataLength = 8U;
					RAMN_memcpy((uint8_t*)RTRTxData,(uint8_t*)DEVICE_HARDWARE_ID_ADDRESS,8); // Copy 8 last bytes of ECU hardware ID
					RAMN_FDCAN_SendMessage(&RTRTxHeader,RTRTxData);
				}
			}
#endif

#if defined(ENABLE_USB)
			if (RAMN_USB_Config.slcanOpened)
			{
				uint8_t index = 0;

				//add prefix if frame is of CAN-FD type
				if(CANRxHeader.FDFormat == FDCAN_FD_CAN)
				{
					if(CANRxHeader.BitRateSwitch  == FDCAN_BRS_ON) slCAN_USBTxBuffer[index++] = '1';
					else slCAN_USBTxBuffer[index++] = '0';
				}
				else
				{
					if (payloadSize > 8)
					{
						payloadSize = 8;
					}
				}

				if(CANRxHeader.RxFrameType == FDCAN_DATA_FRAME)
				{
					//Message with Data
					slCAN_USBTxBuffer[index++] = CANRxHeader.IdType == FDCAN_STANDARD_ID ? 't' : 'T';
				}
				else
				{
					//FDCAN_REMOTE_FRAME is the only other option
					slCAN_USBTxBuffer[index++] = CANRxHeader.IdType == FDCAN_STANDARD_ID ? 'r' : 'R';
					payloadSize = 0; //no payload will be sent.
				}

				if (CANRxHeader.IdType == FDCAN_STANDARD_ID)
				{
					//Standard ID (FDCAN_STANDARD_ID)
					index += uint12toASCII(CANRxHeader.Identifier,&slCAN_USBTxBuffer[index]);
				}
				else
				{
					//Extended ID (FDCAN_EXTENDED_ID)
					index += uint32toASCII(CANRxHeader.Identifier,&slCAN_USBTxBuffer[index]);
				}

				//TODO: unify
				if (CANRxHeader.FDFormat == FDCAN_FD_CAN)
				{
					index += uint4toASCII((CANRxHeader.DataLength) & 0xF,&slCAN_USBTxBuffer[index]);
				}
				else
				{
					index += uint4toASCII((payloadSize) & 0xF,&slCAN_USBTxBuffer[index]);
				}

				for(uint8_t i=0;i<payloadSize;i++)
				{
					index += uint8toASCII(CANRxData[i],&slCAN_USBTxBuffer[index]);
				}

				if (RAMN_USB_Config.slcan_enableTimestamp != 0U)
				{
					index += uint16toASCII(xTaskGetTickCount() % 0xEA60,&slCAN_USBTxBuffer[index]);
				}

				if ((RAMN_USB_Config.addESIFlag != 0U) && (CANRxHeader.FDFormat == FDCAN_FD_CAN) && (CANRxHeader.ErrorStateIndicator == FDCAN_ESI_PASSIVE))
				{
					slCAN_USBTxBuffer[index++] = 'i';
				}
				slCAN_USBTxBuffer[index++] = '\r';
				if (RAMN_USB_SendFromTask(slCAN_USBTxBuffer,index) != RAMN_OK)
				{
					RAMN_USB_Config.slcanOpened = False;
				}
			}
#endif

		}
		else
		{
			Error_Handler();
		}

	}
	/* USER CODE END RAMN_ReceiveCANFunc */
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
		if (CANTxHeader.TxFrameType == FDCAN_REMOTE_FRAME)
		{
			payloadSize = 0;  // no payload for remote requests
		}
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

#if defined(TARGET_ECUA)
	RAMN_ECU_SetDefaultState();
#endif

#if defined(EXPANSION_CHASSIS) || defined(EXPANSION_POWERTRAIN) || defined(EXPANSION_BODY)
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)RAMN_SENSORS_ADCValues, 3);
	RAMN_SENSORS_Init();
#endif
	RAMN_ACTUATORS_Init();
	RAMN_SIM_Init();

#if defined(ENABLE_SCREEN)
	RAMN_SCREEN_Init(&hspi2, &RAMN_PeriodicHandle);
#endif

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
		RAMN_SCREEN_Update(xLastWakeTime);
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
#if defined(ENABLE_USB)
	FDCAN_ErrorCountersTypeDef errorCount;
	FDCAN_ProtocolStatusTypeDef protocolStatus;
	RAMN_FDCAN_Status_t gw_freeze;
	uint32_t err;
#endif
	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//Read all data in a critical section for consistent readings
#if defined(ENABLE_USB)
		taskENTER_CRITICAL();
		err = HAL_FDCAN_GetError(&hfdcan1);
		HAL_FDCAN_GetProtocolStatus(&hfdcan1,&protocolStatus);
		HAL_FDCAN_GetErrorCounters(&hfdcan1, &errorCount);
		RAMN_FDCAN_Status.prevCANError = err;
		gw_freeze = RAMN_FDCAN_Status;
		//Clear the errorCode manually
		hfdcan1.ErrorCode = HAL_FDCAN_ERROR_NONE;
		taskEXIT_CRITICAL();
#endif
#if defined(ENABLE_USB)
		RAMN_DEBUG_PrintCANError(&errorCount, &protocolStatus, &gw_freeze, err);
#endif

#if defined(AUTO_RECOVER_BUSOFF)
		if (protocolStatus.BusOff != 0U)
		{
			RAMN_FDCAN_ResetPeripheral();
		}
#endif

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
					RAMN_UDS_PerformPostAnswerActions(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
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

/* USER CODE BEGIN Header_RAMN_SendUSBFunc */
/**
 * @brief Function implementing the RAMN_SendUSB thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_SendUSBFunc */
void RAMN_SendUSBFunc(void *argument)
{
	/* USER CODE BEGIN RAMN_SendUSBFunc */
#if !defined(ENABLE_USB)
	vTaskDelete(NULL);
#else
	RAMN_USB_Init(&USBD_TxStreamBufferHandle,&RAMN_SendUSBHandle);
	RAMN_CDC_Init(&USBD_RxStreamBufferHandle, &RAMN_ReceiveUSBHandle, &RAMN_SendUSBHandle);

#ifdef ENABLE_USB_AUTODETECT
	//We expect a notification from the serial close/open detection module
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	while (RAMN_CDC_GetTXStatus() != USBD_OK) osDelay(10U); //Wait for TX to be ready
#endif
	/* Infinite loop */
	for(;;)
	{
		size_t size = xStreamBufferReceive(USBD_TxStreamBufferHandle,USBIntermediateTxBuffer,sizeof(USBIntermediateTxBuffer), portMAX_DELAY);
		if (size > 0)
		{
			//Only sends if USB serial port is opened
#ifdef ENABLE_USB_AUTODETECT
			//Make sure no notification is pending
			ulTaskNotifyTake(pdTRUE, 0U);
			if (RAMN_USB_Config.serialOpened == True)
			{
#endif
				RAMN_USB_SendFromTask_Blocking(USBIntermediateTxBuffer,size);
#ifdef ENABLE_USB_AUTODETECT
			}
#endif
		}
	}
#endif
	/* USER CODE END RAMN_SendUSBFunc */
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
