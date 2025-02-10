/*
 * ramn_vehicle_specific.h
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

// This modules defines value which are specific to one vehicle, such as CAN IDs and CAN message payload formats.

#ifndef INC_RAMN_VEHICLE_SPECIFIC_H_
#define INC_RAMN_VEHICLE_SPECIFIC_H_

#include "main.h"

/////////////////////////////////////////
// VEHICLE SPECIFIC DEFINITIONS START  //
/////////////////////////////////////////

// Highest gear that can be selected
#define MAX_GEAR_VALUE 0x06

// Default CAN Message format
#define CAN_MAX_PAYLOAD_SIZE_DEFAULT 	8
typedef volatile /* __attribute__((packed, scalar_storage_order("big-endian"))) */ struct
		{
	uint16_t payload /* :16 */;
	uint16_t counter /* :16 */;
	uint32_t crc32  /* :32 */;
		}
RAMN_DefaultCANFrameFormat_t;

// Definition of Diagnostic IDs
// Note that ISO-TP standard recommends that 0x7e0/0x7e8 pair be assigned to ECM (Engine Control Module) and that 0x7e1/0x7e9 be assigned to TCM (Transmission Control Module)
#if defined(ENABLE_DIAG)

#ifdef ENABLE_UDS
#define UDS_FUNCTIONAL_RX_CANID 0x7df
#endif

#if defined(TARGET_ECUA)

#if defined(ENABLE_UDS)
#define UDS_RX_CANID 0x7e0
#define UDS_TX_CANID UDS_RX_CANID+8
#endif
#if defined(ENABLE_KWP)
#define KWP_RX_CANID 0x7e4
#define KWP_TX_CANID KWP_TX_CANID+8
#endif
#if defined(ENABLE_XCP)
#define XCP_RX_CANID 0x550
#define XCP_TX_CANID XCP_RX_CANID+1
#endif

#elif defined(TARGET_ECUB)

#if defined(ENABLE_UDS)
#define UDS_RX_CANID 0x7e1
#define UDS_TX_CANID UDS_RX_CANID+8
#endif
#if defined(ENABLE_KWP)
#define KWP_RX_CANID 0x7e5
#define KWP_TX_CANID KWP_RX_CANID+8
#endif
#if defined(ENABLE_XCP)
#define XCP_RX_CANID 0x552
#define XCP_TX_CANID XCP_RX_CANID+1
#endif

#elif defined(TARGET_ECUC)

#if defined(ENABLE_UDS)
#define UDS_RX_CANID 0x7e2
#define UDS_TX_CANID UDS_RX_CANID+8
#endif
#if defined(ENABLE_KWP)
#define KWP_RX_CANID 0x7e6
#define KWP_TX_CANID KWP_RX_CANID+8
#endif
#if defined(ENABLE_XCP)
#define XCP_RX_CANID 0x554
#define XCP_TX_CANID XCP_RX_CANID+1
#endif

#elif defined(TARGET_ECUD)

#if defined(ENABLE_UDS)
#define UDS_RX_CANID 0x7e3
#define UDS_TX_CANID UDS_RX_CANID+8
#endif
#if defined(ENABLE_KWP)
#define KWP_RX_CANID 0x7e7
#define KWP_TX_CANID KWP_RX_CANID+8
#endif
#if defined(ENABLE_XCP)
#define XCP_RX_CANID 0x556
#define XCP_TX_CANID XCP_RX_CANID+1
#endif

#endif

#endif

// Default Settings for all messages
#define CAN_SIM_PERIODSMS_DEFAULT						100				   // Period for slow messages
#define CAN_SIM_PERIODSMS_DEFAULT_FAST					10				   // Period for fast messages
#define CAN_SIM_IDTYPE_DEFAULT    						FDCAN_STANDARD_ID  // Standard ID or Extended ID
#define CAN_SIM_FORMAT_DEFAULT    						FDCAN_CLASSIC_CAN  // Classic CAN or CAN-FD
#define CAN_SIM_BRS_DEFAULT       						FDCAN_BRS_OFF      // Bit rate switching ON or OFF
#define CAN_SIM_DLC_DEFAULT   							FDCAN_DLC_BYTES_8  // Default CAN payload size

// Message specific settings
#define CAN_SIM_CONTROL_BRAKE_CANID               		0x24
#define CAN_SIM_CONTROL_BRAKE_PERIODMS		      		CAN_SIM_PERIODSMS_DEFAULT_FAST
#define CAN_SIM_CONTROL_BRAKE_IDTYPE 			  		CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_FORMAT 			  		CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_BRS 				  		CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_DLC 				 		CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_PAYLOADFORMAT_T	 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_CONTROL_ACCEL_CANID           	 		0x39
#define CAN_SIM_CONTROL_ACCEL_PERIODMS		 	 		CAN_SIM_PERIODSMS_DEFAULT_FAST
#define CAN_SIM_CONTROL_BRAKE_IDTYPE 		 	 		CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_FORMAT 		 	 		CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_BRS 		         		CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_DLC 			 	 		CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_BRAKE_PAYLOADFORMAT_T 	 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_CONTROL_STEERING_CANID        			0x62
#define CAN_SIM_CONTROL_STEERING_PERIODMS		  		CAN_SIM_PERIODSMS_DEFAULT_FAST
#define CAN_SIM_CONTROL_STEERING_IDTYPE 				CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_STEERING_FORMAT 				CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_STEERING_BRS 					CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_STEERING_DLC 					CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_STEERING_PAYLOADFORMAT_T 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_CONTROL_SHIFT_CANID           			0x77
#define CAN_SIM_CONTROL_SHIFT_PERIODMS		 			CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_CONTROL_SHIFT_IDTYPE 					CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_SHIFT_FORMAT 					CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_SHIFT_BRS 						CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_SHIFT_DLC 						CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_SHIFT_PAYLOADFORMAT_T 			RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_HORN_CANID            			0x98
#define CAN_SIM_COMMAND_HORN_CANID_PERIODMS		  		CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_COMMAND_HORN_CANID_IDTYPE 				CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_HORN_CANID_FORMAT 				CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_HORN_CANID_BRS 					CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_HORN_CANID_DLC 					CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_HORN_CANID_PAYLOADFORMAT_T 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_CONTROL_SIDEBRAKE_CANID       			0x1D3
#define CAN_SIM_CONTROL_SIDEBRAKE_PERIODMS		  		CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_CONTROL_SIDEBRAKE_IDTYPE 				CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_SIDEBRAKE_FORMAT 				CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_SIDEBRAKE_BRS 					CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_SIDEBRAKE_DLC 					CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_SIDEBRAKE_PAYLOADFORMAT_T 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_TURNINDICATOR_CANID   		  	0x1A7
#define CAN_SIM_COMMAND_TURNINDICATOR_PERIODMS		  	CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_COMMAND_TURNINDICATOR_IDTYPE 		  	CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_TURNINDICATOR_FORMAT 		  	CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_TURNINDICATOR_BRS 			  	CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_TURNINDICATOR_DLC 			  	CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_TURNINDICATOR_PAYLOADFORMAT_T 	RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_CONTROL_ENGINEKEY_CANID		  			0x1B8
#define CAN_SIM_CONTROL_ENGINEKEY_PERIODMS		  		CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_CONTROL_ENGINEKEY_IDTYPE 				CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_ENGINEKEY_FORMAT 				CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_ENGINEKEY_BRS 					CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_ENGINEKEY_DLC 					CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_ENGINEKEY_PAYLOADFORMAT_T 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_LIGHTS_CANID	      			0x150
#define CAN_SIM_COMMAND_LIGHTS_PERIODMS		  			CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_COMMAND_LIGHTS_IDTYPE 					CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_LIGHTS_FORMAT 					CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_LIGHTS_BRS 						CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_LIGHTS_DLC 						CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_LIGHTS_PAYLOADFORMAT_T 			RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_CONTROL_LIGHTS_CANID          			0x1BB
#define CAN_SIM_CONTROL_LIGHTS_PERIODMS		  			CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_CONTROL_LIGHTS_IDTYPE 					CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_LIGHTS_FORMAT 					CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_LIGHTS_BRS 						CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_LIGHTS_DLC 						CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_LIGHTS_PAYLOADFORMAT_T 			RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_BRAKE_CANID			  			0x1A
#define CAN_SIM_COMMAND_BRAKE_PERIODMS		  			CAN_SIM_PERIODSMS_DEFAULT_FAST
#define CAN_SIM_COMMAND_BRAKE_IDTYPE 					CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_BRAKE_FORMAT 					CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_BRAKE_BRS 						CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_BRAKE_DLC 						CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_BRAKE_PAYLOADFORMAT_T 			RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_ACCEL_CANID		      			0x2F
#define CAN_SIM_COMMAND_ACCEL_PERIODMS		  			CAN_SIM_PERIODSMS_DEFAULT_FAST
#define CAN_SIM_COMMAND_ACCEL_IDTYPE 					CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_ACCEL_FORMAT 					CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_ACCEL_BRS 						CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_ACCEL_DLC 						CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_ACCEL_PAYLOADFORMAT_T 			RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_STATUS_RPM_CANID			  			0x43
#define CAN_SIM_STATUS_RPM_PERIODMS		  				CAN_SIM_PERIODSMS_DEFAULT_FAST
#define CAN_SIM_STATUS_RPM_IDTYPE 						CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_STATUS_RPM_FORMAT 						CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_STATUS_RPM_BRS 							CAN_SIM_BRS_DEFAULT
#define CAN_SIM_STATUS_RPM_DLC 							CAN_SIM_DLC_DEFAULT
#define CAN_SIM_STATUS_RPM_PAYLOADFORMAT_T 				RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_STEERING_CANID		  			0x58
#define CAN_SIM_COMMAND_STEERING_PERIODMS		  		CAN_SIM_PERIODSMS_DEFAULT_FAST
#define CAN_SIM_COMMAND_STEERING_IDTYPE 				CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_STEERING_FORMAT 				CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_STEERING_BRS 					CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_STEERING_DLC 					CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_STEERING_PAYLOADFORMAT_T 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_SIDEBRAKE_CANID	      			0x1C9
#define CAN_SIM_COMMAND_SIDEBRAKE_PERIODMS		  		CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_COMMAND_SIDEBRAKE_IDTYPE 				CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_SIDEBRAKE_FORMAT 				CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_SIDEBRAKE_BRS 					CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_SIDEBRAKE_DLC 					CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_SIDEBRAKE_PAYLOADFORMAT_T 		RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_COMMAND_SHIFT_CANID 		  			0x6D
#define CAN_SIM_COMMAND_SHIFT_PERIODMS		  			CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_COMMAND_SHIFT_IDTYPE 					CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_COMMAND_SHIFT_FORMAT 					CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_COMMAND_SHIFT_BRS 						CAN_SIM_BRS_DEFAULT
#define CAN_SIM_COMMAND_SHIFT_DLC 						CAN_SIM_DLC_DEFAULT
#define CAN_SIM_COMMAND_SHIFT_PAYLOADFORMAT_T 			RAMN_DefaultCANFrameFormat_t

#define CAN_SIM_CONTROL_HORN_CANID            			0xA2
#define CAN_SIM_CONTROL_HORN_PERIODMS		  			CAN_SIM_PERIODSMS_DEFAULT
#define CAN_SIM_CONTROL_HORN_IDTYPE 					CAN_SIM_IDTYPE_DEFAULT
#define CAN_SIM_CONTROL_HORN_FORMAT 					CAN_SIM_FORMAT_DEFAULT
#define CAN_SIM_CONTROL_HORN_BRS 						CAN_SIM_BRS_DEFAULT
#define CAN_SIM_CONTROL_HORN_DLC 						CAN_SIM_DLC_DEFAULT
#define CAN_SIM_CONTROL_HORN_PAYLOADFORMAT_T 			RAMN_DefaultCANFrameFormat_t

// This section defines which CAN ID an ECU is configured to receive.
// May be used to setup CAN filters.
#if defined(TARGET_ECUA)
#define RECEIVE_CONTROL_BRAKE
#define RECEIVE_CONTROL_ACCEL
#define RECEIVE_CONTROL_STEERING
#define RECEIVE_CONTROL_SHIFT
#define RECEIVE_CONTROL_SIDEBRAKE
#define RECEIVE_CONTROL_ENGINEKEY
#define RECEIVE_COMMAND_HORN
#define RECEIVE_COMMAND_LIGHTS
#define RECEIVE_CONTROL_LIGHTS
#elif defined(TARGET_ECUB)
#define RECEIVE_COMMAND_LIGHTS
#define RECEIVE_STATUS_RPM
#define RECEIVE_COMMAND_STEERING
#define RECEIVE_COMMAND_SIDEBRAKE
#elif defined(TARGET_ECUC)
#define RECEIVE_CONTROL_ENGINEKEY
#define RECEIVE_COMMAND_BRAKE
#define RECEIVE_COMMAND_ACCEL
#define RECEIVE_STATUS_RPM
#define RECEIVE_COMMAND_SIDEBRAKE
#define RECEIVE_COMMAND_SHIFT
#elif defined(TARGET_ECUD)
#define RECEIVE_CONTROL_BRAKE
#define RECEIVE_CONTROL_SIDEBRAKE
#define RECEIVE_STATUS_RPM
#define RECEIVE_COMMAND_TURNINDICATOR
#define RECEIVE_COMMAND_LIGHTS
#endif

///////////////////////////////////////
// VEHICLE SPECIFIC DEFINITIONS END  //
///////////////////////////////////////

// Union that has both CAN header and data
typedef union
{
	uint8_t rawData[CAN_MAX_PAYLOAD_SIZE_DEFAULT];
	RAMN_DefaultCANFrameFormat_t ramnData;
} RAMN_CANFrameData_t;

// Structure that has both CAN header, data, and transmission information
typedef struct
{
	FDCAN_TxHeaderTypeDef header;
	RAMN_CANFrameData_t data[CAN_MAX_PAYLOAD_SIZE_DEFAULT];
	uint32_t periodms; // Target period for periodic transmission
	uint32_t counter;  // Number of time the message has been sent
	uint32_t lastSent; // Timestamp of last transmission
} RAMN_PeriodicFDCANTx_t;

// Definition of messages that are periodically sent by ECU
#if defined(TARGET_ECUA)
extern RAMN_PeriodicFDCANTx_t msg_command_brake;
extern RAMN_PeriodicFDCANTx_t msg_command_accel;
extern RAMN_PeriodicFDCANTx_t msg_status_RPM;
extern RAMN_PeriodicFDCANTx_t msg_command_steering;
extern RAMN_PeriodicFDCANTx_t msg_command_shift;
extern RAMN_PeriodicFDCANTx_t msg_control_horn;
extern RAMN_PeriodicFDCANTx_t msg_command_parkingbrake;
#endif

#if defined(TARGET_ECUB)
extern RAMN_PeriodicFDCANTx_t msg_control_steering;
extern RAMN_PeriodicFDCANTx_t msg_control_sidebrake;
extern RAMN_PeriodicFDCANTx_t msg_command_lights;
#endif

#if defined(TARGET_ECUC)
extern RAMN_PeriodicFDCANTx_t msg_control_brake;
extern RAMN_PeriodicFDCANTx_t msg_control_accel;
extern RAMN_PeriodicFDCANTx_t msg_control_shift;
extern RAMN_PeriodicFDCANTx_t msg_command_horn;
extern RAMN_PeriodicFDCANTx_t msg_command_turnindicator;
#endif

#if defined(TARGET_ECUD)
extern RAMN_PeriodicFDCANTx_t msg_control_enginekey;
extern RAMN_PeriodicFDCANTx_t msg_control_lights;
#endif

#endif /* INC_RAMN_VEHICLE_SPECIFIC_H_ */
