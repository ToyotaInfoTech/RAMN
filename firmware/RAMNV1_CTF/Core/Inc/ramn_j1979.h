/*
 * ramn_j1979.h
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

// This Modules handles J1979 (OBD-II) Diagnostics
// Currently a dummy File for future implementations
// TODO: Implement

#ifndef INC_RAMN_J1979_H_
#define INC_RAMN_J1979_H_

#include "main.h"
#if defined(ENABLE_UDS) || defined(ENABLE_KWP)
#include "ramn_isotp.h"

RAMN_Result_t RAMN_J1979_ProcessMessage(const uint8_t* data, uint16_t size, uint8_t* answerData, uint16_t* answerSize);

#endif

#endif /* INC_RAMN_J1979_H_ */
