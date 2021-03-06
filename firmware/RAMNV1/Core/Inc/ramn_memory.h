/*
 * ramn_memory.h
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

// This Module handles Writing to memory, including FLASH, Option Bytes, and RAM.

#ifndef INC_RAMN_MEMORY_H_
#define INC_RAMN_MEMORY_H_

#include "ramn_eeprom.h"

#define FLASH_FIRMWARE_BANK_OFFSET 0x00040000

#define DEVICE_HARDWARE_ID_ADDRESS 0x0BFA0590

#define FLASH_START_ADDRESS 		0x08000000
#define FLASH_END_ADDRESS   		0x08080000

#define SRAM1_START_ADDRESS 		0x20000000
#define SRAM1_END_ADDRESS 			0x20030000

#define SRAM2_START_ADDRESS 		0x20030000
#define SRAM2_END_ADDRESS 			0x20040000

#define SYSTEMFLASH_START_ADDRESS 	0x0BF90000
#define SYSTEMFLASH_STOP_ADDRESS 	0x0BF98000

#define OTP_START_ADDRESS 			0x0BFA0000
#define OTP_STOP_ADDRESS			0x0BFA0200

#define OPTIONBYTES_START_ADDRESS 	0x40022040
#define OPTIONBYTES_STOP_ADDRESS	0x40022140

#if defined(TARGET_ECUA)
//Configure Option Bytes for Bootloader Mode  (for ECU A)
RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesBootloaderMode(void);

//Configure Option Bytes for Application Mode (for ECU A)
RAMN_Result_t RAMN_FLASH_ConfigureOptionBytesApplicationMode(void);
#endif

//Changes The "SWAP BANK" Option Byte of the MCU. Used after successful firmware update
RAMN_Result_t RAMN_FLASH_SwitchActiveBank(void);

//Erases The alternative firmware (second bank) of the ECU. Must be called before firmware update.
RAMN_Result_t RAMN_FLASH_EraseAlternativeFirmware(void);

//Copies value of the current EEPROM Emulation Layer to the alternative bank (except values not written yet)
RAMN_Result_t RAMN_FLASH_CopyEEPROMToInactiveBank(void);

//Copies current firmware to the inactive bank. Used to keep a copy of current firmware.
RAMN_Result_t RAMN_FLASH_CopyActiveFirmwareToInactiveBank(void);

//Erases the current EEPROM memory. Used to restore ECU at "factory state"
RAMN_Result_t RAMN_FLASH_EraseActiveEEPROM(void);

//Writes a 64-bit value to flash memory. Only works if current address has been erased before.
RAMN_Result_t RAMN_FLASH_Write64(uint32_t address, uint64_t data);

//Check Whether an area is suitable for firmware programming or not
RAMN_Bool_t RAMN_FLASH_CheckFlashAreaValidForFirmware(uint32_t start, uint32_t end);

//Check Whether an area is suitable for memory writing or not (RAM, not FLASH)
RAMN_Bool_t RAMN_RAM_CheckAreaWritable(uint32_t start, uint32_t end);

//Check Whether an area is suitable for memory reading or not (RAM/FLASH)
RAMN_Bool_t RAMN_MEMORY_CheckAreaReadable(uint32_t start, uint32_t end);
#endif /* INC_RAMN_MEMORY_H_ */
