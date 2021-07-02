/*
 * ramn_vehicle_specific.c
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

#include "ramn_vehicle_specific.h"

#if defined(TARGET_ECUA)
RAMN_PeriodicFDCANTx_t msg_command_brake 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_BRAKE_PERIODMS, 			.header = { .Identifier = CAN_SIM_COMMAND_BRAKE_CANID			, 	.IdType = CAN_SIM_COMMAND_BRAKE_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_BRAKE_DLC, 			.FDFormat = CAN_SIM_COMMAND_BRAKE_FORMAT, 			.BitRateSwitch = CAN_SIM_COMMAND_BRAKE_BRS } };
RAMN_PeriodicFDCANTx_t msg_command_accel 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_ACCEL_PERIODMS, 			.header = { .Identifier = CAN_SIM_COMMAND_ACCEL_CANID			, 	.IdType = CAN_SIM_COMMAND_ACCEL_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_ACCEL_DLC, 			.FDFormat = CAN_SIM_COMMAND_ACCEL_FORMAT, 			.BitRateSwitch = CAN_SIM_COMMAND_ACCEL_BRS } };
RAMN_PeriodicFDCANTx_t msg_status_RPM 					= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_STATUS_RPM_PERIODMS, 				.header = { .Identifier = CAN_SIM_STATUS_RPM_CANID				, 	.IdType = CAN_SIM_STATUS_RPM_IDTYPE, 			.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_STATUS_RPM_DLC, 			.FDFormat = CAN_SIM_STATUS_RPM_FORMAT, 				.BitRateSwitch = CAN_SIM_STATUS_RPM_BRS } };
RAMN_PeriodicFDCANTx_t msg_command_steering 			= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_STEERING_PERIODMS, 		.header = { .Identifier = CAN_SIM_COMMAND_STEERING_CANID		, 	.IdType = CAN_SIM_COMMAND_STEERING_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_STEERING_DLC, 		.FDFormat = CAN_SIM_COMMAND_STEERING_FORMAT, 		.BitRateSwitch = CAN_SIM_COMMAND_STEERING_BRS } };
RAMN_PeriodicFDCANTx_t msg_command_shift 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_SHIFT_PERIODMS, 			.header = { .Identifier = CAN_SIM_COMMAND_SHIFT_CANID			, 	.IdType = CAN_SIM_COMMAND_SHIFT_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_SHIFT_DLC, 			.FDFormat = CAN_SIM_COMMAND_SHIFT_FORMAT, 			.BitRateSwitch = CAN_SIM_COMMAND_SHIFT_BRS } };
RAMN_PeriodicFDCANTx_t msg_control_horn 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_HORN_PERIODMS, 			.header = { .Identifier = CAN_SIM_CONTROL_HORN_CANID			, 	.IdType = CAN_SIM_CONTROL_HORN_IDTYPE, 			.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_HORN_DLC, 			.FDFormat = CAN_SIM_CONTROL_HORN_FORMAT, 			.BitRateSwitch = CAN_SIM_CONTROL_HORN_BRS } };
RAMN_PeriodicFDCANTx_t msg_command_parkingbrake 		= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_SIDEBRAKE_PERIODMS,		.header = { .Identifier = CAN_SIM_COMMAND_SIDEBRAKE_CANID		, 	.IdType = CAN_SIM_COMMAND_SIDEBRAKE_IDTYPE, 	.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_SIDEBRAKE_DLC, 		.FDFormat = CAN_SIM_COMMAND_SIDEBRAKE_FORMAT, 		.BitRateSwitch = CAN_SIM_COMMAND_SIDEBRAKE_BRS } };
#endif

#if defined(TARGET_ECUB)
RAMN_PeriodicFDCANTx_t msg_control_steering 			= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_STEERING_PERIODMS,  		.header = { .Identifier = CAN_SIM_CONTROL_STEERING_CANID		, 	.IdType = CAN_SIM_CONTROL_STEERING_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_STEERING_DLC, 		.FDFormat = CAN_SIM_CONTROL_STEERING_FORMAT, 		.BitRateSwitch = CAN_SIM_CONTROL_STEERING_BRS } };
RAMN_PeriodicFDCANTx_t msg_control_sidebrake 			= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_SIDEBRAKE_PERIODMS, 		.header = { .Identifier = CAN_SIM_CONTROL_SIDEBRAKE_CANID		, 	.IdType = CAN_SIM_CONTROL_SIDEBRAKE_IDTYPE, 	.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_SIDEBRAKE_DLC, 		.FDFormat = CAN_SIM_CONTROL_SIDEBRAKE_FORMAT, 		.BitRateSwitch = CAN_SIM_CONTROL_SIDEBRAKE_BRS } };
RAMN_PeriodicFDCANTx_t msg_command_lights 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_LIGHTS_PERIODMS, 	 		.header = { .Identifier = CAN_SIM_COMMAND_LIGHTS_CANID			, 	.IdType = CAN_SIM_COMMAND_LIGHTS_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_LIGHTS_DLC, 		.FDFormat = CAN_SIM_COMMAND_LIGHTS_FORMAT, 			.BitRateSwitch = CAN_SIM_COMMAND_LIGHTS_BRS } };
#endif

#if defined(TARGET_ECUC)
RAMN_PeriodicFDCANTx_t msg_control_brake 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_BRAKE_PERIODMS, 			.header = { .Identifier = CAN_SIM_CONTROL_BRAKE_CANID			, 	.IdType = CAN_SIM_CONTROL_BRAKE_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_BRAKE_DLC, 			.FDFormat = CAN_SIM_CONTROL_BRAKE_FORMAT, 			.BitRateSwitch = CAN_SIM_CONTROL_BRAKE_BRS } };
RAMN_PeriodicFDCANTx_t msg_control_accel 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_ACCEL_PERIODMS, 			.header = { .Identifier = CAN_SIM_CONTROL_ACCEL_CANID			, 	.IdType = CAN_SIM_CONTROL_BRAKE_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_BRAKE_DLC, 			.FDFormat = CAN_SIM_CONTROL_BRAKE_FORMAT, 			.BitRateSwitch = CAN_SIM_CONTROL_BRAKE_BRS } };
RAMN_PeriodicFDCANTx_t msg_control_shift 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_SHIFT_PERIODMS, 			.header = { .Identifier = CAN_SIM_CONTROL_SHIFT_CANID			, 	.IdType = CAN_SIM_CONTROL_SHIFT_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_SHIFT_DLC, 		    .FDFormat = CAN_SIM_CONTROL_SHIFT_FORMAT, 			.BitRateSwitch = CAN_SIM_CONTROL_SHIFT_BRS } };
RAMN_PeriodicFDCANTx_t msg_command_horn 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_HORN_CANID_PERIODMS, 		.header = { .Identifier = CAN_SIM_COMMAND_HORN_CANID			, 	.IdType = CAN_SIM_COMMAND_HORN_CANID_IDTYPE, 	.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_HORN_CANID_DLC, 	.FDFormat = CAN_SIM_COMMAND_HORN_CANID_FORMAT, 		.BitRateSwitch = CAN_SIM_COMMAND_HORN_CANID_BRS } };
RAMN_PeriodicFDCANTx_t msg_command_turnindicator 		= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_COMMAND_TURNINDICATOR_PERIODMS, 	.header = { .Identifier = CAN_SIM_COMMAND_TURNINDICATOR_CANID	, 	.IdType = CAN_SIM_COMMAND_TURNINDICATOR_IDTYPE, .TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_COMMAND_TURNINDICATOR_DLC, 	.FDFormat = CAN_SIM_COMMAND_TURNINDICATOR_FORMAT, 	.BitRateSwitch = CAN_SIM_COMMAND_TURNINDICATOR_BRS } };
#endif

#if defined(TARGET_ECUD)
RAMN_PeriodicFDCANTx_t msg_control_enginekey 			= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_ENGINEKEY_PERIODMS, 		.header = { .Identifier = CAN_SIM_CONTROL_ENGINEKEY_CANID		, 	.IdType = CAN_SIM_CONTROL_ENGINEKEY_IDTYPE, 	.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_ENGINEKEY_DLC, 		.FDFormat = CAN_SIM_CONTROL_ENGINEKEY_FORMAT, 		.BitRateSwitch = CAN_SIM_CONTROL_ENGINEKEY_BRS } };
RAMN_PeriodicFDCANTx_t msg_control_lights 				= {.lastSent = 0, .counter = 0, .periodms = CAN_SIM_CONTROL_LIGHTS_PERIODMS, 			.header = { .Identifier = CAN_SIM_CONTROL_LIGHTS_CANID			, 	.IdType = CAN_SIM_CONTROL_LIGHTS_IDTYPE, 		.TxFrameType = FDCAN_DATA_FRAME , .DataLength = CAN_SIM_CONTROL_LIGHTS_DLC, 		.FDFormat = CAN_SIM_CONTROL_LIGHTS_FORMAT, 			.BitRateSwitch = CAN_SIM_CONTROL_LIGHTS_BRS } };
#endif

