/*
 * dbc.c
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

#include "ramn_dbc.h"
#include "ramn_signal_defs.h"
#include "ramn_can_database.h"
#include "ramn_sensors.h"
#ifdef ENABLE_J1939_MODE
#include "ramn_j1939.h"
#endif

#define NUMBER_OF_PERIODIC_MSG (sizeof(periodicTxCANMsgs)/sizeof(RAMN_PeriodicFDCANTx_t*))

volatile RAMN_Bool_t RAMN_DBC_RequestSilence = True;

RAMN_DBC_Handle_t RAMN_DBC_Handle = {.command_steer = 0x7FF, .control_shift =0x01, .command_shift = 0x01, .command_lights = RAMN_LIGHTSWITCH_POS1};

// array that holds messages to be sent periodically
static RAMN_PeriodicFDCANTx_t* periodicTxCANMsgs[] = {
#if defined(TARGET_ECUA)
		&msg_command_brake,&msg_command_accel,&msg_status_RPM,&msg_command_steering,&msg_command_shift,&msg_control_horn,&msg_command_parkingbrake
#endif
#if defined(TARGET_ECUB)
		&msg_control_steering, &msg_control_sidebrake, &msg_command_lights
#endif
#if defined(TARGET_ECUC)
		&msg_control_brake, &msg_control_accel, &msg_control_shift, &msg_command_horn, &msg_command_turnindicator
#ifdef ENABLE_J1939_MODE
		, &msg_joystick_buttons
#endif
#endif
#if defined(TARGET_ECUD)
		&msg_control_enginekey, &msg_control_lights
#endif
};

// Function that formats messages with counter/checksum/random/etc.
static void RAMN_DBC_FormatDefaultPeriodicMessage(RAMN_PeriodicFDCANTx_t* msg)
{
	if (msg->counterOffset >= 0)
	{
		uint16_t counter = APPLY_ENDIAN_16(msg->counter);
		RAMN_memcpy(&(msg->data->rawData[msg->counterOffset / 8]), (uint8_t*)&counter, sizeof(counter));
	}

	if (msg->crcOffset >= 0)
	{
		uint32_t crc32 = RAMN_CRC_SoftCalculate(msg->data->rawData, msg->crcOffset / 8);
		RAMN_memcpy(&(msg->data->rawData[msg->crcOffset / 8]), (uint8_t*)&crc32, sizeof(crc32));
	}

	msg->header.ErrorStateIndicator = RAMN_FDCAN_Status.ErrorStateIndicator;
}

void RAMN_DBC_Init(void)
{
#if defined(TARGET_ECUA) && !defined(RAMN_SHOWCASE_MODE)
	RAMN_DBC_RequestSilence = True;
#else
	RAMN_DBC_RequestSilence = False;
#endif

}

void RAMN_DBC_ProcessCANMessage(uint32_t canid, uint32_t dlc, RAMN_CANFrameData_t* dataframe)
{
	if (dlc != 0U)
	{
#ifdef ENABLE_J1939_MODE
		uint32_t pgn = (canid >> 8) & 0x3FFFF;
		uint8_t pf = (uint8_t)(pgn >> 8);
		if (pf < 240) pgn &= 0x3FF00; // PDU1: PS is DA, not part of PGN

		switch(pgn)
		{
		case J1939_PGN_EBC1:
			RAMN_DBC_Handle.control_brake = RAMN_Decode_Control_Brake(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_XBR:
			RAMN_DBC_Handle.command_brake = RAMN_Decode_Command_Brake(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_EEC2:
			RAMN_DBC_Handle.control_accel = RAMN_Decode_Control_Accel(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_TSC1:
			RAMN_DBC_Handle.command_accel = RAMN_Decode_Command_Accel(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_EEC1:
			RAMN_DBC_Handle.status_rpm = RAMN_Decode_Status_RPM(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_VDC2:
			RAMN_DBC_Handle.control_steer = RAMN_Decode_Control_Steering(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_PROPA:
			// PROPA is used for multiple commands based on DA.
			// Since we cleared PS if PF < 240, we need to check the original DA if needed,
			// or we can rely on the fact that different ECUs receive different things.
			// In RAMN, Command_Steering and Control_Horn are both PROPA.
			// We can try to decode both or check the DA from the original canid.
			{
				uint8_t da = (uint8_t)(canid >> 8);
				if (da == J1939_DA_STEERING_CTRL) {
					RAMN_DBC_Handle.command_steer = RAMN_Decode_Command_Steering(&dataframe->rawData[0], dlc);
				} else if (da == J1939_DA_POWERTRAIN_CTRL) {
					RAMN_DBC_Handle.control_horn = RAMN_Decode_Control_Horn(&dataframe->rawData[0], dlc);
				}
			}
			break;
		case J1939_PGN_ETC2:
			RAMN_DBC_Handle.control_shift = RAMN_Decode_Control_Shift(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_PROPB_65282:
			RAMN_DBC_Handle.joystick = RAMN_Decode_JoystickButtons(&dataframe->rawData[0], dlc);
			#ifdef ENABLE_JOYSTICK_CONTROLS
				RAMN_Joystick_Update(RAMN_DBC_Handle.joystick);
			#endif
			break;
		case J1939_PGN_TC1:
			RAMN_DBC_Handle.command_shift = RAMN_Decode_Command_Shift(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_CM3:
			{
				// CM3 is sent by both ECU C (Horn) and ECU D (EngineKey)
				uint8_t sa = (uint8_t)(canid & 0xFF);
				if (sa == J1939_SA_BODY_CTRL) {
					uint8_t enginekey = RAMN_Decode_Control_EngineKey(&dataframe->rawData[0], dlc);
#ifdef RAMN_SHOWCASE_MODE
					RAMN_DBC_Handle.control_enginekey = enginekey;
#else
					if (enginekey != 3) RAMN_DBC_Handle.control_enginekey = enginekey;
#endif
				} else if (sa == J1939_SA_POWERTRAIN_CTRL) {
					uint8_t horn = RAMN_Decode_Command_Horn(&dataframe->rawData[0], dlc);
#ifdef RAMN_SHOWCASE_MODE
					RAMN_DBC_Handle.command_horn = horn;
#else
					if (horn != 3) RAMN_DBC_Handle.command_horn = horn;
#endif
				}
			}
			break;
		case J1939_PGN_B1:
			RAMN_DBC_Handle.control_sidebrake = RAMN_Decode_Control_Sidebrake(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_CCVS1:
			RAMN_DBC_Handle.command_sidebrake = RAMN_Decode_Command_Sidebrake(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_OEL:
			RAMN_DBC_Handle.command_turnindicator = RAMN_Decode_Command_TurnIndicator(&dataframe->rawData[0], dlc);
			break;
		case J1939_PGN_LIGHTS_CMD:
			{
				uint16_t lights = RAMN_Decode_Command_Lights(&dataframe->rawData[0], dlc);
				// Decode_Command_Lights returns POS1 if byte is 0x00, but logic in database.c
				// can result in 0xFF if the input was 0.
				// For J1939, we only update if it's not the 'Not Available' state.
				if ((dataframe->rawData[0] & 0x03) != 3) RAMN_DBC_Handle.command_lights = lights;
			}
			break;
		case J1939_PGN_PROPB_65280:
			RAMN_DBC_Handle.control_lights = RAMN_Decode_Control_Lights(&dataframe->rawData[0], dlc);
			break;
		default:
			break;
		}
#else
		switch(canid)
		{
		case CAN_SIM_CONTROL_BRAKE_CANID:
			RAMN_DBC_Handle.control_brake = RAMN_Decode_Control_Brake(&dataframe->rawData[CAN_SIM_CONTROL_BRAKE_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_COMMAND_BRAKE_CANID:
			RAMN_DBC_Handle.command_brake = RAMN_Decode_Command_Brake(&dataframe->rawData[CAN_SIM_COMMAND_BRAKE_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_CONTROL_ACCEL_CANID:
			RAMN_DBC_Handle.control_accel = RAMN_Decode_Control_Accel(&dataframe->rawData[CAN_SIM_CONTROL_ACCEL_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_COMMAND_ACCEL_CANID:
			RAMN_DBC_Handle.command_accel = RAMN_Decode_Command_Accel(&dataframe->rawData[CAN_SIM_COMMAND_ACCEL_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_STATUS_RPM_CANID:
			RAMN_DBC_Handle.status_rpm = RAMN_Decode_Status_RPM(&dataframe->rawData[CAN_SIM_STATUS_RPM_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_CONTROL_STEERING_CANID:
			RAMN_DBC_Handle.control_steer = RAMN_Decode_Control_Steering(&dataframe->rawData[CAN_SIM_CONTROL_STEERING_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_COMMAND_STEERING_CANID:
			RAMN_DBC_Handle.command_steer = RAMN_Decode_Command_Steering(&dataframe->rawData[CAN_SIM_COMMAND_STEERING_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_CONTROL_SHIFT_CANID:
			RAMN_DBC_Handle.control_shift = RAMN_Decode_Control_Shift(&dataframe->rawData[CAN_SIM_CONTROL_SHIFT_PAYLOAD_OFFSET / 8], dlc);
			if (dlc >= 2U)
			{
				RAMN_DBC_Handle.joystick = RAMN_Decode_Joystick(&dataframe->rawData[CAN_SIM_CONTROL_SHIFT_PAYLOAD_OFFSET / 8], dlc);
				#ifdef ENABLE_JOYSTICK_CONTROLS
					RAMN_Joystick_Update(RAMN_DBC_Handle.joystick);
				#endif
			}
			break;
#ifdef CAN_SIM_JOYSTICK_BUTTONS_CANID
		case CAN_SIM_JOYSTICK_BUTTONS_CANID:
			RAMN_DBC_Handle.joystick = RAMN_Decode_JoystickButtons(&dataframe->rawData[CAN_SIM_JOYSTICK_BUTTONS_PAYLOAD_OFFSET / 8], dlc);
			#ifdef ENABLE_JOYSTICK_CONTROLS
				RAMN_Joystick_Update(RAMN_DBC_Handle.joystick);
			#endif
			break;
#endif
		case CAN_SIM_COMMAND_SHIFT_CANID:
			RAMN_DBC_Handle.command_shift = RAMN_Decode_Command_Shift(&dataframe->rawData[CAN_SIM_COMMAND_SHIFT_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_COMMAND_HORN_CANID:
			RAMN_DBC_Handle.command_horn = RAMN_Decode_Command_Horn(&dataframe->rawData[CAN_SIM_COMMAND_HORN_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_CONTROL_HORN_CANID:
			RAMN_DBC_Handle.control_horn = RAMN_Decode_Control_Horn(&dataframe->rawData[CAN_SIM_CONTROL_HORN_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_CONTROL_SIDEBRAKE_CANID:
			RAMN_DBC_Handle.control_sidebrake = RAMN_Decode_Control_Sidebrake(&dataframe->rawData[CAN_SIM_CONTROL_SIDEBRAKE_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_COMMAND_SIDEBRAKE_CANID:
			RAMN_DBC_Handle.command_sidebrake = RAMN_Decode_Command_Sidebrake(&dataframe->rawData[CAN_SIM_COMMAND_SIDEBRAKE_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_COMMAND_TURNINDICATOR_CANID:
			RAMN_DBC_Handle.command_turnindicator = RAMN_Decode_Command_TurnIndicator(&dataframe->rawData[CAN_SIM_COMMAND_TURNINDICATOR_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_CONTROL_ENGINEKEY_CANID:
			RAMN_DBC_Handle.control_enginekey = RAMN_Decode_Control_EngineKey(&dataframe->rawData[CAN_SIM_CONTROL_ENGINEKEY_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_COMMAND_LIGHTS_CANID:
			RAMN_DBC_Handle.command_lights = RAMN_Decode_Command_Lights(&dataframe->rawData[CAN_SIM_COMMAND_LIGHTS_PAYLOAD_OFFSET / 8], dlc);
			break;
		case CAN_SIM_CONTROL_LIGHTS_CANID:
			RAMN_DBC_Handle.control_lights = RAMN_Decode_Control_Lights(&dataframe->rawData[CAN_SIM_CONTROL_LIGHTS_PAYLOAD_OFFSET / 8], dlc);
			break;
		default:
			break;
		}
#endif
	}
}

void RAMN_DBC_Send(uint32_t tick)
{
	if (RAMN_DBC_RequestSilence == False)
	{
		for(uint16_t i = 0; i < NUMBER_OF_PERIODIC_MSG ; i++)
		{
			if((tick - periodicTxCANMsgs[i]->lastSent) >= periodicTxCANMsgs[i]->periodms)
			{
				RAMN_DBC_FormatDefaultPeriodicMessage(periodicTxCANMsgs[i]);
				RAMN_FDCAN_SendMessage(&(periodicTxCANMsgs[i]->header),(uint8_t*)(periodicTxCANMsgs[i]->data));
				periodicTxCANMsgs[i]->counter++;
				periodicTxCANMsgs[i]->lastSent = tick;
			}
		}
	}
}

#if defined(ENABLE_USB)
void RAMN_DBC_ProcessUSBBuffer(const uint8_t* buf)
{
#if defined(TARGET_ECUA)
#ifdef ENABLE_J1939_MODE
	RAMN_Encode_Command_Brake(ASCIItoUint12(&buf[1]), &msg_command_brake.data->rawData[0]);
	RAMN_Encode_Command_Accel(ASCIItoUint12(&buf[4]), &msg_command_accel.data->rawData[0]);
	RAMN_Encode_Status_RPM(ASCIItoUint12(&buf[7]), &msg_status_RPM.data->rawData[0]);
	RAMN_Encode_Command_Steering(ASCIItoUint12(&buf[10]), &msg_command_steering.data->rawData[0]);
	RAMN_Encode_Command_Shift(ASCIItoUint8(&buf[13]), &msg_command_shift.data->rawData[0]);
	RAMN_Encode_Control_Horn(ASCIItoUint8(&buf[15]), &msg_control_horn.data->rawData[0]);
	RAMN_Encode_Command_Sidebrake(ASCIItoUint8(&buf[17]), &msg_command_parkingbrake.data->rawData[0]);
#else
	RAMN_Encode_Command_Brake(ASCIItoUint12(&buf[1]), &msg_command_brake.data->rawData[CAN_SIM_COMMAND_BRAKE_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Command_Accel(ASCIItoUint12(&buf[4]), &msg_command_accel.data->rawData[CAN_SIM_COMMAND_ACCEL_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Status_RPM(ASCIItoUint12(&buf[7]), &msg_status_RPM.data->rawData[CAN_SIM_STATUS_RPM_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Command_Steering(ASCIItoUint12(&buf[10]), &msg_command_steering.data->rawData[CAN_SIM_COMMAND_STEERING_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Command_Shift(ASCIItoUint8(&buf[13]), &msg_command_shift.data->rawData[CAN_SIM_COMMAND_SHIFT_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Control_Horn(ASCIItoUint8(&buf[15]), &msg_control_horn.data->rawData[CAN_SIM_CONTROL_HORN_PAYLOAD_OFFSET / 8]);
	RAMN_Encode_Command_Sidebrake(ASCIItoUint8(&buf[17]), &msg_command_parkingbrake.data->rawData[CAN_SIM_COMMAND_SIDEBRAKE_PAYLOAD_OFFSET / 8]);
#endif
#endif
}
#endif



