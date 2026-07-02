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
#include "ramn_vehicle_specific.h"
#include "ramn_sensors.h"
#include "ramn_dbc.h"
#include "ramn_traffic.h"

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
#ifdef ENABLE_SPI
	RAMN_SPI_UpdateLED(&LEDState);
#endif
#endif
}

void RAMN_ACTUATORS_SetLampState(uint8_t mask, uint8_t val)
{
	if (val != 0U) RAMN_DBC_Handle.control_lights |= mask;
	else RAMN_DBC_Handle.control_lights &= ~mask;
}

void RAMN_ACTUATORS_ApplyControls(uint32_t tick)
{
	// Payload offset is 0 in both traffic modes, so the default/J1939 encode sites are identical
	// except for which codec bodies run -- that is now selected by the active codec table. The
	// per-ECU structure is preserved: each ECU encodes only the signals it physically produces
	// (e.g. ECU A's command_* normally come from USB/sensors, not from here, outside showcase mode).
#if defined(TARGET_ECUA)
	g_trafficProfile->codec[SIG_CONTROL_HORN].encode((uint16_t)RAMN_DBC_Handle.control_horn, &txRuntime[TXIDX_CONTROL_HORN].data->rawData[0]);

#elif defined(EXPANSION_CHASSIS) //CHASSIS
	g_trafficProfile->codec[SIG_CONTROL_STEERING].encode((uint16_t)RAMN_DBC_Handle.control_steer, &txRuntime[TXIDX_CONTROL_STEERING].data->rawData[0]);
	g_trafficProfile->codec[SIG_CONTROL_SIDEBRAKE].encode((uint16_t)RAMN_DBC_Handle.control_sidebrake, &txRuntime[TXIDX_CONTROL_SIDEBRAKE].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_LIGHTS].encode((uint16_t)RAMN_DBC_Handle.command_lights, &txRuntime[TXIDX_COMMAND_LIGHTS].data->rawData[0]);

#elif defined(EXPANSION_POWERTRAIN) //POWERTRAIN
	g_trafficProfile->codec[SIG_CONTROL_BRAKE].encode((uint16_t)RAMN_DBC_Handle.control_brake, &txRuntime[TXIDX_CONTROL_BRAKE].data->rawData[0]);
	g_trafficProfile->codec[SIG_CONTROL_ACCEL].encode((uint16_t)RAMN_DBC_Handle.control_accel, &txRuntime[TXIDX_CONTROL_ACCEL].data->rawData[0]);
	// Control_Shift + Joystick are combined into one message (special, not a codec-table row).
	if (g_trafficProfile->usesExtendedId)
		RAMN_Encode_Control_Shift_Joystick_J1939((uint8_t)RAMN_DBC_Handle.control_shift, (uint8_t)RAMN_DBC_Handle.joystick, &txRuntime[TXIDX_CONTROL_SHIFT].data->rawData[0]);
	else
		RAMN_Encode_Control_Shift_Joystick_Default((uint8_t)RAMN_DBC_Handle.control_shift, (uint8_t)RAMN_DBC_Handle.joystick, &txRuntime[TXIDX_CONTROL_SHIFT].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_HORN].encode((uint16_t)RAMN_DBC_Handle.command_horn, &txRuntime[TXIDX_COMMAND_HORN].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_TURNINDICATOR].encode((uint16_t)RAMN_DBC_Handle.command_turnindicator, &txRuntime[TXIDX_COMMAND_TURNINDICATOR].data->rawData[0]);
	// In J1939 the joystick buttons ship as their own PGN message (special, not a codec-table row).
	// The slot exists in both modes; only encode it when the active profile actually transmits it.
	if (g_trafficProfile->usesExtendedId)
	{
		RAMN_Encode_JoystickButtons_J1939((uint8_t)RAMN_SENSORS_POWERTRAIN.shiftJoystick, &txRuntime[TXIDX_JOYSTICK_BUTTONS].data->rawData[0]);
	}

#elif defined(EXPANSION_BODY) //BODY
	g_trafficProfile->codec[SIG_CONTROL_ENGINEKEY].encode((uint16_t)RAMN_DBC_Handle.control_enginekey, &txRuntime[TXIDX_CONTROL_ENGINEKEY].data->rawData[0]);
	g_trafficProfile->codec[SIG_CONTROL_LIGHTS].encode((uint16_t)RAMN_DBC_Handle.control_lights, &txRuntime[TXIDX_CONTROL_LIGHTS].data->rawData[0]);

	LEDState = (uint8_t)RAMN_DBC_Handle.control_lights;
#if (LED_TEST_DURATION_MS > 0)
	if((tick < LED_TEST_DURATION_MS) && (LEDTestOver == False)) LEDState = 0xFF;
	else LEDTestOver = True;
#endif
	// Preserves original per-mode behavior: default guarded by ENABLE_SPI, J1939 was unconditional.
#if defined(ENABLE_SPI) || (DEFAULT_TRAFFIC_MODE == TRAFFIC_MODE_J1939)
	RAMN_SPI_UpdateLED(&LEDState);
#endif
#endif

#if defined(RAMN_SHOWCASE_MODE) && defined(TARGET_ECUA)
	g_trafficProfile->codec[SIG_COMMAND_BRAKE].encode((uint16_t)RAMN_DBC_Handle.command_brake, &txRuntime[TXIDX_COMMAND_BRAKE].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_ACCEL].encode((uint16_t)RAMN_DBC_Handle.command_accel, &txRuntime[TXIDX_COMMAND_ACCEL].data->rawData[0]);
	g_trafficProfile->codec[SIG_STATUS_RPM].encode((uint16_t)RAMN_DBC_Handle.status_rpm, &txRuntime[TXIDX_STATUS_RPM].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_STEERING].encode((uint16_t)RAMN_DBC_Handle.command_steer, &txRuntime[TXIDX_COMMAND_STEERING].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_SHIFT].encode((uint16_t)RAMN_DBC_Handle.command_shift, &txRuntime[TXIDX_COMMAND_SHIFT].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_SIDEBRAKE].encode((uint16_t)RAMN_DBC_Handle.command_sidebrake, &txRuntime[TXIDX_COMMAND_PARKINGBRAKE].data->rawData[0]);
#endif
}
