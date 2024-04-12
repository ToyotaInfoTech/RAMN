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

// This modules handles reading and writing of DTC (Data Trouble Codes)

#ifndef INC_RAMN_DTC_H_
#define INC_RAMN_DTC_H_

#include "main.h"

#ifdef ENABLE_EEPROM_EMULATION

#include "ramn_eeprom.h"

//Data format of DTC
#define DTC_FORMAT_IDENTIFIER 0x00

//Inits the module
RAMN_Result_t 	RAMN_DTC_Init();

//This function Enable/Disables the recording of new DTC. Required by UDS to prevent warnings during diagnostics.
void 			RAMN_DTC_SetRecordingStatus(RAMN_Bool_t status);

//Clears all DTC
RAMN_Result_t 	RAMN_DTC_ClearAll();

//Retrieve the DTC at specified index
RAMN_Result_t 	RAMN_DTC_GetIndex(uint32_t dtc_index, uint32_t* pval);

//Add a New DTC
RAMN_Result_t 	RAMN_DTC_AddNew(uint32_t dtc_val);

//Returns the current number of DTC in memory
RAMN_Result_t 	RAMN_DTC_GetNumberOfDTC(uint32_t* pDtcNum);

#endif

#endif /* INC_RAMN_DTC_H_ */
