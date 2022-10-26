/*
 * ramn_kwp2000.h
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

// This Modules handles KWP2000 Diagnostics
// Currently a simplified implementation, for "KWP-only" features (i.e. no redundant with UDS)


#ifndef INC_RAMN_KWP2000_H_
#define INC_RAMN_KWP2000_H_

#include "main.h"

#if defined(ENABLE_KWP)

#include "ramn_isotp.h"
#include "ramn_j1979.h"
#include "ramn_dbc.h"

//ISO-TP Transport Layer Handler
extern RAMN_ISOTPHandler_t RAMN_KWP_ISOTPHandler;

//Initializes the module
RAMN_Result_t 	RAMN_KWP_Init(uint32_t tick);

//Updates the module. Should be called periodically.
RAMN_Result_t 	RAMN_KWP_Update(uint32_t tick);

//Update the TX part of the module. Returns true if a transmission is over.
RAMN_Bool_t 	RAMN_KWP_Continue_TX(uint32_t tick);

//Process a CAN Message addressed to KWP CAN ID. If a diagnostic message has been reconstructed, it is put in strbuf. Returns true if a message has been reconstructed.
RAMN_Bool_t		RAMN_KWP_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);

//Process a fully reconstructed ISO-TP Diag Frame.
void	 		RAMN_KWP_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);

#endif

#endif /* INC_RAMN_KWP2000_H_ */
