/*
 * ramn_debug.c
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

#include "ramn_debug.h"

#if defined(ENABLE_USB)

void RAMN_DEBUG_ReportCANStats(const RAMN_FDCAN_Status_t* local_gw)
{
	RAMN_USB_SendStringFromTask("I");
	RAMN_USB_SendASCIIUint32(local_gw->CANTXRequestCnt);
	RAMN_USB_SendASCIIUint32(local_gw->CANTXSentCnt);
	RAMN_USB_SendASCIIUint32(local_gw->CANRXCnt);
	RAMN_USB_SendASCIIUint32(local_gw->CANRxOverrunCnt);
	RAMN_USB_SendASCIIUint32(RAMN_USB_Config.USBErrCnt);
	RAMN_USB_SendStringFromTask("\r");
}

void RAMN_DEBUG_DumpCANErrorRegisters(const FDCAN_ErrorCountersTypeDef* pErrCnt, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus)
{
	RAMN_USB_SendStringFromTask("E");

	// First, send FDCAN Error flag. 1 Integer only (cf HAL_FDCAN_Error_Code)
	RAMN_USB_SendASCIIUint32(RAMN_FDCAN_Status.prevCANError);

	// Send the error counters. 4 Integers (cf FDCAN_ErrorCountersTypeDef)
	RAMN_USB_SendASCIIUint32(pErrCnt->TxErrorCnt);
	RAMN_USB_SendASCIIUint32(pErrCnt->RxErrorCnt);
	RAMN_USB_SendASCIIUint32(pErrCnt->RxErrorPassive);
	RAMN_USB_SendASCIIUint32(pErrCnt->ErrorLogging);

	// Send Protocol Status integer. 11 Integers.
	RAMN_USB_SendASCIIUint32(pProtocolStatus->LastErrorCode);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->DataLastErrorCode);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->Activity);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->ErrorPassive);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->Warning);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->BusOff);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->RxESIflag);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->RxBRSflag);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->RxFDFflag);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->ProtocolException);
	RAMN_USB_SendASCIIUint32(pProtocolStatus->TDCvalue);

	RAMN_USB_SendStringFromTask("\r");
}

#if defined(ENABLE_USB_DEBUG)

static RAMN_Bool_t RAMN_DEBUG_ENABLE = False;

void RAMN_DEBUG_SetStatus(RAMN_Bool_t status)
{
	RAMN_DEBUG_ENABLE = status;
}

inline void RAMN_DEBUG_Log(const char* src)
{
	if (RAMN_DEBUG_ENABLE == True) RAMN_USB_SendFromTask((uint8_t*)src,RAMN_strlen(src));
}

void	RAMN_DEBUG_PrintCANError(const FDCAN_ErrorCountersTypeDef* pErrorCount, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus, const RAMN_FDCAN_Status_t* pGw_freeze, uint32_t err)
{
	RAMN_DEBUG_Log("d ------------------------------------------------ \r");
	RAMN_DEBUG_Log("d ERROR ! Registers dumped below\r");
	if (RAMN_USB_Config.autoreportErrors != 0U)
	{
		RAMN_DEBUG_DumpCANErrorRegisters(pErrorCount, pProtocolStatus);
		RAMN_DEBUG_ReportCANStats(pGw_freeze);
	}

	if (err != HAL_FDCAN_ERROR_NONE)
	{
		RAMN_DEBUG_Log("d ");
		if (err & HAL_FDCAN_ERROR_TIMEOUT) 			RAMN_DEBUG_Log("HAL_FDCAN_ERROR_TIMEOUT ");
		if (err & HAL_FDCAN_ERROR_NOT_INITIALIZED) 	RAMN_DEBUG_Log("HAL_FDCAN_ERROR_NOT_INITIALIZED ");
		if (err & HAL_FDCAN_ERROR_NOT_READY)		RAMN_DEBUG_Log("HAL_FDCAN_ERROR_NOT_READY ");
		if (err & HAL_FDCAN_ERROR_NOT_STARTED)		RAMN_DEBUG_Log("HAL_FDCAN_ERROR_NOT_STARTED ");
		if (err & HAL_FDCAN_ERROR_NOT_SUPPORTED)	RAMN_DEBUG_Log("HAL_FDCAN_ERROR_NOT_SUPPORTED ");
		if (err & HAL_FDCAN_ERROR_PARAM)			RAMN_DEBUG_Log("HAL_FDCAN_ERROR_PARAM ");
		if (err & HAL_FDCAN_ERROR_PENDING)			RAMN_DEBUG_Log("HAL_FDCAN_ERROR_PENDING ");
		if (err & HAL_FDCAN_ERROR_RAM_ACCESS)		RAMN_DEBUG_Log("HAL_FDCAN_ERROR_RAM_ACCESS ");
		if (err & HAL_FDCAN_ERROR_FIFO_EMPTY)		RAMN_DEBUG_Log("HAL_FDCAN_ERROR_FIFO_EMPTY ");
		if (err & HAL_FDCAN_ERROR_FIFO_FULL)		RAMN_DEBUG_Log("HAL_FDCAN_ERROR_FIFO_FULL ");
		if (err & HAL_FDCAN_ERROR_LOG_OVERFLOW)		RAMN_DEBUG_Log("HAL_FDCAN_ERROR_LOG_OVERFLOW ");
		if (err & HAL_FDCAN_ERROR_RAM_WDG)			RAMN_DEBUG_Log("HAL_FDCAN_ERROR_RAM_WDG ");
		RAMN_DEBUG_Log("\r");

		if (err & HAL_FDCAN_ERROR_PROTOCOL_ARBT) // CAN-FD Protocol Error during arbitration phase
		{
			uint32_t errorCode = pProtocolStatus->LastErrorCode;

			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_PROTOCOL_ARBT -> ");

			if (errorCode == FDCAN_PROTOCOL_ERROR_STUFF) 		RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_STUFF ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_FORM)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_FORM ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_ACK)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_ACK ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT1)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT1 ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT0)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT0 ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_CRC)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_CRC ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_NO_CHANGE)	RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_NO_CHANGE "); //No change since last read
		}
		RAMN_DEBUG_Log("\r");

		if (err & HAL_FDCAN_ERROR_PROTOCOL_DATA) // CAN-FD Protocol Error during data phase
		{
			uint32_t errorCode = pProtocolStatus->DataLastErrorCode;

			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_PROTOCOL_DATA -> ");

			if (errorCode == FDCAN_PROTOCOL_ERROR_STUFF) 		RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_STUFF ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_FORM)  		RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_FORM ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_ACK)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_ACK ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT1)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT1 ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT0)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT0 ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_CRC)			RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_CRC ");
			if (errorCode == FDCAN_PROTOCOL_ERROR_NO_CHANGE)	RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_NO_CHANGE ");
			RAMN_DEBUG_Log("\r");
		}

		if (err & HAL_FDCAN_ERROR_RAM_WDG)	RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_RAM_WDG \r");

		RAMN_DEBUG_Log("d TX ERRORS: 0x");
		RAMN_USB_SendASCIIUint8((pErrorCount->TxErrorCnt)&0xFF);

		RAMN_DEBUG_Log(" (0x");
		RAMN_USB_SendASCIIUint8((pErrorCount->TxErrorCnt >> 3)&0xFF);

		RAMN_DEBUG_Log(" frames) RX ERRORS: 0x");
		RAMN_USB_SendASCIIUint8(pErrorCount->RxErrorCnt&0xFF);

		RAMN_DEBUG_Log(" ERR LOG: 0x");
		RAMN_USB_SendASCIIUint8(pErrorCount->ErrorLogging&0xFF);

		RAMN_DEBUG_Log(" COMP: 0x");
		RAMN_USB_SendASCIIUint8(pProtocolStatus->TDCvalue&0xFF);

		RAMN_DEBUG_Log("\r");
		if (pErrorCount->RxErrorPassive != 0U) RAMN_DEBUG_Log("d ! RX Error has reached error passive level of 128 !\r");

		RAMN_DEBUG_Log("d Current State: ");
		if (pProtocolStatus->Activity 		== FDCAN_COM_STATE_SYNC) 	RAMN_DEBUG_Log("Node is synchronizing on CAN communication");
		else if (pProtocolStatus->Activity 	== FDCAN_COM_STATE_IDLE) 	RAMN_DEBUG_Log("Node is neither receiver nor transmitter");
		else if (pProtocolStatus->Activity 	== FDCAN_COM_STATE_RX) 		RAMN_DEBUG_Log("Node is operating as receiver");
		else if (pProtocolStatus->Activity 	== FDCAN_COM_STATE_TX) 		RAMN_DEBUG_Log("Node is operating as transmitter");
		else  															RAMN_DEBUG_Log("Invalid communication state");
		RAMN_DEBUG_Log("\r");

		if (pProtocolStatus->ErrorPassive != 0U)		RAMN_DEBUG_Log("d The FDCAN is in Error_Passive state\r");
		if (pProtocolStatus->Warning != 0U) 			RAMN_DEBUG_Log("d At least one of the error counters has reached the Error_Warning limit of 96\r");
		if (pProtocolStatus->BusOff != 0U) 				RAMN_DEBUG_Log("d The FDCAN is in Bus_Off state\r");
		if (pProtocolStatus->RxESIflag == 1U) 			RAMN_DEBUG_Log("d Last received CAN FD message had its ESI flag set\r");
		if (pProtocolStatus->RxBRSflag == 1U) 			RAMN_DEBUG_Log("d Last received CAN FD message had its BRS flag set\r");
		if (pProtocolStatus->RxFDFflag == 1U) 			RAMN_DEBUG_Log("d New message received since last protocol status\r");
		if (pProtocolStatus->ProtocolException == 1U) 	RAMN_DEBUG_Log("d Protocol exception event occurred\r");
	}
	else RAMN_DEBUG_Log("d CAN BUS OK\r");

	if (pGw_freeze->CANRxOverrunCnt > 0) RAMN_DEBUG_Log("d Buffer overrun reported on CAN Reception\r");
	if (RAMN_USB_Config.USBErrCnt > 0  ) RAMN_DEBUG_Log("d Error reported by USB module (overflow?) \r");

	RAMN_DEBUG_Log("d ------------------------------------------------ \r\r");
}
#endif
#endif
