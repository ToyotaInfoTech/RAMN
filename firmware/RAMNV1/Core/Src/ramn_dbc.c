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
#include "ramn_traffic.h"
// J1939 PGN/DA/SA constants are needed unconditionally now: both traffic profiles are compiled in and
// the RX dispatch selects the J1939 demux hooks at runtime (via g_trafficProfile->usesExtendedId).
#include "ramn_j1939.h"

#define NUMBER_OF_PERIODIC_MSG NUM_PERIODIC_TX

volatile RAMN_Bool_t RAMN_DBC_RequestSilence = True;

RAMN_DBC_Handle_t RAMN_DBC_Handle = {.command_steer = 0x7FF, .control_shift =0x01, .command_shift = 0x01, .command_lights = RAMN_LIGHTSWITCH_POS1};

// Runtime TX backing store: one slot per periodic message, indexed by the TXIDX_* enum (kept in
// lockstep with the active profile's TX catalog in ramn_traffic_profiles.c). Fully initialized from
// the active profile at boot and on a live switch: payloads by RAMN_DBC_ResetTxPayloads, headers/
// period/field offsets by RAMN_DBC_LoadRuntimeFromProfile.
RAMN_PeriodicFDCANTx_t txRuntime[NUM_PERIODIC_TX];

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

// Writer: store a decoded signal value into the mode-independent DBC handle.
static void RAMN_DBC_StoreSignal(uint8_t sig, uint16_t val)
{
	switch (sig)
	{
	case SIG_COMMAND_BRAKE:         RAMN_DBC_Handle.command_brake = val; break;
	case SIG_CONTROL_BRAKE:         RAMN_DBC_Handle.control_brake = val; break;
	case SIG_COMMAND_ACCEL:         RAMN_DBC_Handle.command_accel = val; break;
	case SIG_CONTROL_ACCEL:         RAMN_DBC_Handle.control_accel = val; break;
	case SIG_STATUS_RPM:            RAMN_DBC_Handle.status_rpm = val; break;
	case SIG_COMMAND_STEERING:      RAMN_DBC_Handle.command_steer = val; break;
	case SIG_CONTROL_STEERING:      RAMN_DBC_Handle.control_steer = val; break;
	case SIG_COMMAND_SHIFT:         RAMN_DBC_Handle.command_shift = val; break;
	case SIG_CONTROL_SHIFT:         RAMN_DBC_Handle.control_shift = (uint8_t)val; break;
	case SIG_COMMAND_HORN:          RAMN_DBC_Handle.command_horn = (uint8_t)val; break;
	case SIG_CONTROL_HORN:          RAMN_DBC_Handle.control_horn = (uint8_t)val; break;
	case SIG_COMMAND_TURNINDICATOR: RAMN_DBC_Handle.command_turnindicator = val; break;
	case SIG_COMMAND_SIDEBRAKE:     RAMN_DBC_Handle.command_sidebrake = val; break;
	case SIG_CONTROL_SIDEBRAKE:     RAMN_DBC_Handle.control_sidebrake = val; break;
	case SIG_CONTROL_ENGINEKEY:     RAMN_DBC_Handle.control_enginekey = val; break;
	case SIG_COMMAND_LIGHTS:        RAMN_DBC_Handle.command_lights = val; break;
	case SIG_CONTROL_LIGHTS:        RAMN_DBC_Handle.control_lights = val; break;
	default: break;
	}
}

// Load the runtime TX state (headers/period/field offsets) from the active traffic profile's
// descriptor table. The descriptor table is the single source of truth; txRuntime[] holds the
// mutable payload/counter/lastSent. Catalogs are indexed by TXIDX_* and statically asserted to fit
// txRuntime[] (see ramn_traffic_profiles.c), so txCount needs no clamping here.
static void RAMN_DBC_LoadRuntimeFromProfile(void)
{
	for (uint32_t i = 0U; i < g_trafficProfile->txCount; i++)
	{
		const RAMN_MsgDescriptor_t* d = &g_trafficProfile->txCatalog[i];
		RAMN_PeriodicFDCANTx_t* m = &txRuntime[i];
		m->header.Identifier    = d->canId;
		m->header.IdType        = d->idType;
		m->header.FDFormat      = d->fdFormat;
		m->header.BitRateSwitch = d->brs;
		m->header.DataLength    = d->dlc;
		m->header.TxFrameType   = FDCAN_DATA_FRAME;
		m->periodms             = d->periodMs;
		m->counterOffset        = d->counterOffset;
		m->crcOffset            = d->crcOffset;
	}
}

// Reset the mutable TX state (payload/counter/lastSent) to the given profile's idle pattern:
// 0xFF is the J1939 'not available' pattern, 0x00 the default -- with two magic idle payloads for
// ECU C default traffic (joystick centered in Control_Shift, horn released). Shared by boot and
// live profile switching so both start from the same state.
static void RAMN_DBC_ResetTxPayloads(const RAMN_TrafficProfile_t* p)
{
	const uint8_t fill = (p->usesExtendedId != 0U) ? 0xFFU : 0x00U;
	for (uint16_t i = 0; i < NUMBER_OF_PERIODIC_MSG; i++)
	{
		for (uint32_t b = 0; b < sizeof(txRuntime[i].data->rawData); b++)
		{
			txRuntime[i].data->rawData[b] = fill;
		}
		txRuntime[i].counter  = 0;
		txRuntime[i].lastSent = 0;
	}
#if defined(TARGET_ECUC)
	if (p->usesExtendedId == 0U)
	{
		const uint8_t idleShift[8] = {0xFF, 0xFF, 0xFF, 125, 126, 0xFF, 0xFF, 0xFF};
		const uint8_t idleHorn[8]  = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
		RAMN_memcpy(txRuntime[TXIDX_CONTROL_SHIFT].data->rawData, idleShift, 8U);
		RAMN_memcpy(txRuntime[TXIDX_COMMAND_HORN].data->rawData, idleHorn, 8U);
	}
#endif
}

void RAMN_DBC_Init(void)
{
#if defined(TARGET_ECUA) && !defined(RAMN_SHOWCASE_MODE)
	RAMN_DBC_RequestSilence = True;
#else
	RAMN_DBC_RequestSilence = False;
#endif

	RAMN_DBC_ResetTxPayloads(g_trafficProfile);
	RAMN_DBC_LoadRuntimeFromProfile();
}

// Switch the active traffic profile at runtime. Gates TX while it repoints g_trafficProfile, resets
// the mutable TX backing store to the new mode's idle pattern, and reloads headers/period/offsets
// from the new catalog. The prior silence state is preserved (a live switch should not un-mute or
// mute the bus on its own).
//
// The full switch is handled live: the FDCAN RX filters are reloaded via RAMN_FDCAN_ReloadProfileFilters
// (the 11-bit standard DBC filters are installed in both modes; only the extended "accept all" slot
// toggles), the J1939 transport is reactive and runtime-gated on g_trafficProfile->usesExtendedId (see
// ramn_customize.c, nothing to start/stop here), and the diagnostic transports accept both standard and
// J1939 addressing at all times, so the ECU stays diagnosable across a switch on every ECU.
void RAMN_DBC_SetProfile(const RAMN_TrafficProfile_t* p)
{
	if ((p == 0) || (p == g_trafficProfile)) return;

	RAMN_Bool_t prevSilence = RAMN_DBC_RequestSilence;
	RAMN_DBC_RequestSilence = True;

	// Repointing g_trafficProfile is not synchronized with the CAN RX task: a frame already being
	// processed may mix old-profile matching with new-profile decoding. Worst case is one signal
	// value from one frame, which the periodic traffic immediately corrects -- accepted by design.
	g_trafficProfile = p;

	RAMN_DBC_ResetTxPayloads(p);
	RAMN_DBC_LoadRuntimeFromProfile();

	// Reload the RX hardware filters for the new mode (11-bit standard set already covers both modes;
	// this toggles the J1939 "accept all extended" filter). No-op on ECUs without hardware filters.
	RAMN_FDCAN_ReloadProfileFilters();

	RAMN_DBC_RequestSilence = prevSilence;
}

void RAMN_DBC_ProcessCANMessage(uint32_t canid, uint32_t dlc, RAMN_CANFrameData_t* dataframe)
{
	if (dlc == 0U) return;

	const uint8_t* p = &dataframe->rawData[0]; // payload offset is 0 in both modes

	// Extract the lookup key: CAN ID in default mode, PGN in J1939 (extended-ID profiles).
	uint32_t key;
	if (g_trafficProfile->usesExtendedId)
	{
		key = (canid >> 8) & 0x3FFFF;
		uint8_t pf = (uint8_t)(key >> 8);
		if (pf < 240) key &= 0x3FF00; // PDU1: PS is DA, not part of PGN
	}
	else
	{
		key = canid;
	}

	for (uint16_t i = 0; i < g_trafficProfile->rxCount; i++)
	{
		if (g_trafficProfile->rxMap[i].rxKey != key) continue;
		uint8_t sig = g_trafficProfile->rxMap[i].signalId;

		if (g_trafficProfile->usesExtendedId)
		{
			// Irregular J1939 cases: shared-PGN demux, guarded stores, joystick side-channel.
			if (key == J1939_PGN_PROPA)
			{
				uint8_t da = (uint8_t)(canid >> 8);
				if (da == J1939_DA_STEERING_CTRL)
					RAMN_DBC_Handle.command_steer = g_trafficProfile->codec[SIG_COMMAND_STEERING].decode(p, dlc);
				else if (da == J1939_DA_POWERTRAIN_CTRL)
					RAMN_DBC_Handle.control_horn = (uint8_t)g_trafficProfile->codec[SIG_CONTROL_HORN].decode(p, dlc);
				return;
			}
			if (key == J1939_PGN_CM3)
			{
				uint8_t sa = (uint8_t)(canid & 0xFF);
				if (sa == J1939_SA_BODY_CTRL)
				{
					uint8_t enginekey = (uint8_t)g_trafficProfile->codec[SIG_CONTROL_ENGINEKEY].decode(p, dlc);
#ifdef RAMN_SHOWCASE_MODE
					RAMN_DBC_Handle.control_enginekey = enginekey;
#else
					if (enginekey != 3) RAMN_DBC_Handle.control_enginekey = enginekey;
#endif
				}
				else if (sa == J1939_SA_POWERTRAIN_CTRL)
				{
					uint8_t horn = (uint8_t)g_trafficProfile->codec[SIG_COMMAND_HORN].decode(p, dlc);
#ifdef RAMN_SHOWCASE_MODE
					RAMN_DBC_Handle.command_horn = horn;
#else
					if (horn != 3) RAMN_DBC_Handle.command_horn = horn;
#endif
				}
				return;
			}
			if (key == J1939_PGN_PROPB_65282)
			{
				RAMN_DBC_Handle.joystick = RAMN_Decode_JoystickButtons_J1939(p, dlc);
#ifdef ENABLE_JOYSTICK_CONTROLS
				RAMN_Joystick_Update(RAMN_DBC_Handle.joystick);
#endif
				return;
			}
			if (sig == SIG_COMMAND_LIGHTS)
			{
				uint16_t lights = g_trafficProfile->codec[SIG_COMMAND_LIGHTS].decode(p, dlc);
				// Only update if not the J1939 'Not Available' state.
				if ((p[0] & 0x03) != 3) RAMN_DBC_Handle.command_lights = lights;
				return;
			}
		}
		else
		{
			// Irregular default case: CONTROL_SHIFT also carries the joystick position.
			if (sig == SIG_CONTROL_SHIFT)
			{
				RAMN_DBC_Handle.control_shift = (uint8_t)g_trafficProfile->codec[SIG_CONTROL_SHIFT].decode(p, dlc);
				if (dlc >= 2U)
				{
					RAMN_DBC_Handle.joystick = RAMN_Decode_Joystick_Default(p, dlc);
#ifdef ENABLE_JOYSTICK_CONTROLS
					RAMN_Joystick_Update(RAMN_DBC_Handle.joystick);
#endif
				}
				return;
			}
		}
		// Regular case: decode via the active codec table and store.
		RAMN_DBC_StoreSignal(sig, g_trafficProfile->codec[sig].decode(p, dlc));
		return;
	}
}

void RAMN_DBC_Send(uint32_t tick)
{
	if (RAMN_DBC_RequestSilence == False)
	{
		// A profile with fewer periodic messages than txRuntime[] leaves the trailing slots unsent
		// (catalog sizes are statically asserted to fit txRuntime[] in ramn_traffic_profiles.c).
		for(uint16_t i = 0; i < g_trafficProfile->txCount ; i++)
		{
			if((tick - txRuntime[i].lastSent) >= txRuntime[i].periodms)
			{
				RAMN_DBC_FormatDefaultPeriodicMessage(&txRuntime[i]);
				RAMN_FDCAN_SendMessage(&(txRuntime[i].header),(uint8_t*)(txRuntime[i].data));
				txRuntime[i].counter++;
				txRuntime[i].lastSent = tick;
			}
		}
	}
}

#if defined(ENABLE_USB)
void RAMN_DBC_ProcessUSBBuffer(const uint8_t* buf)
{
#if defined(TARGET_ECUA)
	// Payload offset is 0 in both traffic modes. Encode through the live profile's codec (not the
	// boot-mode public wrappers) so USB-injected commands follow a runtime traffic-mode switch.
	g_trafficProfile->codec[SIG_COMMAND_BRAKE].encode(ASCIItoUint12(&buf[1]), &txRuntime[TXIDX_COMMAND_BRAKE].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_ACCEL].encode(ASCIItoUint12(&buf[4]), &txRuntime[TXIDX_COMMAND_ACCEL].data->rawData[0]);
	g_trafficProfile->codec[SIG_STATUS_RPM].encode(ASCIItoUint12(&buf[7]), &txRuntime[TXIDX_STATUS_RPM].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_STEERING].encode(ASCIItoUint12(&buf[10]), &txRuntime[TXIDX_COMMAND_STEERING].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_SHIFT].encode((uint16_t)ASCIItoUint8(&buf[13]), &txRuntime[TXIDX_COMMAND_SHIFT].data->rawData[0]);
	g_trafficProfile->codec[SIG_CONTROL_HORN].encode((uint16_t)ASCIItoUint8(&buf[15]), &txRuntime[TXIDX_CONTROL_HORN].data->rawData[0]);
	g_trafficProfile->codec[SIG_COMMAND_SIDEBRAKE].encode((uint16_t)ASCIItoUint8(&buf[17]), &txRuntime[TXIDX_COMMAND_PARKINGBRAKE].data->rawData[0]);
#endif
}
#endif
