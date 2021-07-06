UDS Diagnostics
===============

Description
-----------

The **ramn_uds** module implements UDS diagnostics (ISO 14229).

.. code-block:: C

	//Initializes the module
	RAMN_Result_t 	RAMN_UDS_Init(uint32_t tick);

	//Updates the module. Should be called periodically.
	RAMN_Result_t 	RAMN_UDS_Update(uint32_t tick);

	//Update the TX part of the module. Returns true if a transmission is over.
	RAMN_Bool_t 	RAMN_UDS_Continue_TX(uint32_t tick);

	//Process a CAN Message addressed to UDS CAN ID. If a diagnostic message has been reconstructed, it is put in strbuf. Returns true if a message has been reconstructed.
	RAMN_Bool_t		RAMN_UDS_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick, StreamBufferHandle_t* strbuf);

	//Process a fully reconstructed ISO-TP Diag Frame.
	void	 		RAMN_UDS_ProcessDiagPayload(uint32_t tick, const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);
