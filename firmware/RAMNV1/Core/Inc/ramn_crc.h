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

/////////////////////////////////////////////////////////////////////
// Hardware CRC and Software CRC may use different CRC parameters. //
/////////////////////////////////////////////////////////////////////

//Initializes the module
void RAMN_CRC_Init(CRC_HandleTypeDef* h);

// These functions use the CRC engine (that may be overtaken by the EEPROM emulation layer).
// You can still use them if you ensure there is no conflict
#ifndef ENABLE_EEPROM_EMULATION
// Computes the CRC of specified buffer (Hardware implementation, not compatible with EEPROM emulation).
uint32_t RAMN_CRC_Calculate(const uint8_t* src, uint32_t size);

// Accumulates the CRC with specified buffer  (Hardware implementation, not compatible with EEPROM emulation).
uint32_t RAMN_CRC_Accumulate(const uint8_t* src, uint32_t size);
#endif

// Computes the CRC of specified buffer (Software implementation).
uint32_t RAMN_CRC_SoftCalculate(const uint8_t* buf, uint32_t size);


#endif /* INC_RAMN_CRC_H_ */

