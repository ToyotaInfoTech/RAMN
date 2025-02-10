/*
 * ramn_joystick.h
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

// Module to monitor joystick controls (from any ECU receiving the shift CAN message, by default only A)
// Simple module assuming RAMN's joystick goes back to center between presses.

#ifndef INC_RAMN_JOYSTICK_H_
#define INC_RAMN_JOYSTICK_H_

#include "main.h"

#ifdef ENABLE_JOYSTICK_CONTROLS

#include "cmsis_os.h"
#include "stream_buffer.h"
#include "semphr.h"

// Initializes the module.
void RAMN_Joystick_Init();

// Updates the module with latest joystick input.
void RAMN_Joystick_Update(uint8_t joystick_data);

// Pops the next queued joystick event.
JoystickEventType RAMN_Joystick_Pop(void);

#endif
#endif /* INC_RAMN_JOYSTICK_H_ */
