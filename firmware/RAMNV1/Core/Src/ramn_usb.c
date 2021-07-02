/*
 * ramn_usb.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "ramn_usb.h"

#ifdef ENABLE_USB

#include "usbd_cdc_if.h"

//Buffer that holds outgoing USB data
static StreamBufferHandle_t* usbTxBuffer;

//Pointer to the task currently responsible for sending out USB data
static osThreadId_t* sendTask;

//Semaphore to allow writing to USB from different task
static StaticSemaphore_t USB_TX_SEMAPHORE_STRUCT;
static SemaphoreHandle_t USB_TX_SEMAPHORE;

RAMN_USB_Status_t RAMN_USB_Config =
{
		.slcanOpened 			= False,
#if defined(ENABLE_USB_AUTODETECT)
		.serialOpened			= False,
#else
		.serialOpened			= True,
#endif
		.simulatorActive		= False,
		.slcan_enableTimestamp 	= False,
		.autoreportErrors 		= False,
		.addESIFlag 			= False,
		.USBErrCnt 				= 0U,
		.USBTxOverflowCnt 		= 0U,
};

void RAMN_USB_Init(StreamBufferHandle_t* buffer,  osThreadId_t* pSendTask)
{
	usbTxBuffer 					= buffer;
	sendTask = pSendTask;
	USB_TX_SEMAPHORE 				= xSemaphoreCreateMutexStatic(&USB_TX_SEMAPHORE_STRUCT);
	USBD_errorCallback_ptr 			= &RAMM_USB_ErrorCallback; //callback for USB Errors
#ifdef ENABLE_USB_AUTODETECT
	USBD_serialOpenCallback_ptr 	= &RAMM_USB_SerialOpenCallback; //callback for USB Errors
	USBD_serialCloseCallback_ptr 	= &RAMN_USB_SerialCloseCallback; //callback for USB Errors
#endif
}

void RAMN_USB_SendFromTask_Blocking(uint8_t* data, uint32_t length)
{
	//Start sending
	if(CDC_Transmit_FS((uint8_t*)data,length) != USBD_OK)
	{
		//Should not Happen if serial is still opened
#ifdef ENABLE_USB_AUTODETECT
		if (RAMN_USB_Config.serialOpened == True) RAMN_USB_Config.USBTxOverflowCnt++;
#else
		RAMN_USB_Config.USBTxOverflowCnt++;
#endif
	}
	if (ulTaskNotifyTake(pdTRUE, USB_TX_TIMEOUT_MS) == 0U)
	{
		//Timeout Occurred, report if serial port is still opened
#ifdef ENABLE_USB_AUTODETECT
		if (RAMN_USB_Config.serialOpened == True) RAMN_USB_Config.USBErrCnt++;
#else
		RAMN_USB_Config.USBErrCnt++;
#endif
	}
}

RAMN_Result_t RAMN_USB_SendFromTask(uint8_t* data, uint32_t length)
{
	size_t xBytesSent = 0;
	RAMN_Result_t result = RAMN_OK;
	while (xSemaphoreTake(USB_TX_SEMAPHORE, portMAX_DELAY ) != pdTRUE);
	xBytesSent = xStreamBufferSend(*usbTxBuffer, data, length, portMAX_DELAY );
	xSemaphoreGive(USB_TX_SEMAPHORE);
	if (xBytesSent != length)
	{
		RAMN_USB_Config.USBTxOverflowCnt++;
	}

	return result;
}

void RAMM_USB_ErrorCallback(USBD_HandleTypeDef* hUsbDeviceFS)
{
	RAMN_USB_Config.USBErrCnt += 1;
}

void RAMM_USB_SerialOpenCallback(USBD_HandleTypeDef* hUsbDeviceFS)
{
#ifdef ENABLE_USB_AUTODETECT
	RAMN_USB_Config.serialOpened = True;
#endif
}

//Note that this function  gets called twice when the serial port is closed, and once at startup
void RAMN_USB_SerialCloseCallback(USBD_HandleTypeDef* hUsbDeviceFS)
{
#ifdef ENABLE_USB_AUTODETECT
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	RAMN_USB_Config.serialOpened = False;
	RAMN_USB_Config.slcanOpened = False;
	if (sendTask != NULL)
	{
		//Empty the buffer and let the sending task leave the notify waiting phase
		vTaskNotifyGiveFromISR(*sendTask,&xHigherPriorityTaskWoken); //TODO: from ISR or regular ?

		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		//Note that we do not use the "BufferReset" API because it requires tasks to not be blocked on receiving/sending, which we cannot 100% guarantee
	}
#endif
}

#endif

