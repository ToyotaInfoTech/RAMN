/*
 * ramn_utils.h
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

// This module implements common tools and definitions

#ifndef INC_RAMN_UTILS_H_
#define INC_RAMN_UTILS_H_

#include "main.h"

// Definition for common operations
typedef enum
{
	False       = 0x00,
	True        = 0x01,
} RAMN_Bool_t;

typedef enum
{
	RAMN_OK        = 0x00,
	RAMN_ERROR     = 0x01,
	RAMN_TRY_LATER = 0x02,
} RAMN_Result_t;

// Functions to convert from to uint ASCII. Return number of bytes written
uint32_t  rawtoASCII(uint8_t* dst, const uint8_t* src, uint32_t raw_size);
uint32_t  uint32toASCII(uint32_t src, uint8_t* dst);
uint32_t  uint16toASCII(uint16_t src, uint8_t* dst);
uint32_t  uint12toASCII(uint16_t src, uint8_t* dst);
uint32_t  uint8toASCII(uint8_t src, uint8_t* dst);
uint32_t  uint4toASCII(uint8_t src, uint8_t* dst);

// Converts an integer to BCD representation, with null terminator. May write up to 21 bytes.
// Returns the number of characters written, excluding null terminator.
uint8_t uintToBCD(uint64_t val, char *dst);

// Functions to convert from ASCII to uint
void     ASCIItoRaw(uint8_t* dst, const uint8_t* src, uint32_t raw_size);
uint8_t  ASCIItoUint4 (const uint8_t* src);
uint8_t  ASCIItoUint8 (const uint8_t* src);
uint16_t ASCIItoUint16(const uint8_t* src);
uint16_t ASCIItoUint12(const uint8_t* src);
uint32_t ASCIItoUint32(const uint8_t* src);

// Functions to convert from arrays of bytes to uint32_t or uint16_t
uint32_t convertBytesToUint32(uint8_t *data, uint16_t dataSize);   // Big Endian
uint32_t convertBytesToUint32_l(uint8_t *data, uint16_t dataSize); // Little Endian
uint16_t convertBytesToUint16(uint8_t *data, uint16_t dataSize);   // Big Endian

// Converts uint16_t or uint32_t to byte array
uint16_t convertUint16ToBytes(uint8_t *data, uint16_t dataSize, uint16_t n);
uint16_t convertUint32ToBytes(uint8_t *data, uint16_t dataSize, uint32_t n);

// Functions to convert STM32 HAL FDCAN DLC format (uint32 enumeration) to actual payload size (0 to 64)
uint8_t  DLCtoUINT8(uint32_t dlc_enum);
uint32_t UINT8toDLC(uint8_t dlc);

//Regulator memset operation
void     RAMN_memset(void* dst, uint8_t byte, uint32_t size);

// Regular memcpy operation
void RAMN_memcpy(void* dst, const void* src, uint32_t size);

// Regular strlen operation
uint16_t RAMN_strlen(const char *str);

// Apply required endian, used if CAN data should use Big Endian
// TODO: replace by modifying RAMN_DefaultCANFrameFormat_t defition (?), apply other fields
uint16_t applyEndian16(uint16_t val);

// Wrapper for osDelay
void     RAMN_TaskDelay(uint32_t msec);

#endif /* INC_RAMN_UTILS_H_ */
