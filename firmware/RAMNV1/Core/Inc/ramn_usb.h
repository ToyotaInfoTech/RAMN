/*
 * ramn_usb.h
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

// This module handles USB communications.

#ifndef INC_RAMN_USB_H_
#define INC_RAMN_USB_H_

#include "main.h"

#ifdef ENABLE_USB

#include "cmsis_os.h"
#include "task.h"
#include "semphr.h"
#include "stream_buffer.h"

#include "../../USB_CompositeDevice/App/usbd_cdc_if.h"

// Timeout for USB TX Operation.
#define USB_TX_TIMEOUT_MS 5000U

// Struct to save the configuration of the USB module
typedef struct
{
	volatile RAMN_Bool_t slcanOpened;					// Flag to specify whether the slcan feature (CAN<->USB) is active or not
	volatile RAMN_Bool_t serialOpened;					// Flag to specify whether a receiving opened serial port has been detected
	volatile RAMN_Bool_t simulatorActive;				// Flag to specify whether a driving simulator is connected or not
	volatile RAMN_Bool_t slcan_enableTimestamp;			// Flag to ENABLE/DISABLE Hardware Timestamp of CAN-FD Messages
	volatile RAMN_Bool_t autoreportErrors;				// Automatically dump registers and error messages when an error is detected
	volatile RAMN_Bool_t addESIFlag; 					// When active, an "i" will be added at the end of received CAN-FD frames that had their ESI flag set.
	volatile uint32_t 	 USBErrCnt;						// Number of time USB errors (except overflows) were detected
	volatile uint32_t 	 USBTxOverflowCnt;				// Number of time USB TX Overflows were detected
#ifdef ENABLE_GSUSB
	volatile uint32_t    queueErrorCnt;                 // Number of time a queue push/pop errors were detected
	volatile RAMN_Bool_t gsusbOpened;					// Flag to specify whether the gs_usb feature (CAN<->USB) is active or not
#endif
} RAMN_USB_Status_t;

// Current USB configuration (shared with other modules)
extern RAMN_USB_Status_t RAMN_USB_Config;

// Initializes the module
void 			RAMN_USB_Init(StreamBufferHandle_t* buffer,  osThreadId_t* pSendTask);

// Sends Data over USB. Blocks until the USB module buffer accepted the operation. May take time.
void 			RAMN_USB_SendFromTask_Blocking(uint8_t* data, uint32_t length);

// Sends Data over USB. Returns as soon as buffer is filled.
RAMN_Result_t 	RAMN_USB_SendFromTask(const uint8_t* data, uint32_t length);

// Acquires the USB TX lock. Must be paired with RAMN_USB_ReleaseLock.
void 			RAMN_USB_AcquireLock(void);

// Releases the USB TX lock previously acquired by RAMN_USB_AcquireLock.
void 			RAMN_USB_ReleaseLock(void);

// Sends Data over USB while the USB TX lock is already held by the caller.
RAMN_Result_t 	RAMN_USB_SendFromTask_Locked(const uint8_t* data, uint32_t length);

// Sends a string over serial USB
RAMN_Result_t 	RAMN_USB_SendStringFromTask(const char* data);

// Sends an unsigned byte (in ASCII) over serial USB
RAMN_Result_t 	RAMN_USB_SendASCIIUint8(uint8_t val);

// Sends an unsigned short (in ASCII) over serial USB
RAMN_Result_t 	RAMN_USB_SendASCIIUint16(uint16_t val);

// Sends an unsigned integer (in ASCII) over serial USB
RAMN_Result_t 	RAMN_USB_SendASCIIUint32(uint32_t val);

// Callback for when USB errors are detected
void 			RAMM_USB_ErrorCallback(USBD_HandleTypeDef* hUsbDeviceFS);

#ifdef ENABLE_USB_AUTODETECT

// Callback for when USB Serial Port OPEN has been detected
void 			RAMM_USB_SerialOpenCallback(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);

// Callback for when USB Serial Port CLOSE has been detected
void 			RAMN_USB_SerialCloseCallback(USBD_HandleTypeDef* hUsbDeviceFS, uint8_t index);

#endif

#endif

#endif /* INC_RAMN_USB_H_ */
