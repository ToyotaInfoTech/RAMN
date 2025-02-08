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
	msg_control_steering.data->ramnData.payload = applyEndian16((uint16_t)RAMN_DBC_Handle.control_steer);
	msg_control_sidebrake.data->ramnData.payload = RAMN_DBC_Handle.control_sidebrake;
	msg_command_lights.data->ramnData.payload = RAMN_DBC_Handle.command_lights;
#elif defined(EXPANSION_POWERTRAIN) //POWERTRAIN
	msg_control_brake.data->ramnData.payload = applyEndian16((uint16_t)RAMN_DBC_Handle.control_brake);
	msg_control_accel.data->ramnData.payload = applyEndian16((uint16_t)RAMN_DBC_Handle.control_accel);
	msg_control_shift.data->ramnData.payload = RAMN_DBC_Handle.control_shift | (RAMN_DBC_Handle.joystick << 8);
	msg_command_horn.data->ramnData.payload = RAMN_DBC_Handle.command_horn;
	msg_command_turnindicator.data->ramnData.payload = RAMN_DBC_Handle.command_turnindicator;

#elif defined(EXPANSION_BODY) //BODY
	msg_control_enginekey.data->ramnData.payload = RAMN_DBC_Handle.control_enginekey;
	msg_control_lights.data->ramnData.payload = RAMN_DBC_Handle.control_lights;
#if (LED_TEST_DURATION_MS > 0)
	if((tick < LED_TEST_DURATION_MS) && (LEDTestOver == False)) RAMN_DBC_Handle.control_lights = 0xFF;
	else LEDTestOver = True;
#endif
	RAMN_SPI_UpdateLED((uint8_t*)&(RAMN_DBC_Handle.control_lights));
#endif
}
