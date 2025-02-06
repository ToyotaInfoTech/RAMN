/*
 * ramn_ecucontrol.h
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

// This module handles the POWER ENABLE and BOOT0 pins of ECU B, C, and D.

#ifndef INC_RAMN_ECUCONTROL_H_
#define INC_RAMN_ECUCONTROL_H_

#include "main.h"
#include "cmsis_os.h"

#if defined(TARGET_ECUA)

// Sets the state of the "BOOT0 PIN" of ECU specified by ecuName ('B', 'C', or 'D')
void 	RAMN_ECU_SetBoot0(char ecuName, uint8_t state);

// Sets the state of the "ENABLE PIN" of ECU specified by ecuName ('B', 'C', or 'D')
void 	RAMN_ECU_SetEnable(char ecuName, uint8_t state);

// Sets the state of the "ENABLE PIN" of ECU B C D
void 	RAMN_ECU_SetEnableAll(uint8_t state);

// Sets the state of all "BOOT0" pins for ECU B C D
void 	RAMN_ECU_SetBoot0All(uint8_t state);

// Sets the ECUs to their default state (BOOT0=0, ENABLE=1)
void 	RAMN_ECU_SetDefaultState(void);

#endif

#endif /* INC_RAMN_ECUCONTROL_H_ */
