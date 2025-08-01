/*
 * ramn_canfd.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
#include "ramn_canfd.h"

// FDCAN peripheral parameters
#define TSEG1_MAX 16
#define TSEG2_MAX 8
#define TQ_MAX    (TSEG1_MAX + TSEG2_MAX + 1)
#define PRESCALER_MAX   1024

/*=====================================================================*
 *                            Private global                            *
 *======================================================================*/

// Handle for FDCAN peripheral
static FDCAN_HandleTypeDef* hfdcan;

// Task to notify when the task in charge of transmitting CAN messages should check the buffer.
// Note that it may be notified even when no messages is ready - For example when a TX error has occurred.
static osThreadId_t* sendTask;

// Task to notify when an error was detected.
// It is up to that task to clear error registers, and therefore it should have the highest priority
static osThreadId_t* errTask;

// Buffer to save RX DATA from ISR
static uint8_t CANRxData[64];

// Buffer to save RX Header Data from ISR
static FDCAN_RxHeaderTypeDef CANRxHeader;

// Buffer to save protocol status error from CAN Error Status ISR
static FDCAN_ProtocolStatusTypeDef protocolStatus;

// Semaphore to prevent Stream buffer access from different threads
static SemaphoreHandle_t CAN_TX_SEMAPHORE;
static StaticSemaphore_t CAN_TX_SEMAPHORE_STRUCT;

/*=====================================================================*
 *                            Public global                             *
 *======================================================================*/

StreamBufferHandle_t CANRxDataStreamBufferHandle;
StreamBufferHandle_t CANTxDataStreamBufferHandle;

RAMN_FDCAN_Status_t RAMN_FDCAN_Status = {
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.prevCANError = HAL_FDCAN_ERROR_NONE,
		.sFilterStdConfig = {
				.IdType = FDCAN_STANDARD_ID,
				.FilterIndex = 0,
				.FilterType = FDCAN_FILTER_MASK,
				.FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
				.FilterID1 = 0x0,
				.FilterID2 = 0x0,
		},
		.sFilterExtConfig = {
				.IdType = FDCAN_EXTENDED_ID,
				.FilterIndex = 0,
				.FilterType = FDCAN_FILTER_MASK,
				.FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
				.FilterID1 = 0x0,
				.FilterID2 = 0x0,
		}
};

#ifdef USE_HARDWARE_CAN_FILTERS
// List of Standard CAN IDs that can be received when hardware filters are ON
static const uint16_t recvStdCANIDList[] =
{
#ifdef RECEIVE_CONTROL_BRAKE
		CAN_SIM_CONTROL_BRAKE_CANID,
#endif
#ifdef RECEIVE_COMMAND_BRAKE
		CAN_SIM_COMMAND_BRAKE_CANID,
#endif
#ifdef RECEIVE_CONTROL_ACCEL
		CAN_SIM_CONTROL_ACCEL_CANID,
#endif
#ifdef RECEIVE_COMMAND_ACCEL
		CAN_SIM_COMMAND_ACCEL_CANID,
#endif
#ifdef RECEIVE_STATUS_RPM
		CAN_SIM_STATUS_RPM_CANID,
#endif
#ifdef RECEIVE_CONTROL_STEERING
		CAN_SIM_CONTROL_STEERING_CANID,
#endif
#ifdef RECEIVE_COMMAND_STEERING
		CAN_SIM_COMMAND_STEERING_CANID,
#endif
#ifdef RECEIVE_CONTROL_SHIFT
		CAN_SIM_CONTROL_SHIFT_CANID,
#endif
#ifdef RECEIVE_COMMAND_SHIFT
		CAN_SIM_COMMAND_SHIFT_CANID,
#endif
#ifdef RECEIVE_COMMAND_HORN
		CAN_SIM_COMMAND_HORN_CANID,
#endif
#ifdef RECEIVE_CONTROL_HORN
		CAN_SIM_CONTROL_HORN_CANID,
#endif
#ifdef RECEIVE_CONTROL_SIDEBRAKE
		CAN_SIM_CONTROL_SIDEBRAKE_CANID,
#endif
#ifdef RECEIVE_COMMAND_SIDEBRAKE
		CAN_SIM_COMMAND_SIDEBRAKE_CANID,
#endif
#ifdef RECEIVE_COMMAND_TURNINDICATOR
		CAN_SIM_COMMAND_TURNINDICATOR_CANID,
#endif
#ifdef RECEIVE_CONTROL_ENGINEKEY
		CAN_SIM_CONTROL_ENGINEKEY_CANID,
#endif
#ifdef RECEIVE_COMMAND_LIGHTS
		CAN_SIM_COMMAND_LIGHTS_CANID,
#endif
#ifdef RECEIVE_CONTROL_LIGHTS
		CAN_SIM_CONTROL_LIGHTS_CANID,
#endif
#ifdef ENABLE_UDS
		UDS_RX_CANID,

#ifdef UDS_ACCEPT_FUNCTIONAL_ADDRESSING
		UDS_FUNCTIONAL_RX_CANID,
#endif

#endif
#ifdef ENABLE_KWP
		KWP_RX_CANID,
#endif
#ifdef ENABLE_XCP
		XCP_RX_CANID,
#endif
		RTR_DEMO_ID,

#ifdef ENABLE_MINICTF
		CTF_STANDARD_ID_1,
		CTF_STANDARD_ID_2,
		CTF_STANDARD_ID_3,
		CTF_STANDARD_ID_4,
#endif
};

// List of Extended CAN IDs that can be received when hardware filters are ON
static const uint32_t recvExtCANIDList[] =
{
		CTF_EXTENDED_ID
};

static_assert(sizeof(recvStdCANIDList) <= 28U, "Too many hardware filters, update the code to use other types of filters (such as dual or range)");
static_assert(sizeof(recvExtCANIDList) <= 8U,  "Too many hardware filters, update the code to use other types of filters (such as dual or range)");

#endif


/*=====================================================================*
 *                          Private functions                           *
 *======================================================================*/

static void FDCAN_Error_Handler()
{
#ifdef HANG_ON_ERRORS
	while(1);
#endif
}

#ifdef USE_HARDWARE_CAN_FILTERS
static void FDCAN_SetStandardFilterList(const uint16_t *canidList, uint16_t size)
{
	RAMN_FDCAN_Status.sFilterStdConfig.IdType 		= FDCAN_STANDARD_ID;
	RAMN_FDCAN_Status.sFilterStdConfig.FilterType   = FDCAN_FILTER_MASK; //Use FDCAN_FILTER_DUAL if you need more IDs
	RAMN_FDCAN_Status.sFilterStdConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	RAMN_FDCAN_Status.sFilterStdConfig.FilterID2 	= 0x7FF;
	for(uint8_t i = 0; i < size; i++)
	{
		RAMN_FDCAN_Status.sFilterStdConfig.FilterIndex  = i;
		RAMN_FDCAN_Status.sFilterStdConfig.FilterID1    = canidList[i];
		// Configure Filter for STANDARD CAN IDs
		if (HAL_FDCAN_ConfigFilter(hfdcan, &(RAMN_FDCAN_Status.sFilterStdConfig)) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

static void FDCAN_SetExtendedFilterList(const uint32_t *canidList, uint16_t size)
{
	RAMN_FDCAN_Status.sFilterExtConfig.IdType 		= FDCAN_EXTENDED_ID;
	RAMN_FDCAN_Status.sFilterExtConfig.FilterType   = FDCAN_FILTER_MASK; //Use FDCAN_FILTER_DUAL if you need more IDs
	RAMN_FDCAN_Status.sFilterExtConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	RAMN_FDCAN_Status.sFilterExtConfig.FilterID2 	= 0x7FFFFFFF;

	for(uint8_t i = 0; i < size; i++)
	{
		RAMN_FDCAN_Status.sFilterExtConfig.FilterIndex  = i;
		RAMN_FDCAN_Status.sFilterExtConfig.FilterID1    = canidList[i];
		// Configure Filter for STANDARD CAN IDs
		if (HAL_FDCAN_ConfigFilter(hfdcan, &(RAMN_FDCAN_Status.sFilterExtConfig)) != HAL_OK)
		{
			Error_Handler();
		}
	}
}
#endif

#ifdef ENABLE_DYNAMIC_BITRATE
//TODO: test with more bitrates
static RAMN_Result_t FDCAN_ConfigureBitrate(uint32_t bitrate)
{
	int32_t  error;
	int32_t  best = 0x7FFFFFFF;
	uint32_t fdcanClock;
	uint8_t  tqsum;
	uint16_t prescaler;
	uint8_t  tseg1;
	uint8_t  tseg2;
	uint16_t samplePoint;
	RAMN_Result_t result = RAMN_ERROR;

	// CAN in Automation recommended sample points
	if (bitrate > 800000) samplePoint = 750;
	else if (bitrate > 500000) samplePoint = 800;
	else samplePoint = 875;

	// Threhsold of 5% of the ideal sample point
	uint16_t errorThreshold = samplePoint / 20;

	fdcanClock = FDCAN_PERIPHERAL_CLOCK;

	prescaler = 1;
	tqsum = fdcanClock / bitrate;
	while (tqsum > TQ_MAX)
	{
		prescaler++;
		tqsum = (fdcanClock / prescaler) / bitrate;
	}

	while (prescaler < PRESCALER_MAX)
	{
		tseg1 = tqsum / 2; // Start at 50%
		while (tseg1 < tqsum - 2)
		{
			tseg2 = tqsum - tseg1 - 1;
			error = samplePoint - ((tseg1 * 1000) / tqsum);
			if (error < 0) error = -error;
			if (error < best && error <= errorThreshold) // Only accept if within threshold
			{
				best = error;
				hfdcan->Init.NominalPrescaler = prescaler;
				hfdcan->Init.NominalTimeSeg1 = tseg1;
				hfdcan->Init.NominalTimeSeg2 = tseg2;
				result = RAMN_OK;
			}
			tseg1++;
		}
		prescaler++;
		tqsum = (fdcanClock / prescaler) / bitrate;
	}
	return result;
}
#endif

static void FDCAN_Config(void)
{

	// Pull STBY line of CAN transceiver low (high = transceiver OFF)
	HAL_GPIO_WritePin(FDCAN1_STB_GPIO_Port, FDCAN1_STB_Pin, GPIO_PIN_RESET);

#ifdef USE_HARDWARE_CAN_FILTERS

	HAL_FDCAN_DeInit(hfdcan);
	hfdcan->Init.StdFiltersNbr = sizeof(recvStdCANIDList)/sizeof(*recvStdCANIDList);
	hfdcan->Init.ExtFiltersNbr = sizeof(recvExtCANIDList)/sizeof(*recvExtCANIDList);
	if (HAL_FDCAN_Init(hfdcan) != HAL_OK) Error_Handler();

	// Configure Filter for STANDARD CAN IDs
	FDCAN_SetStandardFilterList(recvStdCANIDList, sizeof(recvStdCANIDList)/sizeof(*recvStdCANIDList));

	// Configure Filter for EXTENDED CAN IDs
	FDCAN_SetExtendedFilterList(recvExtCANIDList, sizeof(recvExtCANIDList)/sizeof(*recvExtCANIDList));
#else
	// Configure Filter for STANDARD CAN IDs
	if (HAL_FDCAN_ConfigFilter(hfdcan, &(RAMN_FDCAN_Status.sFilterStdConfig)) != HAL_OK) Error_Handler();

	// Configure Filter for EXTENDED CAN IDs
	if (HAL_FDCAN_ConfigFilter(hfdcan, &(RAMN_FDCAN_Status.sFilterExtConfig)) != HAL_OK) Error_Handler();
#endif

	// Configure the Filters
	if (HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT , FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK) Error_Handler();

	// Start the Peripheral
	if (HAL_FDCAN_Start(hfdcan) != HAL_OK) Error_Handler();

	// Activate Message Receive Interrupts
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) Error_Handler();

	// Activate RX FIFO errors
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_MESSAGE_LOST | FDCAN_IT_RX_FIFO0_FULL , 0) != HAL_OK) Error_Handler();

	// Activate Message Transmission Complete interrupts
	// Note: we do not activate FDCAN_IT_TX_EVT_FIFO_FULL, as we expect the transmit FIFO to be full from time to time (overflowing elements stay in the stream buffer until FIFO is ready)
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_LIST_TX_FIFO_ERROR, FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2) != HAL_OK) Error_Handler();

	// Activate Notifications for TX COMPLETE event
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2) != HAL_OK) Error_Handler();

	// Activate CAN-FD Controller errors interrupt
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RAM_ACCESS_FAILURE | FDCAN_IT_ERROR_LOGGING_OVERFLOW | FDCAN_IT_RAM_WATCHDOG | FDCAN_IT_ARB_PROTOCOL_ERROR | FDCAN_IT_DATA_PROTOCOL_ERROR, 0) != HAL_OK) Error_Handler();

	// Activate CAN-FD Status errors interrupt
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_ERROR_WARNING | FDCAN_IT_BUS_OFF, 0) != HAL_OK) Error_Handler();
}


/*=====================================================================*
 *                          Public  functions                           *
 *======================================================================*/


void RAMN_FDCAN_Init(FDCAN_HandleTypeDef* handle, osThreadId_t* s, osThreadId_t* e)
{
	hfdcan = handle;
	sendTask = s;
	errTask = e;
	CAN_TX_SEMAPHORE = xSemaphoreCreateMutexStatic(&CAN_TX_SEMAPHORE_STRUCT);
}

void RAMN_FDCAN_ResetPeripheral(void)
{
	//may not be initialized when called, Ignore errors
	HAL_FDCAN_DeInit(hfdcan);
	if (HAL_FDCAN_Init(hfdcan) != HAL_OK) Error_Handler();
	RAMN_FDCAN_ResetStatistics();
	FDCAN_Config();
}

void RAMN_FDCAN_Disable(void)
{
	//Ignore errors, as it is possible for this function to be called before Init.
	HAL_FDCAN_DeInit(hfdcan);
}

RAMN_Result_t RAMN_FDCAN_UpdateBaudrate(uint32_t newSelection)
{
	RAMN_Result_t result = RAMN_OK;
#ifdef ENABLE_DYNAMIC_BITRATE
	switch(newSelection)
	{
	case 0: // 10000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_0);
		break;
	case 1: // 20000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_1);
		break;
	case 2: // 50000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_2);
		break;
	case 3: // 100000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_3);
		break;
	case 4: // 125000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_4);
		break;
	case 5: // 250000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_5);
		break;
	case 6: // 500000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_6);
		break;
	case 7: // 800000
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_7);
		break;
	case 8: // 1M
		result = FDCAN_ConfigureBitrate(FDCAN_BAUDRATE_8);
		break;

	default:
		if(newSelection >= FDCAN_BITRATE_MIN && newSelection <= FDCAN_BITRATE_MAX) result = FDCAN_ConfigureBitrate(newSelection);
		else result = RAMN_ERROR;
		break;
	}
#else
	switch(newSelection)
	{
	case '0': //10K
		hfdcan->Init.NominalPrescaler = 50;
		hfdcan->Init.NominalTimeSeg1  = 20;
		hfdcan->Init.NominalTimeSeg2  = 19;
		break;
	case '1': //20k
		hfdcan->Init.NominalPrescaler = 25;
		hfdcan->Init.NominalTimeSeg1  = 20;
		hfdcan->Init.NominalTimeSeg2  = 19;
		break;
	case '2': //50k
		hfdcan->Init.NominalPrescaler = 10;
		hfdcan->Init.NominalTimeSeg1 = 20;
		hfdcan->Init.NominalTimeSeg2 = 19;
		break;
	case '3': //100k
		hfdcan->Init.NominalPrescaler = 5;
		hfdcan->Init.NominalTimeSeg1 = 20;
		hfdcan->Init.NominalTimeSeg2 = 19;
		break;
	case '4': //125k
		hfdcan->Init.NominalPrescaler = 4;
		hfdcan->Init.NominalTimeSeg1 = 20;
		hfdcan->Init.NominalTimeSeg2 = 19;
		break;
	case '5': //250k
		hfdcan->Init.NominalPrescaler = 2;
		hfdcan->Init.NominalTimeSeg1 = 20;
		hfdcan->Init.NominalTimeSeg2 = 19;
		break;
	case '6': // 500k
		hfdcan->Init.NominalPrescaler = 1;
		hfdcan->Init.NominalTimeSeg1 = 63;
		hfdcan->Init.NominalTimeSeg2 = 16;
		break;
	case '7': // 800k
		hfdcan->Init.NominalPrescaler = 1;
		hfdcan->Init.NominalTimeSeg1 = 12;
		hfdcan->Init.NominalTimeSeg2 = 12;
		break;
	case '8': // 1M
		hfdcan->Init.NominalPrescaler = 1;
		hfdcan->Init.NominalTimeSeg1 = 20;
		hfdcan->Init.NominalTimeSeg2 = 19;
		break;
	default: // 500k
		hfdcan->Init.NominalPrescaler = 1;
		hfdcan->Init.NominalTimeSeg1 = 63;
		hfdcan->Init.NominalTimeSeg2 = 16;
		result = RAMN_ERROR;
		break;
	}
#endif
	return result;
}

void RAMN_FDCAN_ResetStatistics(void)
{
	RAMN_FDCAN_Status.slcanFlags = 0U;
	RAMN_FDCAN_Status.CANTXRequestCnt = 0U;
	RAMN_FDCAN_Status.CANTXSentCnt = 0U;
	RAMN_FDCAN_Status.CANRXCnt = 0U;
	RAMN_FDCAN_Status.CANRxOverrunCnt = 0U;
	RAMN_FDCAN_Status.prevCANError = HAL_FDCAN_ERROR_NONE;
	RAMN_FDCAN_Status.CANErrCnt = 0U;
	RAMN_FDCAN_Status.busOff = False;
}

RAMN_Bool_t RAMN_FDCAN_IsTXBufferSpaceAvailable(uint8_t payloadSize)
{
	return (xStreamBufferSpacesAvailable(CANTxDataStreamBufferHandle) >= sizeof(FDCAN_TxHeaderTypeDef) + payloadSize);
}


RAMN_Result_t RAMN_FDCAN_SendMessage(const FDCAN_TxHeaderTypeDef* header, const uint8_t* data)
{
	size_t xBytesSent;
	size_t messageSize;
	uint8_t dlc;
	RAMN_Result_t result = RAMN_OK;

	dlc    = DLCtoUINT8(header->DataLength);
	if (header->TxFrameType == FDCAN_REMOTE_FRAME) dlc = 0U;
	messageSize = sizeof(FDCAN_TxHeaderTypeDef) + dlc;

	while (xSemaphoreTake(CAN_TX_SEMAPHORE, portMAX_DELAY ) != pdTRUE);
	if (xStreamBufferSpacesAvailable(CANTxDataStreamBufferHandle) < messageSize)
	{
		RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_TX_QUEUE_FULL;
		result = RAMN_TRY_LATER;
	}
	else
	{
		xBytesSent = xStreamBufferSend(CANTxDataStreamBufferHandle, (void *) header, sizeof(FDCAN_TxHeaderTypeDef),portMAX_DELAY);
		if (dlc > 0) xBytesSent += xStreamBufferSend(CANTxDataStreamBufferHandle, (void *) data, dlc, portMAX_DELAY);

		if( xBytesSent != messageSize )
		{
			// Data overrun happened
			RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_DATA_OVERRUN;
			result = RAMN_ERROR;
		}
	}
	xSemaphoreGive(CAN_TX_SEMAPHORE);
	xTaskNotifyGive(*sendTask); //TODO: notify needed ?
	return result;
}

#if defined(TARGET_ECUA)
void RAMN_FDCAN_SetupForSTBootloader(void)
{
	//Assumes a 40MHz clock
	hfdcan->Init.ClockDivider = FDCAN_CLOCK_DIV2;
	hfdcan->Init.FrameFormat = FDCAN_FRAME_FD_BRS;
	hfdcan->Init.Mode = FDCAN_MODE_NORMAL;
	hfdcan->Init.AutoRetransmission = DISABLE;
	hfdcan->Init.TransmitPause = DISABLE;
	hfdcan->Init.ProtocolException = ENABLE;
	hfdcan->Init.NominalPrescaler = 0x1;
	hfdcan->Init.NominalSyncJumpWidth = 0x10;
	hfdcan->Init.NominalTimeSeg1 = 0x3F;
	hfdcan->Init.NominalTimeSeg2 = 0x10;
	hfdcan->Init.DataPrescaler = 0x1;
	hfdcan->Init.DataSyncJumpWidth = 0x4;
	hfdcan->Init.DataTimeSeg1 = 0xF;
	hfdcan->Init.DataTimeSeg2 = 0x4;
}
#endif

/* Interrupt Handler function  -----------------------------------------------*/

//Callback function for FD CAN Receive
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	size_t xBytesSent;
	size_t messageSize;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint8_t dlc;

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, CANRxData) != HAL_OK) FDCAN_Error_Handler();
		RAMN_FDCAN_Status.CANRXCnt++;
		dlc = DLCtoUINT8(CANRxHeader.DataLength);
		messageSize = dlc + sizeof(CANRxHeader);

		if (xStreamBufferSpacesAvailable(CANRxDataStreamBufferHandle) < (messageSize))
		{
			// No place in the buffer, forced to drop the frame
			RAMN_FDCAN_Status.CANRxOverrunCnt++;
			vTaskNotifyGiveFromISR(*errTask,&xHigherPriorityTaskWoken);
		}
		else
		{
			xBytesSent = xStreamBufferSendFromISR(CANRxDataStreamBufferHandle, (void*)&CANRxHeader, sizeof(CANRxHeader), &xHigherPriorityTaskWoken );
			if (dlc > 0) xBytesSent += xStreamBufferSendFromISR(CANRxDataStreamBufferHandle, (void*)CANRxData, dlc, &xHigherPriorityTaskWoken );
			if( xBytesSent != (messageSize)) FDCAN_Error_Handler();
		}
	}

	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL) != RESET)
	{
		//TODO: Change priority of receiving thread for faster emptying (?)
		//FIFO is full, need to accelerate processing of incoming messages
		RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_RX_QUEUE_FULL;
		__HAL_FDCAN_CLEAR_FLAG(hfdcan,FDCAN_IT_RX_FIFO0_FULL); //TODO: Needed?
	}
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != RESET)
	{
		//We lost a message because we were too slow to process it
		RAMN_FDCAN_Status.CANRxOverrunCnt++;
		RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_DATA_OVERRUN;
		__HAL_FDCAN_CLEAR_FLAG(hfdcan,FDCAN_IT_RX_FIFO0_MESSAGE_LOST); //TODO: Needed ?
		vTaskNotifyGiveFromISR(*errTask,&xHigherPriorityTaskWoken);
	}
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	RAMN_FDCAN_Status.CANTXSentCnt++;
	vTaskNotifyGiveFromISR(*sendTask,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	RAMN_FDCAN_Status.CANErrCnt++;
	vTaskNotifyGiveFromISR(*errTask,&xHigherPriorityTaskWoken);
	//hfdcan->ErrorCode &= ~(FDCAN_IR_ELO | FDCAN_IR_WDI | FDCAN_IR_PEA | FDCAN_IR_PED | FDCAN_IR_ARA);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
	HAL_FDCAN_GetProtocolStatus(hfdcan,&protocolStatus);
	if (ErrorStatusITs & FDCAN_IT_ERROR_PASSIVE)
	{
		//Error_Passive status changed
		if(protocolStatus.ErrorPassive == 0U) RAMN_FDCAN_Status.ErrorStateIndicator = FDCAN_ESI_ACTIVE; // 0 means Error active, means no problem.
		else RAMN_FDCAN_Status.ErrorStateIndicator = FDCAN_ESI_PASSIVE; //1 means Error passive, there is a problem
	}
	if (ErrorStatusITs & FDCAN_IT_ERROR_WARNING)
	{
		//Error_Warning status changed
		if(protocolStatus.Warning == 1U) RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_ERROR_WARNING;
		else RAMN_FDCAN_Status.slcanFlags &= ~SLCAN_FLAG_ERROR_WARNING;
	}
	if (ErrorStatusITs & FDCAN_IE_BOE)
	{
		//Bus off status changed
		RAMN_FDCAN_Status.busOff = True;
	}
}

