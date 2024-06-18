/*
 * ramn_joystick.h
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 TOYOTA MOTOR CORPORATION.
  * ALL RIGHTS RESERVED.</center></h2>
  *
  * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
 */

// Module to monitor joysticks control from ECU C
// Simple module assuming RAMN's joystick goes back to center between presses.

#ifndef INC_RAMN_JOYSTICK_H_
#define INC_RAMN_JOYSTICK_H_

#include "main.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "semphr.h"



void RAMN_Joystick_Init();
void RAMN_Joystick_Update(uint8_t joystick_data);
JoystickEventType RAMN_Joystick_Pop(void);

#endif /* INC_RAMN_JOYSTICK_H_ */
