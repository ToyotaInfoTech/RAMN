/*
 * ramn_sensors.h
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

//This module handles reading data from sensors on expansion boards
//Although Sensors may vary in nature (e.g. from Sliding potentiometer and Rotating potentiometer to switch), they all share a common Electrical circuit.
//E.g., All ECUs receive their sensor values from the exact same pins, and therefore ECUs share the same peripheral configuration.

#ifndef INC_RAMN_SENSORS_H_
#define INC_RAMN_SENSORS_H_

#include "main.h"

//Define below variable if the rotating potentiometer is of the logarithmic kind
#define CHASSIS_LOGARITHMIC_POTENTIOMETER

//Possible position of the "4-position Light switch" on the Chassis Expansion
typedef enum
{
  RAMN_LIGHTSWITCH_UNKNOWNSTATE = 0U,
  RAMN_LIGHTSWITCH_POS1 = 1U, // most left
  RAMN_LIGHTSWITCH_POS2 = 2U,
  RAMN_LIGHTSWITCH_POS3 = 3U,
  RAMN_LIGHTSWITCH_POS4 = 4U, // most right
} RAMN_LightSwitchState_t;
#define RAMN_LIGHTSWITCH_FLOATING RAMN_LIGHTSWITCH_POS4

//Possible positions of the "Sidebrake switch" on the Chassis Expansion
typedef enum
{
  RAMN_SIDEBRAKE_DOWN 	= 0U,
  RAMN_SIDEBRAKE_UP 	= 1U,
  RAMN_SIDEBRAKE_UNKNOWN,
} RAMN_SideBrakeState_t;

//Possible positions of the "Engine Key" on the Body Expansion
typedef enum
{
  RAMN_ENGINEKEY_LEFT 	= 1U,
  RAMN_ENGINEKEY_MIDDLE = 2U,
  RAMN_ENGINEKEY_RIGHT 	= 3U,
} RAMN_EngineKeyState_t;
#define RAMN_ENGINEKEY_FLOATING RAMN_ENGINEKEY_LEFT

//Possible positions of the "Shift Lever" joystick on the Powertrain Expansion
typedef enum
{
  RAMN_LEVER_UNKNOWNSTATE,
  RAMN_LEVER_RELEASED,
  RAMN_LEVER_UP,
  RAMN_LEVER_DOWN,
  RAMN_LEVER_RIGHT,
  RAMN_LEVER_LEFT,
  RAMN_LEVER_PUSH,
} RAMN_LeverState_t;

#if defined(EXPANSION_POWERTRAIN)
//Struct that holds previous and current values of the sensors of the Powertrain Expansion
typedef struct
{
	uint16_t brake_pedal;			//Status of Brake potentiometer
	uint16_t accel_pedal;			//Status of Accelerator potentiometer
	RAMN_LeverState_t shift_lever;  //Status of joystick
	uint8_t gear;					//Increase/decrease at joystick up/down
	uint16_t turnIndicatorRequest;	//If joystick is left/right, consider left/right turn signal is requested (FF00 or 00FF)
	RAMN_Bool_t hornRequest;		//If joystick is pushed, consider horn is requested
} RAMN_SENSORS_PowertrainSensors_t;

extern RAMN_SENSORS_PowertrainSensors_t RAMN_SENSORS_POWERTRAIN;
extern RAMN_SENSORS_PowertrainSensors_t RAMN_SENSORS_POWERTRAIN_PREVIOUS;
#endif

#if defined(EXPANSION_CHASSIS)
//Struct that holds previous and current values of the sensors of the Chassis Expansion
typedef struct
{
	uint16_t steering_wheel; //steering wheel. Right is 0x00, middle is 0x7FF, Left is ~0xFFF,
	RAMN_SideBrakeState_t sidebrake_switch;
	RAMN_LightSwitchState_t lights_switch;
} RAMN_SENSORS_ChassisSensors_t;

extern RAMN_SENSORS_ChassisSensors_t RAMN_SENSORS_CHASSIS;
extern RAMN_SENSORS_ChassisSensors_t RAMN_SENSORS_CHASSIS_PREVIOUS;
#endif

#if defined(EXPANSION_BODY)
//Struct that holds previous and current values of the sensors of the Body Expansion
typedef struct
{
	RAMN_EngineKeyState_t engine_key; //steering wheel,  Right is 0x00, Left is ~0xFC2, middle is 0x7FF
} RAMN_SENSORS_BodySensors_t;

extern RAMN_SENSORS_BodySensors_t RAMN_SENSORS_BODY;
extern RAMN_SENSORS_BodySensors_t RAMN_SENSORS_BODY_PREVIOUS;
#endif


#if defined(EXPANSION_CHASSIS) || defined(EXPANSION_POWERTRAIN) || defined(EXPANSION_BODY)
//Number of ADC used by ECU
#define NUMBER_OF_ADC 3U
//Values to save ADC inputs - filled automatically by DMA
extern uint16_t RAMN_SENSORS_ADCValues[NUMBER_OF_ADC];
#endif


//Initializes the Sensors module
void 	RAMN_SENSORS_Init(void);

//Update the Sensors value. Must be called periodically
void 	RAMN_SENSORS_Update(uint32_t tick);

#endif /* INC_RAMN_SENSORS_H_ */
