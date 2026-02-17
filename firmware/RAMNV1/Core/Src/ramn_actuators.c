/*
 * ramn_actuators.c
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

#include "ramn_actuators.h"
#include "ramn_signal_defs.h"
#include "ramn_can_database.h"

#ifdef EXPANSION_BODY
// Byte that store the state of each LED of ECU D.
static uint8_t LEDState;
#endif

#if (LED_TEST_DURATION_MS > 0U)
// Bool set to 1 when the LED Test over is over. Used to avoid redoing the test on SysTick overflow.
static RAMN_Bool_t LEDTestOver = False;
#endif

void RAMN_ACTUATORS_Init(void)
{
#ifdef EXPANSION_BODY
	LEDState = (uint8_t)(RAMN_DBC_Handle.control_lights&0xFF);
	RAMN_SPI_UpdateLED(&LEDState);
#endif
}

void RAMN_ACTUATORS_SetLampState(uint8_t mask, uint8_t val)
{
	if (val != 0U) RAMN_DBC_Handle.control_lights |= mask;
	else RAMN_DBC_Handle.control_lights &= ~mask;
}

void RAMN_ACTUATORS_ApplyControls(uint32_t tick)
{
#if defined(EXPANSION_CHASSIS) //CHASSIS
	RAMN_Encode_Control_Steering((uint16_t)RAMN_DBC_Handle.control_steer, &msg_control_steering.data->rawData[CAN_SIM_CONTROL_STEERING_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Control_Sidebrake((uint8_t)RAMN_DBC_Handle.control_sidebrake, &msg_control_sidebrake.data->rawData[CAN_SIM_CONTROL_SIDEBRAKE_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Command_Lights((uint16_t)RAMN_DBC_Handle.command_lights, &msg_command_lights.data->rawData[CAN_SIM_COMMAND_LIGHTS_PAYLOAD_OFFSET / 8]);

#elif defined(EXPANSION_POWERTRAIN) //POWERTRAIN
	RAMN_Encode_Control_Brake((uint16_t)RAMN_DBC_Handle.control_brake, &msg_control_brake.data->rawData[CAN_SIM_CONTROL_BRAKE_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Control_Accel((uint16_t)RAMN_DBC_Handle.control_accel, &msg_control_accel.data->rawData[CAN_SIM_CONTROL_ACCEL_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Control_Shift_Joystick((uint8_t)RAMN_DBC_Handle.control_shift, (uint8_t)RAMN_DBC_Handle.joystick, &msg_control_shift.data->rawData[CAN_SIM_CONTROL_SHIFT_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Control_Horn((uint8_t)RAMN_DBC_Handle.control_horn, &msg_control_horn.data->rawData[CAN_SIM_CONTROL_HORN_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Command_TurnIndicator((uint16_t)RAMN_DBC_Handle.command_turnindicator, &msg_command_turnindicator.data->rawData[CAN_SIM_COMMAND_TURNINDICATOR_PAYLOAD_OFFSET / 8]);

#elif defined(EXPANSION_BODY) //BODY
	RAMN_Encode_Control_EngineKey((uint8_t)RAMN_DBC_Handle.control_enginekey, &msg_control_enginekey.data->rawData[CAN_SIM_CONTROL_ENGINEKEY_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Control_Lights((uint8_t)RAMN_DBC_Handle.control_lights, &msg_control_lights.data->rawData[CAN_SIM_CONTROL_LIGHTS_PAYLOAD_OFFSET / 8]);

	LEDState = (uint8_t)RAMN_DBC_Handle.control_lights;
#if (LED_TEST_DURATION_MS > 0)
	if((tick < LED_TEST_DURATION_MS) && (LEDTestOver == False)) LEDState = 0xFF;
	else LEDTestOver = True;
#endif
	RAMN_SPI_UpdateLED(&LEDState);
#endif
}
