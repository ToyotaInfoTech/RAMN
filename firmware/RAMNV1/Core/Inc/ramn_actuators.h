/*
 * ramn_actuators.h
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

//Module to apply controls to actuators on RAMN board
//Module also handles updating of actuators values in the DBC handler

#ifndef INC_RAMN_ACTUATORS_H_
#define INC_RAMN_ACTUATORS_H_

#include "main.h"
#include "ramn_spi.h"
#include "ramn_dbc.h"


//Mask for each LED on the BODY expansion
#define LED_BATTERY 		(0x01) //D6
#define LED_CHECKENGINE		(0x02) //D7
#define LED_SIDEBRAKE 		(0x04) //D8
#define LED_TAILLAMP 		(0x08) //D3
#define LED_LOWBEAM 		(0x10) //D4
#define LED_HIGHBEAM 		(0x20) //D5
#define LED_LEFTTURN 		(0x40) //D1
#define LED_RIGHTTURN 		(0x80) //D2

//Init the Module
void 	RAMN_ACTUATORS_Init(void);

//Set the value of the bit specified by "mask" to the value specified by "val" (0 or 1)
void 	RAMN_ACTUATORS_SetLampState(uint8_t mask, uint8_t val);

//Applies requested controls to actuators, e.g. light-up LEDs on the board, or update the payload of outgoing CAN messages
void 	RAMN_ACTUATORS_ApplyControls(uint32_t tick);

#endif /* INC_RAMN_ACTUATORS_H_ */
