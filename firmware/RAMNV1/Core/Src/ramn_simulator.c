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
	RAMN_SIM_AutopilotEnabled = False;
}

void RAMN_SIM_UpdatePeriodic(uint32_t tick)
{

#if defined(TARGET_ECUA)
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

#if defined(EXPANSION_POWERTRAIN)
	// Powertrain ECU sends back data from the Self-Driving agent, except if sensors are above a certain threshold
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
		RAMN_DBC_Handle.control_shift 			= RAMN_DBC_Handle.command_shift;
	}

	RAMN_DBC_Handle.command_horn 			= RAMN_SENSORS_POWERTRAIN.hornRequest;
	RAMN_DBC_Handle.command_turnindicator 	= RAMN_SENSORS_POWERTRAIN.turnIndicatorRequest;
#endif

#if defined(EXPANSION_CHASSIS)
	// Chassis ECU sends back data from the Self-Driving agent, except if steering wheel is not centered
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
			RAMN_DBC_Handle.command_lights = 0x0000; //Engine Warning LED OFF
		}
	}
	else
	{
		RAMN_DBC_Handle.control_steer = RAMN_SENSORS_CHASSIS.steeringPotentiometer;
		RAMN_DBC_Handle.command_lights = 0x0000; //Engine Warning LED OFF
	}
	if ((!RAMN_SIM_AutopilotEnabled) || (RAMN_SENSORS_CHASSIS.sidebrakeSwitch != 0U))
	{
		RAMN_DBC_Handle.control_sidebrake  = RAMN_SENSORS_CHASSIS.sidebrakeSwitch;
	}
	else
	{
		RAMN_DBC_Handle.control_sidebrake = RAMN_DBC_Handle.command_sidebrake;
	}
	RAMN_DBC_Handle.command_lights  |= ((RAMN_SENSORS_CHASSIS.lightsSwitch)&0xFF); //We use command here and not control because we consider we command the simulator's lights
#endif

#if defined(EXPANSION_BODY)
	// Body Expansion simply lights up LED based on current state
	RAMN_DBC_Handle.control_enginekey  = RAMN_SENSORS_BODY.engineKey;
	RAMN_ACTUATORS_SetLampState(LED_BATTERY		, (RAMN_SENSORS_BODY.engineKey == RAMN_ENGINEKEY_MIDDLE) || (RAMN_SENSORS_BODY.engineKey == RAMN_ENGINEKEY_RIGHT));
	RAMN_ACTUATORS_SetLampState(LED_CHECKENGINE	, ((RAMN_DBC_Handle.command_lights&0xFF00) != 0U));
	RAMN_ACTUATORS_SetLampState(LED_SIDEBRAKE	, (RAMN_DBC_Handle.control_brake >= 0x010) || (RAMN_DBC_Handle.control_sidebrake != RAMN_SIDEBRAKE_DOWN));
	RAMN_ACTUATORS_SetLampState(LED_TAILLAMP	, ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS2) || ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS3) || ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS4) );
	RAMN_ACTUATORS_SetLampState(LED_LOWBEAM		, ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS3) || ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS4) );
	RAMN_ACTUATORS_SetLampState(LED_HIGHBEAM	, ((RAMN_DBC_Handle.command_lights&0x00FF) == RAMN_LIGHTSWITCH_POS4) );
	RAMN_ACTUATORS_SetLampState(LED_LEFTTURN	, ((RAMN_DBC_Handle.command_turnindicator&0xFF00) != 0U) & ((tick % 1000) >= 500));
	RAMN_ACTUATORS_SetLampState(LED_RIGHTTURN	, ((RAMN_DBC_Handle.command_turnindicator&0x00FF) != 0U) & ((tick % 1000) >= 500));
#endif
}


