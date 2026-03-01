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
		frameData->timestamp_us = (xTaskGetTickCount() * (1000000 /*us per sec*/ / configTICK_RATE_HZ) );

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
		frameData->timestamp_us = 0;  // timestamps are ignored on send
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

RAMN_Result_t RAMN_GSUSB_SendErrorFrame(const FDCAN_ProtocolStatusTypeDef *protocolStatus, const FDCAN_ErrorCountersTypeDef *errorCount, uint32_t err)
{
	BaseType_t            qret;
	struct gs_host_frame *frameData;

	// Get frame data pointer from pool queue (non-blocking)
	qret = xQueueReceive(RAMN_GSUSB_PoolQueueHandle, &frameData, 0);
	if (qret != pdPASS) return RAMN_ERROR;

	frameData->echo_id = 0xFFFFFFFF;
	frameData->can_id = CAN_ERR_FLAG;
	frameData->can_dlc = CAN_ERR_DLC;
	frameData->channel = 0;
	frameData->flags = 0;
	frameData->reserved = 0;
	frameData->timestamp_us = (xTaskGetTickCount() * (1000000U / configTICK_RATE_HZ));
	RAMN_memset(frameData->data, 0, CAN_ERR_DLC);

	// Bus off
	if (protocolStatus->BusOff != 0U)
	{
		frameData->can_id |= CAN_ERR_BUSOFF;
	}

	// Controller error state
	if ((protocolStatus->ErrorPassive != 0U) || (protocolStatus->Warning != 0U))
	{
		frameData->can_id |= CAN_ERR_CRTL;
		if (protocolStatus->Warning != 0U)
		{
			if (errorCount->TxErrorCnt >= 96U) frameData->data[1] |= CAN_ERR_CRTL_TX_WARNING;
			if (errorCount->RxErrorCnt >= 96U) frameData->data[1] |= CAN_ERR_CRTL_RX_WARNING;
		}
		if (protocolStatus->ErrorPassive != 0U)
		{
			if (errorCount->TxErrorCnt >= 128U) frameData->data[1] |= CAN_ERR_CRTL_TX_PASSIVE;
			if (errorCount->RxErrorPassive != 0U) frameData->data[1] |= CAN_ERR_CRTL_RX_PASSIVE;
		}
	}

	// Protocol errors
	if ((err & (HAL_FDCAN_ERROR_PROTOCOL_ARBT | HAL_FDCAN_ERROR_PROTOCOL_DATA)) != 0U)
	{
		uint32_t lec = protocolStatus->LastErrorCode;
		frameData->can_id |= CAN_ERR_BUSERROR;
		if ((lec != FDCAN_PROTOCOL_ERROR_NONE) && (lec != FDCAN_PROTOCOL_ERROR_NO_CHANGE))
		{
			frameData->can_id |= CAN_ERR_PROT;
			switch (lec)
			{
			case FDCAN_PROTOCOL_ERROR_STUFF: frameData->data[2] = CAN_ERR_PROT_STUFF; break;
			case FDCAN_PROTOCOL_ERROR_FORM:  frameData->data[2] = CAN_ERR_PROT_FORM;  break;
			case FDCAN_PROTOCOL_ERROR_ACK:   frameData->can_id |= CAN_ERR_ACK;        break;
			case FDCAN_PROTOCOL_ERROR_BIT1:  frameData->data[2] = CAN_ERR_PROT_BIT1;  break;
			case FDCAN_PROTOCOL_ERROR_BIT0:  frameData->data[2] = CAN_ERR_PROT_BIT0;  break;
			case FDCAN_PROTOCOL_ERROR_CRC:   frameData->data[3] = CAN_ERR_PROT_LOC_CRC_SEQ; break;
			default:                         frameData->data[2] = CAN_ERR_PROT_UNSPEC; break;
			}
		}
	}

	// Only send if there is actual error info
	if (frameData->can_id == CAN_ERR_FLAG)
	{
		xQueueSendToBack(RAMN_GSUSB_PoolQueueHandle, &frameData, portMAX_DELAY);
		return RAMN_OK;
	}

	qret = xQueueSendToBack(RAMN_GSUSB_SendQueueHandle, &frameData, 0);
	if (qret != pdPASS)
	{
		xQueueSendToBack(RAMN_GSUSB_PoolQueueHandle, &frameData, portMAX_DELAY);
		return RAMN_ERROR;
	}

	return RAMN_OK;
}

#endif
