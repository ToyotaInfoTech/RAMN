/*
 * dbc.c
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

#include "ramn_dbc.h"

volatile uint8_t RAMN_DBC_RequestSilence = True;

RAMN_DBC_Handle_t RAMN_DBC_Handle = {.command_steer = 0x7FF, .control_shift =0x01, .command_shift = 0x01, .horn_count = 0};

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

//Function that formats messages with counter/checksum/random/etc.
static void RAMN_DBC_FormatDefaultPeriodicMessage(RAMN_PeriodicFDCANTx_t* msg)
{
	uint32_t random = RAMN_RNG_Pop32();
	msg->data->ramn_data.counter = msg->counter;
	msg->data->ramn_data.random = random;
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

uint8_t prev_horn;
void RAMN_DBC_ProcessCANMessage(uint32_t canid, uint32_t dlc, const RAMN_CANFrameData_t* dataframe)
{
	//To avoid overloading the ECU with processing of incoming messages, only expected messages are included in the switch/case
	dataframe->ramn_data.payload&0xFFFF;
	if (dlc <= 1) dataframe->ramn_data.payload&0xFF;
	if (dlc != 0)
	{


		switch(canid)
		{
#ifdef RECEIVE_CONTROL_BRAKE
		case CAN_SIM_CONTROL_BRAKE_CANID:
			RAMN_DBC_Handle.control_brake 				= dataframe->ramn_data.payload&0xFFF0;
			break;
#endif
#ifdef RECEIVE_COMMAND_BRAKE
		case CAN_SIM_COMMAND_BRAKE_CANID:
			RAMN_DBC_Handle.command_brake 				= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_CONTROL_ACCEL
		case CAN_SIM_CONTROL_ACCEL_CANID:
			RAMN_DBC_Handle.control_accel 				= dataframe->ramn_data.payload&0xFFF0;
			break;
#endif
#ifdef RECEIVE_COMMAND_ACCEL
		case CAN_SIM_COMMAND_ACCEL_CANID:
			RAMN_DBC_Handle.command_accel 				= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_STATUS_RPM
		case CAN_SIM_STATUS_RPM_CANID:
			RAMN_DBC_Handle.status_rpm  				= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_CONTROL_STEERING
		case CAN_SIM_CONTROL_STEERING_CANID:
			RAMN_DBC_Handle.control_steer 				= dataframe->ramn_data.payload&0xFFF0;
			break;
#endif
#ifdef RECEIVE_COMMAND_STEERING
		case CAN_SIM_COMMAND_STEERING_CANID:
			RAMN_DBC_Handle.command_steer 				= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_CONTROL_SHIFT
		case CAN_SIM_CONTROL_SHIFT_CANID:
			RAMN_DBC_Handle.control_shift				= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_COMMAND_SHIFT
		case CAN_SIM_COMMAND_SHIFT_CANID:
			RAMN_DBC_Handle.command_shift 				= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_COMMAND_HORN
		case CAN_SIM_COMMAND_HORN_CANID:
			RAMN_DBC_Handle.command_horn 				= (dataframe->ramn_data.payload)&0xFF;
			if ((RAMN_DBC_Handle.command_horn != 0x00) && (prev_horn == 0x00))
			{
				RAMN_DBC_Handle.horn_count++;
			}
			prev_horn = RAMN_DBC_Handle.command_horn;
			break;
#endif
#ifdef RECEIVE_CONTROL_HORN
		case CAN_SIM_CONTROL_HORN_CANID:
			RAMN_DBC_Handle.control_horn 				= dataframe->ramn_data.payload&0xFF;
			break;
#endif
#ifdef RECEIVE_CONTROL_SIDEBRAKE
		case CAN_SIM_CONTROL_SIDEBRAKE_CANID:
			RAMN_DBC_Handle.control_sidebrake 			= dataframe->ramn_data.payload&0xFF;
			break;
#endif
#ifdef RECEIVE_COMMAND_SIDEBRAKE
		case CAN_SIM_COMMAND_SIDEBRAKE_CANID:
			RAMN_DBC_Handle.command_sidebrake 			= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_COMMAND_TURNINDICATOR
		case CAN_SIM_COMMAND_TURNINDICATOR_CANID:
			RAMN_DBC_Handle.command_turnindicator		= dataframe->ramn_data.payload;
			break;
#endif
#ifdef RECEIVE_CONTROL_ENGINEKEY
		case CAN_SIM_CONTROL_ENGINEKEY_CANID:
			RAMN_DBC_Handle.control_enginekey 			= dataframe->ramn_data.payload&0xFF;
			break;
#endif
#ifdef RECEIVE_COMMAND_LIGHTS
		case CAN_SIM_COMMAND_LIGHTS_CANID:
			RAMN_DBC_Handle.command_lights 				= dataframe->ramn_data.payload&0xFF;
			break;
#endif
#ifdef RECEIVE_CONTROL_LIGHTS
		case CAN_SIM_CONTROL_LIGHTS_CANID:
			RAMN_DBC_Handle.control_lights 				= dataframe->ramn_data.payload&0xFF;
			break;
#endif
		default:
			break;
		}
	}
}

#define NUMBER_OF_PERIODIC_MSG (sizeof(periodicTxCANMsgs)/sizeof(RAMN_PeriodicFDCANTx_t*))
void RAMN_DBC_Send(uint32_t tick)
{
	for(uint8_t i = 0; i < NUMBER_OF_PERIODIC_MSG ; i++)
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
	msg_command_brake.data->ramn_data.payload 			= ASCIItoUint12(&buf[1]);
	msg_command_accel.data->ramn_data.payload 			= ASCIItoUint12(&buf[4]);
	msg_status_RPM.data->ramn_data.payload 				= ASCIItoUint12(&buf[7]);
	msg_command_steering.data->ramn_data.payload 		= ASCIItoUint12(&buf[10]);
	msg_command_shift.data->ramn_data.payload 			= ASCIItoUint8(&buf[13]);
	msg_control_horn.data->ramn_data.payload 			= ASCIItoUint8(&buf[15]);
	msg_command_parkingbrake.data->ramn_data.payload 	= ASCIItoUint8(&buf[17]);
#endif
}
#endif



