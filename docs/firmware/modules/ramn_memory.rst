Memory Management
=================

Description
-----------

The **ramn_memory** module is a module to access STM32's memory, both RAM and FLASH. It features various functions for safe read/write to memory areas.


.. code-block:: C

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