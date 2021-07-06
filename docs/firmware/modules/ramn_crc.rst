CRC Engine
==========

Description
-----------

The **ramn_crc** module is a wrapper to easily use the CRC engine to compute the value of memory area's CRC (Cyclic Redundancy Check).


.. code-block:: C

	//Initializes the module
	void RAMN_CRC_Init(CRC_HandleTypeDef* h);

	// Computers the CRC of specified buffer
	uint32_t RAMN_CRC_Calculate(const uint8_t* src, uint32_t size);