/*
 * ramn_traffic.h
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
 * Data-driven traffic model. A "traffic profile" bundles everything that differs between the default (RAMN-native) and J1939 CAN traffic modes: the signal codec, the periodic TX catalog, 
 * and the RX map. The active profile is selected by g_trafficProfile and can be repointed at runtime (see RAMN_DBC_SetProfile).
 */

#ifndef INC_RAMN_TRAFFIC_H_
#define INC_RAMN_TRAFFIC_H_

#include <stdint.h>

// One entry per logical signal. Indexes the codec tables and is used as the DBC field selector
typedef enum
{
	SIG_COMMAND_BRAKE = 0,
	SIG_CONTROL_BRAKE,
	SIG_COMMAND_ACCEL,
	SIG_CONTROL_ACCEL,
	SIG_STATUS_RPM,
	SIG_COMMAND_STEERING,
	SIG_CONTROL_STEERING,
	SIG_COMMAND_SHIFT,
	SIG_CONTROL_SHIFT,
	SIG_COMMAND_HORN,
	SIG_CONTROL_HORN,
	SIG_COMMAND_TURNINDICATOR,
	SIG_COMMAND_SIDEBRAKE,
	SIG_CONTROL_SIDEBRAKE,
	SIG_CONTROL_ENGINEKEY,
	SIG_COMMAND_LIGHTS,
	SIG_CONTROL_LIGHTS,
	NUM_SIGNALS
} RAMN_SignalId_t;

// Uniform codec signatures. uint8_t-valued signals are widened to uint16_t so one shape fits all
typedef void     (*RAMN_SignalEncode_t)(uint16_t value, uint8_t* payload);
typedef uint16_t (*RAMN_SignalDecode_t)(const uint8_t* payload, uint32_t dlc);

typedef struct
{
	RAMN_SignalEncode_t encode;
	RAMN_SignalDecode_t decode;
} RAMN_SignalCodec_t;

// "Special" codec signatures: signals that don't fit the uniform shape (two inputs, or a u8 result kept separate from the table).
// Made part of the profile so a live mode switch picks the right body. 
typedef void    (*RAMN_EncodeShiftJoystick_t)(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload);
typedef void    (*RAMN_EncodeU8_t)(uint8_t value, uint8_t* payload);
typedef uint8_t (*RAMN_DecodeU8_t)(const uint8_t* payload, uint32_t dlc);

// One entry per periodically-sent message.
typedef struct
{
	uint32_t canId;
	uint32_t idType;        /* FDCAN_STANDARD_ID / FDCAN_EXTENDED_ID */
	uint32_t fdFormat;
	uint32_t brs;
	uint32_t dlc;
	uint32_t periodMs;
	int16_t  payloadOffset; /* bits; -1 == none */
	int16_t  counterOffset; /* bits; -1 == none */
	int16_t  crcOffset;     /* bits; -1 == none */
	uint8_t  signalId;      /* RAMN_SignalId_t: index into codec table + DBC field */
} RAMN_MsgDescriptor_t;

// One entry per received CAN ID/PGN we decode
typedef struct
{
	uint32_t rxKey;         /* CAN ID (default) or PGN (J1939) */
	uint8_t  signalId;      /* RAMN_SignalId_t */
} RAMN_RxMapEntry_t;

// Complete traffic profile = everything that differs between modes.
typedef struct
{
	const RAMN_SignalCodec_t*   codec;          /* [NUM_SIGNALS] */
	const RAMN_MsgDescriptor_t* txCatalog;      /* periodic TX set for this ECU */
	uint16_t                    txCount;
	const RAMN_RxMapEntry_t*    rxMap;
	uint16_t                    rxCount;
	uint8_t                     usesExtendedId; /* drives CAN filter setup + RX key extraction */
	RAMN_EncodeShiftJoystick_t  encodeShiftJoystick;   /* combined Control_Shift + Joystick TX */
	RAMN_DecodeU8_t             decodeJoystick;        /* joystick position from the shift message */
	RAMN_EncodeU8_t             encodeJoystickButtons; /* J1939 joystick-buttons PGN TX */
	RAMN_DecodeU8_t             decodeJoystickButtons; /* joystick buttons RX */
} RAMN_TrafficProfile_t;

// signalId sentinel for periodic messages whose payload is produced by a special encode hook (not a plain codec-table entry), e.g. the J1939 joystick-buttons message. */
#define SIG_SPECIAL_ENCODE 0xFFU

// Codec tables (defined in ramn_can_database.c)
extern const RAMN_SignalCodec_t codec_default[NUM_SIGNALS];
extern const RAMN_SignalCodec_t codec_j1939[NUM_SIGNALS];

// Traffic profiles for this ECU (defined in ramn_traffic_profiles.c). Both are always compiled in; g_trafficProfile selects the active one
extern const RAMN_TrafficProfile_t profile_default;
extern const RAMN_TrafficProfile_t profile_j1939;

// The live, runtime-repointable active profile. Points at profile_default or profile_j1939.
extern const RAMN_TrafficProfile_t* g_trafficProfile;

// Switch the active traffic profile at runtime (defined in ramn_dbc.c)
void RAMN_DBC_SetProfile(const RAMN_TrafficProfile_t* p);

#endif /* INC_RAMN_TRAFFIC_H_ */
