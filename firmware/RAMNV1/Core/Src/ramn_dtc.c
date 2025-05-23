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

#include "ramn_dtc.h"

#ifdef ENABLE_EEPROM_EMULATION

static RAMN_Bool_t recordingStatus = True; //Set this variable to False to disable DTC recording

void RAMN_DTC_SetRecordingStatus(RAMN_Bool_t status)
{
	recordingStatus = status;
}

RAMN_Result_t RAMN_DTC_Init()
{
	RAMN_Result_t result = RAMN_OK;
	uint32_t dtcNum;
	if (RAMN_EEPROM_Read32(DTC_NUMBER_INDEX,&dtcNum) != EE_OK)
	{
		//Could not read DTC, Try to write a default zero value
		if (RAMN_EEPROM_Write32(DTC_NUMBER_INDEX,0U) != EE_OK)
		{
			result = RAMN_ERROR;
		}
	}
	return result;
}

RAMN_Result_t RAMN_DTC_ClearAll()
{
	RAMN_Result_t result;
	uint32_t numDTC;

	if (RAMN_DTC_GetNumberOfDTC(&numDTC) == RAMN_OK)
	{
		if (numDTC > 0) // Do nothing if number of DTC is already zero
		{
			if (RAMN_EEPROM_Write32(DTC_NUMBER_INDEX, 0U) != EE_OK)
			{
				result =  RAMN_ERROR;
			}
			else
			{
				result =  RAMN_OK;
			}
		} else result = RAMN_OK;
	} else result = RAMN_ERROR;
	return result;
}

RAMN_Result_t RAMN_DTC_GetIndex(uint32_t dtc_index, uint32_t* pval)
{
	RAMN_Result_t result = RAMN_ERROR;
	uint32_t dtcNum = 0U;

	if (RAMN_EEPROM_Read32(DTC_NUMBER_INDEX,&dtcNum) == EE_OK)
	{
		if(dtc_index < dtcNum)
		{
			if (RAMN_EEPROM_Read32(DTC_NUMBER_INDEX+dtc_index+1,pval) == EE_OK)
			{
				result = RAMN_OK;
			}
		}
	}
	return result;
}

RAMN_Result_t RAMN_DTC_AddNew(uint32_t dtc_val)
{
	uint32_t dtcNum = 0U;
	RAMN_Result_t result = RAMN_ERROR;

	if (recordingStatus == False)
	{
		//Do not register new DTC, just answer yes
		result = RAMN_OK;
	}
	else if (RAMN_EEPROM_Read32(DTC_NUMBER_INDEX,&dtcNum) == EE_OK)
	{
		if ((DTC_NUMBER_INDEX + dtcNum + 1) <= DTC_LAST_VALID_ADDRESS)
		{
			if (RAMN_EEPROM_Write32((uint16_t)(DTC_NUMBER_INDEX + dtcNum + 1),(uint32_t)dtc_val) == EE_OK)
			{
				if (RAMN_EEPROM_Write32((uint16_t)(DTC_NUMBER_INDEX),(uint32_t)(dtcNum+1)) == EE_OK)
				{
					result = RAMN_OK;
				}
			}
		}
	}
	return result;
}

RAMN_Result_t RAMN_DTC_GetNumberOfDTC(uint32_t* pDtcNum)
{
	RAMN_Result_t result = RAMN_OK;

	if (RAMN_EEPROM_Read32(DTC_NUMBER_INDEX,pDtcNum) != EE_OK)
	{
		result = RAMN_ERROR;
	}
	return result;
}

#endif
