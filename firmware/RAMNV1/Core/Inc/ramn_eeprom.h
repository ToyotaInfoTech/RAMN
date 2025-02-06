/*
 * ramn_eeprom.h
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

// This modules wraps the EEPROM emulation layer of STMicroelectronics (cf. AN4894)

#ifndef INC_RAMN_EEPROM_H_
#define INC_RAMN_EEPROM_H_

#include "main.h"

#if defined(ENABLE_EEPROM_EMULATION)

#include "eeprom_emul.h"

#if PAGES_NUMBER > 4
#warning "EEPROM layer using more than 4 pages"
#endif

// Index 0x0000 is not supported (reserved by EEPROM Layer)
// Index 0x0001 to 0x00FF are reserved for internal management
//   -> VIN internal logging, DTC number and DTC values
// Index 0x0100 to 0xEFFF can be read and written with arbitrary values
// Index 0xF000 to 0xFFFE have special treatment based on ISO14229
// Index 0xFFFF is not supported (reserved by EEPROM Layer)
#define RESERVED_0000_INDEX	    0x0000
#define VIN_BYTES1_4_INDEX   	0x0001
#define VIN_BYTES5_8_INDEX   	0x0002
#define VIN_BYTES9_12_INDEX  	0x0003
#define VIN_BYTES13_16_INDEX 	0x0004
#define VIN_BYTES17_20_INDEX	0x0005
#define DTC_NUMBER_INDEX		0x0010
#define DTC_LAST_VALID_ADDRESS	0x00FF
#define RESERVED_FFFF_INDEX     0xFFFF

// Initializes the EEPROM Layer. May take some time.
EE_Status 	RAMN_EEPROM_Init();

// Writes a 32-bit value at the specified index. May take some time.
EE_Status 	RAMN_EEPROM_Write32(uint16_t index, uint32_t val);

// Reads a 32-bit value from the specified index.
EE_Status 	RAMN_EEPROM_Read32(uint16_t index, uint32_t* pval);

#endif

#endif /* INC_RAMN_EEPROM_H_ */
