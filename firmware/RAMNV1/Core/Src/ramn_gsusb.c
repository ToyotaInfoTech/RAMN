/*
 * ramn_cdc.c
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

#include "ramn_cdc.h"

#ifdef ENABLE_GSUSB

RAMN_Result_t RAMN_GSUSB_ProcessRX(FDCAN_RxHeaderTypeDef *canRxHeader, uint8_t *canRxData)
{
	RAMN_Result_t         ret;
	BaseType_t            qret;
	struct gs_host_frame *frameData;

	ret = RAMN_OK;

	// Get frame data pointer from pool queue
	qret = xQueueReceive(RAMN_GSUSB_PoolQueueHandle, &frameData, portMAX_DELAY);
	if (qret == pdPASS)
	{
		// Does not support CAN FD yet
		if(canRxHeader->FDFormat == FDCAN_FD_CAN)
		{
#ifdef HANG_ON_ERRORS
			Error_Handler();
#endif
			return RAMN_ERROR;
		}

		frameData->can_id = canRxHeader->Identifier;
		if (canRxHeader->IdType != FDCAN_STANDARD_ID) frameData->can_id |= CAN_EFF_FLAG;
		if (canRxHeader->RxFrameType != FDCAN_DATA_FRAME) frameData->can_id |= CAN_RTR_FLAG;

		frameData->echo_id = 0xFFFFFFFF;
		frameData->channel = 0;
		frameData->flags = 0;
		frameData->can_dlc = canRxHeader->DataLength;
		frameData->timestamp_us = 0;

		if (!(frameData->can_id & CAN_RTR_FLAG)) RAMN_memcpy(frameData->data, canRxData, frameData->can_dlc);

		// Send to task
		qret = xQueueSendToBack(RAMN_GSUSB_SendQueueHandle, &frameData, CAN_QUEUE_TIMEOUT);
		if (qret != pdPASS)
		{
			ret = RAMN_ERROR;
		}
	}
	else ret = RAMN_ERROR;

	return ret;
}

RAMN_Result_t RAMN_GSUSB_ProcessTX(FDCAN_TxHeaderTypeDef *canTxHeader, uint8_t *canRxData)
{
	RAMN_Result_t         ret;
	BaseType_t            qret;
	struct gs_host_frame *frameData;

	ret = RAMN_OK;

	// Get frame data pointer from pool queue
	qret = xQueueReceive(RAMN_GSUSB_PoolQueueHandle, &frameData, CAN_QUEUE_TIMEOUT);
	if (qret == pdPASS)
	{
		// Does not support CAN FD yet
		if(canTxHeader->FDFormat == FDCAN_FD_CAN)
		{
#ifdef HANG_ON_ERRORS
			Error_Handler();
#endif
			return RAMN_ERROR;
		}

		frameData->can_id = canTxHeader->Identifier;
		frameData->echo_id = 0xFFFFFFFF;
		frameData->channel = 0;
		frameData->can_dlc = canTxHeader->DataLength;
		frameData->timestamp_us = 0;
		RAMN_memcpy(frameData->data, canRxData, frameData->can_dlc);

		// Send to task
		qret = xQueueSendToBack(RAMN_GSUSB_SendQueueHandle, &frameData, CAN_QUEUE_TIMEOUT);
		if (qret != pdPASS)
		{
			ret = RAMN_ERROR;
		}
	}
	else ret = RAMN_ERROR;

	return ret;
}

#endif
