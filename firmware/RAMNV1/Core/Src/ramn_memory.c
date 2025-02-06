/*
 * ramn_memory.c
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

#include "ramn_memory.h"

//  Private Functions -----------------------------------

// Using uint32_t to stay consistent with macro
static uint32_t MEMORY_GetInactiveBank()
{
	FLASH_OBProgramInitTypeDef    OBInit;

	HAL_FLASHEx_OBGetConfig(&OBInit);

	if (((OBInit.USERConfig) & (OB_SWAP_BANK_ENABLE)) == OB_SWAP_BANK_ENABLE) return FLASH_BANK_1;
	else return FLASH_BANK_2;
}

static HAL_StatusTypeDef MEMORY_EraseBank(uint32_t bank)
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

RAMN_Bool_t RAMN_FLASH_CheckFlashAreaValidForFirmware(uint32_t start, uint32_t end)
{
	RAMN_Bool_t result = True;

	if (end <= start) result = False; // Consider empty firmware as invalid
	else if ((start < FLASH_START_ADDRESS) || (start >= FLASH_FIRMWARE_END_ADDRESS)) result = False;
	else if ((end <= FLASH_START_ADDRESS) || (end > FLASH_FIRMWARE_END_ADDRESS)) result = False;

	return result;
}

RAMN_Bool_t RAMN_RAM_CheckAreaWritable(uint32_t start, uint32_t end)
{
	uint8_t result = False;

	if (end <= start) result = False; // Consider empty data as invalid
	else if ((start >= SRAM1_START_ADDRESS) && (end <= SRAM2_END_ADDRESS)) result = True;

	return result;
}

RAMN_Bool_t RAMN_MEMORY_CheckAreaReadable(uint32_t start, uint32_t end)
{
	RAMN_Bool_t result = False;

	if (end <= start) result = False; // Consider empty data as invalid
	else if ((start >= SRAM1_START_ADDRESS) 		&& (end <= SRAM2_END_ADDRESS)) result = True;
	else if ((start >= OTP_START_ADDRESS) 			&& (end <= OTP_STOP_ADDRESS)) result = True;
	else if ((start >= SYSTEMFLASH_START_ADDRESS) 	&& (end <= SYSTEMFLASH_STOP_ADDRESS)) result = True;
	else if ((start >= OPTIONBYTES_START_ADDRESS) 	&& (end <= OPTIONBYTES_STOP_ADDRESS)) result = True;
	else if ((start >= FLASH_START_ADDRESS) 		&& (end <= FLASH_END_ADDRESS))
	{
		if (*(const uint16_t*)FLASHSIZE_BASE == 512) result = True; // Flash size may be 256 or 512 depending on uc
		else if (end <= FLASH_START_ADDRESS + ((FLASH_END_ADDRESS-FLASH_START_ADDRESS)/2)) result = True;
	}

	return result;
}

RAMN_Bool_t RAMN_FLASH_isMemoryProtected(void)
{
	FLASH_OBProgramInitTypeDef obHandle;

	HAL_FLASHEx_OBGetConfig(&obHandle);
	if(obHandle.RDPLevel == OB_RDP_LEVEL_0) return False;
	else return True;
}


#if defined(TARGET_ECUA)

// For ECU A work correctly, Option bytes must be configured so that:
// - ECU A boots from OB
// - nBOOT0 is set
// (ECU B/C/D have their boot0 pin controlled by ECU A and therefore should keep default option bytes)
// We therefore check for OB_BOOT0_FROM_PIN bit set and OB_nBOOT0_SET NOT set
// If that is the case, we overwrite the option bytes and restart
RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesApplicationMode(void)
{
	HAL_StatusTypeDef result = HAL_OK;
	FLASH_OBProgramInitTypeDef obHandle;

	HAL_FLASHEx_OBGetConfig(&obHandle);

	if((result != HAL_OK) || (obHandle.USERConfig & OB_BOOT0_FROM_PIN))
	{
		if (HAL_GPIO_ReadPin (SELF_BOOT0_GPIO_Port, SELF_BOOT0_Pin) == GPIO_PIN_RESET)
		{
			//BOOT0 is low, someone probably flashed ECUA firmware to ECUB/C/D by mistake, do not change option bytes.
			result =  HAL_ERROR;
		}
		else
		{
			obHandle.OptionType = OPTIONBYTE_USER;
			obHandle.USERType = OB_USER_nSWBOOT0 | OB_USER_nBOOT0;
			obHandle.USERConfig = obHandle.USERConfig | OB_nBOOT0_SET;
			obHandle.USERConfig = (obHandle.USERConfig & ~(OB_BOOT0_FROM_PIN));

			__disable_irq();
			result |= HAL_FLASH_Unlock();
			result |= HAL_FLASH_OB_Unlock();
			result |= HAL_FLASHEx_OBProgram(&obHandle);
			result |= HAL_FLASH_OB_Launch(); //resets automatically here
			result |= HAL_FLASH_OB_Lock();
			result |= HAL_FLASH_Lock();
			__enable_irq();

		}
	}
	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}

RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesBootloaderMode(void)
{
	FLASH_OBProgramInitTypeDef obHandle;
	HAL_StatusTypeDef result = HAL_OK;

	__disable_irq();
	HAL_FLASHEx_OBGetConfig(&obHandle);

	obHandle.OptionType = OPTIONBYTE_USER;
	obHandle.USERType = OB_USER_nSWBOOT0;
	obHandle.USERConfig = obHandle.USERConfig | OB_BOOT0_FROM_PIN;

	result |= HAL_FLASH_Unlock();
	if (result == HAL_OK) {
		result |=  HAL_FLASH_OB_Unlock();
		if (result == HAL_OK) {
			result |= HAL_FLASHEx_OBProgram(&obHandle);
			if (result == HAL_OK) result |= HAL_FLASH_OB_Launch(); // Should reset here
			else
			{
				result |= HAL_FLASH_OB_Lock();
				result |= HAL_FLASH_Lock();
			}
		}

	}
	__enable_irq();
	return RAMN_ERROR;
}

// This function is essentially the same as RAMN_FLASH_ConfigureRDPOptionByte, but everything is in RAM.
// TODO: merge with RAMN_FLASH_ConfigureRDPOptionByte (?)
__attribute__((__section__(".RamFunc"))) RAMN_Result_t RAMN_FLASH_RemoveMemoryProtection(void)
{
	FLASH_OBProgramInitTypeDef obHandle;
	HAL_StatusTypeDef status;

	// Do not use HAL library as this should be all in RAM

	__disable_irq();
	HAL_FLASHEx_OBGetConfig(&obHandle);

	obHandle.OptionType = OPTIONBYTE_USER | OPTIONBYTE_RDP;
	obHandle.USERType = OB_USER_nSWBOOT0;
	obHandle.USERConfig = obHandle.USERConfig | OB_BOOT0_FROM_PIN;
	obHandle.RDPLevel = OB_RDP_LEVEL_0;

	if(READ_BIT(FLASH->NSCR, FLASH_NSCR_NSLOCK) != 0u)
	{
		WRITE_REG(FLASH->NSKEYR, FLASH_KEY1);
		WRITE_REG(FLASH->NSKEYR, FLASH_KEY2);
	}

	if(READ_BIT(FLASH->NSCR, FLASH_NSCR_OPTLOCK) != 0u)
	{
		WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY1);
		WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY2);
	}

	__HAL_LOCK(&pFlash);
	pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
	status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

	if(status == HAL_OK)
	{
		uint32_t optr_reg_val = 0;
		uint32_t optr_reg_mask = 0;
		MODIFY_REG(FLASH->OPTR, FLASH_OPTR_RDP, obHandle.RDPLevel);

		optr_reg_val |= (obHandle.USERConfig & FLASH_OPTR_nSWBOOT0);
		optr_reg_mask |= FLASH_OPTR_nSWBOOT0;

		MODIFY_REG(FLASH->OPTR, optr_reg_mask, optr_reg_val);
		SET_BIT(FLASH->NSCR, FLASH_NSCR_OPTSTRT);

		status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
	}

	__HAL_UNLOCK(&pFlash);

	if (status == HAL_OK) SET_BIT(FLASH->NSCR, FLASH_NSCR_OBL_LAUNCH);
	else
	{
		SET_BIT(FLASH->NSCR, FLASH_NSCR_OPTLOCK);
		SET_BIT(FLASH->NSCR, FLASH_NSCR_NSLOCK); // Should reset here
	}

	__enable_irq();
	return RAMN_ERROR; 	// Should not reach here.
}

#endif


#ifdef MEMORY_AUTOLOCK

RAMN_Result_t RAMN_FLASH_ConfigureRDPOptionByte(uint8_t val)
{
	HAL_StatusTypeDef result = HAL_OK;
	FLASH_OBProgramInitTypeDef obHandle;

	// User trying to unlock an already unlocked ECU, just return OK.
	if ((RAMN_FLASH_isMemoryProtected() == False) && (val == OB_RDP_LEVEL_0)) return RAMN_OK;

	// User trying to unlock an already locked board, should use the remove protection feature instead.
	else if ((RAMN_FLASH_isMemoryProtected() == True) && (val == OB_RDP_LEVEL_0)) return RAMN_ERROR;

	HAL_FLASHEx_OBGetConfig(&obHandle);

	obHandle.OptionType = OPTIONBYTE_RDP;
	obHandle.RDPLevel = val;

	__disable_irq();
	result |= HAL_FLASH_Unlock();
	result |= HAL_FLASH_OB_Unlock();
	result |= HAL_FLASHEx_OBProgram(&obHandle);
	result |= HAL_FLASH_OB_Launch(); // Should reset here
	result |= HAL_FLASH_OB_Lock();
	result |= HAL_FLASH_Lock();
	__enable_irq();

	return RAMN_ERROR; // Should not reach here
}

#endif


RAMN_Result_t RAMN_FLASH_SwitchActiveBank(void)
{
	HAL_StatusTypeDef result = HAL_OK;
	FLASH_OBProgramInitTypeDef OBInit;

	result = HAL_FLASH_Unlock();
	result |= HAL_FLASH_OB_Unlock();

	HAL_FLASHEx_OBGetConfig(&OBInit);

	OBInit.OptionType = OPTIONBYTE_USER;
	OBInit.USERType   = OB_USER_SWAP_BANK;

	if (((OBInit.USERConfig) & (OB_SWAP_BANK_ENABLE)) == OB_SWAP_BANK_ENABLE) OBInit.USERConfig = OB_SWAP_BANK_DISABLE;
	else OBInit.USERConfig = OB_SWAP_BANK_ENABLE;

	if (result == HAL_OK)
	{
		result |= HAL_FLASHEx_OBProgram (&OBInit);
		if (result == HAL_OK) result |= HAL_FLASH_OB_Launch(); //TODO: Do not reset launch here?
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
	result |= MEMORY_EraseBank(MEMORY_GetInactiveBank());
	result |= HAL_FLASH_Lock();

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
}

RAMN_Result_t RAMN_FLASH_CopyEEPROMToInactiveBank(void)
{
#if defined(ENABLE_EEPROM_EMULATION)
	HAL_StatusTypeDef result;
	uint32_t startAddress = START_PAGE_ADDRESS;
	uint32_t endAddress = FLASH_FIRMWARE_END_ADDRESS;
	result = HAL_FLASH_Unlock();
	for (uint32_t addr = startAddress; addr < endAddress; addr += 8)
	{
		//Only copy if memory is not written
		if((uint64_t)(*(uint64_t*)(addr)) != 0xFFFFFFFFFFFFFFFF) result |= RAMN_FLASH_Write64(addr + 0x00040000, (uint64_t)(*(uint64_t*)(addr)));
	}
	result |= HAL_FLASH_Lock();

	if (result == HAL_OK) return RAMN_OK;
	else return RAMN_ERROR;
#endif
	return RAMN_ERROR;
}

RAMN_Result_t RAMN_FLASH_EraseActiveEEPROM(void)
{
	HAL_StatusTypeDef result;
	uint32_t PageError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;

	result = HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks       = (MEMORY_GetInactiveBank() == FLASH_BANK_1) ? FLASH_BANK_2: FLASH_BANK_1;
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
