KWP2000 Diagnostics
===================

Description
-----------

The **ramn_kwp2000** module is a simplified, incomplete implementation of KWP2000 diagnostics (ISO 14230).


.. code-block:: C

	//Initializes the module
	RAMN_Result_t 	RAMN_KWP_Init(uint32_t tick);

	//Updates the module. Should be called periodically.
	RAMN_Result_t 	RAMN_KWP_Update(uint32_t tick);

	//Update the TX part of the module. Returns true if a transmission is over.
	RAMN_Bool_t 	RAMN_KWP_Continue_TX(uint32_t tick);

	//Process a CAN Message addressed to KWP CAN ID. If a diagnostic message has been reconstructed, it is put in strbuf. Returns true if a message has been reconstructed.
	RAMN_Bool_t		RAMN_KWP_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);

	//Process a fully reconstructed ISO-TP Diag Frame.
	void	 		RAMN_KWP_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);
