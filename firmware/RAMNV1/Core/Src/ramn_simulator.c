/*
 * ramn_simulator.c
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

#include "ramn_simulator.h"

uint8_t RAMN_SIM_AutopilotEnabled;

void RAMN_SIM_Init(void)
{
#if defined(RAMN_SHOWCASE_MODE)
	RAMN_SIM_AutopilotEnabled = True;
#else
	RAMN_SIM_AutopilotEnabled = False;
#endif
}

void RAMN_SIM_UpdatePeriodic(uint32_t tick)
{

#if defined(TARGET_ECUA) && defined(ENABLE_USB)
	if (RAMN_USB_Config.simulatorActive == True)
	{
		uint8_t index = 0U;
		uint8_t statusBuffer[30U];
		statusBuffer[index++] = 'u';
		index += uint12toASCII((uint16_t)RAMN_DBC_Handle.control_brake     ,&statusBuffer[index]);
		index += uint12toASCII((uint16_t)RAMN_DBC_Handle.control_accel     ,&statusBuffer[index]);
		index += uint12toASCII((uint16_t)RAMN_DBC_Handle.control_steer     ,&statusBuffer[index]);
		index += uint8toASCII ((uint8_t)RAMN_DBC_Handle.control_shift      ,&statusBuffer[index]);
		index += uint8toASCII ((uint8_t)RAMN_DBC_Handle.control_lights     ,&statusBuffer[index]);
		index += uint4toASCII ((uint8_t)RAMN_DBC_Handle.control_sidebrake  ,&statusBuffer[index]);
		index += uint4toASCII ((uint8_t)RAMN_DBC_Handle.command_horn       ,&statusBuffer[index]);
		index += uint4toASCII ((uint8_t)RAMN_DBC_Handle.control_enginekey  ,&statusBuffer[index]);
		statusBuffer[index++] = '\r';
		RAMN_USB_SendFromTask(statusBuffer,index);
	}
#endif

#if defined(RAMN_SHOWCASE_MODE) && defined(TARGET_ECUA)
	// Showcase mode: ECU A periodically randomizes command values at 1 Hz.
	// Other ECUs use their default behavior, reacting to commands received via CAN.
	{
		static uint32_t lastRandomTick = 0U;
		static uint8_t firstRun = 1U;

		if (firstRun != 0U)
		{
			lastRandomTick = tick;
			firstRun = 0U;
		}

		if ((tick - lastRandomTick) >= 1000U)
		{
			lastRandomTick = tick;

			RAMN_DBC_Handle.command_brake         = RAMN_RNG_Pop16() & 0xFFF;
			RAMN_DBC_Handle.command_accel          = RAMN_RNG_Pop16() & 0xFFF;
			RAMN_DBC_Handle.command_steer          = RAMN_RNG_Pop16() & 0xFFF;
			RAMN_DBC_Handle.command_shift          = (uint16_t)(RAMN_RNG_Pop8() % (MAX_GEAR_VALUE + 1U));
			RAMN_DBC_Handle.command_sidebrake      = (uint16_t)(RAMN_RNG_Pop8() & 0x01);
			RAMN_DBC_Handle.status_rpm             = RAMN_RNG_Pop16() & 0xFFF;
			RAMN_DBC_Handle.control_horn           = RAMN_RNG_Pop8() & 0x01;
		}
	}
#endif

#if defined(EXPANSION_POWERTRAIN)
	// Powertrain ECU sends back data from the Self-Driving agent, except if sensors are above a certain threshold
#if defined(RAMN_SHOWCASE_MODE)
	RAMN_DBC_Handle.control_brake = RAMN_DBC_Handle.command_brake;
	RAMN_DBC_Handle.control_accel = RAMN_DBC_Handle.command_accel;
	RAMN_DBC_Handle.control_shift = (RAMN_DBC_Handle.command_shift)&0xFF;

	// In Showcase mode, ECUC randomizes the Horn and Turn Indicator commands
	{
		static uint32_t lastRandomTick = 0U;
		if ((tick - lastRandomTick) >= 1000U)
		{
			lastRandomTick = tick;
			RAMN_DBC_Handle.command_horn = RAMN_RNG_Pop8() & 0x01;
			RAMN_DBC_Handle.command_turnindicator = RAMN_RNG_Pop16();
		}
	}
#else
	if ((!RAMN_SIM_AutopilotEnabled) || ((RAMN_SENSORS_POWERTRAIN.brakePotentiometer >= 0x20) || (RAMN_SENSORS_POWERTRAIN.accelPotentiometer >= 0x20)))
	{
		RAMN_DBC_Handle.control_brake = RAMN_SENSORS_POWERTRAIN.brakePotentiometer;
		RAMN_DBC_Handle.control_accel = RAMN_SENSORS_POWERTRAIN.accelPotentiometer;
	}
	else
	{
		RAMN_DBC_Handle.control_brake = RAMN_DBC_Handle.command_brake;
		RAMN_DBC_Handle.control_accel = RAMN_DBC_Handle.command_accel;
	}

	if ((!RAMN_SIM_AutopilotEnabled) || (RAMN_SENSORS_POWERTRAIN.gear != 0U)) // If no gear is specified by user, return the one asked by simulator
	{
		RAMN_DBC_Handle.control_shift 			= RAMN_SENSORS_POWERTRAIN.gear;
	}
	else
	{
		RAMN_DBC_Handle.control_shift 			= (RAMN_DBC_Handle.command_shift)&0xFF;
	}
	RAMN_DBC_Handle.command_horn = RAMN_SENSORS_POWERTRAIN.hornRequest;
#endif
	// Note that we do not allow overriding the joystick, only gear status.
	// It can be implemented by looking at the second byte of command_shift (declared as uint16_t).
	RAMN_DBC_Handle.joystick 				= (uint8_t) RAMN_SENSORS_POWERTRAIN.shiftJoystick;
	RAMN_DBC_Handle.command_turnindicator 	= RAMN_SENSORS_POWERTRAIN.turnIndicatorRequest;
#endif

#if defined(EXPANSION_CHASSIS)
	// Chassis ECU sends back data from the Self-Driving agent, except if steering wheel is not centered
#if defined(RAMN_SHOWCASE_MODE)
	RAMN_DBC_Handle.control_steer  = RAMN_DBC_Handle.command_steer;
	RAMN_DBC_Handle.control_sidebrake = RAMN_DBC_Handle.command_sidebrake;

	// In Showcase mode, ECUB randomizes the Lights command
	{
		static uint32_t lastRandomTick = 0U;
		if ((tick - lastRandomTick) >= 1000U)
		{
			lastRandomTick = tick;
			// Randomize LCMD Byte 1 fields (Bits 1-2, 3-4, 5-6, 7-8) between 0 (de-activate) and 1 (activate)
			// Using enum values 1 (POS1) to 4 (POS4)
			RAMN_DBC_Handle.command_lights = (RAMN_RNG_Pop8() % 4) + 1;
		}
	}
#else
	if (RAMN_SIM_AutopilotEnabled)
	{
		if((RAMN_SENSORS_CHASSIS.steeringPotentiometer <= 0x7E0) || (RAMN_SENSORS_CHASSIS.steeringPotentiometer >= 0x820))
		{
			RAMN_DBC_Handle.control_steer = RAMN_SENSORS_CHASSIS.steeringPotentiometer;
			RAMN_DBC_Handle.command_lights = 0xFF00; //Engine Warning LED ON
		}
		else
		{
			RAMN_DBC_Handle.control_steer  = RAMN_DBC_Handle.command_steer;
			RAMN_DBC_Handle.control_sidebrake = RAMN_DBC_Handle.command_sidebrake;
		}
	}
	else
	{
		RAMN_DBC_Handle.control_steer = RAMN_SENSORS_CHASSIS.steeringPotentiometer;
	}
	if ((!RAMN_SIM_AutopilotEnabled) || (RAMN_SENSORS_CHASSIS.sidebrakeSwitch != 0U))
	{
		RAMN_DBC_Handle.control_sidebrake  = RAMN_SENSORS_CHASSIS.sidebrakeSwitch;
	}
	else
	{
		RAMN_DBC_Handle.control_sidebrake = RAMN_DBC_Handle.command_sidebrake;
	}

	if (RAMN_SENSORS_CHASSIS.lightsSwitch != RAMN_LIGHTSWITCH_POS1) RAMN_DBC_Handle.command_lights = (uint16_t)RAMN_SENSORS_CHASSIS.lightsSwitch;
#endif
#endif

#if defined(EXPANSION_BODY)
	// Body Expansion simply lights up LED based on current state
#if defined(RAMN_SHOWCASE_MODE)
	// In Showcase mode, ECUD randomizes the Engine Key command (Accessory and Ignition bits)
	{
		static uint32_t lastRandomTick = 0U;
		if ((tick - lastRandomTick) >= 1000U)
		{
			lastRandomTick = tick;
			// Randomize CM3 Byte 3 fields: Accessory Power (Bits 3-4) and Ignition Power (Bits 5-6)
			// Using enum values 1 (OFF), 2 (ACC), 3 (IGN)
			RAMN_DBC_Handle.control_enginekey = (RAMN_RNG_Pop8() % 3) + 1;
		}
	}
	RAMN_ACTUATORS_SetLampState(LED_BATTERY		, (RAMN_DBC_Handle.control_enginekey == RAMN_ENGINEKEY_RIGHT)); // Ignition Power state
#else
	RAMN_DBC_Handle.control_enginekey  = RAMN_SENSORS_BODY.engineKey;
	RAMN_ACTUATORS_SetLampState(LED_BATTERY		, (RAMN_SENSORS_BODY.engineKey == RAMN_ENGINEKEY_MIDDLE) || (RAMN_SENSORS_BODY.engineKey == RAMN_ENGINEKEY_RIGHT));
#endif

	// Turn on LED if requested by controls
	RAMN_ACTUATORS_SetLampState(LED_CHECKENGINE	, ((RAMN_DBC_Handle.command_lights&0xFF00) != 0U));
	// Turn on "Check Engine LED" if a CAN error was detected
	if (RAMN_FDCAN_Status.CANErrCnt > 0) RAMN_ACTUATORS_SetLampState(LED_CHECKENGINE , 1U);
	// If bus off, blink
	if (RAMN_FDCAN_Status.busOff == True) RAMN_ACTUATORS_SetLampState(LED_CHECKENGINE ,(tick % 1000) >= 500);

	RAMN_ACTUATORS_SetLampState(LED_SIDEBRAKE	, (RAMN_DBC_Handle.control_brake >= 0x010) || (RAMN_DBC_Handle.control_sidebrake != RAMN_SIDEBRAKE_DOWN));

	RAMN_ACTUATORS_SetLampState(LED_TAILLAMP	, ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS2) || ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS3) || ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS4) );
	RAMN_ACTUATORS_SetLampState(LED_LOWBEAM		, ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS3) || ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS4) );
	RAMN_ACTUATORS_SetLampState(LED_HIGHBEAM	, ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS4) );
	RAMN_ACTUATORS_SetLampState(LED_LEFTTURN	, ((RAMN_DBC_Handle.command_turnindicator&0xFF00) != 0U) & ((tick % 1000) >= 500));
	RAMN_ACTUATORS_SetLampState(LED_RIGHTTURN	, ((RAMN_DBC_Handle.command_turnindicator&0x00FF) != 0U) & ((tick % 1000) >= 500));
#endif
}
