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

// Data-driven definitions for default message format fields
#define RAMN_MSG_PAYLOAD_MASK        0xFFFF
#define RAMN_MSG_COUNTER_MASK        0xFFFF
#define RAMN_MSG_COUNTER_OFFSET      16
#define RAMN_MSG_CRC_MASK            0xFFFFFFFF
#define RAMN_MSG_CRC_OFFSET          32

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
#define KWP_TX_CANID KWP_RX_CANID+8
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

#define J1939_EMPTY_PAYLOAD { .data = { { .rawData = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} } } }

// J1939 ID helper macros and settings. These are needed unconditionally now that both traffic
// profiles (default + J1939) are compiled into every image (see ramn_traffic_profiles.c), so they
// live outside the traffic-mode guard.
#include "ramn_j1939.h"

#define J1939_ID(prio, edp, dp, pf, ps_da, sa) \
    ((((prio) & 0x7) << 26) | \
     (((edp) & 0x1) << 25) | \
     (((dp) & 0x1) << 24) | \
     (((pf) & 0xFF) << 16) | \
     (((ps_da) & 0xFF) << 8) | \
     ((sa) & 0xFF))

/* Verification for PDU1 broadcast (DA=255) */
#define J1939_CHECK_PDU1_DA J1939_ID(6, 0, 0, 2, 255, 11)
#if ((J1939_CHECK_PDU1_DA >> 8) & 0xFF) != 0xFF
  #error "J1939_ID macro failed to correctly insert DA=255 in bits 8-15"
#endif

#define J1939_UCAST_ID(prio, pgn, da, sa) \
    J1939_ID((prio), 0, ((pgn) >> 16) & 0x1, ((pgn) >> 8) & 0xFF, (da), (sa))

#define J1939_BCAST_ID(prio, pgn, sa) \
    J1939_ID((prio), 0, ((pgn) >> 16) & 0x1, ((pgn) >> 8) & 0xFF, (pgn) & 0xFF, (sa))

#define CAN_SIM_J1939_IDTYPE      FDCAN_EXTENDED_ID
#define CAN_SIM_J1939_FORMAT      FDCAN_CLASSIC_CAN
#define CAN_SIM_J1939_BRS         FDCAN_BRS_OFF
#define CAN_SIM_J1939_DLC         FDCAN_DLC_BYTES_8

// Default-mode CAN IDs (11-bit). Both traffic profiles are compiled into every image; these literal
// IDs describe the default (RAMN-native) traffic. They are the single source used by the default TX
// catalog / RX map (ramn_traffic_profiles.c), the hardware RX filter list (ramn_canfd.c), and the
// screen filter list (ramn_screen_canlog.c). The J1939 profile builds its extended IDs from the
// J1939_*CAST_ID macros above.
#define DID_COMMAND_BRAKE          0x1A
#define DID_COMMAND_ACCEL          0x2F
#define DID_STATUS_RPM             0x43
#define DID_COMMAND_STEERING       0x58
#define DID_COMMAND_SHIFT          0x6D
#define DID_COMMAND_SIDEBRAKE      0x1C9
#define DID_COMMAND_TURNINDICATOR  0x1A7
#define DID_COMMAND_HORN           0x98
#define DID_COMMAND_LIGHTS         0x150
#define DID_CONTROL_BRAKE          0x24
#define DID_CONTROL_ACCEL          0x39
#define DID_CONTROL_STEERING       0x62
#define DID_CONTROL_SHIFT          0x77
#define DID_CONTROL_HORN           0xA2
#define DID_CONTROL_SIDEBRAKE      0x1D3
#define DID_CONTROL_ENGINEKEY      0x1B8
#define DID_CONTROL_LIGHTS         0x1BB

// This section defines which CAN ID an ECU is configured to receive.
// May be used to setup CAN filters.
#if defined(TARGET_ECUA)
#define RECEIVE_CONTROL_BRAKE
#define RECEIVE_CONTROL_ACCEL
#define RECEIVE_CONTROL_STEERING
#define RECEIVE_CONTROL_SHIFT
#define RECEIVE_JOYSTICK_BUTTONS
#define RECEIVE_CONTROL_SIDEBRAKE
#define RECEIVE_CONTROL_ENGINEKEY
#define RECEIVE_COMMAND_LIGHTS
#define RECEIVE_CONTROL_LIGHTS
#define RECEIVE_COMMAND_HORN
#elif defined(TARGET_ECUB)
#define RECEIVE_COMMAND_STEERING
#define RECEIVE_COMMAND_SIDEBRAKE
#define RECEIVE_COMMAND_LIGHTS
#define RECEIVE_STATUS_RPM
#elif defined(TARGET_ECUC)
#define RECEIVE_CONTROL_ENGINEKEY
#define RECEIVE_COMMAND_BRAKE
#define RECEIVE_COMMAND_ACCEL
#define RECEIVE_STATUS_RPM
#define RECEIVE_COMMAND_SIDEBRAKE
#define RECEIVE_COMMAND_SHIFT
#define RECEIVE_CONTROL_HORN
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
} RAMN_CANFrameData_t;

// Structure that has both CAN header, data, and transmission information
typedef struct
{
	FDCAN_TxHeaderTypeDef header;
	RAMN_CANFrameData_t data[CAN_MAX_PAYLOAD_SIZE_DEFAULT];
	uint32_t periodms; // Target period for periodic transmission
	uint32_t counter;  // Number of time the message has been sent
	uint32_t lastSent; // Timestamp of last transmission
	int16_t counterOffset;
	int16_t crcOffset;
} RAMN_PeriodicFDCANTx_t;

// Periodic TX message indices for this ECU. They index the runtime TX array (txRuntime[] in
// ramn_dbc.c) and must stay in lockstep with the per-ECU TX catalog row order in
// ramn_traffic_profiles.c. NUM_PERIODIC_TX sizes the runtime array.
#if defined(TARGET_ECUA)
enum {
	TXIDX_COMMAND_BRAKE = 0,
	TXIDX_COMMAND_ACCEL,
	TXIDX_STATUS_RPM,
	TXIDX_COMMAND_STEERING,
	TXIDX_COMMAND_SHIFT,
	TXIDX_CONTROL_HORN,
	TXIDX_COMMAND_PARKINGBRAKE,
	NUM_PERIODIC_TX
};
#elif defined(TARGET_ECUB)
enum {
	TXIDX_CONTROL_STEERING = 0,
	TXIDX_CONTROL_SIDEBRAKE,
	TXIDX_COMMAND_LIGHTS,
	NUM_PERIODIC_TX
};
#elif defined(TARGET_ECUC)
enum {
	TXIDX_CONTROL_BRAKE = 0,
	TXIDX_CONTROL_ACCEL,
	TXIDX_CONTROL_SHIFT,
	TXIDX_COMMAND_HORN,
	TXIDX_COMMAND_TURNINDICATOR,
	TXIDX_JOYSTICK_BUTTONS,
	NUM_PERIODIC_TX
};
#elif defined(TARGET_ECUD)
enum {
	TXIDX_CONTROL_ENGINEKEY = 0,
	TXIDX_CONTROL_LIGHTS,
	NUM_PERIODIC_TX
};
#endif

// Runtime TX backing store (payload/counter/lastSent are mutable; header/period/offsets are loaded
// from the active traffic profile catalog at RAMN_DBC_Init). Defined in ramn_dbc.c, indexed by TXIDX_*.
// Guarded on a target being selected: NUM_PERIODIC_TX only exists for a concrete ECU (host codec-only
// test builds compile ramn_can_database.c with no TARGET_ECU).
#if defined(TARGET_ECUA) || defined(TARGET_ECUB) || defined(TARGET_ECUC) || defined(TARGET_ECUD)
extern RAMN_PeriodicFDCANTx_t txRuntime[NUM_PERIODIC_TX];
#endif

#endif /* INC_RAMN_VEHICLE_SPECIFIC_H_ */
