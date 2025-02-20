/*
 * ramn_diag.c
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

#include "ramn_diag.h"

#if defined(ENABLE_DIAG)

static osThreadId_t* pTaskToNotify;
static StreamBufferHandle_t* udsbuf;
static StreamBufferHandle_t* kwpbuf;
static StreamBufferHandle_t* xcpbuf;

RAMN_Result_t RAMN_DIAG_Init(uint32_t tick, osThreadId_t* pDiagRxTask, StreamBufferHandle_t* pUDSbuf, StreamBufferHandle_t* pKWPbuf, StreamBufferHandle_t* pXCPbuf)
{
	pTaskToNotify = pDiagRxTask;
	udsbuf = pUDSbuf;
	kwpbuf = pKWPbuf;
	xcpbuf = pXCPbuf;
#ifdef ENABLE_EEPROM_EMULATION
	//Initialize Data Trouble Code Module
	RAMN_DTC_Init();
#endif
#ifdef ENABLE_UDS
	RAMN_UDS_Init(tick);
#endif
#ifdef ENABLE_KWP
	RAMN_KWP_Init(tick);
#endif
#ifdef ENABLE_XCP
	RAMN_XCP_Init(tick);
#endif
	return RAMN_OK;
}

RAMN_Result_t RAMN_DIAG_Update(uint32_t tick)
{
	RAMN_Result_t result = RAMN_OK;
#if defined(ENABLE_UDS)
	result |= RAMN_UDS_Update(tick);
#endif
#if defined(ENABLE_KWP)
	result |= RAMN_KWP_Update(tick);
#endif
#if defined(ENABLE_XCP)
	result |= RAMN_XCP_Update(tick);
#endif
	return result;
}

void RAMN_DIAG_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
#if defined(ENABLE_UDS)
	if (RAMN_UDS_ProcessRxCANMessage(pHeader, data, tick, udsbuf) == True)
	{
		xTaskNotifyGive(*pTaskToNotify);
	}
#endif
#if defined(ENABLE_KWP)
	if (RAMN_KWP_ProcessRxCANMessage(pHeader, data, tick, kwpbuf) == True)
	{
		xTaskNotifyGive(*pTaskToNotify);
	}
#endif
#if defined(ENABLE_XCP)
	if (RAMN_XCP_ProcessRxCANMessage(pHeader, data, tick, xcpbuf) == True)
	{
		xTaskNotifyGive(*pTaskToNotify);
	}
#endif
}



#endif
