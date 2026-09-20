/*
 * ramn_serial_cmd.c
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

#include "ramn_serial_cmd.h"

#if (defined(ENABLE_CDC) || defined(RAMN_SERIAL_CMD_TRANSPORT_LPUART1)) && defined(TARGET_ECUA)

static const RAMN_SerialBackend_t* activeBackend = NULL;

#if defined(RAMN_SERIAL_CMD_TRANSPORT_LPUART1) && !defined(ENABLE_USB)
// Provide RAMN_USB_Config instance when USB module is not compiled in.
RAMN_USB_Status_t RAMN_USB_Config =
{
	.slcanOpened            = False,
	.serialOpened           = True,
	.simulatorActive        = False,
	.slcan_enableTimestamp   = False,
	.autoreportErrors       = False,
	.addESIFlag             = False,
	.USBErrCnt              = 0U,
	.USBTxOverflowCnt       = 0U,
};
#endif

void RAMN_Serial_RegisterBackend(const RAMN_SerialBackend_t* backend)
{
	activeBackend = backend;
}

RAMN_Result_t RAMN_Serial_SendFromTask(const uint8_t* data, uint32_t length)
{
	if (activeBackend != NULL && activeBackend->SendFromTask != NULL)
		return activeBackend->SendFromTask(data, length);
	return RAMN_ERROR;
}

RAMN_Result_t RAMN_Serial_SendStringFromTask(const char* data)
{
	if (activeBackend != NULL && activeBackend->SendStringFromTask != NULL)
		return activeBackend->SendStringFromTask(data);
	return RAMN_ERROR;
}

void RAMN_Serial_SendFromTask_Blocking(uint8_t* data, uint32_t length)
{
	if (activeBackend != NULL && activeBackend->SendFromTask_Blocking != NULL)
		activeBackend->SendFromTask_Blocking(data, length);
}

void RAMN_Serial_AcquireLock(void)
{
	if (activeBackend != NULL && activeBackend->AcquireLock != NULL)
		activeBackend->AcquireLock();
}

void RAMN_Serial_ReleaseLock(void)
{
	if (activeBackend != NULL && activeBackend->ReleaseLock != NULL)
		activeBackend->ReleaseLock();
}

RAMN_Result_t RAMN_Serial_SendFromTask_Locked(const uint8_t* data, uint32_t length)
{
	if (activeBackend != NULL && activeBackend->SendFromTask_Locked != NULL)
		return activeBackend->SendFromTask_Locked(data, length);
	return RAMN_ERROR;
}

RAMN_Result_t RAMN_Serial_SendASCIIUint8(uint8_t val)
{
	if (activeBackend != NULL && activeBackend->SendASCIIUint8 != NULL)
		return activeBackend->SendASCIIUint8(val);
	return RAMN_ERROR;
}

RAMN_Result_t RAMN_Serial_SendASCIIUint16(uint16_t val)
{
	if (activeBackend != NULL && activeBackend->SendASCIIUint16 != NULL)
		return activeBackend->SendASCIIUint16(val);
	return RAMN_ERROR;
}

RAMN_Result_t RAMN_Serial_SendASCIIUint32(uint32_t val)
{
	if (activeBackend != NULL && activeBackend->SendASCIIUint32 != NULL)
		return activeBackend->SendASCIIUint32(val);
	return RAMN_ERROR;
}

#endif
