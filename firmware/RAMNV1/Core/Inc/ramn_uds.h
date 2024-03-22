/*
 * ramn_uds.h
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

// This module implements UDS Diagnostics

#ifndef INC_RAMN_UDS_H_
#define INC_RAMN_UDS_H_

#include "main.h"

#if defined(ENABLE_UDS)

#include "ramn_isotp.h"
#include <ramn_memory.h>
#include "ramn_j1979.h"
#include "ramn_dbc.h"
#include "ramn_trng.h"
#include "ramn_crc.h"
#include "ramn_screen.h"
#if defined(ENABLE_EEPROM_EMULATION)
#include "ramn_dtc.h"
#endif

//Timeout of diagnostic session
#define UDS_SESSION_TIMEOUT_MS 				5000

//Maximum RPM that will allow a transition to diagnostic sessions
#define UDS_MAXIMUM_RPM_ACCEPTABLE 			10

//Maximum Timeout before accepting Security Access Attempt
#define SECURITY_ACCESS_RETRY_TIMEOUT_MS 	10

//Maximum Security access attempts before
#define SECURITY_ACCESS_MAX_ATTEMPTS 		5

//Maximum Blocksize for Transfer Data function
#define TRANSFER_DATA_BLOCKSIZE 			0xFF0

//ISO-TP Transport Layer Handler
extern RAMN_ISOTPHandler_t RAMN_UDS_ISOTPHandler;

//Initializes the module
RAMN_Result_t 	RAMN_UDS_Init(uint32_t tick);

//Updates the module. Should be called periodically.
RAMN_Result_t 	RAMN_UDS_Update(uint32_t tick);

//Update the TX part of the module. Returns true if a transmission is over.
RAMN_Bool_t 	RAMN_UDS_Continue_TX(uint32_t tick);

//Process a CAN Message addressed to UDS CAN ID. If a diagnostic message has been reconstructed, it is put in strbuf. Returns true if a message has been reconstructed.
RAMN_Bool_t		RAMN_UDS_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);

//Process a fully reconstructed ISO-TP Diag Frame.
void	 		RAMN_UDS_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);



#endif
#endif /* INC_RAMN_UDS_H_ */
