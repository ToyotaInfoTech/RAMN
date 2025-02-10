/*
 * ramn_trng.h
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

// Module to easily generate random bytes, with automatic refresh
// Uses the STM32 TRNG module

#ifndef INC_RAMN_TRNG_H_
#define INC_RAMN_TRNG_H_

#include "main.h"

#ifdef USE_TRNG_BUFFER
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "semphr.h"
#endif

// When to trigger a "refill" of the TRNG pool, if a stream buffer is used
#define RNG_REFILL_THRESHOLD (TRNG_POOL_SIZE/2)

// Initializes the module.
void 		RAMN_RNG_Init(RNG_HandleTypeDef* handle);

// Returns a random uint8_t from the module.
uint8_t  	RAMN_RNG_Pop8(void);

// Returns a random uint16_t.
uint16_t 	RAMN_RNG_Pop16(void);

// Returns a random uint32_t.
uint32_t 	RAMN_RNG_Pop32(void);

#endif /* INC_RAMN_TRNG_H_ */
