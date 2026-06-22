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
	// Zero-copy IN transfer: the DMA reads directly from 'data', so we must not return until the
	// hardware is done. Poll the real endpoint state (RAMN_CDC_GetTXStatus); the Cplt notification is
	// only a wake-up. The notification counts, so trusting it as the buffer-free signal can desync by one (e.g. a ZLP) and overwrite the buffer mid-DMA.

	// Drop any stale token so the wait below pairs with this transfer only.
	(void)ulTaskNotifyTake(pdTRUE, 0U);

	if (CDC_Transmit_FS((uint8_t*)data, length) != USBD_OK)
	{
		// Transfer did not start, so 'data' is already free to reuse.
		RAMN_USB_Config.USBTxOverflowCnt++;
		return;
	}

	// Block until the IN endpoint is idle, bounded by USB_TX_TIMEOUT_MS.
	uint32_t waited = 0U;
	while (RAMN_CDC_GetTXStatus() != USBD_OK)
	{
		(void)ulTaskNotifyTake(pdTRUE, 1U);   // sleep on Cplt notify or 1 tick, then re-poll TxState
#ifdef ENABLE_USB_AUTODETECT
		// Port closed mid-transfer: the close callback clears serialOpened and wakes us. The transfer
		// never completes, so abort now instead of waiting out the full timeout.
		if (RAMN_USB_Config.serialOpened == False)
		{
			RAMN_CDC_AbortTx();
			break;
		}
#endif
		if (++waited >= USB_TX_TIMEOUT_MS)
		{
			// Stuck transfer: abort so the buffer can be reused safely.
			RAMN_USB_Config.USBErrCnt++;
			RAMN_CDC_AbortTx();
			break;
		}
	}
}

RAMN_Result_t RAMN_USB_SendFromTask(const uint8_t* data, uint32_t length)
{
	size_t xBytesSent = 0;
	RAMN_Result_t result = RAMN_OK;

	while (xSemaphoreTake(USB_TX_SEMAPHORE, portMAX_DELAY ) != pdTRUE);

	// Frame-atomic: only enqueue if the whole frame fits, else drop it to keep the SLCAN stream
	// frame-aligned. Single writer (mutex held) + consumer only frees space, so the send writes in full.
	if (xStreamBufferSpacesAvailable(*usbTxBuffer) >= length)
	{
		xBytesSent = xStreamBufferSend(*usbTxBuffer, data, length, 0U);
	}
	xSemaphoreGive(USB_TX_SEMAPHORE);
	if (xBytesSent != length)
	{
		RAMN_USB_Config.USBTxOverflowCnt++;
		result = RAMN_ERROR;
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

	// Caller already holds USB_TX_SEMAPHORE. Frame-atomic: see RAMN_USB_SendFromTask.
	if (xStreamBufferSpacesAvailable(*usbTxBuffer) >= length)
	{
		xBytesSent = xStreamBufferSend(*usbTxBuffer, data, length, 0U);
	}
	if (xBytesSent != length)
	{
		RAMN_USB_Config.USBTxOverflowCnt++;
		result = RAMN_ERROR;
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
void RAMM_USB_SerialOpenCallback(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index)
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

