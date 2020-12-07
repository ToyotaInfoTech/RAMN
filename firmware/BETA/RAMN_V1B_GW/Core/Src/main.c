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
#include "usbd_cdc_if.h"
#include <sys/_stdint.h>
#include "task.h"
#include "usbd_cdc.h"
#include "usb_device.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

typedef struct{
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t data[8];
} rx_msg_t;

typedef struct{
	CAN_TxHeaderTypeDef TxHeader;
	uint8_t data[8];
} tx_msg_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

RNG_HandleTypeDef hrng;

/* Definitions for receiveCANTask */
osThreadId_t receiveCANTaskHandle;
uint32_t receiveCANTaskBuffer[ 2048 ];
osStaticThreadDef_t receiveCANTaskControlBlock;
const osThreadAttr_t receiveCANTask_attributes = {
		.name = "receiveCANTask",
		.stack_mem = &receiveCANTaskBuffer[0],
		.stack_size = sizeof(receiveCANTaskBuffer),
		.cb_mem = &receiveCANTaskControlBlock,
		.cb_size = sizeof(receiveCANTaskControlBlock),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for receiveUSBTask */
osThreadId_t receiveUSBTaskHandle;
uint32_t receiveUSBTaskBuffer[ 2048 ];
osStaticThreadDef_t receiveUSBTaskControlBlock;
const osThreadAttr_t receiveUSBTask_attributes = {
		.name = "receiveUSBTask",
		.stack_mem = &receiveUSBTaskBuffer[0],
		.stack_size = sizeof(receiveUSBTaskBuffer),
		.cb_mem = &receiveUSBTaskControlBlock,
		.cb_size = sizeof(receiveUSBTaskControlBlock),
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sendCANTask */
osThreadId_t sendCANTaskHandle;
uint32_t sendCANTaskBuffer[ 1024 ];
osStaticThreadDef_t sendCANTaskControlBlock;
const osThreadAttr_t sendCANTask_attributes = {
		.name = "sendCANTask",
		.stack_mem = &sendCANTaskBuffer[0],
		.stack_size = sizeof(sendCANTaskBuffer),
		.cb_mem = &sendCANTaskControlBlock,
		.cb_size = sizeof(sendCANTaskControlBlock),
		.priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for periodicTask */
osThreadId_t periodicTaskHandle;
uint32_t periodicTaskBuffer[ 1024 ];
osStaticThreadDef_t periodicTaskControlBlock;
const osThreadAttr_t periodicTask_attributes = {
		.name = "periodicTask",
		.stack_mem = &periodicTaskBuffer[0],
		.stack_size = sizeof(periodicTaskBuffer),
		.cb_mem = &periodicTaskControlBlock,
		.cb_size = sizeof(periodicTaskControlBlock),
		.priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for queueReceiveUSB */
osMessageQueueId_t queueReceiveUSBHandle;
uint8_t queueReceiveUSBBuffer[ 1024 * sizeof( uint8_t ) ];
osStaticMessageQDef_t queueReceiveUSBControlBlock;
const osMessageQueueAttr_t queueReceiveUSB_attributes = {
		.name = "queueReceiveUSB",
		.cb_mem = &queueReceiveUSBControlBlock,
		.cb_size = sizeof(queueReceiveUSBControlBlock),
		.mq_mem = &queueReceiveUSBBuffer,
		.mq_size = sizeof(queueReceiveUSBBuffer)
};
/* Definitions for queueReceiveCAN */
osMessageQueueId_t queueReceiveCANHandle;
uint8_t queueReceiveBuffer[ 128 * 36 ];
osStaticMessageQDef_t queueReceiveCANControlBlock;
const osMessageQueueAttr_t queueReceiveCAN_attributes = {
		.name = "queueReceiveCAN",
		.cb_mem = &queueReceiveCANControlBlock,
		.cb_size = sizeof(queueReceiveCANControlBlock),
		.mq_mem = &queueReceiveBuffer,
		.mq_size = sizeof(queueReceiveBuffer)
};
/* Definitions for queueSendCAN */
osMessageQueueId_t queueSendCANHandle;
uint8_t queueSendCANBuffer[ 128 * 32 ];
osStaticMessageQDef_t queueSendCANControlBlock;
const osMessageQueueAttr_t queueSendCAN_attributes = {
		.name = "queueSendCAN",
		.cb_mem = &queueSendCANControlBlock,
		.cb_size = sizeof(queueSendCANControlBlock),
		.mq_mem = &queueSendCANBuffer,
		.mq_size = sizeof(queueSendCANBuffer)
};
/* USER CODE BEGIN PV */
CAN_FilterTypeDef  sFilterConfig;

CAN_RxHeaderTypeDef   RxHeader;
uint8_t               RxData[8];
uint32_t              TxMailbox;
#define USB_RECV_BUFFER_SIZE 128
#define USB_SEND_BUFFER_SIZE 128
uint8_t usb_receive_buffer[USB_RECV_BUFFER_SIZE+1];
uint8_t usb_send_buffer[USB_SEND_BUFFER_SIZE+1];
uint8_t receivedBootResponse = 0;

uint8_t CANRxOverflow = 0;
uint8_t selectedBaudrate = 6;

uint8_t slcan_open = 0;
uint8_t carla_active = 0;
uint8_t programming_mode = 0;

tx_msg_t msg1A; //brake
tx_msg_t msg2F; //accel
tx_msg_t msg43; //rpm
tx_msg_t msg58; //steer
tx_msg_t msg6D; //shift
tx_msg_t msgA2; //horn
tx_msg_t msg1C9; //brake
uint16_t lastreadcounter = 0;
uint16_t lastreadcounter_50ms = 0;
uint32_t periodic_counter = 0;
uint32_t periodic_counter50ms = 0;


uint16_t ramn_brake;
uint16_t ramn_accel;
uint16_t ramn_steer;
uint8_t ramn_shift;
uint8_t ramn_horn;
uint8_t ramn_handbrake;

#define NUMBER_OF_PERIODIC_MSG 7
tx_msg_t* periodic_msgs[] = {&msg1A,&msg2F,&msg43,&msg58,&msg6D,&msgA2,&msg1C9};


const uint16_t periodic_msg_ids[NUMBER_OF_PERIODIC_MSG] = {0x1A,0x2F,0x43,0x58,0x6D,0xA2,0x1C9};
uint32_t periodic_msg_random[NUMBER_OF_PERIODIC_MSG] =  {0};
uint32_t rng_counter = 0;

//copied from https://community.st.com/s/question/0D50X00009XkeeW/stm32l476rg-jump-to-bootloader-from-software
typedef void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_RNG_Init(void);
void receiveCANFunction(void *argument);
void receiveUSBTaskFunction(void *argument);
void sendCANFunction(void *argument);
void periodicTaskFunction(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


// mapping of ASCII characters to hex values
const uint8_t ascii_hashmap[] =
{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  !"#$%&'
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ()*+,-./
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
		0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
		0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_
		0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // `abcdefg
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hijklmno
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pqrstuvw
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // xyz{|}~.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // ........
};

const uint8_t nibble_to_ascii[] =
{
		'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};


void resetCAN()
{
	CANRxOverflow = 0;
	//Update CAN adapter with new baudrate settings
	if (HAL_CAN_Init(&hcan1) != HAL_OK)
	{
		Error_Handler();
	}
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

	//Activate Error Notification
	if (HAL_CAN_ActivateNotification(&hcan1,  CAN_IT_RX_FIFO0_OVERRUN) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}

	//Activate Transmit Complete Notification

	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
		/* Notification Error */
		Error_Handler();
	}



}

void updateCANBaudrate(uint8_t val)
{
	switch(val)
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
		//keep 32 divider (125k, same as STM32 CAN Bootloader) as default
		hcan1.Init.Prescaler = 32;
		break;
	}
	resetCAN();
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
	MX_CAN1_Init();
	MX_RNG_Init();
	/* USER CODE BEGIN 2 */

	//set the ID to 0 to prevent thread from sending them
	for(uint8_t i = 0; i < NUMBER_OF_PERIODIC_MSG; i++)
	{
		//periodic_msgs[i]->TxHeader.StdId = 0;
		periodic_msgs[i]->TxHeader.StdId = periodic_msg_ids[i];
		periodic_msgs[i]->TxHeader.ExtId = 0x00;
		periodic_msgs[i]->TxHeader.RTR = CAN_RTR_DATA;
		periodic_msgs[i]->TxHeader.IDE = CAN_ID_STD;
		periodic_msgs[i]->TxHeader.DLC = 8;
		memset(periodic_msgs[i]->data,0,8);
		periodic_msgs[i]->TxHeader.TransmitGlobalTime = DISABLE;
	}
	//Set B00T0 to 0 (i.e. let it boot from flash memory)
	HAL_GPIO_WritePin(ECU_1_BOOT0_GPIO_Port, ECU_1_BOOT0_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ECU_2_BOOT0_GPIO_Port, ECU_2_BOOT0_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ECU_3_BOOT0_GPIO_Port, ECU_3_BOOT0_Pin,GPIO_PIN_RESET);

	//Enable the power supplies of the other ECUs
	//(Voltage >1.2V to enable)
	HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_SET);

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

	/* Create the queue(s) */
	/* creation of queueReceiveUSB */
	queueReceiveUSBHandle = osMessageQueueNew (1024, sizeof(uint8_t), &queueReceiveUSB_attributes);

	/* creation of queueReceiveCAN */
	queueReceiveCANHandle = osMessageQueueNew (128, 36, &queueReceiveCAN_attributes);

	/* creation of queueSendCAN */
	queueSendCANHandle = osMessageQueueNew (128, 32, &queueSendCAN_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	USBD_Queue_UsbRx = queueReceiveUSBHandle;
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of receiveCANTask */
	receiveCANTaskHandle = osThreadNew(receiveCANFunction, NULL, &receiveCANTask_attributes);

	/* creation of receiveUSBTask */
	receiveUSBTaskHandle = osThreadNew(receiveUSBTaskFunction, NULL, &receiveUSBTask_attributes);

	/* creation of sendCANTask */
	sendCANTaskHandle = osThreadNew(sendCANFunction, NULL, &sendCANTask_attributes);

	/* creation of periodicTask */
	periodicTaskHandle = osThreadNew(periodicTaskFunction, NULL, &periodicTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	//start first occurence of random numbers
	rng_counter = 0;
	if (HAL_RNG_GenerateRandomNumber_IT(&hrng) != HAL_OK)
	{
		/* RNG peripheral start-up error */
		Error_Handler();
	}
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
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_RNG;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
	PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_PLLSAI1;
	PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
	PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
	PeriphClkInit.PLLSAI1.PLLSAI1N = 12;
	PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
	PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;
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
	//sFilterConfig.FilterMaskIdLow = 0xFE00; //for CARLA
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	resetCAN();
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
	HAL_GPIO_WritePin(GPIOC, ECU_2_EN_Pin|ECU_2_BOOT0_Pin|ECU_3_BOOT0_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, DC_Pin|LCD_CSE_Pin|ECU_1_EN_Pin|CAN_STB_Pin
			|ECU_1_BOOT0_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : ECU_2_EN_Pin ECU_2_BOOT0_Pin ECU_3_BOOT0_Pin */
	GPIO_InitStruct.Pin = ECU_2_EN_Pin|ECU_2_BOOT0_Pin|ECU_3_BOOT0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : ECU_3_EN_Pin */
	GPIO_InitStruct.Pin = ECU_3_EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ECU_3_EN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : DC_Pin LCD_CSE_Pin ECU_1_EN_Pin CAN_STB_Pin
                           ECU_1_BOOT0_Pin */
	GPIO_InitStruct.Pin = DC_Pin|LCD_CSE_Pin|ECU_1_EN_Pin|CAN_STB_Pin
			|ECU_1_BOOT0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : PB13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Get RX message */
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
	{
		/* Reception Error */
		Error_Handler();
	}
	rx_msg_t msg;
	msg.RxHeader = RxHeader;
	memcpy(msg.data,RxData,RxHeader.DLC);
	xQueueSendFromISR(queueReceiveCANHandle,&msg,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(sendCANTaskHandle,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(sendCANTaskHandle,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(sendCANTaskHandle,&xHigherPriorityTaskWoken);
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
	if ((err & ( HAL_CAN_ERROR_RX_FOV0)) != 0)
	{
		//Receive FIFO0  overrun
		CANRxOverflow = 1;
	}
	if ((err & ( HAL_CAN_ERROR_RX_FOV1)) != 0)
	{
		//Receive FIFO1 overrun
		//CANRxOverflow = 1;
	}
	if ((err & (HAL_CAN_ERROR_EWG|HAL_CAN_ERROR_EPV|HAL_CAN_ERROR_BOF|HAL_CAN_ERROR_STF|HAL_CAN_ERROR_FOR|HAL_CAN_ERROR_BR|HAL_CAN_ERROR_BD|HAL_CAN_ERROR_CRC)) != 0)
	{
		//Problem with frames
		//HAL_CAN_AbortTxRequest(hcan,CAN_TX_MAILBOX2);
	}
	if ((err & HAL_CAN_ERROR_ACK) != 0)
	{
		//No acknowledgement
	}
	HAL_CAN_ResetError(hcan);
}

void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef* hrng, uint32_t random32bit)
{

	/* Straight random number retrieval */

	periodic_msg_random[rng_counter] = random32bit;

	rng_counter++;

	/* HAL_RNG_IRQHandler() disables IT at each interruption,
     so, need to re-enable IT in callback to get several random numbers in a row */
	if (rng_counter < NUMBER_OF_PERIODIC_MSG)
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
	/* init code for USB_DEVICE */
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	for(;;)
	{
		rx_msg_t msg;
		{
			/* Get RX message */
			if (xQueueReceive(queueReceiveCANHandle,&msg,1000) == pdPASS)
			{
				switch(msg.RxHeader.StdId)
				{
				case 0x24:
					ramn_brake = (msg.data[0] << 8) + msg.data[1];
					lastreadcounter = (msg.data[2]<<8) + msg.data[3];
					break;
				case 0x39:
					ramn_accel = (msg.data[0] << 8) + msg.data[1];
					break;
				case 0x62:
					ramn_steer = (msg.data[0] << 8) + msg.data[1];
					break;
				case 0x77:
					ramn_shift = msg.data[0];
					break;
				case 0x98:
					ramn_horn = msg.data[0];
					break;
				case 0x1D3:
					ramn_handbrake = msg.data[0];
					lastreadcounter_50ms =(msg.data[2]<<8) + msg.data[3];
					break;
				}

				if (slcan_open)
				{
					//Only deal with messages with less than 8 bytes, as they should be for CAN
					if (msg.RxHeader.StdId == 0x79)
					{
						receivedBootResponse = 1;
					}
					if(msg.RxHeader.DLC <= 8)
					{
						usb_send_buffer[0] = 't';
						usb_send_buffer[1] = nibble_to_ascii[(msg.RxHeader.StdId >> 8) & 0xF];
						usb_send_buffer[2] = nibble_to_ascii[(msg.RxHeader.StdId >> 4) & 0xF];
						usb_send_buffer[3] = nibble_to_ascii[msg.RxHeader.StdId & 0xF];
						usb_send_buffer[4] = nibble_to_ascii[msg.RxHeader.DLC & 0xF];
						for(uint8_t i=0;i<msg.RxHeader.DLC;i++)
						{
							usb_send_buffer[5+2*i] = nibble_to_ascii[(msg.data[i] >> 4) & 0xF];
							usb_send_buffer[6+2*i] = nibble_to_ascii[msg.data[i] & 0xF];
						}
						usb_send_buffer[5+2*msg.RxHeader.DLC] = '\r';
						usb_send_buffer[6+2*msg.RxHeader.DLC] = '\0';

						while(CDC_Transmit_FS((uint8_t*)usb_send_buffer,6+2*msg.RxHeader.DLC) == USBD_BUSY)
						{
							taskYIELD(); // //sending over usb takes time, so yield to others
						}


					}
				}
			}

		}

	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_receiveUSBTaskFunction */
/**
 * @brief Function implementing the receiveUSBTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_receiveUSBTaskFunction */
void receiveUSBTaskFunction(void *argument)
{
	/* USER CODE BEGIN receiveUSBTaskFunction */
	/* Infinite loop */
	uint8_t val = 0;
	uint8_t n = 0;
	uint32_t counter = 0;
	/*##-5- Configure Transmission process #####################################*/
	tx_msg_t msg;
	msg.TxHeader.StdId = 0x00;
	msg.TxHeader.ExtId = 0x00;
	msg.TxHeader.RTR = CAN_RTR_DATA;
	msg.TxHeader.IDE = CAN_ID_STD;
	msg.TxHeader.DLC = 0;
	msg.TxHeader.TransmitGlobalTime = DISABLE;

	for(;;)
	{
		if (xQueueReceive(queueReceiveUSBHandle, &val, 1000) == pdPASS)
		{
			//Process USB Message
			usb_receive_buffer[n] = val;
			if (n < USB_RECV_BUFFER_SIZE)	{
				n++;
			}
			//Wait for end of command character
			if(val == '\r')
			{
				uint8_t command = usb_receive_buffer[0];

				switch(command)
				{
				case 'r':
				case 't':
					msg.TxHeader.IDE = CAN_ID_STD;
					msg.TxHeader.DLC = (usb_receive_buffer[4] - '0');  //convert the 1 ASCII numeral to uint8_t directly
					msg.TxHeader.StdId = (ascii_hashmap[usb_receive_buffer[1]] << 8) + (ascii_hashmap[usb_receive_buffer[2]] << 4) + (ascii_hashmap[usb_receive_buffer[3]]);
					if(command == 'r')
					{
						msg.TxHeader.RTR = CAN_RTR_REMOTE;
					}
					else
					{
						msg.TxHeader.RTR = CAN_RTR_DATA;
						for(uint8_t i=0; i < (msg.TxHeader.DLC); i++)
						{
							msg.data[i] = (ascii_hashmap[usb_receive_buffer[5+(i*2)]] << 4) + (ascii_hashmap[usb_receive_buffer[6+(i*2)]]);
						}
					}
					//CDC_Transmit_FS((uint8_t*)"\r",1);

					if (xQueueSend(queueSendCANHandle,&msg,0) == pdPASS)
					{
						if(programming_mode) CDC_Transmit_FS((uint8_t*)"\r",1);
					}
					else
					{
						//queue likely full, send NACK
						//CDC_Transmit_FS((uint8_t*)"\a",1);
					}

					break;

				case 'R':
				case 'T':

					msg.TxHeader.IDE = CAN_ID_EXT;
					msg.TxHeader.DLC = (usb_receive_buffer[9] - '0');  //convert the 1 ASCII numeral to uint8_t directly
					usb_receive_buffer[9] = 0; 				       //Overwrite the dlc field by 0 to create a null-terminated CAN ID string
					//msg.TxHeader.ExtId = strtol((uint8_t*)&usb_receive_buffer[1],NULL,16);
					msg.TxHeader.ExtId = (ascii_hashmap[usb_receive_buffer[1]] << 28)  + (ascii_hashmap[usb_receive_buffer[2]] << 24) + (ascii_hashmap[usb_receive_buffer[3]] << 20)  + (ascii_hashmap[usb_receive_buffer[4]] << 16)  + (ascii_hashmap[usb_receive_buffer[5]] << 12)  + (ascii_hashmap[usb_receive_buffer[6]] << 8) + (ascii_hashmap[usb_receive_buffer[7]] << 4) + (ascii_hashmap[usb_receive_buffer[8]]);
					if(command == 'R')
					{
						msg.TxHeader.RTR = CAN_RTR_REMOTE;
					}
					else
					{
						msg.TxHeader.RTR = CAN_RTR_DATA;
						for(uint8_t i=0; i < (msg.TxHeader.DLC); i++)
						{
							msg.data[i] = (ascii_hashmap[usb_receive_buffer[10+(i*2)]] << 4) + (ascii_hashmap[usb_receive_buffer[11+(i*2)]]);
						}
					}

					if (xQueueSend(queueSendCANHandle,&msg,0) == pdPASS)
					{
						//success, send ACK
						//CDC_Transmit_FS((uint8_t*)"\r",1);
					}
					else
					{
						//queue likely full, send NACK
						//CDC_Transmit_FS((uint8_t*)"\a",1);
					}

					break;

				case 'V':
					CDC_Transmit_FS((uint8_t*)"RAMN v1.0BETA\r",14);
					break;

				case 'S':
					usb_receive_buffer[2] = 0;
					selectedBaudrate = ascii_hashmap[usb_receive_buffer[1]];
					updateCANBaudrate(selectedBaudrate);
					CDC_Transmit_FS((uint8_t*)"\r",1);
					break;

				case 'O':
					hcan1.Init.Mode = CAN_MODE_NORMAL;
					slcan_open = 1;
					resetCAN();
					CDC_Transmit_FS((uint8_t*)"\r",1);
					break;
				case 'o':
					//Command to turn off ECUs
					switch(usb_receive_buffer[1])
					{
					case 'C':
						HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_RESET);
						break;
					case 'D':
						HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_RESET);
						break;
					case 'B':
						HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_RESET);
						break;
					default:
						HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_RESET);
						break;
					}

					CDC_Transmit_FS((uint8_t*)"\r",1);
					break;
					case 'd':
						//Command to turn on ECU B and C only
						HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_SET);
						HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_SET);
						CDC_Transmit_FS((uint8_t*)"\r",1);
						break;
					case 'e':
						//Command to turn on specified USB
						switch(usb_receive_buffer[1])
						{
						case 'C':
							HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_SET);
							break;
						case 'D':
							HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_SET);
							break;
						case 'B':
							HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_SET);
							break;
						default:
							HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_SET);
							HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_SET);
							HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_SET);
							break;
						}

						CDC_Transmit_FS((uint8_t*)"\r",1);
						break;

						case 'L':
							CDC_Transmit_FS((uint8_t*)"\r",1);
							hcan1.Init.Mode = CAN_MODE_SILENT;
							slcan_open = 1;
							resetCAN();
							break;
						case 'c':
							CDC_Transmit_FS((uint8_t*)"\r",1);
							hcan1.Init.Mode = CAN_MODE_NORMAL;
							carla_active = 1;
							resetCAN();
							//Set counter to last read counter
							periodic_counter = lastreadcounter;
							periodic_counter50ms = lastreadcounter_50ms;
							vTaskResume(periodicTaskHandle);
							break;
						case 'u':
							//format is u<brake16><accel16><rpm16><steer16><shift8><horn8><handbrake8>
							//eg "u{:04x}{:04x}{:04x}{:04x}{:02x}{:02x}{:02x}\r"
							msg1A.data[0] = (ascii_hashmap[usb_receive_buffer[1]] << 4) + (ascii_hashmap[usb_receive_buffer[2]]);
							msg1A.data[1] = (ascii_hashmap[usb_receive_buffer[3]] << 4) + (ascii_hashmap[usb_receive_buffer[4]]);

							msg2F.data[0] = (ascii_hashmap[usb_receive_buffer[5]] << 4) + (ascii_hashmap[usb_receive_buffer[6]]);
							msg2F.data[1] = (ascii_hashmap[usb_receive_buffer[7]] << 4) + (ascii_hashmap[usb_receive_buffer[8]]);

							msg43.data[0] = (ascii_hashmap[usb_receive_buffer[9]] << 4) + (ascii_hashmap[usb_receive_buffer[10]]);
							msg43.data[1] = (ascii_hashmap[usb_receive_buffer[11]] << 4) + (ascii_hashmap[usb_receive_buffer[12]]);

							msg58.data[0] = (ascii_hashmap[usb_receive_buffer[13]] << 4) + (ascii_hashmap[usb_receive_buffer[14]]);
							msg58.data[1] = (ascii_hashmap[usb_receive_buffer[15]] << 4) + (ascii_hashmap[usb_receive_buffer[16]]);

							msg6D.data[0] = (ascii_hashmap[usb_receive_buffer[17]] << 4) + (ascii_hashmap[usb_receive_buffer[18]]);
							msgA2.data[0] = (ascii_hashmap[usb_receive_buffer[19]] << 4) + (ascii_hashmap[usb_receive_buffer[20]]);
							msg1C9.data[0] = (ascii_hashmap[usb_receive_buffer[21]] << 4) + (ascii_hashmap[usb_receive_buffer[22]]);
							break;

						case 'x':
							vTaskSuspend(periodicTaskHandle);
							carla_active = 0;
							CDC_Transmit_FS((uint8_t*)"\r",1);
						case 'C':
							slcan_open = 0;
							CDC_Transmit_FS((uint8_t*)"\r",1);
							break;

						case 'N':
							CDC_Transmit_FS((uint8_t*)"0000111122223333\r",17);
							break;

						case 'n':
							//Command to restart the board
							programming_mode = 0;
							HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(ECU_1_BOOT0_GPIO_Port, ECU_1_BOOT0_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(ECU_2_BOOT0_GPIO_Port, ECU_2_BOOT0_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(ECU_3_BOOT0_GPIO_Port, ECU_3_BOOT0_Pin,GPIO_PIN_RESET);
							//updateCANBaudrate(selectedBaudrate);
							//Turn Off slcan feature
							//slcan_open = 0;
							osDelay(100);


							//HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_SET);
							//HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_SET);
							//HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_SET);
							
							HAL_NVIC_SystemReset();
							break;

						case 'p':
							programming_mode = 1;
							slcan_open = 1;
							//turn off all ECUs
							HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_RESET);
							HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_RESET);
							//make sure we are in open slcan mode
							updateCANBaudrate(4); //Bootloader default is 125kb
							receivedBootResponse = 0;
							osDelay(100);
							counter = 0;
							switch(usb_receive_buffer[1])
							{
							default:
								break;
							case 'B':
								HAL_GPIO_WritePin(ECU_1_BOOT0_GPIO_Port, ECU_1_BOOT0_Pin,GPIO_PIN_SET);
								HAL_GPIO_WritePin(ECU_1_EN_GPIO_Port, ECU_1_EN_Pin,GPIO_PIN_SET);
								break;
							case 'C':
								HAL_GPIO_WritePin(ECU_2_BOOT0_GPIO_Port, ECU_2_BOOT0_Pin,GPIO_PIN_SET);
								HAL_GPIO_WritePin(ECU_2_EN_GPIO_Port, ECU_2_EN_Pin,GPIO_PIN_SET);
								break;
							case 'D':
								HAL_GPIO_WritePin(ECU_3_BOOT0_GPIO_Port, ECU_3_BOOT0_Pin,GPIO_PIN_SET);
								HAL_GPIO_WritePin(ECU_3_EN_GPIO_Port, ECU_3_EN_Pin,GPIO_PIN_SET);
								break;
							}

							while(counter < 20)
							{
								//Send bootloader requests until answer from target ECU
								msg.TxHeader.ExtId = 0x01;
								msg.TxHeader.RTR = CAN_RTR_DATA;
								msg.TxHeader.IDE = CAN_ID_STD;
								msg.TxHeader.DLC = 0;
								msg.TxHeader.StdId = 0x79;
								while(xQueueSend(queueSendCANHandle,&msg,0) != pdPASS);
								//HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
								osDelay(100);
								counter++;
								//add timeout
								if(receivedBootResponse) break;
							}
							CDC_Transmit_FS((uint8_t*)"\r",1);
							break;
							case 'b':
								HAL_RCC_DeInit();
								SysTick->CTRL = 0;
								SysTick->LOAD = 0;
								SysTick->VAL = 0;
								/** * Step: Disable all interrupts */
								__disable_irq();
								/* ARM Cortex-M Programming Guide to Memory Barrier Instructions.*/
								__DSB();__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

								/* Remap is bot visible at once. Execute some unrelated command! */
								__DSB();
								__ISB();
								JumpToApplication = (void (*)(void)) (*((uint32_t *)(0x1FFF0000 + 4)));
								/* Initialize user application's Stack Pointer */
								__set_MSP(*(__IO uint32_t*) 0x1FFF0000);
								JumpToApplication();
								break; // just in case
							case 's':
							case 'A':
							case 'P':
							case 'F':
							case 'X':
							case 'W':
							case 'M':
							case 'm':
							case 'U':
							case 'Z':
							case 'Q':
							default:
								// transmit BELL sign (\a) for error since we don't support those commands
								CDC_Transmit_FS((uint8_t*)"\a",1);
								break;
				}

				n = 0;
			}
		}
	}

	/* USER CODE END receiveUSBTaskFunction */
}

/* USER CODE BEGIN Header_sendCANFunction */
/**
 * @brief Function implementing the sendCANTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_sendCANFunction */
void sendCANFunction(void *argument)
{
	/* USER CODE BEGIN sendCANFunction */
	for(;;)
	{
		tx_msg_t msg;

		if (xQueueReceive(queueSendCANHandle,&msg,1000) == pdPASS)
		{
			while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
			{
				//No TX space, go nap nap
				ulTaskNotifyTake( pdTRUE, 1000 );
			}

			if (HAL_CAN_AddTxMessage(&hcan1, &msg.TxHeader, msg.data, &TxMailbox) != HAL_OK)
			{
				/* Transmission request Error */
				Error_Handler();
			}

		}

	}



	/* USER CODE END sendCANFunction */
}

/* USER CODE BEGIN Header_periodicTaskFunction */
/**
 * @brief Function implementing the periodicTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_periodicTaskFunction */
void periodicTaskFunction(void *argument)
{
	/* USER CODE BEGIN periodicTaskFunction */
	/* Infinite loop */


	vTaskSuspend(NULL); //put the task in a suspended state by default

	for(;;)
	{
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();


		for(uint8_t i = 0; i < NUMBER_OF_PERIODIC_MSG-1; i++)
		{
			periodic_msgs[i]->data[3] = periodic_counter&0xFF;
			periodic_msgs[i]->data[2] = (periodic_counter>>8)&0xFF;
			memcpy(&(periodic_msgs[i]->data[4]),&(periodic_msg_random[i]),4);
			//memcpy(&(periodic_msgs[i]->data[4]),&counter,4);
			xQueueSend(queueSendCANHandle,periodic_msgs[i],0);
		}

		if(periodic_counter % 5 == 0)
		{
			//50ms  message
			xQueueSend(queueSendCANHandle,&msg1C9,0);
			msg1C9.data[3] = periodic_counter50ms&0xFF;
			msg1C9.data[2] = (periodic_counter50ms>>8)&0xFF;
			memcpy(&(msg1C9.data[4]),&(periodic_msg_random[NUMBER_OF_PERIODIC_MSG-1]),4);
			periodic_counter50ms++;

		}
		periodic_counter++;


		usb_send_buffer[0] = 'u';
		usb_send_buffer[1] = nibble_to_ascii[(ramn_brake >> 12)&0xF];
		usb_send_buffer[2] = nibble_to_ascii[(ramn_brake >> 8)&0xF];
		usb_send_buffer[3] = nibble_to_ascii[(ramn_brake >> 4)&0xF];
		usb_send_buffer[4] = nibble_to_ascii[(ramn_brake)&0xF];

		usb_send_buffer[5] = nibble_to_ascii[(ramn_accel >> 12)&0xF];
		usb_send_buffer[6] = nibble_to_ascii[(ramn_accel >> 8)&0xF];
		usb_send_buffer[7] = nibble_to_ascii[(ramn_accel >> 4)&0xF];
		usb_send_buffer[8] = nibble_to_ascii[(ramn_accel)&0xF];

		usb_send_buffer[9] = nibble_to_ascii[(ramn_steer >> 12)&0xF];
		usb_send_buffer[10] = nibble_to_ascii[(ramn_steer >> 8)&0xF];
		usb_send_buffer[11] = nibble_to_ascii[(ramn_steer >> 4)&0xF];
		usb_send_buffer[12] = nibble_to_ascii[(ramn_steer)&0xF];

		usb_send_buffer[13] = nibble_to_ascii[(ramn_shift >> 4)&0xF];
		usb_send_buffer[14] = nibble_to_ascii[(ramn_shift)&0xF];

		usb_send_buffer[15] = nibble_to_ascii[(ramn_horn >> 4)&0xF];
		usb_send_buffer[16] = nibble_to_ascii[(ramn_horn)&0xF];

		usb_send_buffer[17] = nibble_to_ascii[(ramn_handbrake >> 4)&0xF];
		usb_send_buffer[18] = nibble_to_ascii[(ramn_handbrake)&0xF];

		usb_send_buffer[19] = '\r';
		usb_send_buffer[20] = '\0';


		while(CDC_Transmit_FS((uint8_t*)usb_send_buffer,20) == USBD_BUSY)
		{
			//taskYIELD(); // //sending over usb takes time, so yield to others
		}

		rng_counter = 0;
		if (HAL_RNG_GenerateRandomNumber_IT(&hrng) != HAL_OK)
		{
			/* RNG peripheral start-up error */
			Error_Handler();
		}
		vTaskDelayUntil( &xLastWakeTime, 10 );
	}
	/* USER CODE END periodicTaskFunction */
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
