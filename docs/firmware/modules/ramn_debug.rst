Debug Features
==============

Description
-----------

The **ramn_debug** module contains various functions useful for debugging the ECU's firmware.


.. code-block:: C

	//Function To Enable/Disable Debugging features
	void 	RAMN_DEBUG_SetStatus(RAMN_Bool_t status);

	//Function to dump current CAN statistics over USB
	void 	RAMN_DEBUG_ReportCANStats(const RAMN_FDCAN_Status_t* local_gw);

	//Function to log a message (typically, to a USB serial port)
	//This function is blocking and not thread-safe
	void 	RAMN_DEBUG_Log(const char* src);

	//Function to dump Error Registers over USB (non-human readable)
	void 	RAMN_DEBUG_DumpCANErrorRegisters(const FDCAN_ErrorCountersTypeDef* pErrCnt, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus);

	//Function to display information about CAN Error (human readable)
	void	RAMN_DEBUG_PrintCANError(const FDCAN_ErrorCountersTypeDef* pErrorCount, const FDCAN_ProtocolStatusTypeDef* pProtocolStatus, const RAMN_FDCAN_Status_t* pGw_freeze, uint32_t err);
