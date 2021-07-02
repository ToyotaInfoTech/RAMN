USB firmware
============

The USB software package that comes with STM32CubeIDE is used to implement serial communication over USB (VCP - Virtual COM Port).

On the STM32L552, USB Communications may be achieved by enabling "USB" in the Connectivity section (all parameters disabled) and "USB_DEVICE" as "COMMUNICATION DEVICE CLASS (Virtual Port Com)" in the Middleware section (all parameters disabled).

.. warning:: Make sure to increase the minimum allocated memory in the :ref:`Linker Settings <common_issues>`, and in the freeRTOS settings. The default values used by STM32CubeIDE are not enough. Also, Make sure the USB clock has a value of 48 MHz.

Data can be sent over USB serial using the :C:func:`CDC_Transmit_FS` function.

.. code-block:: C
   
   #include "usbd_cdc_if.h"
   
   CDC_Transmit_FS(usbSendBuffer,sizeof(usbSendBuffer));
   
.. note:: :C:func:`CDC_Transmit_FS` may return USBD_BUSY, in which case the buffer data will not be sent. In practice, we use :C:func:`CDC_Transmit_FS` as below:

   .. code-block:: 
   
		while(CDC_Transmit_FS((uint8_t*)usb_send_buffer,6+2*msg.RxHeader.DLC) == USBD_BUSY) taskYIELD(); 						
   
   A `freeRTOS Message buffer <https://www.freertos.org/RTOS-message-buffer-example.html>`_ cannot be used for buffering, as they can only handle one reader and one writer, but we want to keep the USB serial port accessible to different tasks.
	
Data can be received over USB serial by modifying the :C:func:`CDC_Receive_FS` callback function in :file:`usbd_cdc_if.c`. We use a freeRTOS `Stream Buffer <https://www.freertos.org/RTOS-stream-buffer-example.html>`_ to transfer received characters to the main tasks.

.. code-block:: C
   
	static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
	{
		/* USER CODE BEGIN 6 */
		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
		USBD_CDC_ReceivePacket(&hUsbDeviceFS);

		if(USBD_RxStreamBuffer != NULL){

			if (xStreamBufferSendFromISR( USBD_RxStreamBuffer,( void * ) Buf, *Len, NULL ) != *Len)
			{
				//TODO: report error
			}
		}

		return (USBD_OK);
		/* USER CODE END 6 */
	}