Utils
=====

Description
-----------

The **ramn_utils** module implements various useful functions that are used by other modules.


.. code-block:: C

	//Functions to convert from to uint ASCII. Returns number of bytes written
	uint32_t  rawtoASCII(uint8_t* dst, const uint8_t* src, uint32_t raw_size);
	uint32_t  uint32toASCII(uint32_t src, uint8_t* dst);
	uint32_t  uint16toASCII(uint16_t src, uint8_t* dst);
	uint32_t  uint12toASCII(uint16_t src, uint8_t* dst);
	uint32_t  uint8toASCII(uint8_t src, uint8_t* dst);
	uint32_t  uint4toASCII(uint8_t src, uint8_t* dst);

	//Functions to convert from ASCII to uint
	void     ASCIItoRaw(uint8_t* dst, const uint8_t* src, uint32_t raw_size);
	uint8_t  ASCIItoUint4 (const uint8_t* src);
	uint8_t  ASCIItoUint8 (const uint8_t* src);
	uint16_t ASCIItoUint16(const uint8_t* src);
	uint16_t ASCIItoUint12(const uint8_t* src);
	uint32_t ASCIItoUint32(const uint8_t* src);

	//Functions to convert STM32 HAL FDCAN DLC format (uint32 enumeration) to actual payload size (0 to 64)
	uint8_t  DLCtoUINT8(uint32_t dlc_enum);
	uint32_t UINT8toDLC(uint8_t dlc);

	//Regular memcpy operation
	void RAMN_memcpy(uint8_t* dst, const uint8_t* src, uint32_t size);