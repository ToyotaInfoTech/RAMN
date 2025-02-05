/*
 * ramn_canfd.h
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

// This module is used to handle CAN-FD peripheral-level communications.

#ifndef INC_RAMN_CANFD_H_
#define INC_RAMN_CANFD_H_

#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "task.h"
#include "semphr.h"

#define CANFD_TX_TIMEOUT_MS 1000

#define SLCAN_FLAG_RX_QUEUE_FULL 1U
#define SLCAN_FLAG_TX_QUEUE_FULL (1U << 1U)
#define SLCAN_FLAG_ERROR_WARNING (1U << 2U)
#define SLCAN_FLAG_DATA_OVERRUN (1U << 3U)
#define SLCAN_FLAG_ERROR_PASSIVE (1U << 5U)
#define SLCAN_FLAG_ARBITRATION_LOST (1U << 6U) //Not reported by STM32L5 CAN-FD CONTROLLER
#define SLCAN_FLAG_BUS_ERROR (1U << 7U)

// linkControlModeIdentifier baudrate definition

#define FDCAN_BITRATE_MIN 10000
#define FDCAN_BITRATE_MAX 1000000

#define FDCAN_BAUDRATE_0  9600
#define FDCAN_BAUDRATE_1  19200
#define FDCAN_BAUDRATE_2  38400
#define FDCAN_BAUDRATE_3  57600
#define FDCAN_BAUDRATE_4  115200
#define FDCAN_BAUDRATE_5  125000
#define FDCAN_BAUDRATE_6  250000
#define FDCAN_BAUDRATE_7  500000
#define FDCAN_BAUDRATE_8  1000000

// Struct to save CAN-FD Module Information
typedef struct
{
	volatile uint32_t ErrorStateIndicator;			//Last Known Error State of FDCAN Peripheral (cf. FDCAN_error_state_indicator)
	volatile uint8_t slcanFlags;					//Flags as defined by LAWICEL CAN232 protocol. "ARBITRATION LOST" not supported.
	volatile uint32_t prevCANError; 				//Error register may be cleared by Error handling - we use this variable to save the state each time we clear it.
	volatile uint32_t CANTXRequestCnt;				//Number of messages were requested to be sent. May not match actual SENT messages if errors occurred.
	volatile uint32_t CANTXSentCnt;					//Number of messages Sent by the ECU
	volatile uint32_t CANRXCnt;						//Number of messages received by the ECU
	volatile uint32_t CANRxOverrunCnt;				//Number of time RX Buffer overrun were detected
	volatile uint32_t CANErrCnt;					//Number of time CAN Errors were detected
	volatile RAMN_Bool_t  busOff;					//Set to True if peripheral transitioned to bus off
	FDCAN_FilterTypeDef sFilterStdConfig; 			//Filter Config for STANDARD CAN ID Messages
	FDCAN_FilterTypeDef sFilterExtConfig; 			//Filter Config for EXTENDED CAN ID Messages
} RAMN_FDCAN_Status_t;

// Element to store current status of CAN message box
extern RAMN_FDCAN_Status_t RAMN_FDCAN_Status;

// Stream buffer for incoming messages
extern StreamBufferHandle_t CANRxDataStreamBufferHandle;

// Stream buffer for outgoing messages
extern StreamBufferHandle_t CANTxDataStreamBufferHandle;

// Interrupt Service Routines called by HAL library ---------------------------

// ISR called when Messages received in FIFO0
void 			HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
// ISR called when message was successfully sent
void 			HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
// Callback for errors related to CAN-FD Controller
void 			HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
// Callback for errors related to CAN-FD bus (NOT errors within CAN Controller)
void 			HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs);

// FDCAN Peripheral Handling functions ---------------------------

// Initialize module with FDCAN controller handle, and main threads to notify.
void 			RAMN_FDCAN_Init(FDCAN_HandleTypeDef* handle, osThreadId_t* s, osThreadId_t* e);

// Resets the FDCAN with values saved in gw, then restarts the peripheral.
// Resets statistics but do not resets filter configurations.
void 			RAMN_FDCAN_ResetPeripheral(void);

// Disables the FDCAN peripheral.
void 			RAMN_FDCAN_Disable(void);

// Update the FDCAN peripheral with specified baudrate (slcan format: '0' to '8'). Not valid until peripheral is reset.
// Required after baudrate change or mode change (e.g. listen mode).
RAMN_Result_t 	RAMN_FDCAN_UpdateBaudrate(uint8_t newSelection);

// Resets statistics kept of the module. Does NOT reset the filter.
void 			RAMN_FDCAN_ResetStatistics(void);

// Function to check whether the current TX buffer can accommodate a message with specified payload size.
RAMN_Bool_t 	RAMN_FDCAN_IsTXBufferSpaceAvailable(uint8_t payloadSize);

// Function to send a CAN Message. Will not block unless semaphore is unavailable.
/*  CAN TX Header as below
 * 		CANTxHeader.Identifier //11 bit for standard, 29bit for extended
		CANTxHeader.IdType // FDCAN_STANDARD_ID or FDCAN_EXTENDED_ID
		CANTxHeader.TxFrameType // FDCAN_DATA_FRAME or FDCAN_REMOTE_FRAME
		CANTxHeader.DataLength // FDCAN_DLC_BYTES_0 to FDCAN_DLC_BYTES_64
		CANTxHeader.ErrorStateIndicator //FDCAN_ESI_ACTIVE or FDCAN_ESI_PASSIVE
		CANTxHeader.BitRateSwitch // FDCAN_BRS_OFF or FDCAN_BRS_ON
		CANTxHeader.FDFormat // FDCAN_CLASSIC_CAN or FDCAN_FD_CAN
		CANTxHeader.TxEventFifoControl // FDCAN_NO_TX_EVENTS or FDCAN_STORE_TX_EVENTS
		CANTxHeader.MessageMarker //0x00 to 0xFF
 */
RAMN_Result_t 	RAMN_FDCAN_SendMessage(const FDCAN_TxHeaderTypeDef* header, const uint8_t* data);

#if defined(TARGET_ECUA)
// Setups the peripheral for communication with ROM FDCAN Bootloader. Cf AN5405.
void RAMN_FDCAN_SetupForSTBootloader(void);
#endif



#endif /* INC_RAMN_CANFD_H_ */
