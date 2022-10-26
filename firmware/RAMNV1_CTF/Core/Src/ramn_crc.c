/*
 * ramn_crc.c
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

#include "ramn_crc.h"

//pointer to Handle for CRC engine
static CRC_HandleTypeDef* pCRCHandle;

void RAMN_CRC_Init(CRC_HandleTypeDef* h)
{
	pCRCHandle = h;
}

uint32_t RAMN_CRC_Calculate(const uint8_t* src, uint32_t size)
{
	return HAL_CRC_Calculate(pCRCHandle, (uint32_t*)(src), size);
}
