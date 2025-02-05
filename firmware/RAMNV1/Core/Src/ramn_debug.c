/*
 * ramn_debug.c
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

#include "ramn_debug.h"

#if defined(ENABLE_USB)
#include <string.h>


#if defined(DEBUG)
static RAMN_Bool_t RAMN_DEBUG_ENABLE = False;
#else
static RAMN_Bool_t RAMN_DEBUG_ENABLE = False;
#endif

void RAMN_DEBUG_SetStatus(RAMN_Bool_t status)
{
	RAMN_DEBUG_ENABLE = status;
}

void RAMN_DEBUG_ReportCANStats(const RAMN_FDCAN_Status_t* local_gw)
{
	uint8_t usbSendBuffer[256];
	uint32_t index = 0U;
	usbSendBuffer[index++] = 'I';

	index += uint32toASCII(local_gw->CANTXRequestCnt,&usbSendBuffer[index]);
	index += uint32toASCII(local_gw->CANTXSentCnt,&usbSendBuffer[index]);
	index += uint32toASCII(local_gw->CANRXCnt,&usbSendBuffer[index]);
	index += uint32toASCII(local_gw->CANRxOverrunCnt ,&usbSendBuffer[index]);
	index += uint32toASCII(RAMN_USB_Config.USBErrCnt,&usbSendBuffer[index]);

	usbSendBuffer[index++] = '\r';
	RAMN_USB_SendFromTask(usbSendBuffer,index);
}


inline void RAMN_DEBUG_Log(const char* src)
{
#if defined(ENABLE_USB)
	if (RAMN_DEBUG_ENABLE == True) RAMN_USB_SendFromTask((uint8_t*)src,strlen(src));
#endif
}

#if defined(ENABLE_USB)

void RAMN_DEBUG_DumpCANErrorRegisters(const FDCAN_ErrorCountersTypeDef* pErrCnt, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus)
{
	uint32_t index = 0U;
	uint8_t usbSendBuffer[256];

	usbSendBuffer[index++] = 'E';
	//First, send FDCAN Error flag. 1 Integer only (cf HAL_FDCAN_Error_Code)
	index += uint32toASCII(RAMN_FDCAN_Status.prevCANError,&usbSendBuffer[index]);

	//Send the error counters. 4 Integers(cf FDCAN_ErrorCountersTypeDef)
	index += uint32toASCII(pErrCnt->TxErrorCnt,&usbSendBuffer[index]);
	index += uint32toASCII(pErrCnt->RxErrorCnt,&usbSendBuffer[index]);
	index += uint32toASCII(pErrCnt->RxErrorPassive,&usbSendBuffer[index]);
	index += uint32toASCII(pErrCnt->ErrorLogging,&usbSendBuffer[index]);

	//Send Protocol Status integer. 11 Integers. TODO: optimize ? lots of unused data.
	index += uint32toASCII(pProtocolStatus->LastErrorCode,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->DataLastErrorCode,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->Activity,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->ErrorPassive,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->Warning,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->BusOff,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->RxESIflag,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->RxBRSflag,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->RxFDFflag,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->ProtocolException,&usbSendBuffer[index]);
	index += uint32toASCII(pProtocolStatus->TDCvalue,&usbSendBuffer[index]);

	usbSendBuffer[index++] = '\r';
	RAMN_USB_SendFromTask(usbSendBuffer,index);
}

void	RAMN_DEBUG_PrintCANError(const FDCAN_ErrorCountersTypeDef* pErrorCount, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus, const RAMN_FDCAN_Status_t* pGw_freeze, uint32_t err)
{
	uint8_t smallTempBuffer[32];
	RAMN_DEBUG_Log("d ------------------------------------------------ \r");
	RAMN_DEBUG_Log("d ERROR ! Registers dumped below\r");
	if (RAMN_USB_Config.autoreportErrors != 0U)
	{
		RAMN_DEBUG_DumpCANErrorRegisters(pErrorCount, pProtocolStatus);
		RAMN_DEBUG_ReportCANStats(pGw_freeze);
	}

	if (err != HAL_FDCAN_ERROR_NONE)
	{
		if (err & HAL_FDCAN_ERROR_TIMEOUT)
		{
			//timeout error
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_TIMEOUT\r");
			//Error_Handler();
		}
		if (err & HAL_FDCAN_ERROR_NOT_INITIALIZED)
		{
			//CAN-FD Peripheral is not initialized
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_NOT_INITIALIZED\r");
			//Error_Handler();
		}
		if (err & HAL_FDCAN_ERROR_NOT_READY)
		{
			//CAN-FD Peripheral is not ready
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_NOT_READY\r");
			//Error_Handler();
		}
		if (err & HAL_FDCAN_ERROR_NOT_STARTED)
		{
			//CAN-FD Peripheral is not started
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_NOT_STARTED\r");
			//Error_Handler();
		}
		if (err & HAL_FDCAN_ERROR_NOT_SUPPORTED)
		{
			//CAN-FD Peripheral required mode is not supported
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_NOT_SUPPORTED\r");
			//Error_Handler();
		}
		if (err & HAL_FDCAN_ERROR_PARAM)
		{
			//CAN-FD Peripheral had wrong parameters
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_PARAM\r");
			//Error_Handler();
		}
		if (err & HAL_FDCAN_ERROR_PENDING)
		{
			//CAN-FD Peripheral operation is pending
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_PENDING\r");
			//Error_Handler();
		}
		if (err & HAL_FDCAN_ERROR_RAM_ACCESS)
		{
			//CAN-FD Peripheral RAM Access failure
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_RAM_ACCESS\r");
			//Error_Handler();
		}

		if (err & HAL_FDCAN_ERROR_FIFO_EMPTY)
		{
			//CAN-FD FIFO is empty (or full ? source code unclear)
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_FIFO_EMPTY\r");
		}
		if (err & HAL_FDCAN_ERROR_FIFO_FULL)
		{
			//CAN-FD FIFO is full (or empty ? source code unclear)
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_FIFO_FULL\r");
		}

		if (err & HAL_FDCAN_ERROR_LOG_OVERFLOW)
		{
			//CAN-FD error counter flow - not a real problem
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_LOG_OVERFLOW\r");
		}
		if (err & HAL_FDCAN_ERROR_RAM_WDG)
		{
			//CAN-FD RAM Watchdog event
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_RAM_WDG\r");
		}
		if (err & HAL_FDCAN_ERROR_PROTOCOL_ARBT)
		{
			//CAN-FD Protocol Error during arbitration phase
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_PROTOCOL_ARBT -> ");
			RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_BUS_ERROR;

			uint32_t errorCode = pProtocolStatus->LastErrorCode;
			if (errorCode == FDCAN_PROTOCOL_ERROR_STUFF)
			{
				//Stuffing Error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_STUFF\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_FORM)
			{
				//Form Error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_FORM\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_ACK)
			{
				//Acknowledge Error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_ACK\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT1)
			{
				// Bit 1 (recessive) error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT1\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT0)
			{
				// Bit 0 (dominant) error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT0\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_CRC)
			{
				// CRC error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_CRC\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_NO_CHANGE)
			{
				//No change since last read
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_NO_CHANGE\r");
			}
		}
		if (err & HAL_FDCAN_ERROR_PROTOCOL_DATA)
		{
			//CAN-FD Protocol Error during data phase
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_PROTOCOL_DATA -> ");

			RAMN_FDCAN_Status.slcanFlags |= SLCAN_FLAG_BUS_ERROR;
			uint32_t errorCode = pProtocolStatus->DataLastErrorCode;
			if (errorCode == FDCAN_PROTOCOL_ERROR_STUFF)
			{
				//Stuffing Error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_STUFF\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_FORM)
			{
				//Form Error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_FORM\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_ACK)
			{
				//Acknowledge Error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_ACK\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT1)
			{
				// Bit 1 (recessive) error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT1\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_BIT0)
			{
				// Bit 0 (dominant) error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_BIT0\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_CRC)
			{
				// CRC error
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_CRC\r");
			}
			if (errorCode == FDCAN_PROTOCOL_ERROR_NO_CHANGE)
			{
				//No change since last read
				RAMN_DEBUG_Log("FDCAN_PROTOCOL_ERROR_NO_CHANGE\r");
			}
		}
		if (err & HAL_FDCAN_ERROR_RAM_WDG)
		{
			//CAN-FD RAM Watchdog event
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_RAM_WDG\r");
		}
		if (err & HAL_FDCAN_ERROR_RAM_WDG)
		{
			//CAN-FD RAM Watchdog event
			RAMN_DEBUG_Log("d HAL_FDCAN_ERROR_RAM_WDG\r");
		}

		smallTempBuffer[2] = '\0'; //Terminate string for all subsequent byte writes

		uint8toASCII((pErrorCount->TxErrorCnt)&0xFF,smallTempBuffer); //TODO: Need to divide by 8 to get actual TX errors because of a bug, or maybe there are 8 retries by default ?
		RAMN_DEBUG_Log("d TX ERRORS: 0x");
		RAMN_DEBUG_Log((char*)smallTempBuffer);

		uint8toASCII((pErrorCount->TxErrorCnt >> 3)&0xFF,smallTempBuffer); //TODO: Need to divide by 8 to get actual TX errors because of a bug, or maybe there are 8 retries by default ?
		RAMN_DEBUG_Log(" (0x");
		RAMN_DEBUG_Log((char*)smallTempBuffer);
		uint8toASCII(pErrorCount->RxErrorCnt&0xFF,smallTempBuffer);
		RAMN_DEBUG_Log(" frames) RX ERRORS: 0x");
		RAMN_DEBUG_Log((char*)smallTempBuffer);

		uint8toASCII(pErrorCount->ErrorLogging&0xFF,smallTempBuffer);
		RAMN_DEBUG_Log(" ERR LOG: 0x");
		RAMN_DEBUG_Log((char*)smallTempBuffer);

		uint8toASCII(pProtocolStatus->TDCvalue&0xFF,smallTempBuffer);
		RAMN_DEBUG_Log(" COMP: 0x");
		RAMN_DEBUG_Log((char*)smallTempBuffer);

		RAMN_DEBUG_Log("\r");
		if (pErrorCount->RxErrorPassive != 0U) RAMN_DEBUG_Log("! RX Error has reached error passive level of 128 !\r");

		RAMN_DEBUG_Log("d Current State: ");
		if (pProtocolStatus->Activity == FDCAN_COM_STATE_SYNC  ) RAMN_DEBUG_Log("Node is synchronizing on CAN communication\r");
		else if (pProtocolStatus->Activity == FDCAN_COM_STATE_IDLE  ) RAMN_DEBUG_Log("Node is neither receiver nor transmitter\r");
		else if (pProtocolStatus->Activity == FDCAN_COM_STATE_RX    ) RAMN_DEBUG_Log("Node is operating as receiver\r");
		else if (pProtocolStatus->Activity == FDCAN_COM_STATE_TX    ) RAMN_DEBUG_Log("Node is operating as transmitter\r");
		else  RAMN_DEBUG_Log("Invalid communication state\r");

		if (pProtocolStatus->ErrorPassive != 0U)RAMN_DEBUG_Log("d The FDCAN is in Error_Passive state\r");

		if (pProtocolStatus->Warning != 0U) RAMN_DEBUG_Log("d At least one of the error counters has reached the Error_Warning limit of 96\r");

		if (pProtocolStatus->BusOff != 0U) RAMN_DEBUG_Log("d The FDCAN is in Bus_Off state\r");

		if (pProtocolStatus->RxESIflag == 1U) RAMN_DEBUG_Log("d Last received CAN FD message had its ESI flag set\r");

		if (pProtocolStatus->RxBRSflag == 1U) RAMN_DEBUG_Log("d Last received CAN FD message had its BRS flag set\r");

		if (pProtocolStatus->RxFDFflag == 1U) RAMN_DEBUG_Log("d New message received since last protocol status\r");

		if (pProtocolStatus->ProtocolException == 1U) RAMN_DEBUG_Log("d Protocol exception event occurred \r");
	}
	else
	{
		RAMN_DEBUG_Log("d CAN BUS OK\r");
	}

	if (pGw_freeze->CANRxOverrunCnt > 0) RAMN_DEBUG_Log("Buffer overrun reported on CAN Reception\r");
	if (RAMN_USB_Config.USBErrCnt > 0  ) RAMN_DEBUG_Log("Error reported by USB module (overflow?) \r");

	RAMN_DEBUG_Log("d ------------------------------------------------ \r\r");
}
#endif

#endif
