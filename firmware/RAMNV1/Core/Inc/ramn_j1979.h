/*
 * ramn_j1979.h
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

// This Modules handles J1979 (OBD-II) Diagnostics
// Currently a dummy File for future implementations

#ifndef INC_RAMN_J1979_H_
#define INC_RAMN_J1979_H_

#include "main.h"



#if defined(ENABLE_UDS) || defined(ENABLE_KWP)

#include "ramn_isotp.h"

#ifdef ENABLE_J1979
#include "ramn_dbc.h"
#include "ramn_sensors.h"
#ifdef ENABLE_EEPROM_EMULATION
#include "ramn_dtc.h"
#endif
#endif

#define J1979_NRC_GR 		0x10 // General Reject
#define J1979_NRC_SNS 		0x11 // Service Not Supported
#define J1979_NRC_SFNSIF 	0x12 // Subfunction not supported / Invalid Format
#define J1979_NRC_BRR 		0x21 // Busy - Repeat Request
#define J1979_NRC_CNCORSE 	0x22 // Conditions Not Correct / Request Sequence Error
#define J1979_RP			0x78 // Request correctly received / Response Pending

// Processes a received J1979 message.
RAMN_Result_t RAMN_J1979_ProcessMessage(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);

#endif

#endif /* INC_RAMN_J1979_H_ */
