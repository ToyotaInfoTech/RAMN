TRNG Management
===============

Description
-----------

The **ramn_trng** module controls the TRNG (True Random Number Generator) peripheral. It automatically generates random number and fills them to a stream buffer, which can be emptied through the "Pop" functions.


.. code-block:: C

	//Callback for TRNG Peripheral ISR
	void 		HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef *hrng, uint32_t random32bit);

	//Callback for TRNG Errors
	void 		HAL_RNG_ErrorCallback(RNG_HandleTypeDef *hrng);

	//Initializes the module
	void 		RAMN_RNG_Init(RNG_HandleTypeDef* handle);

	//Get one random uint8_t from the module
	uint8_t  	RAMN_RNG_Pop8(void);

	//Get one random uint16_t from the module
	uint16_t 	RAMN_RNG_Pop16(void);

	//Get one random uint32_t
	uint32_t 	RAMN_RNG_Pop32(void);