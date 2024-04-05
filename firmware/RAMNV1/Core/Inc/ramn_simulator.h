/*
 * ramn_simulator.h
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

// This module handles the "simulator" than creates the relationship between Sensors/Incoming CAN DATA and Actuators/Outgoing CAN DATA

#ifndef INC_RAMN_SIMULATOR_H_
#define INC_RAMN_SIMULATOR_H_

#include "main.h"
#include "ramn_sensors.h"
#include "ramn_actuators.h"
#include "ramn_dbc.h"

#ifdef TARGET_ECUA
#include "ramn_usb.h"
#endif

//Variable that must be set to 1 by UDS first to enable autopilot controls (otherwise ECU will ignore command messages)
extern uint8_t autopilot_enabled;

//Initializes the module
void 	RAMN_SIM_Init(void);

//Update the simulator
void 	RAMN_SIM_UpdatePeriodic(uint32_t tick);


#endif /* INC_RAMN_SIMULATOR_H_ */
