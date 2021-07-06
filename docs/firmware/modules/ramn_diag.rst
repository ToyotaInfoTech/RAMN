Diagnostics
===========

Description
-----------

The **ramn_diag** module is a module that combines various diagnostics features (UDS, KWP2000, XCP, etc.) implemented by RAMN.


.. code-block:: C

	//Function to init diagnostic module with default values.
	RAMN_Result_t  	RAMN_DIAG_Init(uint32_t tick, osThreadId_t* pDiagRxTask, StreamBufferHandle_t* kwpbuf, StreamBufferHandle_t* udsbuf, StreamBufferHandle_t* xcpbuf);

	//Function to call periodically to update the Diag module.
	RAMN_Result_t 	RAMN_DIAG_Update(uint32_t tick);

	//Function to call to process an incoming CAN message. Will not block for long and may be called from a high-priority task.
	void 			RAMN_DIAG_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);
