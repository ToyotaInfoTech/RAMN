/*
 * ramn_trng.h
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

// Modules to easily generate random bytes, with automatic refresh

#ifndef INC_RAMN_TRNG_H_
#define INC_RAMN_TRNG_H_

#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "semphr.h"

//When to trigger a "refill" of the TRNG pool
#define RNG_REFILL_THRESHOLD (TRNG_POOL_SIZE/2)

//Callback for TRNG Peripheral ISR
void 		HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef *hrng, uint32_t random32bit);

//Callback for TRNG Errors
void 		HAL_RNG_ErrorCallback(RNG_HandleTypeDef *hrng);

//Initializes the module
void 		RAMN_RNG_Init(RNG_HandleTypeDef* handle);

//Get one random uint8_t from the module
uint8_t  	RAMN_RNG_Pop8(void);

//Get one random uint16_t from the module
uint16_t 	RAMN_RNG_Pop16(void);

//Get one random uint32_t
uint32_t 	RAMN_RNG_Pop32(void);

#endif /* INC_RAMN_TRNG_H_ */
