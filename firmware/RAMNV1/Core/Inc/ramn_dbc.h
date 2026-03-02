/*
 * dbc.h
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

// This Module is used too handle periodic data sending and receiving over CAN/CAN-FD

#ifndef INC_RAMN_DBC_H_
#define INC_RAMN_DBC_H_

#include "main.h"
#include "ramn_canfd.h"
#include "ramn_crc.h"
#ifdef ENABLE_JOYSTICK_CONTROLS
#include "ramn_joystick.h"
#endif

typedef volatile struct
{
	volatile uint16_t control_brake; 			//CANID_CONTROL_BRAKE
	volatile uint16_t command_brake; 			//CANID_COMMAND_BRAKE
	volatile uint16_t control_accel; 			//CANID_CONTROL_ACCEL
	volatile uint16_t command_accel; 			//CANID_COMMAND_ACCEL
	volatile uint16_t control_steer; 			//CANID_CONTROL_STEERING
	volatile uint16_t command_steer; 			//CANID_COMMAND_STEERING
	volatile uint8_t  control_shift;			//CANID_CONTROL_SHIFT
	volatile uint16_t command_shift;			//CANID_COMMAND_SHIFT
	volatile uint16_t control_sidebrake; 		//CANID_CONTROL_SIDEBRAKE
	volatile uint16_t command_sidebrake; 		//CANID_COMMAND_SIDEBRAKE
	volatile uint16_t status_rpm; 				//CANID_STATUS_RPM
	volatile uint8_t command_horn;				//CANID_COMMAND_HORN
	volatile uint8_t control_horn; 				//CANID_CONTROL_HORN
	volatile uint16_t command_lights; 			//CANID_COMMAND_LIGHTS
	volatile uint16_t command_turnindicator;	//CANID_COMMAND_TURNINDICATOR
	volatile uint16_t control_enginekey;		//CANID_CONTROL_ENGINEKEY
	volatile uint16_t control_lights; 			//CANID_CONTROL_LIGHTS
	volatile uint8_t joystick;					// Second byte of SHIFT, saved for convenience

} RAMN_DBC_Handle_t;

// DBC Database, always holds the most recent input and output of CAN/Sensors/Actuators
extern RAMN_DBC_Handle_t RAMN_DBC_Handle;

// Used to ask an ECU to stop transmitting periodic CAN messages
extern volatile RAMN_Bool_t RAMN_DBC_RequestSilence;

// Function to Init the DBC handler
void 	RAMN_DBC_Init(void);

// Function to update the DBC when a CAN messages has been received
void 	RAMN_DBC_ProcessCANMessage(uint32_t canid, uint32_t dlc, RAMN_CANFrameData_t* dataframe);

// Function to request the sending of CAN messages maintained by the DBC handler
void 	RAMN_DBC_Send(uint32_t tick);

#if defined(ENABLE_USB)
// Function to update the DBC when a USB message has been received
void 	RAMN_DBC_ProcessUSBBuffer(const uint8_t* buf);
#endif



#endif /* INC_RAMN_DBC_H_ */
