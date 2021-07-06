CAN-FD Peripheral Handling
==========================

Description
-----------

the **ramn_canfd** module is in charge of handling `CAN-FD <https://en.wikipedia.org/wiki/CAN_FD>`_ communications.

The actual peripheral is handled by the `HAL Library of the STM32L5 <https://www.st.com/resource/en/user_manual/dm00669466-description-of-stm32l5-hal-and-lowlayer-drivers-stmicroelectronics.pdf>`_.

.. code-block:: C

	//ISR called when Messages received in FIFO0
	void 			HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
	//ISR called when message was successfully sent
	void 			HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
	//Callback for errors related to CAN-FD Controller
	void 			HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
	//Callback for errors related to CAN-FD bus (NOT errors within CAN Controller)
	void 			HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs);

	//FDCAN Peripheral Handling functions ---------------------------

	//Initialize module with FDCAN controller handle, and main threads to notify
	void 			RAMN_FDCAN_Init(FDCAN_HandleTypeDef* handle, osThreadId_t* s, osThreadId_t* e);

	//Resets the FDCAN with values saved in gw, then restarts the peripheral.
	//Resets statistics but do not resets filter configurations.
	void 			RAMN_FDCAN_ResetPeripheral(void);

	//Disable the FDCAN peripheral
	void 			RAMN_FDCAN_Disable(void);

	//Update the FDCAN peripheral with specified baudrate (slcan format: '0' to '8'). Not valid until peripheral is reset.
	//required after baudrate change or mode change (e.g. listen mode)
	void 			RAMN_FDCAN_UpdateBaudrate(uint8_t newSelection);

	//Reset statistics kept of the module. Does NOT reset the filter.
	void 			RAMN_FDCAN_ResetStatistics(void);

	//Function to check whether the current TX buffer can accomodate a message with specified payload Size
	RAMN_Bool_t 	RAMN_FDCAN_IsTXBufferSpaceAvailable(uint8_t payloadSize);

	//Function to send a CAN Message. Will not block unless semaphore is unavailable.
	RAMN_Result_t 	RAMN_FDCAN_SendMessage(const FDCAN_TxHeaderTypeDef* header, const uint8_t* data);

	//Setup the peripheral for communication with ROM FDCAN Bootloader. Cf AN5405
	void RAMN_FDCAN_SetupForSTBootloader(void);

Data structures
---------------

A received message is a struct with the following fields:

.. code-block:: C

	* uint32_t Identifier
	* uint32_t IdType
	* uint32_t RxFrameType
	* uint32_t DataLength
	* uint32_t ErrorStateIndicator
	* uint32_t BitRateSwitch
	* uint32_t FDFormat
	* uint32_t RxTimestamp
	* uint32_t FilterIndex
	* uint32_t IsFilterMatchingFrame

A message to send is a struct with the following fields:

.. code-block:: C

	* uint32_t Identifier
	* uint32_t IdType
	* uint32_t TxFrameType
	* uint32_t DataLength
	* uint32_t ErrorStateIndicator
	* uint32_t BitRateSwitch
	* uint32_t FDFormat
	* uint32_t TxEventFifoControl
	* uint32_t MessageMarker

.. warning:: The DataLength field must be sent using a dedicated enumeration, *NOT* by setting the value directly. For example, the DataLength of a 1-byte message should be set not to "1" but to :code:`FDCAN_DLC_BYTES_1` (which is actually equal to 0x00010000U)
