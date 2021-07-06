USB Management
==============

Description
-----------

The **ramn_usb** module handles USB communication. It is only used by ECU A.

.. code-block:: C

	//Initializes the module
	void 			RAMN_USB_Init(StreamBufferHandle_t* buffer,  osThreadId_t* pSendTask);

	//Sends Data over USB. Blocks until the USB module buffer accepted the operation. May take time.
	void 			RAMN_USB_SendFromTask_Blocking(uint8_t* data, uint32_t length);

	//Sends Data over USB. Returns as soon as buffer is filled.
	RAMN_Result_t 	RAMN_USB_SendFromTask(uint8_t* data, uint32_t length);

	//Callback for when USB errors are detected
	void 			RAMM_USB_ErrorCallback(USBD_HandleTypeDef* hUsbDeviceFS);

	//Callback for when USB Serial Port OPEN has been detected
	void 			RAMM_USB_SerialOpenCallback(USBD_HandleTypeDef* hUsbDeviceFS);

	//Callback for when USB Serial Port CLOSE has been detected
	void 			RAMN_USB_SerialCloseCallback(USBD_HandleTypeDef* hUsbDeviceFS);
