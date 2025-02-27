/*
 * ramn_customize.c
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

#include "ramn_customize.h"
#include "ramn_canfd.h"
#include "ramn_sensors.h"
#include "ramn_dbc.h"

#ifdef ENABLE_CDC
#include "ramn_cdc.h"
#endif

#ifdef ENABLE_UART
#include "ramn_uart.h"
#endif

// Loop counter for RAMN_CUSTOM_Update
static uint32_t loopCounter = 0;

// Number of time RAMN_CUSTOM_TIM6ISR has been called (by default, time in s from boot)
static volatile uint32_t tim6val = 0;

void 	RAMN_CUSTOM_Init(uint32_t tick)
{
	loopCounter = 0;
}

// Called when a CAN message is received (Hardware filters should be configured separately in ramn_canfd.c; with recvStdCANIDList and recvExtCANIDList)
// Note that by default, ECU A has no filter.
void	RAMN_CUSTOM_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick)
{
	// Fields that you may want to use:
	// pHeader->Identifier: (11-bit val for standard, 29-bit for extended)
	// pHeader->IdType: FDCAN_STANDARD_ID or FDCAN_EXTENDED_ID
	// pHeader->RxFrameType:  FDCAN_DATA_FRAME or FDCAN_REMOTE_FRAME
	// DataLength: length of CAN payload, FDCAN_DLC_BYTES_0 (0) to FDCAN_DLC_BYTES_8 (8) for CAN, FDCAN_DLC_BYTES_0 (0) to FDCAN_DLC_BYTES_64 (0xF, Not 64) for CAN-FD.
	// pHeader->ErrorStateIndicator: For CAN-FD, either FDCAN_ESI_ACTIVE or FDCAN_ESI_PASSIVE
	// pHeader->BitRateSwitch: For CAN-FD, either FDCAN_BRS_OFF or FDCAN_BRS_ON
	// pHeader->FDFormat: FDCAN_CLASSIC_CAN or FDCAN_FD_CAN
	// pHeader->RxTimestamp: 16-bit value for RX timestamp, MAY NOT BE CONFIGURED CORRECTLY
	// See FilterIndex and IsFilterMatchingFrame for additional fields.
}

#ifdef ENABLE_CDC
// This function is called when a USB serial (CDC) line is received (terminated by \r, which is not included in the buffer).
// if you need another type of line terminator, modify CDC_Receive_FS in usbd_cdc_if.c.
// Return True to ask the ECU to skip this line, return False to continue processing as usual.
RAMN_Bool_t RAMN_CUSTOM_ProcessCDCLine(uint8_t* buffer, uint32_t size)
{
	// If you return True, you can entirely override USB communications, meaning that ECU A will lose the ability to forward slcan commands.
	// This means that you will lose the ability to use RAMN scripts (including reflashing over USB DFU).
	// Only return True if that is the behavior that you expect, and have another method for ECU A reflashing.
	// If you want to make sure that you (at least) keep the option to reprogram ECU A, uncomment the line below and keep it at the beginning.
	// if (size > 0U && buffer[1] == 'D') return False;

	return False; // WARNING read comments above before editing
}
#endif

// Called periodically from main task
void RAMN_CUSTOM_Update(uint32_t tick)
{
	// This function is called by a dedicated periodic task, which means code can here won't block other functionalities (such as receiving CAN messages).
	// Modify SIM_LOOP_CLOCK_MS if you want to use another period than 10ms.

	// Code here is executed every 10ms

	if ((loopCounter % 10) == 0)
	{
		// Code here is executed every 100ms
	}

	if ((loopCounter % 100) == 0)
	{
		// Code here is executed every 1s

		// Example: send UART data every second
#ifdef ENABLE_UART
		RAMN_UART_SendStringFromTask("Hello from RAMN\r");
#endif

		// Example: Send CAN message every second (payload size 8)
		/*
		FDCAN_TxHeaderTypeDef header;
		uint8_t data[8U];

		header.BitRateSwitch = FDCAN_BRS_OFF;	// Bitrate switching OFF (only needed for CAN-FD, but set anyway); other option is FDCAN_BRS_ON.
		header.FDFormat = FDCAN_CLASSIC_CAN; 	// Classic CAN; other option is FDCAN_FD_CAN.
		header.TxFrameType = FDCAN_DATA_FRAME;	// Data frame; other option is FDCAN_REMOTE_FRAME, only for classic CAN.
		header.IdType = FDCAN_STANDARD_ID;		// Standard identifer; other optin is FDCAN_EXTENDED_ID for extended.
		header.Identifier = 0x123; 				// Identifier.
		header.DataLength = 8U;  				// DLC (Payload size).

		// Decide CAN message payload content
		RAMN_memset(data, 0x77, 8U); // write 0x77 8 times

		// Send message
		RAMN_FDCAN_SendMessage(&header,data);
		*/


		// Example: Execute every second, only if joystick is currently pressed down; only from ECU C (which is in charge of the sensor)
		// This is based on physical sensor data (ramn_sensors.h)
		/*
		if (RAMN_SENSORS_POWERTRAIN.shiftJoystick == RAMN_SHIFT_PUSH)
		{
			// Do something
		}
		*/

		// Example: Execute every second, only if joystick is currently pressed down; from ANOTHER ECU (other than ECU C)
		// This is based on the latest joystick CAN message received (ramn_dbc.h)
		// You need to make sure that the joystick CAN message is processed by adding #define RECEIVE_CONTROL_SHIFT in vehicle_specific.h
		/*
		if (RAMN_DBC_Handle.joystick == RAMN_SHIFT_PUSH)
		{
			// Do something
		}
		*/


	}

	loopCounter += 1; 	//You may want to add a check for integer overflow.
}

/* TIMERS */

// TIM16 is configured as a free running timer (e.g., to use for accurate timing measurements). Default: 1MHz counter.
// TIM6  is configured as a trigger periodically calling an ISR. Default: every 1s.

// To reset TIM16 (e.g., to start a measurement), use:
// __HAL_TIM_SET_COUNTER(&htim16, 0);
// To read the value of TIM16 (to get your timing measurement), use:
// __HAL_TIM_GET_COUNTER(&htim16);  (should return uint16_t)


// Function periodically called by timer (not depending on freeRTOS)
void RAMN_CUSTOM_TIM6ISR(TIM_HandleTypeDef *htim)
{
	// WARNING: THIS FUNCTION IS CALLED BY AN ISR AND SHOULD RETURN QUICKLY.
	// You can only use freeRTOS features that end with "FromISR"

	tim6val++;
}


/* TASK HOOKS */

// The functions below are called by tasks that are started but not used (e.g., USB task when USB is not active).
// They can be used to implement tasks that will not interfere with the main periodic task.
// Note that the priority of these tasks is typically higher than the periodic task, therefore they MUST periodically let other tasks execute (e.g. by calling vTaskDelayUntil or osDelay).
// Alternatively, if you want to execute slow and long code, you may alter the priority of the task.
// If you do not need these functions, use vTaskDelete(NULL) to delete the task.
// In all cases, make sure that you only modify the behavior of the targeted ECU, and not all ECUs (e.g., by using #ifdef TARGET_ECUB or #ifndef TARGET_ECUA).

#ifndef ENABLE_CDC
void RAMN_CUSTOM_CustomTask1(void *argument)
{
	//Called by RAMN_ReceiveUSBFunc
	vTaskDelete(NULL);
}

void RAMN_CUSTOM_CustomTask2(void *argument)
{
	//Called by RAMN_SendUSBFunc
	vTaskDelete(NULL);
}
#endif

#ifndef ENABLE_GSUSB
void RAMN_CUSTOM_CustomTask3(void *argument)
{
	//Called by RAMN_RxTask2Func
	vTaskDelete(NULL);
}

void RAMN_CUSTOM_CustomTask4(void *argument)
{
	//Called by RAMN_TxTask2Func
	vTaskDelete(NULL);
}
#endif

#ifndef ENABLE_DIAG
void RAMN_CUSTOM_CustomTask5(void *argument)
{
	//RAMN_DiagRXFunc
	vTaskDelete(NULL);
}

void RAMN_CUSTOM_CustomTask6(void *argument)
{
	//RAMN_DiagTXFunc
	vTaskDelete(NULL);
}
#endif


/* HARDWARE INTERFACE HOOKS */

#ifdef ENABLE_I2C
void RAMN_CUSTOM_ReceiveI2C(uint8_t buf[], uint16_t buf_size)
{
	// Warning: This function is called within an ISR. It should not use freeRTOS functions not available to ISRs, and should not be blocking.
	// See RAMNV1.ioc for I2C device address (likely 0x77)
	// Note that by default, buf_size is fixed and equal to I2C_RX_BUFFER_SIZE. Function will NOT be called if fewer bytes are received.
	// You'll need to modify HAL_I2C_AddrCallback and HAL_I2C_SlaveRxCpltCallback in main.c if you need another behavior.
}

void RAMN_CUSTOM_PrepareTransmitDataI2C(uint8_t buf[], uint16_t buf_size)
{
	// Warning: This function is called within an ISR. It should not use freeRTOS functions not available to ISRs, and should not be blocking.
	// Note that you cannot modify buf_size, only buf.
	// You'll need to modify HAL_I2C_AddrCallback in main.c if you need another behavior.
}
#endif

#ifdef ENABLE_UART

// You can send UART data using RAMN_UART_SendStringFromTask or RAMN_UART_SendFromTask, which are both non-blocking.

void RAMN_CUSTOM_ReceiveUART(uint8_t buf[], uint16_t buf_size)
{
	// By default, this function receives commands line by line, without endline character (\r)
	// You can modify this behavior in main.c (look for HAL_UART_Receive_IT and  HAL_UART_RxCpltCallback)
}
#endif
