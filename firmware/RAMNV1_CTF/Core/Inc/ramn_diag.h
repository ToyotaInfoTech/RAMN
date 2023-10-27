/*
 * ramn_diag.h
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

// This module is used to handle all diagnostics features (UDS, KWP, etc.) in a common module

#ifndef INC_RAMN_DIAG_H_
#define INC_RAMN_DIAG_H_

#include "main.h"

#if defined(ENABLE_DIAG)

#include "ramn_uds.h"
#ifdef ENABLE_KWP
#include "ramn_kwp2000.h"
#endif
#include "ramn_xcp.h"
#include "task.h"

//Function to init diagnostic module with default values.
RAMN_Result_t  	RAMN_DIAG_Init(uint32_t tick, osThreadId_t* pDiagRxTask, StreamBufferHandle_t* kwpbuf, StreamBufferHandle_t* udsbuf, StreamBufferHandle_t* xcpbuf);

//Function to call periodically to update the Diag module.
RAMN_Result_t 	RAMN_DIAG_Update(uint32_t tick);

//Function to call to process an incoming CAN message. Will not block for long and may be called from a high-priority task.
void 			RAMN_DIAG_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);

#endif

#endif /* INC_RAMN_DIAG_H_ */
