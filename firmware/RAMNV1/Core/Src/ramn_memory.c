/*
 * ramn_memory.c
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

#include "ramn_memory.h"

//  Private Functions -----------------------------------


static uint32_t getInactiveBank()
{
	FLASH_OBProgramInitTypeDef    OBInit;

	HAL_FLASH_OB_Unlock();
	OBInit.OptionType = OPTIONBYTE_USER;
	OBInit.USERType   = OB_USER_SWAP_BANK;
	HAL_FLASHEx_OBGetConfig(&OBInit);
	HAL_FLASH_OB_Lock();

	if (((OBInit.USERConfig) & (OB_SWAP_BANK_ENABLE)) == OB_SWAP_BANK_ENABLE) return FLASH_BANK_1;
	else return FLASH_BANK_2;
}

static HAL_StatusTypeDef EraseBank(uint32_t bank)
{
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks       = bank;
	EraseInitStruct.Page        = 0;
	EraseInitStruct.NbPages     = 128;

	return HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
}

// Exported Functions -----------------------------------


#if defined(TARGET_ECUA)

//To work correctly, Option bytes must be configured so that:
// - ECU A boots from OB
// - nBOOT0 is set
//We therefore check for  OB_BOOT0_FROM_PIN bit set and OB_nBOOT0_SET NOT set
//If that is the case, we overwrite the option bytes and restart
//We do not check for errors as improperly configured board cannot report them anyway.
RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesApplicationMode(void)
{
	HAL_StatusTypeDef result = HAL_OK;
	FLASH_OBProgramInitTypeDef obHandle;
	HAL_FLASHEx_OBGetConfig(&obHandle);

	if((obHandle.USERConfig & OB_BOOT0_FROM_PIN) ||!(obHandle.USERConfig & OB_nBOOT0_SET))
	{
		FLASH_OBProgramInitTypeDef obHandle;
		HAL_FLASHEx_OBGetConfig(&obHandle);

		obHandle.OptionType = OPTIONBYTE_USER;
		obHandle.USERType = OB_USER_nSWBOOT0 | OB_USER_nBOOT0;
		obHandle.USERConfig = obHandle.USERConfig | OB_nBOOT0_SET;
		obHandle.USERConfig = (obHandle.USERConfig & ~(OB_BOOT0_FROM_PIN));

		result |= HAL_FLASH_Unlock();
		result |= HAL_FLASH_OB_Unlock();
		result |= HAL_FLASHEx_OBProgram(&obHandle);
		result |= HAL_FLASH_OB_Launch(); //resets automatically here
		result |= HAL_FLASH_OB_Lock();
		result |= HAL_FLASH_Lock();
	}

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}

#endif

RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesBootloaderMode(void)
{
	//TODO: Do not overwrite option bytes, but simply jump to bootloader ?
	FLASH_OBProgramInitTypeDef obHandle;
	HAL_StatusTypeDef result = HAL_OK;

	HAL_FLASHEx_OBGetConfig(&obHandle);

	obHandle.OptionType = OPTIONBYTE_USER;
	obHandle.USERType = OB_USER_nSWBOOT0;
	obHandle.USERConfig = obHandle.USERConfig | OB_BOOT0_FROM_PIN;

	result |= HAL_FLASH_Unlock();
	if (result == HAL_OK) {
		result |=  HAL_FLASH_OB_Unlock();
		if (result == HAL_OK) {
			result |= HAL_FLASHEx_OBProgram(&obHandle);
			if (result == HAL_OK)
			{
				result |= HAL_FLASH_OB_Launch(); //Reset will be generated
			}
			else
			{
				result |= HAL_FLASH_OB_Lock();
				result |= HAL_FLASH_Lock();
			}
		}

	}

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}


RAMN_Result_t RAMN_FLASH_SwitchActiveBank(void)
{
	HAL_StatusTypeDef result = HAL_OK;
	FLASH_OBProgramInitTypeDef OBInit;
	result = HAL_FLASH_Unlock();

	result |= HAL_FLASH_OB_Unlock();

	HAL_FLASHEx_OBGetConfig(&OBInit);

	OBInit.OptionType = OPTIONBYTE_USER;
	OBInit.USERType   = OB_USER_SWAP_BANK;

	if (((OBInit.USERConfig) & (OB_SWAP_BANK_ENABLE)) == OB_SWAP_BANK_ENABLE)
	{
		OBInit.USERConfig = OB_SWAP_BANK_DISABLE;
	}
	else
	{
		OBInit.USERConfig = OB_SWAP_BANK_ENABLE;
	}

	if (result == HAL_OK)
	{
		result |= HAL_FLASHEx_OBProgram (&OBInit);
		if (result == HAL_OK)
		{
			//TODO: Do not reset launch here?
			result |= HAL_FLASH_OB_Launch();
		}
	}

	result |= HAL_FLASH_OB_Lock();
	result |= HAL_FLASH_Lock();

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}

RAMN_Result_t RAMN_FLASH_EraseAlternativeFirmware(void)
{
	HAL_StatusTypeDef result;
	result =  HAL_FLASH_Unlock();
	result |= EraseBank(getInactiveBank());
	result |= HAL_FLASH_Lock();

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}

RAMN_Result_t RAMN_FLASH_CopyEEPROMToInactiveBank(void)
{
#if defined(ENABLE_EEPROM_EMULATION)
	HAL_StatusTypeDef result;
	uint32_t startAddress = START_PAGE_ADDRESS;
	uint32_t endAddress = 0x08040000;
	result = HAL_FLASH_Unlock();
	for (uint32_t addr = startAddress; addr < endAddress; addr += 8)
	{
		//Only copy if memory is not written
		if((uint64_t)(*(uint64_t*)(addr)) != 0xFFFFFFFFFFFFFFFF)
		{
			result |= RAMN_FLASH_Write64(addr + 0x00040000, (uint64_t)(*(uint64_t*)(addr)));
		}
	}
	result |= HAL_FLASH_Lock();

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
#endif
	return RAMN_OK;
}

RAMN_Result_t RAMN_FLASH_EraseActiveEEPROM(void)
{
	HAL_StatusTypeDef result;
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;

	result = HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks       = (getInactiveBank() == FLASH_BANK_1) ? FLASH_BANK_2: FLASH_BANK_1;
	EraseInitStruct.Page        = 124;
	EraseInitStruct.NbPages     = 4;

	result |= HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
	result |= HAL_FLASH_Lock();

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}

RAMN_Result_t RAMN_FLASH_Write64(uint32_t address, uint64_t data)
{
	HAL_StatusTypeDef result;
	result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}

RAMN_Bool_t RAMN_FLASH_CheckFlashAreaValidForFirmware(uint32_t start, uint32_t end)
{
	RAMN_Bool_t result = True;
	if ((start < 0x08000000) || (start >= 0x0803E000))
	{
		result = False;
	}
	if ((end < 0x08000000) || (end >= 0x0803E000))
	{
		result = False;
	}
	if (start > end)
	{
		result = False;
	}
	return result;
}

RAMN_Bool_t RAMN_RAM_CheckAreaWritable(uint32_t start, uint32_t end)
{
	uint8_t result = False;
	if ((start >= SRAM1_START_ADDRESS) && (end <= SRAM1_END_ADDRESS))
	{
		result = True;
	}
	else if ((start >= SRAM2_START_ADDRESS) && (end <= SRAM2_END_ADDRESS))
	{
		result = True;
	}
	return result;
}

RAMN_Bool_t RAMN_MEMORY_CheckAreaReadable(uint32_t start, uint32_t end)
{
	RAMN_Bool_t result = False;
	if ((start >= FLASH_START_ADDRESS) && (end <= FLASH_END_ADDRESS))
	{
		result = True;
	}
	else if ((start >= SRAM1_START_ADDRESS) && (end <= SRAM1_END_ADDRESS))
	{
		result = True;
	}
	else if ((start >= SRAM2_START_ADDRESS) && (end <= SRAM2_END_ADDRESS))
	{
		result = True;
	}
	else if ((start >= OTP_START_ADDRESS) && (end <= OTP_STOP_ADDRESS))
	{
		result = True;
	}
	/*
	else if ((start >= SYSTEMFLASH_START_ADDRESS) && (end < SYSTEMFLASH_STOP_ADDRESS))
	{
		result = True;
	}
	else if ((start >= OPTIONBYTES_START_ADDRESS) && (end < OPTIONBYTES_STOP_ADDRESS))
	{
		result = True;
	}*/

	return result;
}
