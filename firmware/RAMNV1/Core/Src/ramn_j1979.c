/*
 * ramn_j1979.c
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

#include "ramn_j1979.h"

#if defined(ENABLE_UDS) || defined(ENABLE_KWP)

#define J1979_DEFAULT_ERROR_CODE 0x11

RAMN_Result_t RAMN_J1979_ProcessMessage(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize)
{
	switch(data[0])
	{
	case 0x01: //Show Current Data
	case 0x02: //Show Freeze Data
	case 0x03: //Show stored Diagnostic Trouble Codes
	case 0x04: //Clear Diagnostic Trouble Codes and stored values
	case 0x05: //Test results, oxygen sensor monitoring (non CAN only)
	case 0x06: //Test results, other component/system monitoring (Test results, oxygen sensor monitoring for CAN only)
	case 0x07: //Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)
	case 0x08: //Control operation of on-board component/system
	case 0x09: //Request vehicle information
	case 0x0A: //Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)
	default:
		answerData[0] = 0x7F;
		answerData[1] = data[0];
		answerData[2] = J1979_DEFAULT_ERROR_CODE;
		*answerSize = 3; //TODO: implement padding with 0xAA
		break;
	}
	return RAMN_OK;
}

#endif
