/*
 * ramn_serial_cmd.h
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

// Transport-agnostic serial command interface.
// This module provides a uniform API for sending serial output from CLI/sLCAN
// command processors regardless of the underlying physical transport
// (USB CDC or LPUART1).

#ifndef INC_RAMN_SERIAL_CMD_H_
#define INC_RAMN_SERIAL_CMD_H_

#include "main.h"

#if (defined(ENABLE_CDC) || defined(RAMN_SERIAL_CMD_TRANSPORT_LPUART1)) && defined(TARGET_ECUA)

#include "ramn_utils.h"

// Type definition for the serial send functions
typedef RAMN_Result_t (*RAMN_SerialSendFunc_t)(const uint8_t* data, uint32_t length);
typedef RAMN_Result_t (*RAMN_SerialSendStringFunc_t)(const char* data);
typedef void (*RAMN_SerialSendBlockingFunc_t)(uint8_t* data, uint32_t length);
typedef void (*RAMN_SerialAcquireLockFunc_t)(void);
typedef void (*RAMN_SerialReleaseLockFunc_t)(void);
typedef RAMN_Result_t (*RAMN_SerialSendLockedFunc_t)(const uint8_t* data, uint32_t length);
typedef RAMN_Result_t (*RAMN_SerialSendASCIIUint8Func_t)(uint8_t val);
typedef RAMN_Result_t (*RAMN_SerialSendASCIIUint16Func_t)(uint16_t val);
typedef RAMN_Result_t (*RAMN_SerialSendASCIIUint32Func_t)(uint32_t val);

// Backend function table
typedef struct {
	RAMN_SerialSendFunc_t            SendFromTask;
	RAMN_SerialSendStringFunc_t      SendStringFromTask;
	RAMN_SerialSendBlockingFunc_t    SendFromTask_Blocking;
	RAMN_SerialAcquireLockFunc_t     AcquireLock;
	RAMN_SerialReleaseLockFunc_t     ReleaseLock;
	RAMN_SerialSendLockedFunc_t      SendFromTask_Locked;
	RAMN_SerialSendASCIIUint8Func_t  SendASCIIUint8;
	RAMN_SerialSendASCIIUint16Func_t SendASCIIUint16;
	RAMN_SerialSendASCIIUint32Func_t SendASCIIUint32;
} RAMN_SerialBackend_t;

// Register the active serial backend. Must be called before any serial command processing.
void RAMN_Serial_RegisterBackend(const RAMN_SerialBackend_t* backend);

// Transport-agnostic send functions used by CLI/sLCAN command processors.
RAMN_Result_t RAMN_Serial_SendFromTask(const uint8_t* data, uint32_t length);
RAMN_Result_t RAMN_Serial_SendStringFromTask(const char* data);
void          RAMN_Serial_SendFromTask_Blocking(uint8_t* data, uint32_t length);
void          RAMN_Serial_AcquireLock(void);
void          RAMN_Serial_ReleaseLock(void);
RAMN_Result_t RAMN_Serial_SendFromTask_Locked(const uint8_t* data, uint32_t length);
RAMN_Result_t RAMN_Serial_SendASCIIUint8(uint8_t val);
RAMN_Result_t RAMN_Serial_SendASCIIUint16(uint16_t val);
RAMN_Result_t RAMN_Serial_SendASCIIUint32(uint32_t val);

// When UART transport is selected but USB is not compiled in, the command processor
// (ramn_cdc.c) still references RAMN_USB_Status_t/RAMN_USB_Config for slcan state.
// Provide the struct and extern here so it compiles without ramn_usb.h.
#if defined(RAMN_SERIAL_CMD_TRANSPORT_LPUART1) && !defined(ENABLE_USB)

typedef struct
{
	volatile RAMN_Bool_t slcanOpened;
	volatile RAMN_Bool_t serialOpened;
	volatile RAMN_Bool_t simulatorActive;
	volatile RAMN_Bool_t slcan_enableTimestamp;
	volatile RAMN_Bool_t autoreportErrors;
	volatile RAMN_Bool_t addESIFlag;
	volatile uint32_t    USBErrCnt;
	volatile uint32_t    USBTxOverflowCnt;
} RAMN_USB_Status_t;

extern RAMN_USB_Status_t RAMN_USB_Config;

#endif

#endif

#endif /* INC_RAMN_SERIAL_CMD_H_ */
