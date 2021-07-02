/*
 * ramn_crc.h
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

// This module can be used to compute CRC (Cyclic Redundancy Check)

#ifndef INC_RAMN_CRC_H_
#define INC_RAMN_CRC_H_

#include "main.h"

//Initializes the module
void RAMN_CRC_Init(CRC_HandleTypeDef* h);

// Computers the CRC of specified buffer
uint32_t RAMN_CRC_Calculate(const uint8_t* src, uint32_t size);

#endif /* INC_RAMN_CRC_H_ */

