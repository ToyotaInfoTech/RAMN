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

#define NUMBER_OF_PERIODIC_MSG (sizeof(periodicTxCANMsgs)/sizeof(RAMN_PeriodicFDCANTx_t*))

volatile RAMN_Bool_t RAMN_DBC_RequestSilence = True;

RAMN_DBC_Handle_t RAMN_DBC_Handle = {.command_steer = 0x7FF, .control_shift =0x01, .command_shift = 0x01};

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
#endif
#if defined(TARGET_ECUD)
		&msg_control_enginekey, &msg_control_lights
#endif
};

// Function that formats messages with counter/checksum/random/etc.
static void RAMN_DBC_FormatDefaultPeriodicMessage(RAMN_PeriodicFDCANTx_t* msg)
{
	msg->data->ramnData.counter = applyEndian16(msg->counter);
	msg->data->ramnData.crc32 = RAMN_CRC_SoftCalculate(msg->data->rawData,4U);
	msg->header.ErrorStateIndicator = RAMN_FDCAN_Status.ErrorStateIndicator;
}

void RAMN_DBC_Init(void)
{
#if defined(TARGET_ECUA)
	RAMN_DBC_RequestSilence = True;
#else
	RAMN_DBC_RequestSilence = False;
#endif

}

void RAMN_DBC_ProcessCANMessage(uint32_t canid, uint32_t dlc, RAMN_CANFrameData_t* dataframe)
{
	// Ignore fields other than useful data
	dataframe->ramnData.payload = dataframe->ramnData.payload&0xFFFF;
	if (dlc <= 1U) dataframe->ramnData.payload = dataframe->ramnData.payload&0xFF; //TODO: reject dlc == 2U message instead of casting them?

	// To avoid overloading the ECU with processing of incoming messages, only expected messages are included in the switch/case
	if (dlc != 0)
	{
		switch(canid)
		{
#ifdef RECEIVE_CONTROL_BRAKE
		case CAN_SIM_CONTROL_BRAKE_CANID:
			RAMN_DBC_Handle.control_brake 				= applyEndian16(dataframe->ramnData.payload);
			break;
#endif
#ifdef RECEIVE_COMMAND_BRAKE
		case CAN_SIM_COMMAND_BRAKE_CANID:
			RAMN_DBC_Handle.command_brake 				= applyEndian16(dataframe->ramnData.payload);
			break;
#endif
#ifdef RECEIVE_CONTROL_ACCEL
		case CAN_SIM_CONTROL_ACCEL_CANID:
			RAMN_DBC_Handle.control_accel 				= applyEndian16(dataframe->ramnData.payload);
			break;
#endif
#ifdef RECEIVE_COMMAND_ACCEL
		case CAN_SIM_COMMAND_ACCEL_CANID:
			RAMN_DBC_Handle.command_accel 				= applyEndian16(dataframe->ramnData.payload);
			break;
#endif
#ifdef RECEIVE_STATUS_RPM
		case CAN_SIM_STATUS_RPM_CANID:
			RAMN_DBC_Handle.status_rpm  				= applyEndian16(dataframe->ramnData.payload);
			break;
#endif
#ifdef RECEIVE_CONTROL_STEERING
		case CAN_SIM_CONTROL_STEERING_CANID:
			RAMN_DBC_Handle.control_steer 				= applyEndian16(dataframe->ramnData.payload);
			break;
#endif
#ifdef RECEIVE_COMMAND_STEERING
		case CAN_SIM_COMMAND_STEERING_CANID:
			RAMN_DBC_Handle.command_steer 				= applyEndian16(dataframe->ramnData.payload);
			break;
#endif
#ifdef RECEIVE_CONTROL_SHIFT
		case CAN_SIM_CONTROL_SHIFT_CANID:
			RAMN_DBC_Handle.control_shift				=  dataframe->ramnData.payload&0xFF;
			if (dlc >= 2U)
			{
				RAMN_DBC_Handle.joystick					= (dataframe->ramnData.payload>>8)&0xFF;
				//Init joystick for screen controls
			#ifdef ENABLE_JOYSTICK_CONTROLS
				RAMN_Joystick_Update(RAMN_DBC_Handle.joystick);
			#endif
			}
			break;
#endif
#ifdef RECEIVE_COMMAND_SHIFT
		case CAN_SIM_COMMAND_SHIFT_CANID:
			RAMN_DBC_Handle.command_shift 				= dataframe->ramnData.payload;
			break;
#endif
#ifdef RECEIVE_COMMAND_HORN
		case CAN_SIM_COMMAND_HORN_CANID:
			RAMN_DBC_Handle.command_horn 				= (dataframe->ramnData.payload)&0xFF;
			break;
#endif
#ifdef RECEIVE_CONTROL_HORN
		case CAN_SIM_CONTROL_HORN_CANID:
			RAMN_DBC_Handle.control_horn 				= dataframe->ramnData.payload&0xFF;
			break;
#endif
#ifdef RECEIVE_CONTROL_SIDEBRAKE
		case CAN_SIM_CONTROL_SIDEBRAKE_CANID:
			RAMN_DBC_Handle.control_sidebrake 			= dataframe->ramnData.payload&0xFF;
			break;
#endif
#ifdef RECEIVE_COMMAND_SIDEBRAKE
		case CAN_SIM_COMMAND_SIDEBRAKE_CANID:
			RAMN_DBC_Handle.command_sidebrake 			= dataframe->ramnData.payload;
			break;
#endif
#ifdef RECEIVE_COMMAND_TURNINDICATOR
		case CAN_SIM_COMMAND_TURNINDICATOR_CANID:
			RAMN_DBC_Handle.command_turnindicator		= dataframe->ramnData.payload;
			break;
#endif
#ifdef RECEIVE_CONTROL_ENGINEKEY
		case CAN_SIM_CONTROL_ENGINEKEY_CANID:
			RAMN_DBC_Handle.control_enginekey 			= dataframe->ramnData.payload&0xFF;
			break;
#endif
#ifdef RECEIVE_COMMAND_LIGHTS
		case CAN_SIM_COMMAND_LIGHTS_CANID:
			RAMN_DBC_Handle.command_lights 				= dataframe->ramnData.payload&0xFFFF;
			break;
#endif
#ifdef RECEIVE_CONTROL_LIGHTS
		case CAN_SIM_CONTROL_LIGHTS_CANID:
			RAMN_DBC_Handle.control_lights 				= dataframe->ramnData.payload&0xFF;
			break;
#endif
		default:
			break;
		}
	}
}

void RAMN_DBC_Send(uint32_t tick)
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

#if defined(ENABLE_USB)
void RAMN_DBC_ProcessUSBBuffer(const uint8_t* buf)
{
#if defined(TARGET_ECUA)
	msg_command_brake.data->ramnData.payload 			= applyEndian16(ASCIItoUint12(&buf[1]));
	msg_command_accel.data->ramnData.payload 			= applyEndian16(ASCIItoUint12(&buf[4]));
	msg_status_RPM.data->ramnData.payload 				= applyEndian16(ASCIItoUint12(&buf[7]));
	msg_command_steering.data->ramnData.payload 		= applyEndian16(ASCIItoUint12(&buf[10]));
	msg_command_shift.data->ramnData.payload 			= ASCIItoUint8(&buf[13]);
	msg_control_horn.data->ramnData.payload 			= ASCIItoUint8(&buf[15]);
	msg_command_parkingbrake.data->ramnData.payload 	= ASCIItoUint8(&buf[17]);
#endif
}
#endif



