/*
 * ramn_isotp.c
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

#include "ramn_isotp.h"

#if defined(ENABLE_ISOTP)

// Private Functions ----------------------------------

#define ISOTP_RX 0x1
#define ISOTP_TX 0x2

// Padding byte used to fill CAN-FD frames up to a valid CAN_DL.
#ifdef ISOTP_ANSWER_PADDING_BYTE
#define ISOTP_FD_PAD_BYTE ISOTP_ANSWER_PADDING_BYTE
#else
#define ISOTP_FD_PAD_BYTE 0x00U
#endif

// Rounds a raw payload length (0..64) up to the next valid CAN_DL and returns the FDCAN DLC enum.
// For len <= 8 the enum equals the byte count, so classic-CAN framing is unaffected.
static uint32_t isotp_len_to_dlc(uint8_t len)
{
	if (len <= 8U)  return (uint32_t)len;
	if (len <= 12U) return 9U;
	if (len <= 16U) return 10U;
	if (len <= 20U) return 11U;
	if (len <= 24U) return 12U;
	if (len <= 32U) return 13U;
	if (len <= 48U) return 14U;
	return 15U; // up to 64 bytes
}

static void report_Error(RAMN_ISOTPHandler_t* handler, uint8_t dir, RAMN_ISOTP_N_RESULT errCode)
{
	if (dir & ISOTP_RX)
	{
		handler->lastRxResult = errCode;   // Record error for upper layer inspection
		handler->rxStatus = ISOTP_RX_IDLE; // Get ready for what is next
	}
	if (dir & ISOTP_TX)
	{
		handler->lastTxResult = errCode;   // Record error for upper layer inspection
		handler->txStatus = ISOTP_TX_FINISHED; // Needs to be put back to "IDLE" manually
	}
}

static void ISOTP_ReceiveSF(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data)
{
	uint16_t size;
	uint8_t  offset;

	if(handler->rxStatus != ISOTP_RX_IDLE)
	{
		// Should report error, but proceed with receiving
		report_Error(handler, ISOTP_RX, N_UNEXP_PDU);
	}

	if (dlc <= 8U)
	{
		// CLASSICAL CAN / CAN_DL <= 8: SF_DL in the low nibble, payload from byte #1
		size = data[0]&0xF;
		offset = 1U;
	}
	else
	{
		// CAN_DL > 8: must use the escape sequence (low nibble 0), SF_DL in byte #1, payload from byte #2
		if ((data[0]&0xF) != 0U) return; // Standard says to just ignore (9.6.2.2)
		size = data[1];
		offset = 2U;
	}

	// Standard says to not report, just ignore invalid lengths/DLCs
	if ((size > 0U) && (size <= (dlc - offset)) && (size <= ISOTP_RXBUFFER_SIZE))
	{
		// Valid data received, copy to buffer and signal buffer ready
		RAMN_memcpy(handler->rxData,&data[offset],size);
		handler->rxCount = size;
		handler->rxStatus = ISOTP_RX_FINISHED;
	}
}

static void ISOTP_ReceiveFF(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data)
{
	if(handler->rxStatus != ISOTP_RX_IDLE)
	{
		// Should report error, but proceed with receiving
		report_Error(handler, ISOTP_RX, N_UNEXP_PDU);
	}

	if(dlc >= 8) // Standard says to just ignore invalid DLC
	{
		uint32_t ffdl;
		uint8_t  offset;

		if (((data[0]&0xF) == 0U) && (data[1] == 0U))
		{
			// Escape sequence: 32-bit FF_DL in bytes #2..#5, payload from byte #6
			ffdl = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];
			offset = 6U;
		}
		else
		{
			// 12-bit FF_DL, payload from byte #2
			ffdl = (uint32_t)((data[0]&0xF) << 8) | data[1];
			offset = 2U;
		}

		// Standard says to just ignore invalid payload sizes
		if (ffdl > 7)
		{
			// Prepare to receive data
			if(ffdl <= ISOTP_RXBUFFER_SIZE)
			{
				uint8_t firstChunk = dlc - offset; // RX_DL is the FF's CAN_DL (9.5.4)
				handler->rxExpectedSize = (uint16_t)ffdl;
				handler->rxDL         = dlc;
				RAMN_memcpy(handler->rxData, &data[offset], firstChunk);
				handler->rxCount      = firstChunk;
				handler->rxStatus     = ISOTP_RX_TRANSFERRING;
				handler->selfFCFlag   = 0U;
				handler->rxMustSendCF = 1U;
				handler->rxFrameCount = 1U;
			}
			else
			{
				handler->selfFCFlag   = 2U;
				handler->rxMustSendCF = 1U;
				report_Error(handler, ISOTP_RX, N_WFT_OVRN);
			}
		}
	}
}

static void ISOTP_ReceiveCF(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data)
{
	uint8_t  size;
	uint16_t waitingBytes;
	uint8_t  frameIndex;

	if(handler->rxStatus != ISOTP_RX_TRANSFERRING) return;

	frameIndex = data[0] & 0xF;
	if (frameIndex != ((handler->rxFrameCount) % 0x10))
	{
		report_Error(handler, ISOTP_RX, N_WRONG_SN);
		return;
	}

	uint16_t maxCFData = (handler->rxDL > 0U) ? (uint16_t)(handler->rxDL - 1U) : 7U; // RX_DL - 1
	waitingBytes = handler->rxExpectedSize - handler->rxCount;
	if ((waitingBytes > maxCFData) && (dlc != handler->rxDL))
	{
		// Should ignore (a non-final CF must use the full RX_DL)
	}
	else if ((waitingBytes <= maxCFData) && dlc < (waitingBytes +1))
	{
		// Should ignore (final CF too short)
	}
	else
	{
		handler->rxFrameCount++;
		size = (waitingBytes <= maxCFData) ? (uint8_t)waitingBytes : (uint8_t)(dlc - 1);
		RAMN_memcpy(&(handler->rxData[handler->rxCount]),&data[1],size);
		handler->rxCount += size;
		if (handler->rxCount == handler->rxExpectedSize)
		{
			handler->rxStatus = ISOTP_RX_FINISHED;
		}
		else
		{
			if (handler->selfBlockSize != 0)
			{
				if (((handler->rxFrameCount-1) % handler->selfBlockSize) == 0U) // Have we reached max size block ?
				{
					handler->rxMustSendCF = 1U; // Request a "CF" frame to be sent, to ask for the rest
				}
			}
		}
	}
}

static void ISOTP_ReceiveFC(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data, uint32_t tick)
{
	// A FC frame is only meaningful while a TX is ongoing. Ignore otherwise.
	RAMN_Bool_t txOngoing = (RAMN_Bool_t)((handler->txStatus == ISOTP_TX_WAITING_FLAG) || (handler->txStatus == ISOTP_TX_TRANSFERRING));

	handler->targetFCFlag = data[0]&0xF;

	switch(handler->targetFCFlag)
	{
	case 0U:   // "Continue To Send"
		// BS and STmin are only relevant for ContinueToSend (ISO 15765-2 9.6.5.1)
		if (dlc > 1) handler->targetBlockSize = data[1];
		else handler->targetBlockSize = 0U;
		if (dlc > 2) handler->targetST = data[2];
		else handler->targetST = 0U;
		// ISO_TP says we should just ignore if not expecting
		if(handler->txStatus == ISOTP_TX_WAITING_FLAG)
		{
			handler->txStatus = ISOTP_TX_TRANSFERRING;
		}
		break;

	case 1U:  // "WAIT"
		// BS and STmin shall be ignored for WAIT (ISO 15765-2 9.6.5.1)
		// Ignore if not expecting
		if(txOngoing)
		{
			handler->txStatus = ISOTP_TX_WAITING_FLAG;
			handler->txLastTimestamp = tick; // WAIT restarts the N_Bs timer
		}
		break;

	case 2U:  // "Abort"
		// Only meaningful while transmitting; a stray OVFLW must not disturb an idle TX
		if(txOngoing) report_Error(handler, ISOTP_TX, N_BUFFER_OVFLW);
		break;

	default:  // Reserved/invalid FlowStatus
		if(txOngoing) report_Error(handler, ISOTP_TX, N_INVALID_FS);
		break;
	}
}


// Exported Functions ----------------------------------

void RAMN_ISOTP_Init(RAMN_ISOTPHandler_t* handler, FDCAN_TxHeaderTypeDef* FCMsgHeader)
{
	handler->rxStatus = 0U;
	handler->rxCount = 0U;
	handler->rxExpectedSize = 0U;
	handler->rxMustSendCF = 0U;
	handler->rxLastTimestamp = 0U;

	handler->txStatus = 0U;
	handler->txIndex = 0U;
	handler->txSize = 0U;
	handler->txLastTimestamp = 0U;

	handler->targetFCFlag = 0U;
	handler->targetBlockSize = 0U;
	handler->targetST = 0U;

	handler->lastRxResult = N_OK;
	handler->lastTxResult = N_OK;

	handler->selfFCFlag = 0U;
	handler->selfBlockSize = ISOTP_CONSECUTIVE_BLOCK_SIZE;
	handler->selfST = ISOTP_CONSECUTIVE_ST;

	handler->rxFrameCount = 0U;
	handler->txFrameCount = 0U;
	handler->rxDL = 0U;
	handler->rxWasFD = 0U;
	handler->txIsFD = 0U;

	handler->pFC_CANHeader = FCMsgHeader;

	for(uint32_t i = 0; i < ISOTP_RXBUFFER_SIZE; i++) handler->rxData[i] = 0U;
	for(uint32_t i = 0; i < ISOTP_TXBUFFER_SIZE; i++) handler->txData[i] = 0U;

}

void RAMN_ISOTP_ProcessRxMsg(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data, RAMN_Bool_t isCanFD, const uint32_t tick)
{
	handler->rxLastTimestamp = tick; // Consider any message good to update the "alive" timer

#ifdef ISOTP_REQUIRE_PADDING
	// If padding required, only process messages with DLC of 8
	if (dlc != 8U) return;
#endif

	if (dlc > 0)
	{
		switch((data[0] & 0xF0) >> 4) // Header: first 4 bits of message
		{
		case 0x0U: // SINGLE FRAME (SF)
			// Remember the request format so the answer can mirror it. Only request frames (SF/FF)
			handler->rxWasFD = (isCanFD != False) ? 1U : 0U;
			ISOTP_ReceiveSF(handler, dlc, data);
			break;

		case 0x1U: // FIRST FRAME (FF)
			handler->rxWasFD = (isCanFD != False) ? 1U : 0U;
			ISOTP_ReceiveFF(handler, dlc, data);
			break;

		case 0x2U: // CONSECUTIVE FRAME (CF)
			// ISO-TP standard says we should just ignore if CF not expected
			ISOTP_ReceiveCF(handler, dlc, data);
			break;

		case 0x3U: // FLOW CONTROL FRAME (FC)
			ISOTP_ReceiveFC(handler, dlc, data, tick);
			break;

		default:  // INVALID HEADER
			// Standard says we should just ignore
			break;
		}
	}

	if (handler->rxMustSendCF == True)
	{
		uint8_t dlc;
		uint8_t data[64U]; // large enough to optionally pad a CAN-FD FlowControl up to ISOTP_TX_DL
		RAMN_ISOTP_GetFCFrame(handler,&dlc,data);
#ifdef ISOTP_FD_FULL_PADDING
		// Optionally pad a CAN-FD FlowControl frame up to the configured ISOTP_TX_DL.
		if ((handler->rxWasFD != 0U) && (dlc < (uint8_t)ISOTP_TX_DL))
		{
			for (uint8_t i = dlc; i < (uint8_t)ISOTP_TX_DL; i++) data[i] = ISOTP_FD_PAD_BYTE;
			dlc = (uint8_t)ISOTP_TX_DL;
		}
#endif
		// isotp_len_to_dlc maps any valid CAN_DL to its DLC (UINT8toDLC is identity, only valid for <=8)
		handler->pFC_CANHeader->DataLength = isotp_len_to_dlc(dlc);
		// Mirror the request's frame format for the FlowControl frame
		handler->pFC_CANHeader->FDFormat = handler->rxWasFD ? FDCAN_FD_CAN : FDCAN_CLASSIC_CAN;
		// Ignore potential error. If we miss the answer window, it is up to the diag tool to reduce speed.
		RAMN_FDCAN_SendMessage(handler->pFC_CANHeader,data);
	}
}

RAMN_Bool_t RAMN_ISOTP_GetFCFrame(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data)
{
	RAMN_Bool_t wroteValidMessage = False;

	if(handler->rxMustSendCF != False)
	{
		data[0U] = 0x30 | handler->selfFCFlag;
		data[1U] = handler->selfBlockSize;
		data[2U] = handler->selfST;

#ifdef ISOTP_ANSWER_PADDING_BYTE
		*dlc = 8U;
		for (uint8_t i = 3U; i < 8U; i++)
		{
			data[i] = ISOTP_ANSWER_PADDING_BYTE;
		}
#else
		*dlc = 3U;
#endif
		wroteValidMessage = True;
		handler->rxMustSendCF = False;
	}
	return wroteValidMessage;
}

RAMN_Bool_t RAMN_ISOTP_GetNextTxMsg(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data, uint32_t tick)
{
	RAMN_Bool_t wroteValidMessage = False;

	if( handler->txStatus == ISOTP_TX_TRANSFERRING)
	{
		uint8_t minDelayMs = handler->targetST;
		// 0x00-0x7F are ms, 0xF1-0xF9 are 100-900us.
		// All other (reserved) values must be treated as the longest STmin (0x7F = 127ms).
		if (minDelayMs > 0x7F)
		{
			if ((minDelayMs >= 0xF1) && (minDelayMs <= 0xF9)) minDelayMs = 1U; // Consider "microseconds" values as 1ms
			else minDelayMs = 0x7FU;
		}
		if ((tick - handler->txLastTimestamp) >= (uint32_t) minDelayMs)
		{
			// Effective CAN_DL: mirror the request's format. A classic request must be answered with
			// classic (<=8 byte) frames regardless of the configured ISOTP_TX_DL.
			uint8_t txDL = handler->txIsFD ? (uint8_t)ISOTP_TX_DL : 8U;
			if(handler->txIndex == 0)
			{
				// Single frame
				if(handler->txSize <= 7U)
				{
					// Fits in a CLASSICAL SingleFrame (SF_DL in the low nibble)
					data[0] = (uint8_t)handler->txSize;
					RAMN_memcpy(&data[1],handler->txData,handler->txSize);
					*dlc = (uint8_t)handler->txSize + 1U;      // *dlc = 8U; if padding preferred
					handler->txIndex = handler->txSize;
					handler->txFrameCount = 1U;
					handler->txStatus = ISOTP_TX_FINISHED;
				}
				else if ((txDL > 8U) && (handler->txSize <= (uint16_t)(txDL - 2U)))
				{
					// Fits in a CAN-FD SingleFrame using the escape sequence (SF_DL in byte #1)
					data[0] = 0x00U;
					data[1] = (uint8_t)handler->txSize;
					RAMN_memcpy(&data[2],handler->txData,handler->txSize);
					*dlc = (uint8_t)handler->txSize + 2U;
					handler->txIndex = handler->txSize;
					handler->txFrameCount = 1U;
					handler->txStatus = ISOTP_TX_FINISHED;
				}
				else if (handler->txSize > 0xFFFU)
				{
					// Large message (> 4095 bytes): First Frame with the 32-bit escape sequence
					data[0] = 0x10U;
					data[1] = 0x00U;
					data[2] = (uint8_t)(((uint32_t)handler->txSize >> 24) & 0xFFU);
					data[3] = (uint8_t)(((uint32_t)handler->txSize >> 16) & 0xFFU);
					data[4] = (uint8_t)(((uint32_t)handler->txSize >> 8) & 0xFFU);
					data[5] = (uint8_t)(handler->txSize & 0xFFU);
					RAMN_memcpy(&data[6],handler->txData,(txDL - 6U));
					*dlc = txDL;
					handler->txIndex = (txDL - 6U);
					handler->txFrameCount = 1U;
					handler->txStatus = ISOTP_TX_WAITING_FLAG; // Wait for Flow Control "FC" frame
				}
				else
				{
					data[0] = (0x10) | ((handler->txSize >> 8) & 0xF);
					data[1] = handler->txSize & 0xFF;
					RAMN_memcpy(&data[2],handler->txData,(txDL - 2U));
					*dlc = txDL;
					handler->txIndex = (txDL - 2U);
					handler->txFrameCount = 1U;
					handler->txStatus = ISOTP_TX_WAITING_FLAG; // Wait for Flow Control "FC" frame
				}
			}
			else
			{
				// Sending a Consecutive Frame "CF"
				uint16_t remainingBytesCount = handler->txSize - handler->txIndex;
				data[0] = 0x20 | ((handler->txFrameCount % 0x10) & 0xFF);

				if (remainingBytesCount > (txDL - 1U))
				{
					// Not Last Frame: fill the full CAN_DL
					RAMN_memcpy(&data[1],&(handler->txData[handler->txIndex]),(txDL - 1U));
					*dlc = txDL;
					handler->txIndex += (txDL - 1U);
					if (handler->targetBlockSize != 0)
					{
						if ((handler->txFrameCount % handler->targetBlockSize) == 0U)
						{
							// Need to wait for next FC frame from target
							handler->txStatus = ISOTP_TX_WAITING_FLAG;
						}
					}
				}
				else
				{
					// Last Frame
					RAMN_memcpy(&data[1],&(handler->txData[handler->txIndex]),remainingBytesCount);
					*dlc = (uint8_t)remainingBytesCount + 1U;
					handler->txIndex += remainingBytesCount;
					handler->txStatus = ISOTP_TX_FINISHED;
				}
				handler->txFrameCount++;

			}
			wroteValidMessage = True;
			// Update the last tx timestamp for any message, including FC frames (?)
			handler->txLastTimestamp = tick;
		}
	}
#ifdef ISOTP_ANSWER_PADDING_BYTE
	// Add padding if required
	if(wroteValidMessage == True)
	{
		if (*dlc < 8U)
		{
			for (uint8_t i = *dlc; i < 8U; i++)
			{
				data[i] = ISOTP_ANSWER_PADDING_BYTE;
			}
			*dlc = 8U;
		}
	}
#endif
	return wroteValidMessage;
}

RAMN_Result_t RAMN_ISOTP_Update(RAMN_ISOTPHandler_t* pHandler, uint32_t tick)
{

	if ((pHandler->rxStatus == ISOTP_RX_TRANSFERRING))
	{
		int32_t lapse = 	tick - pHandler->rxLastTimestamp;
		if (lapse > ISOTP_RX_TIMEOUT_MS)
		{
			report_Error(pHandler, ISOTP_RX,N_TIMEOUT_Cr);
		}
	}
	return RAMN_OK;
}

RAMN_Bool_t RAMN_ISOTP_Continue_TX(RAMN_ISOTPHandler_t* pHandler, uint32_t tick, FDCAN_TxHeaderTypeDef* pTxHeader)
{
	RAMN_Bool_t txFinished = False;
	uint8_t dlc;
	uint8_t data[64];

	if ((pHandler->txStatus == ISOTP_TX_TRANSFERRING) || (pHandler->txStatus == ISOTP_TX_WAITING_FLAG))
	{
		uint32_t lapse = 	tick - pHandler->txLastTimestamp;
		if (lapse > ISOTP_TX_TIMEOUT_MS)
		{
			report_Error(pHandler, ISOTP_TX,N_TIMEOUT_Bs);
		}
	}

	if (RAMN_ISOTP_GetNextTxMsg(pHandler,&dlc,data,tick) != False)
	{
		uint8_t  targetLen = dlc;
#ifdef ISOTP_FD_FULL_PADDING
		// Optionally pad a CAN-FD answer frame all the way to the configured ISOTP_TX_DL.
		if (pHandler->txIsFD && (targetLen < (uint8_t)ISOTP_TX_DL)) targetLen = (uint8_t)ISOTP_TX_DL;
#endif
		uint32_t dlcEnum   = isotp_len_to_dlc(targetLen);
		uint8_t  paddedLen = DLCtoUINT8(dlcEnum);
		// Round a CAN-FD frame up to the next valid CAN_DL (12/16/.../64) and fill the gap
		for (uint8_t i = dlc; i < paddedLen; i++) data[i] = ISOTP_FD_PAD_BYTE;
		pTxHeader->DataLength = dlcEnum;
		// Mirror the request's frame format for the answer
		pTxHeader->FDFormat = pHandler->txIsFD ? FDCAN_FD_CAN : FDCAN_CLASSIC_CAN;
		while (RAMN_FDCAN_SendMessage(pTxHeader,data) != RAMN_OK) RAMN_TaskDelay(1U); // Queue is full, need to wait
	}

	if (pHandler->txStatus == ISOTP_TX_FINISHED)
	{
		txFinished = True;
		pHandler->txStatus = ISOTP_TX_IDLE;
	}

	return txFinished;
}


RAMN_Result_t RAMN_ISOTP_RequestTx(RAMN_ISOTPHandler_t* handler, uint32_t tick)
{
	RAMN_Result_t result = RAMN_ERROR;
	// Any message up to the TX buffer can be sent: <= 4095 bytes uses a 12-bit FF_DL, larger uses the
	// 32-bit escape sequence FirstFrame (both encoded in RAMN_ISOTP_GetNextTxMsg).
	if(handler->txSize <= ISOTP_TXBUFFER_SIZE)
	{
		if (handler->txStatus != ISOTP_TX_IDLE)
		{
			// New request even though ongoing transfer wasn't over
			report_Error(handler, ISOTP_TX, N_ERROR);
		}
		handler->txIndex = 0U;
		handler->txFrameCount = 0U;
		handler->txLastTimestamp = tick;
		// Snapshot the request format now so the whole response stays consistent even if a new
		// request (of a different format) arrives mid-transmission and mutates rxWasFD
		handler->txIsFD = handler->rxWasFD;
		handler->txStatus = ISOTP_TX_TRANSFERRING;
		result = RAMN_OK;
	}
	else
	{
#ifdef HANG_ON_ERRORS
		Error_Handler();
#endif
	}
	return result;
}




#endif
