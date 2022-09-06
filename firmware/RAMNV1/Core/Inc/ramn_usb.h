/*
 * ramn_usb.h
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

// This module handles USB communication

#ifndef INC_RAMN_USB_H_
#define INC_RAMN_USB_H_

#include "main.h"
#include "cmsis_os.h"
#include "task.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "usbd_cdc_if.h"

#ifdef ENABLE_USB

//Timeout for USB TX Operation.
#define USB_TX_TIMEOUT_MS 5000

//Struct to save the configuration of the USB module
typedef struct
{
	volatile RAMN_Bool_t slcanOpened;					//Flag to specify whether the slcan feature (CAN<->USB) is active or not
	volatile RAMN_Bool_t serialOpened;					//Flag to specify whether a receiving opened serial port has been detected
	volatile RAMN_Bool_t simulatorActive;				//Flag to specify whether a driving simulator is connected or not
	volatile RAMN_Bool_t slcan_enableTimestamp;			//Flag to ENABLE/DISABLE Hardware Timestamp of CAN-FD Messages
	volatile RAMN_Bool_t autoreportErrors;				//Automatically dump registers and error messages when an error is detected
	volatile RAMN_Bool_t addESIFlag; 					//When active, an "i" will be added at the end of received CAN-FD frames that had their ESI flag set.
	volatile uint32_t 	 USBErrCnt;						//Number of time USB errors (except overflows) were detected
	volatile uint32_t 	 USBTxOverflowCnt;				//Number of time USB TX Overflows were detected
} RAMN_USB_Status_t;

extern RAMN_USB_Status_t RAMN_USB_Config;

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

#endif

#endif /* INC_RAMN_USB_H_ */
