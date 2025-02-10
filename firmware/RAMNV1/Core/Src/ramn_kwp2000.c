/*
 * ramn_kwp2000.c
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

#include "ramn_kwp2000.h"

#if defined(ENABLE_KWP)

// KWP Commands (cf. // Cf https://books.google.co.jp/books?id=ZyMPTGipUGsC)
#define KWP_COMMAND_DIAGNOSTIC_SESSION  			0x10
#define	KWP_COMMAND_ECU_RESET		 				0x11
#define KWP_COMMAND_READ_FREEZE_FRAME_DATA			0x12
#define KWP_COMMAND_READ_DTC						0x13
#define KWP_COMMAND_CLEAR_DTC						0x14
#define KWP_COMMAND_READ_DTC_STATUS					0x17
#define KWP_COMMAND_READ_DTC_BY_STATUS				0x18
#define KWP_COMMAND_ECU_INFO						0x21
#define KWP_COMMAND_DEAD_DATA_BY_ID					0x22
#define KWP_COMMAND_READ_DATA_BY_ADDRESS			0x23
#define KWP_COMMAND_SET_DATA_RATES					0x26
#define KWP_COMMAND_SECURITY_ACCESS					0x27
#define KWP_COMMAND_DISABLE_TRANSMISSION			0x28
#define KWP_COMMAND_ENABLE_TRANSMISSION				0x29
#define KWP_COMMAND_DYNAMICALLY_DEFINE_LOCAL_ID		0x2C
#define KWP_COMMAND_WRITE_DATA_BY_ID				0x2E
#define KWP_COMMAND_IO_CONTROL_BY_COMMON_ID			0x2F
#define KWP_COMMAND_IO_CONTROL_BY_LOCAL_ID			0x30
#define KWP_COMMAND_START_ROUTINE_BY_ID				0x31
#define KWP_COMMAND_STOP_ROUTINE_BY_ID				0x32
#define KWP_COMMAND_REQUEST_ROUTINE_RESULTS_BY_ID	0x33
#define KWP_COMMAND_REQUEST_DOWNLOAD				0x34
#define KWP_COMMAND_REQUEST_UPLOAD					0x35
#define KWP_COMMAND_TRANSFER_DATA					0x36
#define KWP_COMMAND_REQUEST_TRANSFER_EXIT			0x37
#define KWP_COMMAND_WRITE_DATA_BY_LOCAL_ID			0x3B
#define KWP_COMMAND_WRITE_MEMORY_BY_ADDRESS			0x3D
#define KWP_COMMAND_TESTER_PRESENT					0x3E
#define KWP_COMMAND_NETWORK_CONFIGURATION			0x84
#define KWP_COMMAND_CONTROL_DTC_SETTINGS			0x85
#define KWP_COMMAND_RESPONSE_ON_EVENT				0x86

// KWP Errors
#define KWP_NRC_SERVICE_NOT_SUPPORTED			0x11
#define KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF	0x12

// TODO: refactor module
// Common pointer to avoid passing answer data as argument each sub-function
static uint8_t* kwp_answerData;
static uint16_t* kwp_answerSize;

static FDCAN_TxHeaderTypeDef kwpMsgHeader =
{
		.Identifier  = KWP_TX_CANID,
		.DataLength = FDCAN_DLC_BYTES_0,
		.IdType = FDCAN_STANDARD_ID,
		.TxFrameType = FDCAN_DATA_FRAME,
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.BitRateSwitch = FDCAN_BRS_OFF,
		.FDFormat = FDCAN_CLASSIC_CAN,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
		. MessageMarker = 0,
};

static FDCAN_TxHeaderTypeDef kwpFCMsgHeader =
{
		.Identifier  = KWP_TX_CANID,
		.DataLength = FDCAN_DLC_BYTES_0,
		.IdType = FDCAN_STANDARD_ID,
		.TxFrameType = FDCAN_DATA_FRAME,
		.ErrorStateIndicator = FDCAN_ESI_ACTIVE,
		.BitRateSwitch = FDCAN_BRS_OFF,
		.FDFormat = FDCAN_CLASSIC_CAN,
		.TxEventFifoControl = FDCAN_NO_TX_EVENTS,
		. MessageMarker = 0,
};

static void RAMN_KWP_FormatNegativeResponse(const uint8_t* data, uint8_t errCode)
{
	kwp_answerData[0] = 0x7F;
	kwp_answerData[1] = data[0];
	kwp_answerData[2] = errCode;
	*kwp_answerSize = 3U;
}

static void RAMN_KWP_FormatPositiveResponseEcho(const uint8_t* data, uint16_t size)
{
	kwp_answerData[0] = data[0] + 0x40;
	for(uint16_t i = 1; i < size; i++)
	{
		kwp_answerData[i] = data[i];
	}
	*kwp_answerSize = 1U;
}

static void RAMN_KWP_DiagnosticSessionControl(const uint8_t* data, uint16_t size)
{
	// Dummy implementation, accept only Default Diag session / Programming Session
	if (size >= 2)
	{
		if ((data[1] == 0x81) || (data[1] == 0x85)) RAMN_KWP_FormatPositiveResponseEcho(data, 1U);
		else RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
	}
	else
	{
		RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
	}
}

static void RAMN_KWP_ECUReset(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ReadFreezeFrameData(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ClearDTC(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ReadDTC(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ReadDTCStatus(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ReadDTCByStatus(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ReadECUInfo(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ReadDataByIdentifier(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ReadMemoryByAddress(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_SetDataRates(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_SecurityAccess(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_DisableTransmission(const uint8_t* data, uint16_t size)
{
	if (size >= 2U)
	{
		if (data[1U] == 0x01)
		{
			RAMN_DBC_RequestSilence = True;
			RAMN_KWP_FormatPositiveResponseEcho(data, 1U);
		}
		else if (data[1U] == 0x02) {
			RAMN_DBC_RequestSilence = True;
			/* No response  */
		}
		else
		{
			RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
		}
	}
	else
	{
		RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
	}
}

static void RAMN_KWP_EnableTransmission(const uint8_t* data, uint16_t size)
{
	if (size >= 2U)
	{
		if (data[1U] == 0x01)
		{
			RAMN_DBC_RequestSilence = False;
			RAMN_KWP_FormatPositiveResponseEcho(data, 1U);
		}
		else if (data[1U] == 0x02) {
			RAMN_DBC_RequestSilence = False;
			/* No response  */
		}
		else
		{
			RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
		}
	}
	else
	{
		RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
	}
}

static void RAMN_KWP_DynamicallyDefineLocalID(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_WriteDataByIdentifier(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_IOControlByCommonID(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_IOControlByLocalID(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_StartRoutineByID(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_StopRoutineByID(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_RequestRoutineResultsByID(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_RequestDownload(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_RequestUpload(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_TransferData(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_RequestTransferExit(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_WriteDataByID(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_WriteMemoryByAddress(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_TesterPresent(const uint8_t* data, uint16_t size)
{
	if (size >= 2U)
	{
		if (data[1U] == 0x01) RAMN_KWP_FormatPositiveResponseEcho(data, 1U);
		else if (data[1U] == 0x02) { /* No response  */ }
		else
		{
			RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
		}
	}
	else
	{
		RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SUBFUNCTION_NOT_SUPPORTED_IF);
	}
}

static void RAMN_KWP_NetworkConfiguration(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}

static void RAMN_KWP_ControlDTCSettings(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}


static void RAMN_KWP_ResponseOnEvent(const uint8_t* data, uint16_t size)
{
	RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
}


// EXPORTED --------------------------------------------

RAMN_ISOTPHandler_t RAMN_KWP_ISOTPHandler;


RAMN_Result_t RAMN_KWP_Init(uint32_t tick)
{
	RAMN_ISOTP_Init(&RAMN_KWP_ISOTPHandler,&kwpFCMsgHeader);
	return RAMN_OK;
}


RAMN_Result_t RAMN_KWP_Update(uint32_t tick)
{
	return RAMN_ISOTP_Update(&RAMN_KWP_ISOTPHandler,tick);
}

RAMN_Bool_t RAMN_KWP_Continue_TX(uint32_t tick)
{
	return RAMN_ISOTP_Continue_TX(&RAMN_KWP_ISOTPHandler, tick, &kwpMsgHeader);
}

RAMN_Bool_t RAMN_KWP_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf)
{
	size_t xBytesSent;
	RAMN_Bool_t result = False;
	if (pHeader->Identifier == KWP_RX_CANID)
	{
		RAMN_ISOTP_ProcessRxMsg(&RAMN_KWP_ISOTPHandler,DLCtoUINT8(pHeader->DataLength),data, tick);

		//If a ISO-TP has been received, copy it to buffer
		if (RAMN_KWP_ISOTPHandler.rxStatus == ISOTP_RX_FINISHED)
		{
			xBytesSent = xStreamBufferSend(*strbuf, (void *) &(RAMN_KWP_ISOTPHandler.rxCount), sizeof(RAMN_KWP_ISOTPHandler.rxCount), portMAX_DELAY );
			xBytesSent += xStreamBufferSend(*strbuf, (void *) RAMN_KWP_ISOTPHandler.rxData, RAMN_KWP_ISOTPHandler.rxCount, portMAX_DELAY );
			if( xBytesSent != (RAMN_KWP_ISOTPHandler.rxCount + sizeof(RAMN_KWP_ISOTPHandler.rxCount) )) Error_Handler();
			RAMN_KWP_ISOTPHandler.rxStatus = ISOTP_RX_IDLE;
			result = True;
		}
	}
	return result;
}

void RAMN_KWP_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	kwp_answerData = answerData;
	kwp_answerSize = answerSize;
	*kwp_answerSize = 0U; // Empty Response by default

	if (size > 0U)
	{
		if (data[0] < 0x10)
		{
			// J1979 command
			RAMN_J1979_ProcessMessage(data, size, answerData, answerSize);
		}
		else
		{
			switch (data[0]) {
			case KWP_COMMAND_DIAGNOSTIC_SESSION:
				RAMN_KWP_DiagnosticSessionControl(data, size);
				break;
			case KWP_COMMAND_ECU_RESET:
				RAMN_KWP_ECUReset(data, size);
				break;
			case KWP_COMMAND_READ_FREEZE_FRAME_DATA:
				RAMN_KWP_ReadFreezeFrameData(data,size);
				break;
			case KWP_COMMAND_CLEAR_DTC:
				RAMN_KWP_ClearDTC(data, size);
				break;
			case KWP_COMMAND_READ_DTC:
				RAMN_KWP_ReadDTC(data, size);
				break;
			case KWP_COMMAND_READ_DTC_STATUS:
				RAMN_KWP_ReadDTCStatus(data, size);
				break;
			case KWP_COMMAND_READ_DTC_BY_STATUS:
				RAMN_KWP_ReadDTCByStatus(data, size);
				break;
			case KWP_COMMAND_ECU_INFO:
				RAMN_KWP_ReadECUInfo(data, size);
				break;
			case KWP_COMMAND_DEAD_DATA_BY_ID:
				RAMN_KWP_ReadDataByIdentifier(data, size);
				break;
			case KWP_COMMAND_READ_DATA_BY_ADDRESS:
				RAMN_KWP_ReadMemoryByAddress(data, size);
				break;
			case KWP_COMMAND_SET_DATA_RATES:
				RAMN_KWP_SetDataRates(data, size);
				break;
			case KWP_COMMAND_SECURITY_ACCESS:
				RAMN_KWP_SecurityAccess(data, size);
				break;
			case KWP_COMMAND_DISABLE_TRANSMISSION:
				RAMN_KWP_DisableTransmission(data, size);
				break;
			case KWP_COMMAND_ENABLE_TRANSMISSION:
				RAMN_KWP_EnableTransmission(data, size);
				break;
			case KWP_COMMAND_DYNAMICALLY_DEFINE_LOCAL_ID:
				RAMN_KWP_DynamicallyDefineLocalID(data, size);
				break;
			case KWP_COMMAND_WRITE_DATA_BY_ID:
				RAMN_KWP_WriteDataByIdentifier(data, size);
				break;
			case KWP_COMMAND_IO_CONTROL_BY_COMMON_ID:
				RAMN_KWP_IOControlByCommonID(data, size);
				break;
			case KWP_COMMAND_IO_CONTROL_BY_LOCAL_ID:
				RAMN_KWP_IOControlByLocalID(data, size);
				break;
			case KWP_COMMAND_START_ROUTINE_BY_ID:
				RAMN_KWP_StartRoutineByID(data,size);
				break;
			case KWP_COMMAND_STOP_ROUTINE_BY_ID:
				RAMN_KWP_StopRoutineByID(data, size);
				break;
			case KWP_COMMAND_REQUEST_ROUTINE_RESULTS_BY_ID:
				RAMN_KWP_RequestRoutineResultsByID(data, size);
				break;
			case KWP_COMMAND_REQUEST_DOWNLOAD:
				RAMN_KWP_RequestDownload(data, size);
				break;
			case KWP_COMMAND_REQUEST_UPLOAD:
				RAMN_KWP_RequestUpload(data, size);
				break;
			case KWP_COMMAND_TRANSFER_DATA:
				RAMN_KWP_TransferData(data, size);
				break;
			case KWP_COMMAND_REQUEST_TRANSFER_EXIT:
				RAMN_KWP_RequestTransferExit(data, size);
				break;
			case KWP_COMMAND_WRITE_DATA_BY_LOCAL_ID:
				RAMN_KWP_WriteDataByID(data, size);
				break;
			case KWP_COMMAND_WRITE_MEMORY_BY_ADDRESS:
				RAMN_KWP_WriteMemoryByAddress(data, size);
				break;
			case KWP_COMMAND_TESTER_PRESENT:
				RAMN_KWP_TesterPresent(data, size);
				break;
			case KWP_COMMAND_NETWORK_CONFIGURATION:
				RAMN_KWP_NetworkConfiguration(data, size);
				break;
			case KWP_COMMAND_CONTROL_DTC_SETTINGS:
				RAMN_KWP_ControlDTCSettings(data, size);
				break;
			case KWP_COMMAND_RESPONSE_ON_EVENT:
				RAMN_KWP_ResponseOnEvent(data, size);
				break;
			default:
				RAMN_KWP_FormatNegativeResponse(data, KWP_NRC_SERVICE_NOT_SUPPORTED);
				break;
			}
		}
	}
}

#endif
