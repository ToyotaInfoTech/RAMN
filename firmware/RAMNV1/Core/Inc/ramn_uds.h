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
#if defined(TARGET_ECUB) || defined(TARGET_ECUC) || defined(TARGET_ECUD)
#include "ramn_simulator.h"
#endif
#if defined(ENABLE_EEPROM_EMULATION)
#include "ramn_dtc.h"
#endif
#if defined(ENABLE_MINICTF)
#include "ramn_ctf.h"
#endif

//UDS Error Codes
#define UDS_NRC_GR   	0x10  	//General Reject
#define UDS_NRC_SNS  	0x11 	//Service not supported
#define UDS_NRC_SFNS 	0x12 	//Sub-Function not supported
#define UDS_NRC_IMLOIF 	0x13	//Incorrect message length or invalid format
#define UDS_NRC_RTL 	0x14 	//Response too long
#define UDS_NRC_BRR 	0x21	//Busy repeat request
#define UDS_NRC_CNC 	0x22	//Conditions not correct
#define UDS_NRC_RSE 	0x24	//Request sequence error
#define UDS_NRC_NRFSC 	0x25	//No response from sub-net component
#define UDS_NRC_FPEORA 	0x26	//Failure prevents execution of requested action
#define UDS_NRC_ROOR 	0x31	//Request out of range
#define UDS_NRC_SAD 	0x33	//Security access denied
#define UDS_NRC_IK 		0x35	//Invalid key
#define UDS_NRC_ENOA 	0x36	//Exceeded number of attempts
#define UDS_NRC_RTDNE 	0x37	//Required time delay not expired
#define UDS_NRC_RBEDLSD 0x38	//Reserved by Extended Data Link Security Document
#define UDS_NRC_UDNA 	0x70	//Upload/Download not accepted
#define UDS_NRC_TDS 	0x71	//Transfer data suspended
#define UDS_NRC_GPF 	0x72	//General programming failure
#define UDS_NRC_WBSC 	0x73	//Wrong Block Sequence Counter
#define UDS_NRC_RCRRP 	0x78	//Request correctly received, but response is pending
#define UDS_NRC_SFNSIAS 0x7E	//Sub-Function not supported in active session
#define UDS_NRC_SNSIAS 	0x7F	//Service not supported in active session
#define UDS_NRC_VSTH    0x88    //Vehicle Speed too High


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

//Same as above, but for frames received with functional addressing (command broadcast)
void RAMN_UDS_ProcessDiagPayloadFunctional(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);

//Performs an action after the ECU sent the UDS answer. Used by services that cannot answer after execution, e.g., because they reset the board.
void 			RAMN_UDS_PerformPostAnswerActions(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);


#endif
#endif /* INC_RAMN_UDS_H_ */
