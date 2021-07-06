EEPROM Emulation
================

Description
-----------

The **ramn_eeprom** module is a wrapper of STM32's EEPROM emulation layer. It can be used to read/write data on the STM32L5's flash memory.


.. code-block:: C

	//Inits the EEPROM Layer. May take some time.
	EE_Status 	RAMN_EEPROM_Init();

	//Writes a 32-bit value at the specified index. May take some time.
	EE_Status 	RAMN_EEPROM_Write32(uint16_t index, uint32_t val);

	//Reads a 32-bit value from the specified index.
	EE_Status 	RAMN_EEPROM_Read32(uint16_t index, uint32_t* pval);