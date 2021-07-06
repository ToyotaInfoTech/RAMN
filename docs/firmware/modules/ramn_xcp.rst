XCP Diagnostics
===============

Description
-----------

The **ramn_xcp** module implements XCP diagnostics. It is currently a simplified, PoC implementation, not compliant with the standard and partially incorrect.


.. code-block:: C

	//Initializes the module
	RAMN_Result_t 	RAMN_XCP_Init(uint32_t tick);

	//Updates the module. Should be called periodically.
	RAMN_Bool_t 	RAMN_XCP_Update(uint32_t tick);

	//Sends outgoing messages
	RAMN_Bool_t		RAMN_XCP_Continue_TX(uint32_t tick, const uint8_t* data, uint16_t size);

	//Process a CAN Message addressed to XCP CAN ID. If a diagnostic messages has been reconstructed, it is put in strbuf. Returns true if a message has been reconstructed.
	RAMN_Bool_t		RAMN_XCP_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);

	//Process a reconstructed diagnostics message
	void			RAMN_XCP_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);
