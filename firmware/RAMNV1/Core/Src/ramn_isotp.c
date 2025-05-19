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
static void report_Error(RAMN_ISOTPHandler_t* handler, uint8_t dir, RAMN_ISOTP_N_RESULT errCode)
{
	if (dir & ISOTP_RX)
	{
		// TODO: RX error processing
		handler->rxStatus = ISOTP_RX_IDLE; // Get ready for what is next
	}
	if (dir & ISOTP_TX)
	{
		// TODO: TX error processing
		handler->txStatus = ISOTP_TX_FINISHED; // Needs to be put back to "IDLE" manually
	}
}

static void ISOTP_ReceiveSF(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data)
{
	uint8_t size;

	size = data[0]&0xF;
	if(handler->rxStatus != ISOTP_RX_IDLE)
	{
		// Should report error, but proceed with receiving
		report_Error(handler, ISOTP_RX, N_UNEXP_PDU);
	}

	// Standard says to not report, just ignore invalid lengths
	if ((size > 0) && (size <= 7U))
	{
		if (size <= (dlc-1U)) // Standard says to just ignore invalid DLCs
		{
			// Valid data received, copy to buffer and signal buffer ready
			RAMN_memcpy(handler->rxData,&data[1],size);
			handler->rxCount = size;
			handler->rxStatus = ISOTP_RX_FINISHED;
		}
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
		handler->rxExpectedSize = ((data[0]&0xF) << 8) | data[1];
		// Standard says to just ignore invalid payload sizes
		if (handler->rxExpectedSize > 7)
		{
			// Prepare to receive data
			if(handler->rxExpectedSize <= ISOTP_RXBUFFER_SIZE)
			{
				RAMN_memcpy(handler->rxData, &data[2], 6);
				handler->rxCount      = 6;
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

	waitingBytes = handler->rxExpectedSize - handler->rxCount;
	if ((waitingBytes > 7) && dlc != 8)
	{
		// Should ignore
	}
	else if ((waitingBytes <= 7) && dlc < (waitingBytes +1))
	{
		// Should ignore
	}
	else
	{
		handler->rxFrameCount++;
		size = waitingBytes < 7 ? waitingBytes : dlc - 1;
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

static void ISOTP_ReceiveFC(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data)
{
	handler->targetFCFlag = data[0]&0xF;

	if (dlc > 1) handler->targetBlockSize = data[1];
	else handler->targetBlockSize = 0U;
	if (dlc > 2) handler->targetST = data[2];
	else handler->targetST = 0U;

	switch(handler->targetFCFlag)
	{
	case 0U:   // "Continue To Send"
		// ISO_TP says we should just ignore if not expecting
		if(handler->txStatus == ISOTP_TX_WAITING_FLAG)
		{
			handler->txStatus = ISOTP_TX_TRANSFERRING;
		}
		break;

	case 1U:  // "WAIT"
		// Ignore if not expecting
		if((handler->txStatus == ISOTP_TX_WAITING_FLAG) || (handler->txStatus == ISOTP_TX_TRANSFERRING))
		{
			handler->txStatus = ISOTP_TX_WAITING_FLAG;
		}
		break;

	case 2U:  // "Abort"
		report_Error(handler, ISOTP_TX, N_BUFFER_OVFLW);
		break;

	default:
		report_Error(handler, ISOTP_TX, N_INVALID_FS);
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

	handler->selfFCFlag = 0U;
	handler->selfBlockSize = ISOTP_CONSECUTIVE_BLOCK_SIZE;
	handler->selfST = ISOTP_CONSECUTIVE_ST;

	handler->rxFrameCount = 0U;
	handler->txFrameCount = 0U;

	handler->pFC_CANHeader = FCMsgHeader;

	for(uint32_t i = 0; i < ISOTP_RXBUFFER_SIZE; i++) handler->rxData[i] = 0U;
	for(uint32_t i = 0; i < ISOTP_TXBUFFER_SIZE; i++) handler->txData[i] = 0U;

}

void RAMN_ISOTP_ProcessRxMsg(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data, const uint32_t tick)
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
			ISOTP_ReceiveSF(handler, dlc, data);
			break;

		case 0x1U: // FIRST FRAME (FF)
			ISOTP_ReceiveFF(handler, dlc, data);
			break;

		case 0x2U: // CONSECUTIVE FRAME (CF)
			// ISO-TP standard says we should just ignore if CF not expected
			ISOTP_ReceiveCF(handler, dlc, data);
			break;

		case 0x3U: // FLOW CONTROL FRAME (FC)
			ISOTP_ReceiveFC(handler, dlc, data);
			break;

		default:  // INVALID HEADER
			// Standard says we should just ignore
			break;
		}
	}

	if (handler->rxMustSendCF == True)
	{
		uint8_t dlc;
		uint8_t data[8U];
		RAMN_ISOTP_GetFCFrame(handler,&dlc,data);
		handler->pFC_CANHeader->DataLength = UINT8toDLC(dlc);
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
		if (minDelayMs >= 0xF1) minDelayMs = 1U ; // Consider "microseconds" values as 1ms
		if ((tick - handler->txLastTimestamp) >= (uint32_t) minDelayMs)
		{
			if(handler->txIndex == 0)
			{
				// Single frame
				if(handler->txSize <= 7)
				{
					// Fits in a single frame (FM)
					data[0] = (uint8_t)handler->txSize;
					RAMN_memcpy(&data[1],handler->txData,handler->txSize);
					*dlc = handler->txSize +1U;      // *dlc = 8U; if padding preferred
					handler->txIndex = handler->txSize;
					handler->txFrameCount = 1U;
					handler->txStatus = ISOTP_TX_FINISHED;
				}
				else
				{
					// Large message, requires several messages
					// First sent a "First Frame" (FF)
					data[0] = (0x10) | ((handler->txSize >> 8) & 0xF);
					data[1] = handler->txSize & 0xFF;
					RAMN_memcpy(&data[2],handler->txData,6);
					*dlc = 8U;
					handler->txIndex = 6U;
					handler->txFrameCount = 1U;
					handler->txStatus = ISOTP_TX_WAITING_FLAG; // Wait for Flow Control "FC" frame
				}
			}
			else
			{
				// Sending a Consecutive Frame "CF"
				uint16_t remainingBytesCount = handler->txSize - handler->txIndex;
				data[0] = 0x20 | ((handler->txFrameCount % 0x10) & 0xFF);

				if (remainingBytesCount > 7U)
				{
					// Not Last Frame
					RAMN_memcpy(&data[1],&(handler->txData[handler->txIndex]),7);
					*dlc = 8U;
					handler->txIndex += 7;
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
			report_Error(pHandler, ISOTP_RX,N_TIMEOUT_Cr); // TODO: better report ?
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
			report_Error(pHandler, ISOTP_TX,N_TIMEOUT_Bs); // TODO: better report ?
		}
	}

	if (RAMN_ISOTP_GetNextTxMsg(pHandler,&dlc,data,tick) != False)
	{
		pTxHeader->DataLength = UINT8toDLC(dlc);
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
	uint8_t result = RAMN_ERROR;
	if(handler->txSize <= ISOTP_TXBUFFER_SIZE)
	{
		if (handler->txStatus != ISOTP_TX_IDLE)
		{
			// New request even though ongoing transfer wasn't over
			report_Error(handler, ISOTP_TX, N_TIMEOUT_A);
		}
		handler->txIndex = 0U;
		handler->txFrameCount = 0U;
		handler->txLastTimestamp = tick;
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
