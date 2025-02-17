/*
 * ramn_customize.h
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

// This module provides an simple way to customize RAMN
#ifndef INC_RAMN_CUSTOMIZE_H_
#define INC_RAMN_CUSTOMIZE_H_

#include "main.h"

// Function that is called at startup
void 	RAMN_CUSTOM_Init(uint32_t tick);

// Function (from a dedicated task and not an ISR) that is called when a CAN message is received
void	RAMN_CUSTOM_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);

// Function that is called periodically
void 	RAMN_CUSTOM_Update(uint32_t tick);

// Function called periodically by a timer (default: every 1 second).
// Can be modified to execute something periodically with an accurate, arbitrary timer (Update TIM6 settings).
void 	RAMN_CUSTOM_TIM6ISR(TIM_HandleTypeDef *htim);

// Functions called by different tasks

#ifndef ENABLE_CDC
void RAMN_CUSTOM_CustomTask1(void *argument);
void RAMN_CUSTOM_CustomTask2(void *argument);
#endif

#ifndef ENABLE_GSUSB
void RAMN_CUSTOM_CustomTask3(void *argument);
void RAMN_CUSTOM_CustomTask4(void *argument);
#endif

#ifndef ENABLE_DIAG
void RAMN_CUSTOM_CustomTask5(void *argument);
void RAMN_CUSTOM_CustomTask6(void *argument);
#endif

#ifdef ENABLE_I2C
// Function (in ISR) that is called when data was received on the I2C2 interface
void RAMN_CUSTOM_ReceiveI2C(uint8_t buf[], uint16_t buf_size);

// Function (in ISR) that is called when data was requested to be transmitted on the I2C2 interface.
void RAMN_CUSTOM_PrepareTransmitDataI2C(uint8_t buf[], uint16_t buf_size);
#endif

#ifdef ENABLE_UART

// Function called when UART data was received (with \r endline); can be customized for other data formats.
void RAMN_CUSTOM_ReceiveUART(uint8_t buf[], uint16_t buf_size);
#endif

#endif /* INC_RAMN_CUSTOMIZE_H_ */
