/*
 * ramn_debug.h
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

// This module handles debug features, mainly outputting messages on a serial terminal

#ifndef INC_RAMN_DEBUG_H_
#define INC_RAMN_DEBUG_H_

#include "main.h"
#include "ramn_usb.h"
#include "ramn_canfd.h"

#if defined(ENABLE_USB)
//Function To Enable/Disable Debugging features
void 	RAMN_DEBUG_SetStatus(RAMN_Bool_t status);

//Function to dump current CAN statistics over USB
void 	RAMN_DEBUG_ReportCANStats(const RAMN_FDCAN_Status_t* local_gw);

//Function to log a message (typically, to a USB serial port)
//This function is blocking and not thread-safe
void 	RAMN_DEBUG_Log(const char* src);

//Function to dump Error Registers over USB (non-human readable)
void 	RAMN_DEBUG_DumpCANErrorRegisters(const FDCAN_ErrorCountersTypeDef* pErrCnt, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus);

//Function to display information about CAN Error (human readable)
void	RAMN_DEBUG_PrintCANError(const FDCAN_ErrorCountersTypeDef* pErrorCount, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus, const RAMN_FDCAN_Status_t* pGw_freeze, uint32_t err);


#endif

#endif /* INC_RAMN_DEBUG_H_ */
