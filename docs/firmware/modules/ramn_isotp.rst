ISO-TP Layer
============

Description
-----------

The **ramn_isotp** module is a simplified implementation of ISO-TP (ISOISO 15765).


.. code-block:: C

	//This Function initializes the specified ISO-TP Handler
	void 			RAMN_ISOTP_Init(RAMN_ISOTPHandler_t* handler, FDCAN_TxHeaderTypeDef* FCMsgHeader);

	//This Function adds a message for process by the ISO TP Engine. It assumes CAN ID has already been checked
	void		 	RAMN_ISOTP_ProcessRxMsg(RAMN_ISOTPHandler_t* handler, uint8_t dlc, const uint8_t* data, const uint32_t tick);

	//This function formats the next "FC Frame" CAN Message to be sent next. User must call this function periodically and send the message if it returns True.
	RAMN_Bool_t 	RAMN_ISOTP_GetFCFrame(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data);

	//This function formats the next CAN Message to be sent (except FC Frames). User must call this function periodically and send the message if it returns True.
	RAMN_Bool_t 	RAMN_ISOTP_GetNextTxMsg(RAMN_ISOTPHandler_t* handler, uint8_t* dlc, uint8_t* data, uint32_t tick);

	//This function Updates the ISO-TP Engine, and must be called periodically.
	RAMN_Result_t 	RAMN_ISOTP_Update(RAMN_ISOTPHandler_t* pHandler, uint32_t tick);

	//This function updates the TX part of the ISO-TP Engine. Must be called periodically when sending.
	RAMN_Bool_t 	RAMN_ISOTP_Continue_TX(RAMN_ISOTPHandler_t* pHandler, uint32_t tick, FDCAN_TxHeaderTypeDef* pTxHeader);

	//This function request the ISO-TP Engine to process an outgoing message.
	RAMN_Result_t 	RAMN_ISOTP_RequestTx(RAMN_ISOTPHandler_t* handler, uint32_t tick);
