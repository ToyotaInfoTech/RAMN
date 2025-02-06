/*
 * ramn_eeprom.c
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

#include "ramn_eeprom.h"


#if defined(ENABLE_EEPROM_EMULATION)

EE_Status RAMN_EEPROM_Init()
{
	EE_Status ee_status = EE_OK;

	HAL_FLASH_Unlock();
	ee_status = EE_Init(EE_FORCED_ERASE);

	if(ee_status != EE_OK) EE_Format(EE_FORCED_ERASE); //Could not read memory, try reformatting

	HAL_FLASH_Lock();
	return ee_status;
}

EE_Status RAMN_EEPROM_Write32(uint16_t index, uint32_t val)
{
	EE_Status ee_status = EE_OK;

	HAL_FLASH_Unlock();
	ee_status = EE_WriteVariable32bits(index, val);

	if ((ee_status & EE_STATUSMASK_CLEANUP) == EE_STATUSMASK_CLEANUP) ee_status |= EE_CleanUp();
	if ((ee_status & EE_STATUSMASK_ERROR) == EE_STATUSMASK_ERROR);

	HAL_FLASH_Lock();
	return ee_status;
}

EE_Status RAMN_EEPROM_Read32(uint16_t index, uint32_t* pval)
{
	EE_Status ee_status = EE_OK;
	ee_status|= EE_ReadVariable32bits(index, pval);
	return ee_status;
}

#endif
