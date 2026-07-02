/*
 * ramn_traffic_profiles.c
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2026 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/*
 * The single place where "default vs J1939" traffic is described as data. Both profiles are built
 * side by side (into every image), so g_trafficProfile can be repointed at runtime.
 *
 * The per-message CAN_SIM_* macros in ramn_vehicle_specific.h collide between modes (same names),
 * so this file does NOT use them for the catalogs. Instead each profile is built from primitives:
 *   - default: the DID_* literal CAN IDs (from ramn_vehicle_specific.h), the CAN_SIM_*_DEFAULT
 *     settings, and the RAMN_MSG_* field offsets -- all unconditional;
 *   - J1939:   the J1939_UCAST_ID/J1939_BCAST_ID macros + PGN/SA/DA constants -- all unconditional.
 */

#include "ramn_vehicle_specific.h"
#include "ramn_traffic.h"
#include "ramn_can_database.h"

/* -------------------------------------------------------------------------------------------------
 * Row builders. Field order matches RAMN_MsgDescriptor_t:
 * { canId, idType, fdFormat, brs, dlc, periodMs, payloadOffset, counterOffset, crcOffset, signalId }
 * ---------------------------------------------------------------------------------------------- */

/* Default row: standard 11-bit ID, classic CAN, 8 bytes, counter@16 / CRC@32. */
#define DEF_ROW(canid, period, sig) \
	{ (canid), CAN_SIM_IDTYPE_DEFAULT, CAN_SIM_FORMAT_DEFAULT, CAN_SIM_BRS_DEFAULT, CAN_SIM_DLC_DEFAULT, \
	  (period), RAMN_MSG_PAYLOAD_OFFSET, RAMN_MSG_COUNTER_OFFSET, RAMN_MSG_CRC_OFFSET, (sig) }

/* J1939 row: extended 29-bit ID, no counter/CRC (offsets -1). */
#define J1939_ROW(canid, period, dlc, sig) \
	{ (canid), CAN_SIM_J1939_IDTYPE, CAN_SIM_J1939_FORMAT, CAN_SIM_J1939_BRS, (dlc), \
	  (period), -1, -1, -1, (sig) }

/* Default-mode 11-bit CAN IDs (DID_*) now live in ramn_vehicle_specific.h so the TX catalog / RX map
 * here, the hardware RX filters (ramn_canfd.c), and the screen filter list (ramn_screen_canlog.c) all
 * share one source. */

/* -------------------------------------------------------------------------------------------------
 * Per-ECU TX catalogs. Rows are indexed by TXIDX_* so they cannot fall out of lockstep with the
 * txRuntime[] backing store in ramn_dbc.c.
 * ---------------------------------------------------------------------------------------------- */

#if defined(TARGET_ECUA)
static const RAMN_MsgDescriptor_t txCatalog_default[] = {
	[TXIDX_COMMAND_BRAKE]        = DEF_ROW(DID_COMMAND_BRAKE,     CAN_SIM_PERIODSMS_DEFAULT_FAST, SIG_COMMAND_BRAKE),
	[TXIDX_COMMAND_ACCEL]        = DEF_ROW(DID_COMMAND_ACCEL,     CAN_SIM_PERIODSMS_DEFAULT_FAST, SIG_COMMAND_ACCEL),
	[TXIDX_STATUS_RPM]           = DEF_ROW(DID_STATUS_RPM,        CAN_SIM_PERIODSMS_DEFAULT_FAST, SIG_STATUS_RPM),
	[TXIDX_COMMAND_STEERING]     = DEF_ROW(DID_COMMAND_STEERING,  CAN_SIM_PERIODSMS_DEFAULT_FAST, SIG_COMMAND_STEERING),
	[TXIDX_COMMAND_SHIFT]        = DEF_ROW(DID_COMMAND_SHIFT,     CAN_SIM_PERIODSMS_DEFAULT,      SIG_COMMAND_SHIFT),
	[TXIDX_CONTROL_HORN]         = DEF_ROW(DID_CONTROL_HORN,      CAN_SIM_PERIODSMS_DEFAULT,      SIG_CONTROL_HORN),
	[TXIDX_COMMAND_PARKINGBRAKE] = DEF_ROW(DID_COMMAND_SIDEBRAKE, CAN_SIM_PERIODSMS_DEFAULT,      SIG_COMMAND_SIDEBRAKE),
};
static const RAMN_MsgDescriptor_t txCatalog_j1939[] = {
	[TXIDX_COMMAND_BRAKE]        = J1939_ROW(J1939_UCAST_ID(1, J1939_PGN_XBR,  J1939_DA_BRAKE_SYSTEM,  J1939_SA_HEADWAY_CTRL), 20,  CAN_SIM_J1939_DLC, SIG_COMMAND_BRAKE),
	[TXIDX_COMMAND_ACCEL]        = J1939_ROW(J1939_UCAST_ID(3, J1939_PGN_TSC1, J1939_DA_ENGINE,        J1939_SA_HEADWAY_CTRL), 10,  CAN_SIM_J1939_DLC, SIG_COMMAND_ACCEL),
	[TXIDX_STATUS_RPM]           = J1939_ROW(J1939_BCAST_ID(3, J1939_PGN_EEC1,                         J1939_SA_HEADWAY_CTRL), 20,  CAN_SIM_J1939_DLC, SIG_STATUS_RPM),
	[TXIDX_COMMAND_STEERING]     = J1939_ROW(J1939_UCAST_ID(2, J1939_PGN_PROPA, J1939_DA_STEERING_CTRL, J1939_SA_HEADWAY_CTRL), 20, CAN_SIM_J1939_DLC, SIG_COMMAND_STEERING),
	[TXIDX_COMMAND_SHIFT]        = J1939_ROW(J1939_UCAST_ID(3, J1939_PGN_TC1,  J1939_DA_TRANSMISSION,  J1939_SA_HEADWAY_CTRL), 50,  CAN_SIM_J1939_DLC, SIG_COMMAND_SHIFT),
	[TXIDX_CONTROL_HORN]         = J1939_ROW(J1939_UCAST_ID(6, J1939_PGN_PROPA, J1939_DA_POWERTRAIN_CTRL, J1939_SA_HEADWAY_CTRL), 100, CAN_SIM_J1939_DLC, SIG_CONTROL_HORN),
	[TXIDX_COMMAND_PARKINGBRAKE] = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_CCVS1,                        J1939_SA_HEADWAY_CTRL), 100, CAN_SIM_J1939_DLC, SIG_COMMAND_SIDEBRAKE),
};
#endif

#if defined(TARGET_ECUB)
static const RAMN_MsgDescriptor_t txCatalog_default[] = {
	[TXIDX_CONTROL_STEERING]  = DEF_ROW(DID_CONTROL_STEERING,  CAN_SIM_PERIODSMS_DEFAULT_FAST, SIG_CONTROL_STEERING),
	[TXIDX_CONTROL_SIDEBRAKE] = DEF_ROW(DID_CONTROL_SIDEBRAKE, CAN_SIM_PERIODSMS_DEFAULT,      SIG_CONTROL_SIDEBRAKE),
	[TXIDX_COMMAND_LIGHTS]    = DEF_ROW(DID_COMMAND_LIGHTS,    CAN_SIM_PERIODSMS_DEFAULT,      SIG_COMMAND_LIGHTS),
};
static const RAMN_MsgDescriptor_t txCatalog_j1939[] = {
	[TXIDX_CONTROL_STEERING]  = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_VDC2,        J1939_SA_STEERING_CTRL),       10,   CAN_SIM_J1939_DLC, SIG_CONTROL_STEERING),
	[TXIDX_CONTROL_SIDEBRAKE] = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_B1,          J1939_SA_BRAKES_DRIVE_AXLE_1), 1000, CAN_SIM_J1939_DLC, SIG_CONTROL_SIDEBRAKE),
	[TXIDX_COMMAND_LIGHTS]    = J1939_ROW(J1939_BCAST_ID(3, J1939_PGN_LIGHTS_CMD,  J1939_SA_CHASSIS_CTRL_1),      100,  CAN_SIM_J1939_DLC, SIG_COMMAND_LIGHTS),
};
#endif

#if defined(TARGET_ECUC)
/* Default catalog stops before TXIDX_JOYSTICK_BUTTONS (J1939-only message): the array is one entry
 * shorter, so txCount excludes it and the slot is never sent in default mode. */
static const RAMN_MsgDescriptor_t txCatalog_default[] = {
	[TXIDX_CONTROL_BRAKE]         = DEF_ROW(DID_CONTROL_BRAKE,        CAN_SIM_PERIODSMS_DEFAULT_FAST, SIG_CONTROL_BRAKE),
	[TXIDX_CONTROL_ACCEL]         = DEF_ROW(DID_CONTROL_ACCEL,        CAN_SIM_PERIODSMS_DEFAULT_FAST, SIG_CONTROL_ACCEL),
	[TXIDX_CONTROL_SHIFT]         = DEF_ROW(DID_CONTROL_SHIFT,        CAN_SIM_PERIODSMS_DEFAULT,      SIG_CONTROL_SHIFT),
	[TXIDX_COMMAND_HORN]          = DEF_ROW(DID_COMMAND_HORN,         CAN_SIM_PERIODSMS_DEFAULT,      SIG_COMMAND_HORN),
	[TXIDX_COMMAND_TURNINDICATOR] = DEF_ROW(DID_COMMAND_TURNINDICATOR, CAN_SIM_PERIODSMS_DEFAULT,     SIG_COMMAND_TURNINDICATOR),
};
static const RAMN_MsgDescriptor_t txCatalog_j1939[] = {
	[TXIDX_CONTROL_BRAKE]         = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_EBC1,        J1939_SA_POWERTRAIN_CTRL), 100, CAN_SIM_J1939_DLC, SIG_CONTROL_BRAKE),
	[TXIDX_CONTROL_ACCEL]         = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_EEC2,        J1939_SA_POWERTRAIN_CTRL), 50,  CAN_SIM_J1939_DLC, SIG_CONTROL_ACCEL),
	[TXIDX_CONTROL_SHIFT]         = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_ETC2,        J1939_SA_TRANSMISSION),    100, CAN_SIM_J1939_DLC, SIG_CONTROL_SHIFT),
	[TXIDX_COMMAND_HORN]          = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_CM3,         J1939_SA_POWERTRAIN_CTRL), 100, CAN_SIM_J1939_DLC, SIG_COMMAND_HORN),
	[TXIDX_COMMAND_TURNINDICATOR] = J1939_ROW(J1939_BCAST_ID(3, J1939_PGN_OEL,         J1939_SA_SHIFT_CONSOLE),   100, CAN_SIM_J1939_DLC, SIG_COMMAND_TURNINDICATOR),
	/* J1939-only: joystick buttons ship as their own PGN (special encode hook), DLC literal 8. */
	[TXIDX_JOYSTICK_BUTTONS]      = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_PROPB_65282, J1939_SA_SHIFT_CONSOLE),   50,  8,                 SIG_SPECIAL_ENCODE),
};
#endif

#if defined(TARGET_ECUD)
static const RAMN_MsgDescriptor_t txCatalog_default[] = {
	[TXIDX_CONTROL_ENGINEKEY] = DEF_ROW(DID_CONTROL_ENGINEKEY, CAN_SIM_PERIODSMS_DEFAULT, SIG_CONTROL_ENGINEKEY),
	[TXIDX_CONTROL_LIGHTS]    = DEF_ROW(DID_CONTROL_LIGHTS,    CAN_SIM_PERIODSMS_DEFAULT, SIG_CONTROL_LIGHTS),
};
static const RAMN_MsgDescriptor_t txCatalog_j1939[] = {
	[TXIDX_CONTROL_ENGINEKEY] = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_CM3,          J1939_SA_BODY_CTRL), 100, CAN_SIM_J1939_DLC, SIG_CONTROL_ENGINEKEY),
	[TXIDX_CONTROL_LIGHTS]    = J1939_ROW(J1939_BCAST_ID(6, J1939_PGN_PROPB_65280,  J1939_SA_BODY_CTRL), 100, CAN_SIM_J1939_DLC, SIG_CONTROL_LIGHTS),
};
#endif

/* Catalogs can never be larger than the txRuntime[] backing store they are loaded into. */
_Static_assert(sizeof(txCatalog_default) / sizeof(txCatalog_default[0]) <= NUM_PERIODIC_TX, "txCatalog_default larger than txRuntime[]");
_Static_assert(sizeof(txCatalog_j1939)  / sizeof(txCatalog_j1939[0])  == NUM_PERIODIC_TX, "txCatalog_j1939 must fill txRuntime[]");

/* -------------------------------------------------------------------------------------------------
 * RX maps (global per mode -- the decode switch is not ECU-specific; the CAN hardware filter limits
 * what is actually received). rxKey is the 11-bit CAN ID in default mode, or the PGN in J1939.
 * Entries whose decode needs a sub-demux (shared PGN) or a guard/side-effect carry SIG_SPECIAL_ENCODE
 * or are recognised by rxKey in the ProcessCANMessage hooks.
 * ---------------------------------------------------------------------------------------------- */

static const RAMN_RxMapEntry_t rxMap_default[] = {
	{ DID_CONTROL_BRAKE,         SIG_CONTROL_BRAKE },
	{ DID_COMMAND_BRAKE,         SIG_COMMAND_BRAKE },
	{ DID_CONTROL_ACCEL,         SIG_CONTROL_ACCEL },
	{ DID_COMMAND_ACCEL,         SIG_COMMAND_ACCEL },
	{ DID_STATUS_RPM,            SIG_STATUS_RPM },
	{ DID_CONTROL_STEERING,      SIG_CONTROL_STEERING },
	{ DID_COMMAND_STEERING,      SIG_COMMAND_STEERING },
	{ DID_CONTROL_SHIFT,         SIG_CONTROL_SHIFT },      /* also joystick side-channel */
	{ DID_COMMAND_SHIFT,         SIG_COMMAND_SHIFT },
	{ DID_COMMAND_HORN,          SIG_COMMAND_HORN },
	{ DID_CONTROL_HORN,          SIG_CONTROL_HORN },
	{ DID_CONTROL_SIDEBRAKE,     SIG_CONTROL_SIDEBRAKE },
	{ DID_COMMAND_SIDEBRAKE,     SIG_COMMAND_SIDEBRAKE },
	{ DID_COMMAND_TURNINDICATOR, SIG_COMMAND_TURNINDICATOR },
	{ DID_CONTROL_ENGINEKEY,     SIG_CONTROL_ENGINEKEY },
	{ DID_COMMAND_LIGHTS,        SIG_COMMAND_LIGHTS },
	{ DID_CONTROL_LIGHTS,        SIG_CONTROL_LIGHTS },
};

static const RAMN_RxMapEntry_t rxMap_j1939[] = {
	{ J1939_PGN_EBC1,         SIG_CONTROL_BRAKE },
	{ J1939_PGN_XBR,          SIG_COMMAND_BRAKE },
	{ J1939_PGN_EEC2,         SIG_CONTROL_ACCEL },
	{ J1939_PGN_TSC1,         SIG_COMMAND_ACCEL },
	{ J1939_PGN_EEC1,         SIG_STATUS_RPM },
	{ J1939_PGN_VDC2,         SIG_CONTROL_STEERING },
	{ J1939_PGN_ETC2,         SIG_CONTROL_SHIFT },
	{ J1939_PGN_TC1,          SIG_COMMAND_SHIFT },
	{ J1939_PGN_B1,           SIG_CONTROL_SIDEBRAKE },
	{ J1939_PGN_CCVS1,        SIG_COMMAND_SIDEBRAKE },
	{ J1939_PGN_OEL,          SIG_COMMAND_TURNINDICATOR },
	{ J1939_PGN_LIGHTS_CMD,   SIG_COMMAND_LIGHTS },      /* guarded store (see ProcessCANMessage) */
	{ J1939_PGN_PROPB_65280,  SIG_CONTROL_LIGHTS },
	{ J1939_PGN_PROPB_65282,  SIG_SPECIAL_ENCODE },      /* joystick buttons */
	{ J1939_PGN_PROPA,        SIG_SPECIAL_ENCODE },      /* demux by DA: command_steer | control_horn */
	{ J1939_PGN_CM3,          SIG_SPECIAL_ENCODE },      /* demux by SA: control_enginekey | command_horn */
};

/* -------------------------------------------------------------------------------------------------
 * Profiles + live pointer.
 * ---------------------------------------------------------------------------------------------- */

const RAMN_TrafficProfile_t profile_default = {
	.codec          = codec_default,
	.txCatalog      = txCatalog_default,
	.txCount        = (uint16_t)(sizeof(txCatalog_default) / sizeof(txCatalog_default[0])),
	.rxMap          = rxMap_default,
	.rxCount        = (uint16_t)(sizeof(rxMap_default) / sizeof(rxMap_default[0])),
	.usesExtendedId = 0,
};

const RAMN_TrafficProfile_t profile_j1939 = {
	.codec          = codec_j1939,
	.txCatalog      = txCatalog_j1939,
	.txCount        = (uint16_t)(sizeof(txCatalog_j1939) / sizeof(txCatalog_j1939[0])),
	.rxMap          = rxMap_j1939,
	.rxCount        = (uint16_t)(sizeof(rxMap_j1939) / sizeof(rxMap_j1939[0])),
	.usesExtendedId = 1,
};

/* Power-on default profile selected by the compile-time DEFAULT_TRAFFIC_MODE knob. This is the only
 * mode #if left in the data path; it just chooses the initial pointer. */
#if DEFAULT_TRAFFIC_MODE == TRAFFIC_MODE_J1939
const RAMN_TrafficProfile_t* g_trafficProfile = &profile_j1939;
#else
const RAMN_TrafficProfile_t* g_trafficProfile = &profile_default;
#endif
