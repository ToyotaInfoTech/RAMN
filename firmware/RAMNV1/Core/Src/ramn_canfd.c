/*
 * ramn_canfd.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 TOYOTA MOTOR CORPORATION.
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


//Handle for FDCAN peripheral
static FDCAN_HandleTypeDef* hfdcan;

//Task to notify when the task in charge of transmitting CAN messages should check the buffer
//Note that it may be notified even when no messages is ready - For example when a TX error has occurred.
static osThreadId_t* sendTask;

//Task to notify when an error was detected.
//It is up to that task to clear error registers, and therefore it should have the highest priority
static osThreadId_t* errTask;

//Buffer to save RX DATA from ISR
static uint8_t CANRxData[64];

//Buffer to save RX Header Data from ISR
static FDCAN_RxHeaderTypeDef CANRxHeader;

//Buffer to save protocol status error from CAN Error Status ISR
static FDCAN_ProtocolStatusTypeDef protocolStatus;

//Semaphore to prevent Stream buffer access from different threads
static SemaphoreHandle_t CAN_TX_SEMAPHORE;
static StaticSemaphore_t CAN_TX_SEMAPHORE_STRUCT;

/* Private prototypes -----------------------------------------------*/

static void RAMN_FDCAN_Error_Handler()
{
//TODO: hang or resume
}

static void FDCAN_Config(void)
{

	// Pull STBY line of CAN transceiver low (high = transceiver OFF)
	//HAL_GPIO_WritePin(FDCAN1_STB_GPIO_Port, FDCAN1_STB_Pin, GPIO_PIN_RESET);

	//Configure Filter for STANDARD CAN IDs
	if (HAL_FDCAN_ConfigFilter(hfdcan, &(RAMN_FDCAN_Status.sFilterStdConfig)) != HAL_OK) Error_Handler();

	//Configure Filter for EXTENDED CAN IDs
	if (HAL_FDCAN_ConfigFilter(hfdcan, &(RAMN_FDCAN_Status.sFilterExtConfig)) != HAL_OK) Error_Handler();

	//Configure the Filters
	if (HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT , FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK) Error_Handler();

	//Start the Peripheral
	if (HAL_FDCAN_Start(hfdcan) != HAL_OK) Error_Handler();

	//Activate Message Receive Interrupts
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) Error_Handler();

	//Activate RX FIFO errors
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_MESSAGE_LOST | FDCAN_IT_RX_FIFO0_FULL , 0) != HAL_OK) Error_Handler();

	//Activate Message Transmission Complete interrupts
	//Note: we do not activate FDCAN_IT_TX_EVT_FIFO_FULL, as we expect the transmit FIFO to be full from time to time (overflowing elements stay in the stream buffer until FIFO is ready)
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_LIST_TX_FIFO_ERROR, FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2) != HAL_OK) Error_Handler();

	//Activate Notifications for TX COMPLETE event
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_TX_COMPLETE, FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2) != HAL_OK) Error_Handler();

	// Activate CAN-FD Controller errors interrupt
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RAM_ACCESS_FAILURE | FDCAN_IT_ERROR_LOGGING_OVERFLOW | FDCAN_IT_RAM_WATCHDOG | FDCAN_IT_ARB_PROTOCOL_ERROR | FDCAN_IT_DATA_PROTOCOL_ERROR, 0) != HAL_OK) Error_Handler();

	//Activate CAN-FD Status errors interrupt
	if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_ERROR_WARNING | FDCAN_IT_BUS_OFF, 0) != HAL_OK) Error_Handler();
}

/* Exported  -----------------------------------------------*/

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
				.FilterID1 = 0xFFFFFFFF,
				.FilterID2 = 0xFFFFFFFF,
		}
};

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

void RAMN_FDCAN_UpdateBaudrate(uint8_t newSelection)
{
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
		break;
	}
}

void RAMN_FDCAN_ResetStatistics(void)
{
	RAMN_FDCAN_Status.slcan_flags = 0U;
	RAMN_FDCAN_Status.CANTXRequestCnt = 0U;
	RAMN_FDCAN_Status.CANTXSentCnt = 0U;
	RAMN_FDCAN_Status.CANRXCnt = 0U;
	RAMN_FDCAN_Status.CANRxOverrunCnt = 0U;
	RAMN_FDCAN_Status.prevCANError = HAL_FDCAN_ERROR_NONE;
	RAMN_FDCAN_Status.CANErrCnt = 0U;
}

RAMN_Bool_t RAMN_FDCAN_IsTXBufferSpaceAvailable(uint8_t payloadSize)
{
	return (xStreamBufferSpacesAvailable(CANTxDataStreamBufferHandle) >= sizeof(FDCAN_TxHeaderTypeDef) + payloadSize);
}

//TODO: Make this Function thread safe ?
RAMN_Result_t RAMN_FDCAN_SendMessage(const FDCAN_TxHeaderTypeDef* header, const uint8_t* data)
{
	size_t xBytesSent;
	size_t messageSize;
	uint8_t dlc;
	RAMN_Result_t result = RAMN_OK;

	dlc    = DLCtoUINT8(header->DataLength);
	messageSize = sizeof(FDCAN_TxHeaderTypeDef) + dlc;

	if (header->TxFrameType == FDCAN_REMOTE_FRAME) dlc = 0U;
	while (xSemaphoreTake(CAN_TX_SEMAPHORE, portMAX_DELAY ) != pdTRUE);
	if (xStreamBufferSpacesAvailable(CANTxDataStreamBufferHandle) < messageSize)
	{
		RAMN_FDCAN_Status.slcan_flags |= SLCAN_FLAG_TX_QUEUE_FULL;
		result = RAMN_ERROR;
	}
	else
	{
		xBytesSent = xStreamBufferSend(CANTxDataStreamBufferHandle, (void *) header, sizeof(FDCAN_TxHeaderTypeDef),portMAX_DELAY);
		if (dlc > 0) xBytesSent += xStreamBufferSend(CANTxDataStreamBufferHandle, (void *) data, dlc, portMAX_DELAY);

		if( xBytesSent != messageSize )
		{
			//Data overrun happened
			RAMN_FDCAN_Status.slcan_flags |= SLCAN_FLAG_DATA_OVERRUN;
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
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, CANRxData) != HAL_OK) RAMN_FDCAN_Error_Handler();
		RAMN_FDCAN_Status.CANRXCnt++;
		dlc = DLCtoUINT8(CANRxHeader.DataLength);
		messageSize = dlc + sizeof(CANRxHeader);

		if (xStreamBufferSpacesAvailable(CANRxDataStreamBufferHandle) < (messageSize))
		{
			// No place in the buffer, forced to drop the frame
			RAMN_FDCAN_Status.CANRxOverrunCnt++;
			//Close slcan by default
			vTaskNotifyGiveFromISR(*errTask,&xHigherPriorityTaskWoken);
		}
		else
		{
			xBytesSent = xStreamBufferSendFromISR(CANRxDataStreamBufferHandle, (void*)&CANRxHeader, sizeof(CANRxHeader), &xHigherPriorityTaskWoken );
			if (dlc > 0) xBytesSent += xStreamBufferSendFromISR(CANRxDataStreamBufferHandle, (void*)CANRxData, dlc, &xHigherPriorityTaskWoken );
			if( xBytesSent != (messageSize)) RAMN_FDCAN_Error_Handler();
		}
	}

	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL) != RESET)
	{
		//TODO: Change priority of receiving thread for faster emptying (?)
		//FIFO is full, need to accelerate processing of incoming messages
		RAMN_FDCAN_Status.slcan_flags |= SLCAN_FLAG_RX_QUEUE_FULL;
		__HAL_FDCAN_CLEAR_FLAG(hfdcan,FDCAN_IT_RX_FIFO0_FULL); //TODO: Needed ?
	}
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != RESET)
	{
		//We lost a message because we were too slow to process it
		RAMN_FDCAN_Status.CANRxOverrunCnt++;
		RAMN_FDCAN_Status.slcan_flags |= SLCAN_FLAG_DATA_OVERRUN;
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
		if(protocolStatus.Warning == 1U) RAMN_FDCAN_Status.slcan_flags |= SLCAN_FLAG_ERROR_WARNING;
		else RAMN_FDCAN_Status.slcan_flags &= ~SLCAN_FLAG_ERROR_WARNING;
	}
	if (ErrorStatusITs & FDCAN_IE_BOE)
	{
		//Bus off status changed
		RAMN_FDCAN_Status.busOff = True;
	}
}

