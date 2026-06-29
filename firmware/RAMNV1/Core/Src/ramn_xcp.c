/*
 * ramn_xcp.c
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

#include "ramn_xcp.h"

#if defined(ENABLE_XCP)

#include "ramn_trng.h"

/* XCP ERRROR CODES */
#define XCP_ERR_CMD_SYNCH 						0x00
#define XCP_ERR_CMD_BUSY						0x10
#define XCP_ERR_DAQ_ACTIVE						0x11
#define XCP_ERR_PGM_ACTIVE						0x12
#define XCP_ERR_CMD_UNKNOWN						0x20
#define XCP_ERR_CMD_SYNTAX						0x21
#define XCP_ERR_OUT_OF_RANGE					0x22
#define XCP_ERR_WRITE_PROTECTED 				0x23
#define XCP_ERR_ACCESS_DENIED   				0x24
#define XCP_ERR_ACCESS_LOCKED					0x25
#define XCP_ERR_PAGE_NOT_VALID  				0x26
#define XCP_ERR_MODE_NOT_VALID 	 				0x27
#define XCP_ERR_SEGMENT_NOT_VALID				0x28
#define XCP_ERR_SEQUENCE						0x29
#define XCP_ERR_DAQ_CONFIG						0x2A
#define XCP_ERR_MEMORY_OVERFLOW     			0x30
#define XCP_ERR_GENERIC							0x31
#define XCP_ERR_VERIFY							0x32

/* XCP COMMAND BYTES */
#define XCP_COMMAND_CONNECT						0xFF
#define XCP_COMMAND_DISCONNECT					0xFE
#define XCP_COMMAND_GET_STATUS					0xFD
#define XCP_COMMAND_SYNCH						0xFC
#define XCP_COMMAND_GET_COMM_MODE_INFO 			0xFB
#define XCP_COMMAND_GET_ID						0xFA
#define XCP_COMMAND_SET_REQUEST					0xF9
#define XCP_COMMAND_GET_SEED					0xF8
#define XCP_COMMAND_UNLOCK						0xF7
#define XCP_COMMAND_SET_MTA						0xF6
#define XCP_COMMAND_UPLOAD						0xF5
#define XCP_COMMAND_SHORT_UPLOAD				0xF4
#define XCP_COMMAND_BUILD_CHECKSUM				0xF3
#define XCP_COMMAND_TRANSPORT_LAYER_CMD			0xF2
#define XCP_COMMAND_USER_CMD					0xF1
#define XCP_COMMAND_DOWNLOAD					0xF0
#define XCP_COMMAND_DOWNLOAD_NEXT				0xEF
#define XCP_COMMAND_DOWNLOAD_MAX				0xEE
#define XCP_COMMAND_SHORT_DOWNLOAD				0xED
#define XCP_COMMAND_MODIFY_BITS					0xEC
#define XCP_COMMAND_SET_CAL_PAGE				0xEB
#define XCP_COMMAND_GET_CAL_PAGE				0xEA
#define XCP_COMMAND_GET_PAG_PROCESSOR_INFO		0xE9
#define XCP_COMMAND_GET_SEGMENT_INFO			0xE8
#define XCP_COMMAND_GET_PAGE_INFO				0xE7
#define XCP_COMMAND_SET_SEGMENT_MODE			0xE6
#define XCP_COMMAND_GET_SEGMENT_MODE			0xE5
#define XCP_COMMAND_COPY_CAL_PAGE				0xE4
#define XCP_COMMAND_CLEAR_DAQ_LIST				0xE3
#define XCP_COMMAND_SET_DAQ_PTR					0xE2
#define XCP_COMMAND_WRITE_DAQ					0xE1
#define XCP_COMMAND_SET_DAQ_LIST_MODE			0xE0
#define XCP_COMMAND_GET_DAQ_LIST_MODE			0xDF
#define XCP_COMMAND_START_STOP_DAQ_LIST			0xDE
#define XCP_COMMAND_START_STOP_SYNCH			0xDD
#define XCP_COMMAND_GET_DAQ_CLOCK				0xDC
#define XCP_COMMAND_READ_DAQ					0xDB
#define XCP_COMMAND_GET_DAQ_PROCESSOR_INFO		0xDA
#define XCP_COMMAND_GET_DAQ_RESOLUTION_INFO		0xD9
#define XCP_COMMAND_GET_DAQ_LIST_INFO			0xD8
#define XCP_COMMAND_GET_DAQ_EVENT_INFO			0xD7
#define XCP_COMMAND_FREE_DAQ					0xD6
#define XCP_COMMAND_ALLOC_DAQ					0xD5
#define XCP_COMMAND_ALLOC_ODT					0xD4
#define XCP_COMMAND_ALLOC_ODT_ENTRY				0xD3
#define XCP_COMMAND_PROGRAM_START				0xD2
#define XCP_COMMAND_PROGRAM_CLEAR				0xD1
#define XCP_COMMAND_PROGRAM						0xD0
#define XCP_COMMAND_PROGRAM_RESET				0xCF
#define XCP_COMMAND_GET_PGM_PROCESSOR_INFO		0xCE
#define XCP_COMMAND_GET_SECTOR_INFO				0xCD
#define XCP_COMMAND_PROGRAM_PREPARE				0xCC
#define XCP_COMMAND_PROGRAM_FORMAT				0xCB
#define XCP_COMMAND_PROGRAM_NEXT				0xCA
#define XCP_COMMAND_PROGRAM_MAX					0xC9
#define XCP_COMMAND_PROGRAM_VERIFY				0xC8

/* XCP CHECKSUM TYPES (BUILD_CHECKSUM) */
#define XCP_CHECKSUM_TYPE_ADD_14				0x03 // add BYTE into a DWORD

// Exported variables ----------------------------------

RAMN_XCPHandler_t RAMN_XCP_Handler;

// Private variables ----------------------------------

// Common pointers to avoid passing answer data as argument each sub-function
static uint8_t* xcp_answerData;
static uint16_t* xcp_answerSize;

// XCP Device Name
#ifndef XCP_DEVICE_NAME
#if defined(TARGET_ECUB)
const char XCP_DEVICE_NAME[] = "ECUB";
#elif defined(TARGET_ECUC)
const char XCP_DEVICE_NAME[] = "ECUC";
#elif defined(TARGET_ECUD)
const char XCP_DEVICE_NAME[] = "ECUD";
#else
const char XCP_DEVICE_NAME[] = "ECUA";
#endif
#endif

// Header used to request transmission of XCP answer CAN message
FDCAN_TxHeaderTypeDef RAMN_XCP_TxMsgHeader =
{
		.Identifier  = XCP_TX_CANID,
		.DataLength = FDCAN_DLC_BYTES_0,
		.IdType = FDCAN_STANDARD_ID,
		.TxFrameType = FDCAN_DATA_FRAME,
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.BitRateSwitch = FDCAN_BRS_OFF,
		.FDFormat = FDCAN_CLASSIC_CAN,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
		. MessageMarker = 0,
};

// Resets session to default state
static void XCP_ResetSession(uint32_t tick)
{
	RAMN_XCP_Handler.connected 			= False;
	RAMN_XCP_Handler.lastRXTimestamp 	= tick;
	RAMN_XCP_Handler.authenticated 		= False;
	RAMN_XCP_Handler.seedRequested 		= False;
	RAMN_XCP_Handler.currentSeed 		= 0U;
	RAMN_XCP_Handler.mtaPointer			= 0U;
}

static void XCP_FormatNegativeAnswer(uint8_t errCode)
{
	xcp_answerData[0] = 0xFE;
	xcp_answerData[1] = errCode;
	*xcp_answerSize = 2U;
}

static void XCP_Connect(const uint8_t* data, uint16_t size)
{
	uint8_t mode;
	if (size < 2U) mode = 0U;
	else mode = data[1U];

	switch(mode)
	{
	case 0x00:
		XCP_ResetSession(RAMN_XCP_Handler.lastRXTimestamp);
		RAMN_XCP_Handler.connected = True;
		xcp_answerData[1] = 0x00; // No Resource
		xcp_answerData[2] = 0x01; // Motorola (big-endian) format
		xcp_answerData[3] = 0x08; // 8-bytes max CTO
		xcp_answerData[4] = 0x08; // 8-bytes max DTO
		xcp_answerData[5] = 0x00; // 8-bytes max DTO
		xcp_answerData[6] = 0x01; // Ver1
		xcp_answerData[7] = 0x01; // Ver1
		*xcp_answerSize = 8U;
		break;
	default:
		XCP_FormatNegativeAnswer(XCP_ERR_OUT_OF_RANGE);
		break;
	}
}

static void XCP_Disconnect(const uint8_t* data, uint16_t size)
{
	XCP_ResetSession(RAMN_XCP_Handler.lastRXTimestamp);
	*xcp_answerSize = 1U;
}

static void XCP_GetStatus(const uint8_t* data, uint16_t size)
{
	xcp_answerData[1] = 0x00; //No Status
	xcp_answerData[2] = (RAMN_XCP_Handler.authenticated == True) ? 0 : (1 << 4); //Programming protection
	xcp_answerData[3] = 0x00;
	xcp_answerData[4] = 0x00;
	xcp_answerData[5] = 0x00;
	*xcp_answerSize = 6U;
}

static void RAMN_XCP_GetID(const uint8_t* data, uint16_t size)
{
	if (size < 2U)
	{
		XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNTAX);
	}
	else
	{
		switch(data[1])
		{
		case 0x00:
			xcp_answerData[1] = data[1];
			xcp_answerData[2] = 0;
			xcp_answerData[3] = 0;
			xcp_answerData[4] = 0; // length is big-endian (MSB first)
			xcp_answerData[5] = 0;
			xcp_answerData[6] = 0;
			xcp_answerData[7] = sizeof(XCP_DEVICE_NAME)-1; //non-null terminated, big-endian LSB
			*xcp_answerSize = 8U;
			RAMN_XCP_Handler.mtaPointer = (uint32_t)(uintptr_t)&XCP_DEVICE_NAME;
			break;
		default:
			XCP_FormatNegativeAnswer(XCP_ERR_OUT_OF_RANGE);
			break;
		}
	}
}

static void XCP_Upload(const uint8_t* data, uint16_t size)
{
	if (size < 2U) XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNTAX);
	else
	{
		if ((data[1] > 7))
		{
			XCP_FormatNegativeAnswer(XCP_ERR_OUT_OF_RANGE);
		}
		else if (RAMN_MEMORY_CheckAreaReadable(RAMN_XCP_Handler.mtaPointer, RAMN_XCP_Handler.mtaPointer + (uint32_t)data[1]) != True)
		{
			XCP_FormatNegativeAnswer(XCP_ERR_OUT_OF_RANGE);
		}
		else
		{
			xcp_answerData[0] = 0xFF;
			for(uint8_t i = 0; i < data[1] ; i++)
			{
				xcp_answerData[i+1] = (uint8_t)*(uint8_t*)(uintptr_t)(RAMN_XCP_Handler.mtaPointer);
				RAMN_XCP_Handler.mtaPointer++;
			}
			*xcp_answerSize = (uint16_t)data[1] + 1U;
		}
	}
}

static void XCP_ShortUpload(const uint8_t* data, uint16_t size)
{
	if (size < 8U) XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNTAX);
	else
	{
		RAMN_XCP_Handler.mtaPointer = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
		XCP_Upload(data, size); 
	}
}

static void XCP_SetMTA(const uint8_t* data, uint16_t size)
{
	if (size != 8U) XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNTAX);
	else
	{
		RAMN_XCP_Handler.mtaPointer = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
		*xcp_answerSize = 1U;
	}
}

static void XCP_GetCommModeInfo(const uint8_t* data, uint16_t size)
{
	xcp_answerData[1] = 0x00; // reserved
	xcp_answerData[2] = 0x00; // COMM_MODE_OPTIONAL (no master block mode, no interleaved)
	xcp_answerData[3] = 0x00; // reserved
	xcp_answerData[4] = 0x00; // MAX_BS (block mode unsupported)
	xcp_answerData[5] = 0x00; // MIN_ST
	xcp_answerData[6] = 0x00; // QUEUE_SIZE
	xcp_answerData[7] = 0x01; // XCP driver version
	*xcp_answerSize = 8U;
}

static void XCP_BuildChecksum(const uint8_t* data, uint16_t size)
{
	if (size < 8U) XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNTAX);
	else
	{
		uint32_t blockSize = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
		// CheckAreaReadable uses an exclusive end and rejects empty/overflowing ranges.
		if (RAMN_MEMORY_CheckAreaReadable(RAMN_XCP_Handler.mtaPointer, RAMN_XCP_Handler.mtaPointer + blockSize) != True)
		{
			XCP_FormatNegativeAnswer(XCP_ERR_OUT_OF_RANGE);
		}
		else
		{
			uint32_t checksum = 0U;
			for (uint32_t i = 0U; i < blockSize; i++)
			{
				checksum += (uint32_t)*(uint8_t*)(uintptr_t)(RAMN_XCP_Handler.mtaPointer);
				RAMN_XCP_Handler.mtaPointer++;
			}
			xcp_answerData[1] = XCP_CHECKSUM_TYPE_ADD_14; // checksum type
			xcp_answerData[2] = 0x00; // reserved
			xcp_answerData[3] = 0x00; // reserved
			xcp_answerData[4] = (uint8_t)((checksum >> 24) & 0xFF); // big-endian
			xcp_answerData[5] = (uint8_t)((checksum >> 16) & 0xFF);
			xcp_answerData[6] = (uint8_t)((checksum >> 8 ) & 0xFF);
			xcp_answerData[7] = (uint8_t)((checksum      ) & 0xFF);
			*xcp_answerSize = 8U;
		}
	}
}

static void XCP_GetSeed(const uint8_t* data, uint16_t size)
{
	if (size < 2U) XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNTAX);
	else if (RAMN_XCP_Handler.authenticated == True)
	{
		// Resource already unlocked/unprotected: seed length 0
		xcp_answerData[1] = 0x00;
		*xcp_answerSize = 2U;
	}
	else if (data[1] == 0x00) // mode 0x00: request (first part of) seed
	{
		RAMN_XCP_Handler.currentSeed = RAMN_RNG_Pop32();
		RAMN_XCP_Handler.seedRequested = True;
		xcp_answerData[1] = 0x04; // seed length
		xcp_answerData[2] = (uint8_t)((RAMN_XCP_Handler.currentSeed >> 24) & 0xFF); // big-endian
		xcp_answerData[3] = (uint8_t)((RAMN_XCP_Handler.currentSeed >> 16) & 0xFF);
		xcp_answerData[4] = (uint8_t)((RAMN_XCP_Handler.currentSeed >> 8 ) & 0xFF);
		xcp_answerData[5] = (uint8_t)((RAMN_XCP_Handler.currentSeed      ) & 0xFF);
		*xcp_answerSize = 6U;
	}
	else // mode 0x01: remaining part, but seed fits in one packet
	{
		xcp_answerData[1] = 0x00;
		*xcp_answerSize = 2U;
	}
}

static void XCP_Unlock(const uint8_t* data, uint16_t size)
{
	if (size < 6U) XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNTAX); // need 4 key bytes
	else if (RAMN_XCP_Handler.seedRequested != True) XCP_FormatNegativeAnswer(XCP_ERR_SEQUENCE);
	else
	{
		uint32_t key = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];
		if (key == (RAMN_XCP_Handler.currentSeed ^ 0x12345678))
		{
			RAMN_XCP_Handler.authenticated = True;
			RAMN_XCP_Handler.seedRequested = False;
			xcp_answerData[1] = 0x00; // nothing still protected
			*xcp_answerSize = 2U;
		}
		else
		{
			// Wrong key: reset the session so the client must reconnect (per spec).
			XCP_FormatNegativeAnswer(XCP_ERR_ACCESS_LOCKED);
			XCP_ResetSession(RAMN_XCP_Handler.lastRXTimestamp);
		}
	}
}

static void XCP_RespondZeroInfo(uint16_t answerSize)
{
	for (uint16_t i = 1U; i < answerSize; i++) xcp_answerData[i] = 0x00;
	*xcp_answerSize = answerSize;
}

// Exported Functions ----------------------------------

RAMN_Result_t RAMN_XCP_Init(uint32_t tick)
{
	XCP_ResetSession(tick);
	return RAMN_OK;
}

RAMN_Bool_t RAMN_XCP_Update(uint32_t tick)
{
	if (RAMN_XCP_Handler.connected)
	{
		if ((tick - RAMN_XCP_Handler.lastRXTimestamp) > XCP_RX_TIMEOUT) XCP_ResetSession(tick);
	}
	return RAMN_OK;
}

RAMN_Bool_t RAMN_XCP_Continue_TX(uint32_t tick, const uint8_t* data, uint16_t size)
{
	RAMN_XCP_TxMsgHeader.DataLength = UINT8toDLC((uint8_t)size);
	while (RAMN_FDCAN_SendMessage(&RAMN_XCP_TxMsgHeader,data) != RAMN_OK) osDelay(10U); //Queue is full, need to wait
	return True;
}

RAMN_Bool_t RAMN_XCP_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf)
{
	size_t xBytesSent;
	uint16_t size;
	RAMN_Bool_t result = False;
	RAMN_Bool_t matched = False;

#ifdef ENABLE_J1939_MODE
	uint8_t prio = (pHeader->Identifier >> 26) & 0x7;
	uint8_t pf = (pHeader->Identifier >> 16) & 0xFF;
	uint8_t da = (pHeader->Identifier >> 8) & 0xFF;
	uint8_t sa = pHeader->Identifier & 0xFF;

	// Proprietary A (PF 0xEF) is used for KWP2000 and XCP. Physical only.
	// TSA must be 0x3F or 0x5A for XCP.
	if (pHeader->IdType == FDCAN_EXTENDED_ID && pf == 0xEF && da == J1939_ECU_SA)
	{
		if (sa == 0x3F || sa == 0x5A)
		{
			// Avoid ECUC (SA 0x5A) responding to itself if another ECU is using TSA 0x5A
			if (sa != J1939_ECU_SA)
			{
				RAMN_XCP_TxMsgHeader.Identifier = J1939_UCAST_ID(prio, 0xEF00, sa, J1939_ECU_SA);
				RAMN_XCP_TxMsgHeader.IdType = FDCAN_EXTENDED_ID;
				matched = True;
			}
		}

		if (matched == False)
		{
			RAMN_XCP_TxMsgHeader.Identifier = XCP_TX_CANID;
			RAMN_XCP_TxMsgHeader.IdType = FDCAN_STANDARD_ID;
		}
	}
#else
	if (pHeader->Identifier == XCP_RX_CANID)
	{
		matched = True;
	}
#endif

	if (matched == True)
	{
		size = (uint16_t)DLCtoUINT8(pHeader->DataLength);
		if (size > 0U)
		{
			// Got an XCP payload, should be forwarded to diag thread
			xBytesSent = xStreamBufferSend(*strbuf, (void*) &(size), sizeof(size), portMAX_DELAY );
			xBytesSent += xStreamBufferSend(*strbuf, (void*) data, size, portMAX_DELAY );
			if(xBytesSent != (size + sizeof(size))) Error_Handler();
			result = True;
		}
	}
	return result;
}

void RAMN_XCP_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	xcp_answerData = answerData;
	xcp_answerSize = answerSize;
	RAMN_XCP_Handler.lastRXTimestamp = tick;
	if (size > 0U)
	{
		xcp_answerData[0U] = 0xFF; //Positive Response by default
		if ((data[0U] == XCP_COMMAND_CONNECT) || (RAMN_XCP_Handler.connected == True))
		{
			switch(data[0U]) //Analyze command byte
			{
			case XCP_COMMAND_CONNECT:
				XCP_Connect(data,size);
				break;
			case XCP_COMMAND_DISCONNECT:
				XCP_Disconnect(data,size);
				break;
			case XCP_COMMAND_GET_STATUS:
				XCP_GetStatus(data,size);
				break;
			case XCP_COMMAND_SYNCH:
				XCP_FormatNegativeAnswer(XCP_ERR_CMD_SYNCH);
				break;
			case XCP_COMMAND_GET_COMM_MODE_INFO:
				XCP_GetCommModeInfo(data,size);
				break;
			case XCP_COMMAND_GET_ID:
				RAMN_XCP_GetID(data,size);
				break;
			case XCP_COMMAND_GET_SEED:
				XCP_GetSeed(data,size);
				break;
			case XCP_COMMAND_UNLOCK:
				XCP_Unlock(data,size);
				break;
			case XCP_COMMAND_UPLOAD:
				XCP_Upload(data,size);
				break;
			case XCP_COMMAND_SHORT_UPLOAD:
				XCP_ShortUpload(data,size);
				break;
			case XCP_COMMAND_SET_MTA:
				XCP_SetMTA(data,size);
				break;
			case XCP_COMMAND_BUILD_CHECKSUM:
				XCP_BuildChecksum(data,size);
				break;
			case XCP_COMMAND_GET_DAQ_PROCESSOR_INFO:
				XCP_RespondZeroInfo(8U);
				break;
			case XCP_COMMAND_GET_DAQ_RESOLUTION_INFO:
				XCP_RespondZeroInfo(8U);
				break;
			case XCP_COMMAND_GET_PAG_PROCESSOR_INFO:
				XCP_RespondZeroInfo(3U);
				break;
			case XCP_COMMAND_GET_PGM_PROCESSOR_INFO:
				XCP_RespondZeroInfo(3U);
				break;
			default:
				XCP_FormatNegativeAnswer(XCP_ERR_CMD_UNKNOWN);
				break;
			}
		}
	}
}


#endif
