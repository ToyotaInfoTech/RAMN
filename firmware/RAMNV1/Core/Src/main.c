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
 * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
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
#ifdef ENABLE_USB
#include "ramn_usb.h"
#endif

#ifdef ENABLE_CDC
#include "ramn_cdc.h"
#endif

#ifdef ENABLE_GSUSB
#include "ramn_gsusb.h"
#endif

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
#include "ramn_screen_manager.h"
#endif
#if defined(ENABLE_MINICTF)
#include "ramn_ctf.h"
#endif
#include "ramn_customize.h"
#ifdef ENABLE_UART
#include "ramn_uart.h"
#endif
#include "usb_device.h"
#include "usbd_gsusb_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */
typedef StaticQueue_t osStaticMessageQDef_t;
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

I2C_HandleTypeDef hi2c2;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef hlpuart1;

RNG_HandleTypeDef hrng;

SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi2_rx;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim16;

PCD_HandleTypeDef hpcd_USB_FS;

/* Definitions for RAMN_ReceiveUSB */
osThreadId_t RAMN_ReceiveUSBHandle;
const osThreadAttr_t RAMN_ReceiveUSB_attributes = {
  .name = "RAMN_ReceiveUSB",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
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
uint32_t RAMN_PeriodicBuffer[ 256 ];
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
uint32_t RAMN_DiagRXBuffer[ 256 ];
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
uint32_t RAMN_DiagTXBuffer[ 256 ];
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
const osThreadAttr_t RAMN_SendUSB_attributes = {
  .name = "RAMN_SendUSB",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 256 * 4
};
/* Definitions for RAMN_RxTask2 */
osThreadId_t RAMN_RxTask2Handle;
const osThreadAttr_t RAMN_RxTask2_attributes = {
  .name = "RAMN_RxTask2",
  .priority = (osPriority_t) osPriorityHigh1,
  .stack_size = 512 * 4
};
/* Definitions for RAMN_TxTask2 */
osThreadId_t RAMN_TxTask2Handle;
const osThreadAttr_t RAMN_TxTask2_attributes = {
  .name = "RAMN_TxTask2",
  .priority = (osPriority_t) osPriorityHigh2,
  .stack_size = 256 * 4
};
/* USER CODE BEGIN PV */

#ifdef ENABLE_GSUSB
/* Definitions for RAMN_GSUSB_RecvQueue */
osMessageQueueId_t RAMN_GSUSB_RecvQueueHandle;
__attribute__ ((section (".buffers"))) uint8_t RAMN_GSUSB_RecvQueueBuffer[ GSUSB_RECV_QUEUE_SIZE * sizeof( uint32_t ) ];
osStaticMessageQDef_t RAMN_GSUSB_RecvQueueControlBlock;
const osMessageQueueAttr_t RAMN_GSUSB_RecvQueue_attributes = {
		.name = "RAMN_GSUSB_RecvQueue",
		.cb_mem = &RAMN_GSUSB_RecvQueueControlBlock,
		.cb_size = sizeof(RAMN_GSUSB_RecvQueueControlBlock),
		.mq_mem = &RAMN_GSUSB_RecvQueueBuffer,
		.mq_size = sizeof(RAMN_GSUSB_RecvQueueBuffer)
};
/* Definitions for RAMN_GSUSB_PoolQueue */
osMessageQueueId_t RAMN_GSUSB_PoolQueueHandle;
__attribute__ ((section (".buffers"))) uint8_t RAMN_GSUSB_PoolQueueBuffer[ GSUSB_POOL_QUEUE_SIZE * sizeof( uint32_t ) ];
osStaticMessageQDef_t RAMN_GSUSB_PoolQueueControlBlock;
const osMessageQueueAttr_t RAMN_GSUSB_PoolQueue_attributes = {
		.name = "RAMN_GSUSB_PoolQueue",
		.cb_mem = &RAMN_GSUSB_PoolQueueControlBlock,
		.cb_size = sizeof(RAMN_GSUSB_PoolQueueControlBlock),
		.mq_mem = &RAMN_GSUSB_PoolQueueBuffer,
		.mq_size = sizeof(RAMN_GSUSB_PoolQueueBuffer)
};
/* Definitions for RAMN_GSUSB_SendQueue */
osMessageQueueId_t RAMN_GSUSB_SendQueueHandle;
__attribute__ ((section (".buffers"))) uint8_t RAMN_GSUSB_SendQueueBuffer[ GSUSB_SEND_QUEUE_SIZE * sizeof( uint32_t ) ];
osStaticMessageQDef_t RAMN_GSUSB_SendQueueControlBlock;
const osMessageQueueAttr_t RAMN_GSUSB_SendQueue_attributes = {
		.name = "RAMN_GSUSB_SendQueue",
		.cb_mem = &RAMN_GSUSB_SendQueueControlBlock,
		.cb_size = sizeof(RAMN_GSUSB_SendQueueControlBlock),
		.mq_mem = &RAMN_GSUSB_SendQueueBuffer,
		.mq_size = sizeof(RAMN_GSUSB_SendQueueBuffer)
};
#endif

#if defined(ENABLE_CDC)

// Stream buffer for USB RX data, filled by ISR and emptied by receiving task.
static StaticStreamBuffer_t USB_RX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t USB_RX_BUFFER[USB_RX_BUFFER_SIZE];
StreamBufferHandle_t USBD_RxStreamBufferHandle;

// Stream buffer for USB TX data, filled by transmitting task(s).
static StaticStreamBuffer_t USB_TX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t USB_TX_BUFFER[USB_TX_BUFFER_SIZE];
StreamBufferHandle_t USBD_TxStreamBufferHandle;

// Intermediary buffer to empty USB_TX_BUFFER and pass it to CDC drivers (data currently being transmitted over USB).
__attribute__ ((section (".buffers")))  uint8_t USBIntermediateTxBuffer[APP_TX_DATA_SIZE];

// Holds currently processed (slcan) USB command.
__attribute__ ((section (".buffers")))  uint8_t USBRxBuffer[USB_COMMAND_BUFFER_SIZE];

// Holds currently processed (slcan) USB command's answer (Used by CAN receiving task).
__attribute__ ((section (".buffers")))  uint8_t slCAN_USBTxBuffer[0x200];

#endif

#if defined(ENABLE_UART)

// Stream buffer for UART RX data, filled by ISR and emptied by receiving task.
static StaticStreamBuffer_t UART_RX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t UART_RX_BUFFER[UART_RX_BUFFER_SIZE];
StreamBufferHandle_t UART_RxStreamBufferHandle;

// Stream buffer for UART TX data, filled by transmitting task(s).
static StaticStreamBuffer_t UART_TX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t UART_TX_BUFFER[UART_TX_BUFFER_SIZE];
StreamBufferHandle_t UART_TxStreamBufferHandle;

// Holds command currently being received over UART, used by ISR
__attribute__ ((section (".buffers"))) static uint8_t  UART_recvBuf[UART_RX_COMMAND_BUFFER_SIZE];

// Holds data currently being transmitted over UART, used as an argument to HAL library.
__attribute__ ((section (".buffers")))  uint8_t UARTIntermediateTxBuffer[UART_TX_COMMAND_BUFFER_SIZE];

// Holds data currently being processed, used by receiving task.
__attribute__ ((section (".buffers")))  uint8_t UARTRxBuffer[UART_RX_COMMAND_BUFFER_SIZE];

// Buffer to receive next UART char, used by ISR.
static uint8_t uart_rx_data[1];

// Current index of uart command.
static uint16_t uart_current_index = 0;

#endif

static RAMN_Bool_t USB_CLI_ENABLE;

#if defined(ENABLE_I2C)
__attribute__ ((section (".buffers"))) uint8_t i2c_rxBuf[I2C_RX_BUFFER_SIZE];
__attribute__ ((section (".buffers"))) uint8_t i2c_txBuf[I2C_TX_BUFFER_SIZE] = {'R', 'A', 'M', 'N'};
#endif

#if defined(ENABLE_DIAG)

// Holds currently processed Diag Command from CAN.
// Aligned to enable easy shell code execution.
__attribute__ ((section (".buffers"), aligned(4)))  uint8_t diagRxbuf[0xFFF+2];
// Holds currently generated Diag Command Answer for CAN.
__attribute__ ((section (".buffers")))  uint8_t diagTxbuf[0xFFF];

#endif

// Buffers for UDS commands, only allocated if enabled.
#if defined(ENABLE_UDS)

static StaticStreamBuffer_t UDS_ISOTP_RX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t UDS_ISOTP_RX_BUFFER[UDS_ISOTP_RX_BUFFER_SIZE];

static StaticStreamBuffer_t UDS_ISOTP_TX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t UDS_ISOTP_TX_BUFFER[UDS_ISOTP_RX_BUFFER_SIZE];

#endif

// Buffers for KWP commands, only allocated if enabled.
#if defined(ENABLE_KWP)

static StaticStreamBuffer_t KWP_ISOTP_RX_BUFFER_STRUCT;
__attribute__ ((section (".buffers"))) static uint8_t KWP_ISOTP_RX_BUFFER[KWP_ISOTP_RX_BUFFER_SIZE];

static StaticStreamBuffer_t KWP_ISOTP_TX_BUFFER_STRUCT;
__attribute__ ((section (".buffers"))) static uint8_t KWP_ISOTP_TX_BUFFER[KWP_ISOTP_RX_BUFFER_SIZE];

#endif

// Buffers for XCP commands, only allocated if enabled.
#if defined(ENABLE_XCP)

static StaticStreamBuffer_t XCP_RX_BUFFER_STRUCT;
__attribute__ ((section (".buffers"))) static uint8_t XCP_RX_BUFFER[XCP_RX_BUFFER_SIZE];

static StaticStreamBuffer_t XCP_TX_BUFFER_STRUCT;
__attribute__ ((section (".buffers"))) static uint8_t XCP_TX_BUFFER[XCP_RX_BUFFER_SIZE];

#endif

// Buffers for CAN Messages.
static StaticStreamBuffer_t CAN_RX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t CAN_RX_BUFFER[CAN_RX_BUFFER_SIZE];

static StaticStreamBuffer_t CAN_TX_BUFFER_STRUCT;
__attribute__ ((section (".buffers")))  static uint8_t CAN_TX_BUFFER[CAN_TX_BUFFER_SIZE];

// Handle for diag stream buffers, even unallocated ones.
StreamBufferHandle_t UdsRxDataStreamBufferHandle;
StreamBufferHandle_t UdsTxDataStreamBufferHandle;

StreamBufferHandle_t KwpRxDataStreamBufferHandle;
StreamBufferHandle_t KwpTxDataStreamBufferHandle;

StreamBufferHandle_t XcpRxDataStreamBufferHandle;
StreamBufferHandle_t XcpTxDataStreamBufferHandle;

// Header for RTR answers
FDCAN_TxHeaderTypeDef RTRTxHeader;
uint8_t RTRTxData[8];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ICACHE_Init(void);
static void MX_RNG_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_SPI2_Init(void);
static void MX_ADC1_Init(void);
static void MX_IWDG_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C2_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM16_Init(void);
void RAMN_ReceiveUSBFunc(void *argument);
void RAMN_ReceiveCANFunc(void *argument);
void RAMN_SendCANFunc(void *argument);
void RAMN_PeriodicTaskFunc(void *argument);
void RAMN_ErrorTaskFunc(void *argument);
void RAMN_DiagRXFunc(void *argument);
void RAMN_DiagTXFunc(void *argument);
void RAMN_SendUSBFunc(void *argument);
void RAMN_RxTask2Func(void *argument);
void RAMN_TxTask2Func(void *argument);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if defined(GENERATE_RUNTIME_STATS)
volatile unsigned long ulHighFrequencyTimerTicks;

void configureTimerForRunTimeStats(void) {
	ulHighFrequencyTimerTicks = 0U;
	HAL_TIM_Base_Start_IT(&htim7);
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
	// Check that ECU A BOOT option bytes are properly configured to hardware BOOT0, which should be pulled-up by default.
	RAMN_FLASH_ConfigureOptionBytesApplicationMode();
#endif

#if defined(MEMORY_AUTOLOCK)
	if(RAMN_FLASH_isMemoryProtected() == False) RAMN_FLASH_ConfigureRDPOptionByte(RDP_OPTIONBYTE);
#endif

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

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
  MX_I2C2_Init();
  MX_LPUART1_UART_Init();
  MX_USB_PCD_Init();
  MX_TIM7_Init();
  MX_TIM6_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

#ifdef START_IN_CLI_MODE
	USB_CLI_ENABLE = 1U;
#else
	USB_CLI_ENABLE = 0U;
#endif

#if defined(ENABLE_CDC)
	USBD_RxStreamBufferHandle   = xStreamBufferCreateStatic(USB_RX_BUFFER_SIZE,sizeof(uint8_t),USB_RX_BUFFER,&USB_RX_BUFFER_STRUCT);
	USBD_TxStreamBufferHandle   = xStreamBufferCreateStatic(USB_TX_BUFFER_SIZE,sizeof(uint8_t),USB_TX_BUFFER,&USB_TX_BUFFER_STRUCT);
#endif

#if defined(ENABLE_UART)
	UART_RxStreamBufferHandle   = xStreamBufferCreateStatic(UART_RX_BUFFER_SIZE,sizeof(uint8_t),UART_RX_BUFFER,&UART_RX_BUFFER_STRUCT);
	UART_TxStreamBufferHandle   = xStreamBufferCreateStatic(UART_TX_BUFFER_SIZE,sizeof(uint8_t),UART_TX_BUFFER,&UART_TX_BUFFER_STRUCT);
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
	// Make sure other ECUs are turned off.
	RAMN_ECU_SetEnableAll(0U);
	// Make sure BOOT0 is low (application mode).
	RAMN_ECU_SetBoot0All(0U);
#endif

#if defined(ENABLE_SPI) && !defined(ENABLE_SCREEN) //if screen is enabled, we initialize it in the screen module.
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

#ifdef USE_TRNG_BUFFER
	// Enable TRNG module
	if (HAL_RNG_GenerateRandomNumber_IT(&hrng) != HAL_OK)
	{
#ifdef HANG_ON_ERRORS
		Error_Handler();
#endif
	}
#endif

	// Automatically add a DTC if none is stored in memory
#ifdef ENABLE_EEPROM_EMULATION
	uint32_t dtcCnt = 0U;

	if (RAMN_DTC_GetNumberOfDTC(&dtcCnt) == RAMN_OK)
	{
		if (dtcCnt == 0U)
		{
			// No DTC, add one per ECU
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic" // needed bc 0b binary constants are an extension
#ifdef TARGET_ECUA
			uint32_t dtcVal = 0b11 << 30; // "11" for network ("U")
			dtcVal |= 0x0029 << 16; // Bus A Performance, FTB 0
#elif defined(TARGET_ECUB)
			uint32_t dtcVal = 0b01 << 30;// "01" for chassis ("C")
			dtcVal |= 0x0563 << 16; // Calibration ROM Checksum Error, FTB 0
#elif defined(TARGET_ECUC)
			uint32_t dtcVal = 0b00 << 30;// "00" for powertrain ("P")
			dtcVal |= 0x0172 << 16; // System too Rich, FTB 0
#elif defined(TARGET_ECUD)
			uint32_t dtcVal = 0b10 << 30;// "10" for body ("B")
			dtcVal |= 0x0091 << 16; // Active switch wrong state, FTB 0
#endif
#pragma GCC diagnostic pop

			dtcVal |= 1 << 2; //mark DTC as pending.
			RAMN_DTC_AddNew(dtcVal);
		}
	}
#endif
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
#ifdef ENABLE_GSUSB
	/* creation of RAMN_GSUSB_RecvQueue */
	RAMN_GSUSB_RecvQueueHandle = osMessageQueueNew (GSUSB_RECV_QUEUE_SIZE, sizeof(uint32_t), &RAMN_GSUSB_RecvQueue_attributes);

	/* creation of RAMN_GSUSB_PoolQueue */
	RAMN_GSUSB_PoolQueueHandle = osMessageQueueNew (GSUSB_POOL_QUEUE_SIZE, sizeof(uint32_t), &RAMN_GSUSB_PoolQueue_attributes);

	/* creation of RAMN_GSUSB_SendQueue */
	RAMN_GSUSB_SendQueueHandle = osMessageQueueNew (GSUSB_SEND_QUEUE_SIZE, sizeof(uint32_t), &RAMN_GSUSB_SendQueue_attributes);
#endif

#ifdef ENABLE_USB
	MX_USB_Device_Init();
#endif
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

  /* creation of RAMN_RxTask2 */
  RAMN_RxTask2Handle = osThreadNew(RAMN_RxTask2Func, NULL, &RAMN_RxTask2_attributes);

  /* creation of RAMN_TxTask2 */
  RAMN_TxTask2Handle = osThreadNew(RAMN_TxTask2Func, NULL, &RAMN_TxTask2_attributes);

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
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */
#ifdef ENABLE_ADC
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
  hfdcan1.Init.NominalSyncJumpWidth = 16;
  hfdcan1.Init.NominalTimeSeg1 = 60;
  hfdcan1.Init.NominalTimeSeg2 = 19;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 16;
  hfdcan1.Init.DataTimeSeg1 = 15;
  hfdcan1.Init.DataTimeSeg2 = 4;
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
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */
#ifdef ENABLE_I2C
  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10D19CE4;
  hi2c2.Init.OwnAddress1 = 238;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */
#endif
  /* USER CODE END I2C2_Init 2 */

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
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */
#ifdef ENABLE_UART
  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */
#endif
  /* USER CODE END LPUART1_Init 2 */

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
#ifdef ENABLE_SPI
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
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
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
#endif
  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 9999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 7999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */
#ifdef GENERATE_RUNTIME_STATS
  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 7999;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */
#endif
  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 79;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

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
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
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
  HAL_GPIO_WritePin(GPIOA, ECUD_EN_Pin|Other_nCS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_nCS_Pin|LCD_DC_Pin|ECUB_EN_Pin|FDCAN1_STB_Pin
                          |ECUB_BOOT0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ECUC_EN_Pin ECUC_BOOT0_Pin ECUD_BOOT0_Pin */
  GPIO_InitStruct.Pin = ECUC_EN_Pin|ECUC_BOOT0_Pin|ECUD_BOOT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ECUD_EN_Pin Other_nCS_Pin */
  GPIO_InitStruct.Pin = ECUD_EN_Pin|Other_nCS_Pin;
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

  /*Configure GPIO pin : SELF_BOOT0_Pin */
  GPIO_InitStruct.Pin = SELF_BOOT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SELF_BOOT0_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

#ifdef ENABLE_I2C

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c != 0)
	{
		RAMN_CUSTOM_ReceiveI2C(i2c_rxBuf, I2C_RX_BUFFER_SIZE);
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c != 0)
	{

	}
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t transferDirect, uint16_t AddrMatchCode)
{
	if (hi2c != 0U)
	{
		if (transferDirect == I2C_DIRECTION_TRANSMIT)
		{
			if (HAL_I2C_Slave_Sequential_Receive_IT(hi2c, i2c_rxBuf, I2C_RX_BUFFER_SIZE, I2C_FIRST_AND_LAST_FRAME) != HAL_OK) Error_Handler();
		}
		else if (transferDirect == I2C_DIRECTION_RECEIVE)
		{
			RAMN_CUSTOM_PrepareTransmitDataI2C(i2c_txBuf, I2C_TX_BUFFER_SIZE);
			if (HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, i2c_txBuf, I2C_TX_BUFFER_SIZE, I2C_FIRST_AND_LAST_FRAME) != HAL_OK) Error_Handler();
		}
		else Error_Handler();
	}
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c != 0)
	{
		HAL_I2C_EnableListen_IT(hi2c);
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	uint32_t err = HAL_I2C_GetError(hi2c);

	//if (err != HAL_I2C_ERROR_AF)
	if (err != HAL_I2C_ERROR_NONE)
	{
		HAL_I2C_EnableListen_IT(hi2c);
	}
	else
	{
		//Error_Handler();
	}
}

#endif

#ifdef ENABLE_UART

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	// End of transmit
	if(RAMN_SendUSBHandle != NULL)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(RAMN_SendUSBHandle,&xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//TODO: better error reporting
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (uart_current_index >= sizeof(UART_recvBuf))
	{
		uart_current_index = 0;
	}
	else if(uart_rx_data[0] != '\r')
	{
		UART_recvBuf[uart_current_index] = uart_rx_data[0];
		uart_current_index++;
	}
	else
	{
		if ((uart_current_index > 0) && (uart_current_index <= sizeof(UART_recvBuf))) //Don't forward empty commands or commands longer than buffer
		{
			if(UART_RxStreamBufferHandle != NULL){

				if (xStreamBufferSendFromISR(UART_RxStreamBufferHandle,&uart_current_index, 2U, NULL ) != 2U)
				{
#ifdef HANG_ON_ERRORS
					Error_Handler()
#endif
				}
				else
				{
					if (xStreamBufferSendFromISR(UART_RxStreamBufferHandle,UART_recvBuf, uart_current_index, NULL ) != uart_current_index)
					{
#ifdef HANG_ON_ERRORS
						Error_Handler()
#endif
					}
					else
					{
						vTaskNotifyGiveFromISR(RAMN_ReceiveUSBHandle,&xHigherPriorityTaskWoken);
						portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
					}
				}
			}
		}
		uart_current_index = 0;
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types" // needed to avoid passing the technically correct (but strange looking) uart_rx_data
	HAL_UART_Receive_IT(&hlpuart1, &uart_rx_data, 1);
#pragma GCC diagnostic pop
}
#endif

/* USER CODE END 4 */

/* USER CODE BEGIN Header_RAMN_ReceiveUSBFunc */
/**
 * @brief  Function implementing the RAMN_ReceiveUSB thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_ReceiveUSBFunc */
void RAMN_ReceiveUSBFunc(void *argument)
{
  /* USER CODE BEGIN 5 */
#if defined(ENABLE_CDC)
	/* init code for USB_Device */
	for(;;)
	{
		uint16_t commandLength;
		size_t xBytesReceived;
		RAMN_Bool_t invalidBuffer = False;

		xBytesReceived = xStreamBufferReceive(USBD_RxStreamBufferHandle, (void *)&commandLength, 2U, portMAX_DELAY);
		if ((xBytesReceived != 2U) || (commandLength > USB_COMMAND_BUFFER_SIZE)) invalidBuffer = True;
		else
		{
			xBytesReceived = xStreamBufferReceive(USBD_RxStreamBufferHandle, (void*)USBRxBuffer, commandLength, portMAX_DELAY);
			if (xBytesReceived != commandLength) invalidBuffer = True;
		}

		if (invalidBuffer == True)
		{
			// Error counter should already have been increased by CDC_Receive_FS
			if (USB_CLI_ENABLE == True) RAMN_USB_SendStringFromTask("Error processing USB Buffer.\r");
			else RAMN_USB_SendFromTask((uint8_t*)"\a",1U);
			continue;
		}

		if (RAMN_CUSTOM_ProcessCDCLine(USBRxBuffer, commandLength) == False) // If not asked to skip
		{
			if (USB_CLI_ENABLE == True)
			{
				if (RAMN_CDC_ProcessCLIBuffer(USBRxBuffer, commandLength) == True) USB_CLI_ENABLE = !USB_CLI_ENABLE;
			}
			else
			{
				if (RAMN_CDC_ProcessSLCANBuffer(USBRxBuffer, commandLength) == True) USB_CLI_ENABLE = !USB_CLI_ENABLE;
			}
		}
	}
#elif defined(ENABLE_UART)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types" //needed to avoid passing the technically correct (but strange looking) uart_rx_data
	HAL_UART_Receive_IT(&hlpuart1, &uart_rx_data, 1); // Start receiving characters, one by one by default (slow)
#pragma GCC diagnostic pop

	for(;;)
	{
		uint16_t commandLength;
		size_t xBytesReceived;
		xBytesReceived = xStreamBufferReceive(UART_RxStreamBufferHandle, (void *)&commandLength, 2U, portMAX_DELAY );
		if (xBytesReceived != 2U)
		{
#ifdef HANG_ON_ERRORS
			Error_Handler();
#endif
		}

		xBytesReceived = xStreamBufferReceive(UART_RxStreamBufferHandle, (void*)UARTRxBuffer, commandLength,portMAX_DELAY);
		if (xBytesReceived != commandLength)
		{
#ifdef HANG_ON_ERRORS
			Error_Handler();
#endif
		}

		RAMN_CUSTOM_ReceiveUART(UARTRxBuffer, commandLength);

	}
#else
	RAMN_CUSTOM_CustomTask1(argument);
#endif
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_RAMN_ReceiveCANFunc */
/**
 * @brief Function implementing the RAMN_ReceiveCAN thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_ReceiveCANFunc */
void RAMN_ReceiveCANFunc(void *argument)
{
  /* USER CODE BEGIN RAMN_ReceiveCANFunc */
	FDCAN_RxHeaderTypeDef CANRxHeader;
	uint8_t CANRxData[64U];

	for(;;)
	{
		if (xStreamBufferReceive(CANRxDataStreamBufferHandle, (void *)&CANRxHeader,sizeof(CANRxHeader), portMAX_DELAY) == sizeof(CANRxHeader))
		{
			uint8_t payloadSize = DLCtoUINT8(CANRxHeader.DataLength);
			if (payloadSize > 0)
			{
				if (xStreamBufferReceive(CANRxDataStreamBufferHandle, (void *) CANRxData,payloadSize, portMAX_DELAY ) != payloadSize)
				{
#ifdef HANG_ON_ERRORS
					Error_Handler();
#endif
				}
			}

			if(CANRxHeader.RxFrameType == FDCAN_DATA_FRAME) {
				RAMN_DBC_ProcessCANMessage(CANRxHeader.Identifier,payloadSize,(RAMN_CANFrameData_t*)CANRxData);
#if defined(ENABLE_DIAG)
				RAMN_DIAG_ProcessRxCANMessage(&CANRxHeader, CANRxData, xTaskGetTickCount());
#endif
#if defined(ENABLE_SCREEN)
				RAMN_SCREENMANAGER_ProcessRxCANMessage(&CANRxHeader, CANRxData, xTaskGetTickCount());
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
					RAMN_memcpy((uint8_t*)RTRTxData,(uint8_t*)HARDWARE_UNIQUE_ID_ADDRESS,8); // Copy 8 last bytes of ECU hardware ID
					RAMN_FDCAN_SendMessage(&RTRTxHeader,RTRTxData);
				}
			}
#endif

#if defined(ENABLE_MINICTF) && defined(TARGET_ECUD)
			RAMN_CTF_ProcessRxCANMessage(&CANRxHeader, CANRxData, xTaskGetTickCount());
#endif
			RAMN_CUSTOM_ProcessRxCANMessage(&CANRxHeader, CANRxData, xTaskGetTickCount());

#if defined(ENABLE_CDC)
			if (RAMN_USB_Config.slcanOpened)
			{
				uint8_t index = 0;

				// Add prefix if frame is of CAN-FD type
				if(CANRxHeader.FDFormat == FDCAN_FD_CAN)
				{
					if(CANRxHeader.BitRateSwitch  == FDCAN_BRS_ON) slCAN_USBTxBuffer[index++] = '1';
					else slCAN_USBTxBuffer[index++] = '0';
				}
				else
				{
					if (payloadSize > 8U) payloadSize = 8U;
				}

				if(CANRxHeader.RxFrameType == FDCAN_DATA_FRAME)
				{
					// Message with Data
					slCAN_USBTxBuffer[index++] = CANRxHeader.IdType == FDCAN_STANDARD_ID ? 't' : 'T';
				}
				else
				{
					// FDCAN_REMOTE_FRAME is the only other option
					slCAN_USBTxBuffer[index++] = CANRxHeader.IdType == FDCAN_STANDARD_ID ? 'r' : 'R';
					payloadSize = 0; //no payload will be sent.
				}

				if (CANRxHeader.IdType == FDCAN_STANDARD_ID)
				{
					// Standard ID (FDCAN_STANDARD_ID)
					index += uint12toASCII(CANRxHeader.Identifier,&slCAN_USBTxBuffer[index]);
				}
				else
				{
					// Extended ID (FDCAN_EXTENDED_ID)
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
					index += uint16toASCII((xTaskGetTickCount() * (1000 /*ms per sec*/ / configTICK_RATE_HZ) ) % 0xEA60 /* 60,000 ms*/,&slCAN_USBTxBuffer[index]);
				}

				if ((RAMN_USB_Config.addESIFlag != 0U) && (CANRxHeader.FDFormat == FDCAN_FD_CAN) && (CANRxHeader.ErrorStateIndicator == FDCAN_ESI_PASSIVE))
				{
					slCAN_USBTxBuffer[index++] = 'i';
				}
				slCAN_USBTxBuffer[index++] = '\r';
				if (RAMN_USB_SendFromTask(slCAN_USBTxBuffer,index) != RAMN_OK)
				{
#ifdef CLOSE_DEVICE_ON_USB_TX_OVERFLOW
					// USB overflow, user probably forgot to close the device
					RAMN_USB_Config.slcanOpened = False;
#endif
				}
			}
#endif

#ifdef ENABLE_GSUSB
			if(RAMN_USB_Config.gsusbOpened && GSUSB_IsConnected((USBD_HandleTypeDef*)hpcd_USB_FS.pData))
			{
				if(RAMN_GSUSB_ProcessRX(&CANRxHeader, CANRxData) == RAMN_ERROR)
				{
					RAMN_USB_Config.queueErrorCnt++;
				}
			}
#endif
		}
		else
		{
#ifdef HANG_ON_ERRORS
			Error_Handler();
#endif
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
		if (CANTxHeader.TxFrameType == FDCAN_REMOTE_FRAME) payloadSize = 0U;  // No payload for remote requests
		if (payloadSize > 0U)
		{
			index = 0U;
			while(index < payloadSize) index += xStreamBufferReceive(CANTxDataStreamBufferHandle, (void *)(CANTxData+index),payloadSize-index, portMAX_DELAY);
		}

		// Wait for TX space to be available in FIFO
		while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) < 1)
		{
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY );
		}

		if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &CANTxHeader, CANTxData) != HAL_OK)
		{
#ifdef HANG_ON_ERRORS
			Error_Handler();
#endif
		}

#ifdef ENABLE_GSUSB
		/* echo transmission
		if(RAMN_USB_Config.gsusbOpened && GSUSB_IsConnected((USBD_HandleTypeDef*)hpcd_USB_FS.pData))
		{
			// Wait for queue empty
			while (uxQueueMessagesWaiting(RAMN_GSUSB_PoolQueueHandle) < FDCAN_GetQueueSize())
			{
				ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			}
			RAMN_SocketCAN_SendTX(&CANTxHeader, CANTxData);
		}
		 */
#endif

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

#ifdef ENABLE_I2C
	if (HAL_I2C_EnableListen_IT(&hi2c2) != HAL_OK) Error_Handler();
#endif

#ifdef ENABLE_ADC
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)RAMN_SENSORS_ADCValues, NUMBER_OF_ADC);
	RAMN_SENSORS_Init();
#endif
	RAMN_ACTUATORS_Init();
	RAMN_SIM_Init();
#ifdef ENABLE_MINICTF
	RAMN_CTF_Init(xTaskGetTickCount());
#endif
	RAMN_CUSTOM_Init(xTaskGetTickCount());
	HAL_TIM_Base_Start_IT(&htim6); 	// Enable custom function timer
	HAL_TIM_Base_Start(&htim16); 	// Enable custom measurement timer

#if defined(ENABLE_SCREEN)
	RAMN_SCREENMANAGER_Init(&hspi2, &RAMN_PeriodicHandle);
#endif

	// Init joystick for screen controls
#ifdef ENABLE_JOYSTICK_CONTROLS
	RAMN_Joystick_Init();
#endif

	/* Infinite loop */
	for(;;)
	{
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
#ifdef WATCHDOG_ENABLE
		if(HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
		{
#ifdef HANG_ON_ERRORS
			Error_Handler();
#endif
		}
#endif
		RAMN_SENSORS_Update(xLastWakeTime);
		RAMN_SIM_UpdatePeriodic(xLastWakeTime);
		RAMN_ACTUATORS_ApplyControls(xLastWakeTime);
		if (RAMN_DBC_RequestSilence == 0U)
		{
			RAMN_DBC_Send(xLastWakeTime);
#ifdef ENABLE_MINICTF
			RAMN_CTF_Update(xLastWakeTime);
#endif
		}
		RAMN_CUSTOM_Update(xLastWakeTime);

#if defined(ENABLE_DIAG)
		RAMN_DIAG_Update(xLastWakeTime);
#endif

#ifdef ENABLE_SCREEN
		RAMN_SCREENMANAGER_Update(xLastWakeTime);
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
	//TODO: report Errors to GS_USB
	FDCAN_ErrorCountersTypeDef errorCount;
	FDCAN_ProtocolStatusTypeDef protocolStatus;
	RAMN_FDCAN_Status_t gw_freeze;
	uint32_t err;

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// Read all data in a critical section for consistent readings
		taskENTER_CRITICAL();
		err = HAL_FDCAN_GetError(&hfdcan1);
		HAL_FDCAN_GetProtocolStatus(&hfdcan1,&protocolStatus);
		HAL_FDCAN_GetErrorCounters(&hfdcan1, &errorCount);
		RAMN_FDCAN_Status.prevCANError = err;
		gw_freeze = RAMN_FDCAN_Status;
		// Clear the errorCode manually
		hfdcan1.ErrorCode = HAL_FDCAN_ERROR_NONE;

		// It seems there is a bug with HAL calling this interrupt infinitely because of auto-retransmission, prevent getting stuck
		// TODO find better fix
		if ((gw_freeze.CANErrCnt > 0x20000) && ((RAMN_FDCAN_Status.prevCANError & FDCAN_IR_PEA) != 0U))
		{
			RAMN_FDCAN_Status.busOff = True;
			RAMN_FDCAN_Disable();
		}

		taskEXIT_CRITICAL();

#if defined(ENABLE_USB) && defined(ENABLE_USB_DEBUG)
		if (RAMN_DEBUG_ENABLE == True) RAMN_DEBUG_PrintCANError(&errorCount, &protocolStatus, &gw_freeze, err);
#endif

#ifdef ENABLE_USB
		if(err & HAL_FDCAN_ERROR_PROTOCOL_DATA) RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_BUS_ERROR;
#endif

#if defined(AUTO_RECOVER_BUSOFF)
		if (protocolStatus.BusOff != 0U)
		{
			osDelay(100);
			RAMN_FDCAN_ResetPeripheral();
		}
#endif

		// If a transmission failed, the "transmission complete" notification will not be sent. We may need to wake up the CAN send thread (?) TODO: check
		// Normally, we should only require it if protocolStatus.Activity == FDCAN_COM_STATE_TX, but it is safer to notify the thread whatever the error is.
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
	RAMN_CUSTOM_CustomTask5(argument);
#else
	uint16_t diagRxSize;
	uint8_t addressing = 0;
	uint16_t index;
	uint16_t diagTxSize;
	size_t xBytesSent;

	for(;;)
	{
#ifdef ENABLE_UDS
		//Check UDS
		if (xStreamBufferBytesAvailable(UdsRxDataStreamBufferHandle) >= sizeof(addressing))
		{
			if (xStreamBufferReceive(UdsRxDataStreamBufferHandle, (void *)&addressing,sizeof(addressing), portMAX_DELAY) == sizeof(addressing))
			{
				if (xStreamBufferBytesAvailable(UdsRxDataStreamBufferHandle) >= sizeof(diagRxSize))
				{
					if (xStreamBufferReceive(UdsRxDataStreamBufferHandle, (void *)&diagRxSize,sizeof(diagRxSize), portMAX_DELAY) == sizeof(diagRxSize))
					{
						if (diagRxSize <= 0xFFF)
						{
							index = 0;
							if (diagRxSize > 0U)
							{
								while (index != diagRxSize)
								{
									index += xStreamBufferReceive(UdsRxDataStreamBufferHandle, (void *)&diagRxbuf[index],diagRxSize-index, portMAX_DELAY);
								}
							}

							if (addressing == 0U)
							{
								RAMN_UDS_ProcessDiagPayload(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
								if (diagTxSize > 0U)
								{
									xBytesSent = xStreamBufferSend(UdsTxDataStreamBufferHandle, (void *) &diagTxSize, sizeof(diagTxSize), portMAX_DELAY );
									xBytesSent += xStreamBufferSend(UdsTxDataStreamBufferHandle, (void *) diagTxbuf, diagTxSize, portMAX_DELAY );
									if(xBytesSent != (diagTxSize + sizeof(diagTxSize) ))
									{
#ifdef HANG_ON_ERRORS
										Error_Handler();
#endif
									}
									xTaskNotifyGive(RAMN_DiagTXHandle);
								}
								RAMN_UDS_PerformPostAnswerActions(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
							}
							else
							{
								RAMN_UDS_ProcessDiagPayloadFunctional(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
								if (diagTxSize > 0U)
								{
									uint8_t shouldSendAnswer = False;
									if (diagTxbuf[0] != 0x7F)
									{
										shouldSendAnswer = True;
									}
									else if ((diagTxSize > 2U) && (diagTxbuf[2] != UDS_NRC_SNS) && (diagTxbuf[2] != UDS_NRC_SFNS) && (diagTxbuf[2] != UDS_NRC_ROOR))
									{
										shouldSendAnswer = True;
									}

									if (shouldSendAnswer != False)
									{
										xBytesSent = xStreamBufferSend(UdsTxDataStreamBufferHandle, (void *) &diagTxSize, sizeof(diagTxSize), portMAX_DELAY );
										xBytesSent += xStreamBufferSend(UdsTxDataStreamBufferHandle, (void *) diagTxbuf, diagTxSize, portMAX_DELAY );
										if( xBytesSent != (diagTxSize + sizeof(diagTxSize) ))
										{
#ifdef HANG_ON_ERRORS
											Error_Handler();
#endif
										}
										xTaskNotifyGive(RAMN_DiagTXHandle);
									}
								}
								RAMN_UDS_PerformPostAnswerActions(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
							}
						}
					}
				}
			}
		}
#endif
#ifdef ENABLE_KWP
		// Check KWP
		if (xStreamBufferBytesAvailable(KwpRxDataStreamBufferHandle) >= sizeof(diagRxSize))
		{
			if (xStreamBufferReceive(KwpRxDataStreamBufferHandle, (void *)&diagRxSize,sizeof(diagRxSize), portMAX_DELAY) == sizeof(diagRxSize))
			{
				if (diagRxSize <= 0xFFF)
				{
					index = 0;
					while (index != diagRxSize)
					{
						index += xStreamBufferReceive(KwpRxDataStreamBufferHandle, (void *)((uint8_t *)diagRxbuf+index),diagRxSize-index, portMAX_DELAY);
					}


					RAMN_KWP_ProcessDiagPayload(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
					if (diagTxSize > 0U)
					{
						xBytesSent = xStreamBufferSend(KwpTxDataStreamBufferHandle, (void *) &diagTxSize, sizeof(diagTxSize), portMAX_DELAY );
						xBytesSent += xStreamBufferSend(KwpTxDataStreamBufferHandle, (void *) diagTxbuf, diagTxSize, portMAX_DELAY );
						if( xBytesSent != (diagTxSize + sizeof(diagTxSize) ))
						{
#ifdef HANG_ON_ERRORS
							Error_Handler();
#endif
						}
						xTaskNotifyGive(RAMN_DiagTXHandle);
					}
				}
			}
		}
#endif
#ifdef ENABLE_XCP
		// Check XCP
		if (xStreamBufferBytesAvailable(XcpRxDataStreamBufferHandle) >= sizeof(diagRxSize))
		{
			if (xStreamBufferReceive(XcpRxDataStreamBufferHandle, (void *)&diagRxSize,sizeof(diagRxSize), portMAX_DELAY) == sizeof(diagRxSize))
			{
				if (diagRxSize <= 0xFFF)
				{
					index = 0;
					while (index != diagRxSize)
					{
						index += xStreamBufferReceive(XcpRxDataStreamBufferHandle, (void *)((uint8_t *)diagRxbuf+index),diagRxSize-index, portMAX_DELAY);
					}


					RAMN_XCP_ProcessDiagPayload(xTaskGetTickCount(), diagRxbuf, diagRxSize, diagTxbuf, &diagTxSize);
					if (diagTxSize > 0U)
					{
						xBytesSent = xStreamBufferSend(XcpTxDataStreamBufferHandle, (void *) &diagTxSize, sizeof(diagTxSize), portMAX_DELAY );
						xBytesSent += xStreamBufferSend(XcpTxDataStreamBufferHandle, (void *) diagTxbuf, diagTxSize, portMAX_DELAY );
						if( xBytesSent != (diagTxSize + sizeof(diagTxSize) ))
						{
#ifdef HANG_ON_ERRORS
							Error_Handler();
#endif
						}
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
	RAMN_CUSTOM_CustomTask6(argument);
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
							if (xStreamBufferBytesAvailable(UdsTxDataStreamBufferHandle) >= sizeof(RAMN_UDS_ISOTPHandler.txSize))
							{
								// We received another TX request even though the ongoing transfer isn't over.
								// User may be manually sending ISO-TP requests and forgot the FLOW CONTROL frames.
								// Consider the transfer over and move to next request.
								break;
							}
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
							if (xStreamBufferBytesAvailable(KwpTxDataStreamBufferHandle) >= sizeof(RAMN_UDS_ISOTPHandler.txSize))
							{
								// We received another TX request even though the ongoing transfer isn't over.
								// User may be manually sending ISO-TP requests and forgot the FLOW CONTROL frames.
								// Consider the transfer over and move to next request.
								break;
							}
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
#if defined(ENABLE_CDC)
	RAMN_USB_Init(&USBD_TxStreamBufferHandle,&RAMN_SendUSBHandle);
	RAMN_CDC_Init(&USBD_RxStreamBufferHandle, &RAMN_ReceiveUSBHandle, &RAMN_SendUSBHandle);

#ifdef ENABLE_USB_AUTODETECT
	//We expect a notification from the serial close/open detection module
	//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	while (RAMN_CDC_GetTXStatus() != USBD_OK) osDelay(10U); //Wait for TX to be ready
#endif
	/* Infinite loop */
	for(;;)
	{
		size_t size = xStreamBufferReceive(USBD_TxStreamBufferHandle,USBIntermediateTxBuffer,sizeof(USBIntermediateTxBuffer), portMAX_DELAY);
		if (size > 0U)
		{
			// Only sends if USB serial port is opened
#ifdef ENABLE_USB_AUTODETECT
			// Make sure no notification is pending
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
#elif defined(ENABLE_UART)
	RAMN_UART_Init(&UART_TxStreamBufferHandle,&RAMN_SendUSBHandle);
	for(;;)
	{
		size_t size = xStreamBufferReceive(UART_TxStreamBufferHandle,UARTIntermediateTxBuffer,sizeof(UARTIntermediateTxBuffer), portMAX_DELAY);
		if (size > 0)
		{
			HAL_UART_Transmit_IT(&hlpuart1,UARTIntermediateTxBuffer, size);
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}
	}
#else
	RAMN_CUSTOM_CustomTask2(argument);
#endif
  /* USER CODE END RAMN_SendUSBFunc */
}

/* USER CODE BEGIN Header_RAMN_RxTask2Func */
/**
 * @brief Function implementing the RAMN_RxTask2 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_RxTask2Func */
void RAMN_RxTask2Func(void *argument)
{
  /* USER CODE BEGIN RAMN_RxTask2Func */
	/* Infinite loop */
	// On ECU A, this task is used for gs_usb
#ifndef ENABLE_GSUSB
	RAMN_CUSTOM_CustomTask3(argument);
#else
	BaseType_t            ret;
	struct gs_host_frame *recvFrame;
	FDCAN_TxHeaderTypeDef CANTxHeader;
	uint8_t CANTxData[64];

	/* Infinite loop */
	for(;;)
	{
		ret = xQueueReceive(RAMN_GSUSB_RecvQueueHandle, &recvFrame, portMAX_DELAY);
		if (ret != pdPASS)
		{
			RAMN_USB_Config.queueErrorCnt++;
		}
		else
		{

			// TODO implement CAN-FD

			//recvFrame->echo_id
			//recvFrame.flags
			//recvFrame.reserved


			CANTxHeader.Identifier = (recvFrame->can_id)&0x1FFFFFFF;
			CANTxHeader.DataLength  = recvFrame->can_dlc;
			if (recvFrame->can_id & CAN_RTR_FLAG) CANTxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
			else CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;

			if (recvFrame->can_id & CAN_EFF_FLAG) CANTxHeader.IdType = FDCAN_EXTENDED_ID;
			else CANTxHeader.IdType = FDCAN_STANDARD_ID;

			CANTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
			CANTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
			CANTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
			CANTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;

			//CANTxHeader.MessageMarker = 0U;

			if (CANTxHeader.TxFrameType == FDCAN_DATA_FRAME)
			{
				RAMN_memcpy(CANTxData, recvFrame->data, DLCtoUINT8(recvFrame->can_dlc));
			}

			//TODO: implement better error reports
			if (RAMN_FDCAN_SendMessage(&CANTxHeader,CANTxData) == RAMN_OK)
			{
				// for host candump
				ret = xQueueSendToBack(RAMN_GSUSB_SendQueueHandle, &recvFrame, CAN_QUEUE_TIMEOUT);
				if (ret != pdPASS)
				{
					RAMN_USB_Config.queueErrorCnt++;
					// Drop frame and return buffer to pool
					xQueueSendToBack(RAMN_GSUSB_PoolQueueHandle, &recvFrame, portMAX_DELAY);
				}
			}
		}
	}
#endif
  /* USER CODE END RAMN_RxTask2Func */
}

/* USER CODE BEGIN Header_RAMN_TxTask2Func */
/**
 * @brief Function implementing the RAMN_TxTask2 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RAMN_TxTask2Func */
void RAMN_TxTask2Func(void *argument)
{
  /* USER CODE BEGIN RAMN_TxTask2Func */
	// On ECU A, this task is used for gs_usb
#ifndef ENABLE_GSUSB
	RAMN_CUSTOM_CustomTask4(NULL);
#else
	BaseType_t            ret;
	struct gs_host_frame *frame;

	/* Infinite loop */
	for(;;)
	{
		ret = xQueueReceive(RAMN_GSUSB_SendQueueHandle, &frame, portMAX_DELAY);
		if(ret != pdPASS)
		{
			RAMN_USB_Config.queueErrorCnt++;
		}
		else
		{
			if (USBD_GSUSB_SendFrame(hpcd_USB_FS.pData, frame) == USBD_OK)
			{
				// Return buffer to pool
				ret = xQueueSendToBack(RAMN_GSUSB_PoolQueueHandle, &frame, portMAX_DELAY);
				if(ret != pdPASS)
				{
					RAMN_USB_Config.queueErrorCnt++;
				}
				else if(uxQueueMessagesWaiting(RAMN_GSUSB_PoolQueueHandle) == FDCAN_GetQueueSize())
				{
					xTaskNotifyGive(RAMN_SendCANHandle);
				}
			}
			else
			{
				// Send frame after
				ret = xQueueSendToFront(RAMN_GSUSB_SendQueueHandle, &frame, CAN_QUEUE_TIMEOUT);
				if(ret != pdPASS)
				{
					RAMN_USB_Config.queueErrorCnt++;
				}
			}
		}
	}
#endif
  /* USER CODE END RAMN_TxTask2Func */
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
#ifdef GENERATE_RUNTIME_STATS
	if (htim->Instance == TIM7)
	{
		ulHighFrequencyTimerTicks++;
	}
	else
#endif
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
		else if (htim->Instance == TIM6)
		{
			RAMN_CUSTOM_TIM6ISR(htim);
		}

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
