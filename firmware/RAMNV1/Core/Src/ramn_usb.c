/*
 * ramn_usb.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 TOYOTA MOTOR CORPORATION.
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

#include "usbd_cdc.h"
#include "ramn_utils.h"

// Pointer to buffer that holds outgoing USB data
static StreamBufferHandle_t* usbTxBuffer;

// Pointer to the task currently responsible for sending out USB data
static osThreadId_t* sendTask;

// Semaphore to allow writing to USB from different task
static StaticSemaphore_t USB_TX_SEMAPHORE_STRUCT;
static SemaphoreHandle_t USB_TX_SEMAPHORE;

// Public configuration
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
#ifdef ENABLE_GSUSB
		.queueErrorCnt          = 0U
#endif
};

void RAMN_USB_Init(StreamBufferHandle_t* buffer,  osThreadId_t* pSendTask)
{
	usbTxBuffer 					= buffer;
	sendTask 						= pSendTask;
	USB_TX_SEMAPHORE 				= xSemaphoreCreateMutexStatic(&USB_TX_SEMAPHORE_STRUCT);
	USBD_errorCallback_ptr 			= &RAMM_USB_ErrorCallback; // Callback for USB Errors
#ifdef ENABLE_USB_AUTODETECT
	USBD_serialOpenCallback_ptr 	= &RAMM_USB_SerialOpenCallback; // Callback for USB Errors
	USBD_serialCloseCallback_ptr 	= &RAMN_USB_SerialCloseCallback; // Callback for USB Errors
#endif
}

void RAMN_USB_SendFromTask_Blocking(uint8_t* data, uint32_t length)
{
	// Start sending
	if(CDC_Transmit_FS((uint8_t*)data,length) != USBD_OK)
	{
		// Should not Happen if serial is still opened
#ifdef ENABLE_USB_AUTODETECT
		if (RAMN_USB_Config.serialOpened == True)
		{
			RAMN_USB_Config.USBTxOverflowCnt++;
			RAMN_USB_Config.serialOpened = False;
		}
#else
		RAMN_USB_Config.USBTxOverflowCnt++;
#endif
	}
	if (ulTaskNotifyTake(pdTRUE, USB_TX_TIMEOUT_MS) == 0U)
	{
		// Timeout Occurred, report if serial port is still opened
#ifdef ENABLE_USB_AUTODETECT
		if (RAMN_USB_Config.serialOpened == True)
		{
			RAMN_USB_Config.USBErrCnt++;
			RAMN_USB_Config.serialOpened = False;
		}
#else
		RAMN_USB_Config.USBErrCnt++;
#endif
	}
}

RAMN_Result_t RAMN_USB_SendFromTask(const uint8_t* data, uint32_t length)
{
	size_t xBytesSent = 0;
	RAMN_Result_t result = RAMN_OK;

	while (xSemaphoreTake(USB_TX_SEMAPHORE, portMAX_DELAY ) != pdTRUE);

	xBytesSent = xStreamBufferSend(*usbTxBuffer, data, length, 2000U);
	xSemaphoreGive(USB_TX_SEMAPHORE);
	if (xBytesSent != length)
	{
		RAMN_USB_Config.USBTxOverflowCnt++;
		result = RAMN_ERROR;
		while( xStreamBufferReset(*usbTxBuffer) != pdPASS) osDelay(10); //Clear buffer
	}

	return result;
}

void RAMN_USB_AcquireLock(void)
{
	while (xSemaphoreTake(USB_TX_SEMAPHORE, portMAX_DELAY ) != pdTRUE);
}

void RAMN_USB_ReleaseLock(void)
{
	xSemaphoreGive(USB_TX_SEMAPHORE);
}

RAMN_Result_t RAMN_USB_SendFromTask_Locked(const uint8_t* data, uint32_t length)
{
	size_t xBytesSent = 0;
	RAMN_Result_t result = RAMN_OK;

	xBytesSent = xStreamBufferSend(*usbTxBuffer, data, length, 2000U);
	if (xBytesSent != length)
	{
		RAMN_USB_Config.USBTxOverflowCnt++;
		result = RAMN_ERROR;
		while( xStreamBufferReset(*usbTxBuffer) != pdPASS) osDelay(10); //Clear buffer
	}

	return result;
}

RAMN_Result_t RAMN_USB_SendStringFromTask(const char* data)
{
	return RAMN_USB_SendFromTask((uint8_t*)data, RAMN_strlen(data));
}

RAMN_Result_t RAMN_USB_SendASCIIUint8(uint8_t val)
{
	uint8_t tmp[2U];
	uint8toASCII(val,tmp);
	return RAMN_USB_SendFromTask(tmp, 2U);
}

RAMN_Result_t RAMN_USB_SendASCIIUint16(uint16_t val)
{
	uint8_t tmp[4U];
	uint16toASCII(val,tmp);
	return RAMN_USB_SendFromTask(tmp, 4U);
}

RAMN_Result_t RAMN_USB_SendASCIIUint32(uint32_t val)
{
	uint8_t tmp[8U];
	uint32toASCII(val,tmp);
	return RAMN_USB_SendFromTask(tmp, 8U);
}

void RAMM_USB_ErrorCallback(USBD_HandleTypeDef* hUsbDeviceFS)
{
	RAMN_USB_Config.USBErrCnt += 1;
}

#ifdef ENABLE_USB_AUTODETECT
void RAMM_USB_SerialOpenCallback(USBD_HandleTypeDef* hUsbDeviceFS)
{
	RAMN_USB_Config.serialOpened = True;
}

// Note that this function  gets called twice when the serial port is closed, and once at startup
void RAMN_USB_SerialCloseCallback(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index)
{
	if(index == 0U)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		RAMN_USB_Config.serialOpened = False;
		RAMN_USB_Config.slcanOpened = False;
		if (sendTask != NULL)
		{
			// Empty the buffer and let the sending task leave the notify waiting phase
			vTaskNotifyGiveFromISR(*sendTask,&xHigherPriorityTaskWoken); //TODO: from ISR or regular ?

			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
			// Note that we do not use the "BufferReset" API because it requires tasks to not be blocked on receiving/sending, which we cannot 100% guarantee
		}
	}
}
#endif

#endif

