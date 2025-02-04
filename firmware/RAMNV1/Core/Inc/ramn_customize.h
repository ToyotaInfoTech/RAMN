/*
 * ramn_customize.h
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2024 TOYOTA MOTOR CORPORATION.
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

//Function that is called at startup
void 	RAMN_CUSTOM_Init(uint32_t tick);

//Function (from a dedicated task and not an ISR) that is called when a CAN message is received
void	RAMN_CUSTOM_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);

//Function that is called periodically
void 	RAMN_CUSTOM_Update(uint32_t tick);

#ifdef ENABLE_I2C
//Function (in ISR) that is called when data was received on the I2C2 interface
void RAMN_CUSTOM_ReceiveI2C(uint8_t buf[], uint16_t buf_size);

//Function (in ISR) that is called when data was requested to be transmitted on the I2C2 interface.
void RAMN_CUSTOM_PrepareTransmitDataI2C(uint8_t buf[], uint16_t buf_size);
#endif
#endif /* INC_RAMN_CUSTOMIZE_H_ */
