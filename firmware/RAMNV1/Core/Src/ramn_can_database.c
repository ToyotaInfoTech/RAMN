#include "ramn_can_database.h"
#include "ramn_signal_defs.h"
#include "ramn_sensors.h"
#include "ramn_utils.h"
#include "ramn_traffic.h"

#ifdef CPYTHON_TESTING
#ifndef RAMN_memcpy
#define RAMN_memcpy memcpy
#ifndef RAMN_memset
#define RAMN_memset memset
#endif
#endif
#endif

/*
 * Signal codec. Each logical signal has a "_Default" (RAMN-native bit-packed) and a "_J1939"
 * (SAE J1939 SPN/PGN) body; both are always compiled so a single image can switch modes at
 * runtime via the codec tables at the bottom of this file. uint8_t-valued signals are widened to
 * uint16_t here so one RAMN_SignalCodec_t shape fits all; the public wrapper functions (which keep
 * the historical signatures for existing callers and the host tests) cast at the boundary.
 */

/* ==========================================================================================
 * Default (RAMN-native) codec bodies
 * ========================================================================================== */

void RAMN_Encode_Command_Brake_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_BRAKE_MASK, COMMAND_BRAKE_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Command_Brake_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_BRAKE_MASK, COMMAND_BRAKE_OFFSET);
}

void RAMN_Encode_Control_Brake_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, CONTROL_BRAKE_MASK, CONTROL_BRAKE_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Control_Brake_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, CONTROL_BRAKE_MASK, CONTROL_BRAKE_OFFSET);
}

void RAMN_Encode_Command_Accel_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_ACCEL_MASK, COMMAND_ACCEL_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Command_Accel_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_ACCEL_MASK, COMMAND_ACCEL_OFFSET);
}

void RAMN_Encode_Control_Accel_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, CONTROL_ACCEL_MASK, CONTROL_ACCEL_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Control_Accel_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, CONTROL_ACCEL_MASK, CONTROL_ACCEL_OFFSET);
}

void RAMN_Encode_Status_RPM_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, STATUS_RPM_MASK, STATUS_RPM_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Status_RPM_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, STATUS_RPM_MASK, STATUS_RPM_OFFSET);
}

void RAMN_Encode_Command_Steering_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_STEERING_MASK, COMMAND_STEERING_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Command_Steering_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_STEERING_MASK, COMMAND_STEERING_OFFSET);
}

void RAMN_Encode_Control_Steering_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, CONTROL_STEERING_MASK, CONTROL_STEERING_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Control_Steering_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, CONTROL_STEERING_MASK, CONTROL_STEERING_OFFSET);
}

void RAMN_Encode_Command_Shift_Default(uint16_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, COMMAND_SHIFT_MASK, COMMAND_SHIFT_OFFSET);
    payload[0] = packed;
}
uint16_t RAMN_Decode_Command_Shift_Default(const uint8_t* payload, uint32_t dlc) {
    (void)dlc;
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, COMMAND_SHIFT_MASK, COMMAND_SHIFT_OFFSET);
}

/* Control Shift & Joystick (combined) -- special: two inputs into one message. */
void RAMN_Encode_Control_Shift_Joystick_Default(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload) {
    RAMN_memset(payload, 0xFF, 8);
    uint16_t packed = 0xFFFF;
    packed &= ~((uint16_t)(CONTROL_SHIFT_MASK << CONTROL_SHIFT_OFFSET));
    packed |= (uint16_t)PACK_SIGNAL(shift_value, CONTROL_SHIFT_MASK, CONTROL_SHIFT_OFFSET);
    packed &= ~((uint16_t)(JOYSTICK_MASK << JOYSTICK_OFFSET));
    packed |= (uint16_t)PACK_SIGNAL(joystick_value, JOYSTICK_MASK, JOYSTICK_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Control_Shift_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    if (dlc >= 2U) {
        RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
        return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_SHIFT_MASK, CONTROL_SHIFT_OFFSET);
    }
    return 0;
}
uint8_t RAMN_Decode_Joystick_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    if (dlc >= 2U) {
        RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
        return (uint8_t)UNPACK_SIGNAL(packed, JOYSTICK_MASK, JOYSTICK_OFFSET);
    }
    return 0;
}
void RAMN_Encode_JoystickButtons_Default(uint8_t joystick_state, uint8_t* payload) {
    RAMN_memset(payload, 0xFF, 8);
    payload[0] = joystick_state;
}
uint8_t RAMN_Decode_JoystickButtons_Default(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    return payload[0];
}

void RAMN_Encode_Command_Horn_Default(uint16_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, COMMAND_HORN_MASK, COMMAND_HORN_OFFSET);
    payload[0] = packed;
}
uint16_t RAMN_Decode_Command_Horn_Default(const uint8_t* payload, uint32_t dlc) {
    (void)dlc;
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, COMMAND_HORN_MASK, COMMAND_HORN_OFFSET);
}

void RAMN_Encode_Control_Horn_Default(uint16_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_HORN_MASK, CONTROL_HORN_OFFSET);
    payload[0] = packed;
}
uint16_t RAMN_Decode_Control_Horn_Default(const uint8_t* payload, uint32_t dlc) {
    (void)dlc;
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_HORN_MASK, CONTROL_HORN_OFFSET);
}

void RAMN_Encode_Command_TurnIndicator_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_TURNINDICATOR_MASK, COMMAND_TURNINDICATOR_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Command_TurnIndicator_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_TURNINDICATOR_MASK, COMMAND_TURNINDICATOR_OFFSET);
}

void RAMN_Encode_Command_Sidebrake_Default(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_SIDEBRAKE_MASK, COMMAND_SIDEBRAKE_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}
uint16_t RAMN_Decode_Command_Sidebrake_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_SIDEBRAKE_MASK, COMMAND_SIDEBRAKE_OFFSET);
}

void RAMN_Encode_Control_Sidebrake_Default(uint16_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_SIDEBRAKE_MASK, CONTROL_SIDEBRAKE_OFFSET);
    payload[0] = packed;
}
uint16_t RAMN_Decode_Control_Sidebrake_Default(const uint8_t* payload, uint32_t dlc) {
    (void)dlc;
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_SIDEBRAKE_MASK, CONTROL_SIDEBRAKE_OFFSET);
}

void RAMN_Encode_Control_EngineKey_Default(uint16_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_ENGINEKEY_MASK, CONTROL_ENGINEKEY_OFFSET);
    payload[0] = packed;
}
uint16_t RAMN_Decode_Control_EngineKey_Default(const uint8_t* payload, uint32_t dlc) {
    (void)dlc;
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_ENGINEKEY_MASK, CONTROL_ENGINEKEY_OFFSET);
}

void RAMN_Encode_Command_Lights_Default(uint16_t value, uint8_t* payload) {
    payload[0] = (uint8_t)(value & 0xFF);
    payload[1] = (uint8_t)((value >> 8) & 0xFF);
}
uint16_t RAMN_Decode_Command_Lights_Default(const uint8_t* payload, uint32_t dlc) {
    uint16_t result = (uint16_t)payload[0];
    if (dlc >= 2U) result |= ((uint16_t)payload[1] << 8);
    return result;
}

void RAMN_Encode_Control_Lights_Default(uint16_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_LIGHTS_MASK, CONTROL_LIGHTS_OFFSET);
    payload[0] = packed;
}
uint16_t RAMN_Decode_Control_Lights_Default(const uint8_t* payload, uint32_t dlc) {
    (void)dlc;
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_LIGHTS_MASK, CONTROL_LIGHTS_OFFSET);
}

/* ==========================================================================================
 * J1939 (SAE) codec bodies
 * ========================================================================================== */

void RAMN_Encode_Command_Brake_J1939(uint16_t value, uint8_t* payload) {
    /* Initialize payload to J1939 "Not Available" */
    RAMN_memset(payload, 0xFF, 8);
    /* SPN 2920 (XBR External Deceleration Demand). Bytes 1-2. PGN 1024 (XBR). */
    uint16_t j1939_val = 64254;
    if (value * 2 <= 64254) {
        j1939_val = 64254 - (value * 2);
    } else {
        j1939_val = 0; /* Max deceleration */
    }
    payload[0] = (uint8_t)(j1939_val & 0xFF);
    payload[1] = (uint8_t)((j1939_val >> 8) & 0xFF);
}
uint16_t RAMN_Decode_Command_Brake_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 2920 (XBR External Deceleration Demand). Bytes 1-2. PGN 1024 (XBR). */
    uint16_t j1939_val = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
    if (j1939_val > 64254) return 0;
    return (uint16_t)((64254U - j1939_val) / 2U);
}

void RAMN_Encode_Control_Brake_J1939(uint16_t value, uint8_t* payload) {
    RAMN_memset(payload, 0xFF, 8);
    /* SPN 521 (Brake Pedal Position). Byte 2. PGN 61441 (EBC1). */
    uint8_t j1939_val = (uint8_t)((value * 250U) / 4095U);
    payload[1] = j1939_val;
}
uint16_t RAMN_Decode_Control_Brake_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 521 (Brake Pedal Position). Byte 2. PGN 61441 (EBC1). */
    uint8_t j1939_val = payload[1];
    if (j1939_val > 250) return (j1939_val == 0xFF) ? 0 : 4095;
    return (uint16_t)((j1939_val * 4095U) / 250U);
}

void RAMN_Encode_Command_Accel_J1939(uint16_t value, uint8_t* payload) {
    RAMN_memset(payload, 0xFF, 8);
    /* SPN 898 (Engine Requested Speed/Speed Limit). Bytes 2-3. PGN 0 (TSC1). */
    uint16_t j1939_val = value;
    payload[1] = (uint8_t)(j1939_val & 0xFF);
    payload[2] = (uint8_t)((j1939_val >> 8) & 0xFF);
}
uint16_t RAMN_Decode_Command_Accel_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 3U) return 0;
    /* SPN 898 (Engine Requested Speed/Speed Limit). Bytes 2-3. PGN 0 (TSC1). */
    uint16_t val = (uint16_t)payload[1] | ((uint16_t)payload[2] << 8);
    return (val == 0xFFFF) ? 0 : val;
}

void RAMN_Encode_Control_Accel_J1939(uint16_t value, uint8_t* payload) {
    RAMN_memset(payload, 0xFF, 8);
    /* SPN 91 (Accelerator Pedal Position 1). Byte 2. PGN 61443 (EEC2). */
    uint8_t j1939_val = (uint8_t)((value * 250U) / 4095U);
    payload[1] = j1939_val;
}
uint16_t RAMN_Decode_Control_Accel_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 91 (Accelerator Pedal Position 1). Byte 2. PGN 61443 (EEC2). */
    uint8_t j1939_val = payload[1];
    if (j1939_val > 250) return (j1939_val == 0xFF) ? 0 : 4095;
    return (uint16_t)((j1939_val * 4095U) / 250U);
}

void RAMN_Encode_Status_RPM_J1939(uint16_t value, uint8_t* payload) {
    RAMN_memset(payload, 0xFF, 8);
    /* SPN 190 (Engine Speed). Bytes 4-5. PGN 61444 (EEC1). Scale: 0.125 rpm/bit. */
    uint16_t j1939_val = value;
    payload[3] = (uint8_t)(j1939_val & 0xFF);
    payload[4] = (uint8_t)((j1939_val >> 8) & 0xFF);
}
uint16_t RAMN_Decode_Status_RPM_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 5U) return 0;
    /* SPN 190 (Engine Speed). Bytes 4-5. PGN 61444 (EEC1). */
    uint16_t val = (uint16_t)payload[3] | ((uint16_t)payload[4] << 8);
    return (val == 0xFFFF) ? 0 : val;
}

void RAMN_Encode_Command_Steering_J1939(uint16_t value, uint8_t* payload) {
    /* PGN 61184 (Proprietary A). Bytes 1-2. */
    RAMN_memset(payload, 0xFF, 8);
    payload[0] = (uint8_t)(value & 0xFF);
    payload[1] = (uint8_t)((value >> 8) & 0xFF);
}
uint16_t RAMN_Decode_Command_Steering_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    uint16_t val = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
    return (val == 0xFFFF) ? 2048 : val;
}

void RAMN_Encode_Control_Steering_J1939(uint16_t value, uint8_t* payload) {
    /* SPN 2928 (Steering Wheel Angle). Bytes 1-2. PGN 61449 (VDC2). */
    RAMN_memset(payload, 0xFF, 8);
    uint16_t j1939_val = (value > 35456) ? 65535 : (uint16_t)(value + 30079); /* 2048 -> 32127 (0 rad) */
    payload[0] = (uint8_t)(j1939_val & 0xFF);
    payload[1] = (uint8_t)((j1939_val >> 8) & 0xFF);
}
uint16_t RAMN_Decode_Control_Steering_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 2048;
    /* SPN 2928 (Steering Wheel Angle). Bytes 1-2. PGN 61449 (VDC2). */
    uint16_t j1939_val = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
    if (j1939_val == 0xFFFF) return 2048;
    if (j1939_val < 30079) return 0;
    return (uint16_t)(j1939_val - 30079);
}

void RAMN_Encode_Command_Shift_J1939(uint16_t value, uint8_t* payload) {
    /* SPN 525 (Transmission Requested Gear). Byte 3. PGN 256 (TC1). */
    RAMN_memset(payload, 0xFF, 8);
    payload[2] = (uint8_t)(value + 125);
}
uint16_t RAMN_Decode_Command_Shift_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 3U) return 0;
    /* SPN 525 (Transmission Requested Gear). Byte 3. PGN 256 (TC1). */
    uint8_t raw = payload[2];
    if (raw < 125 || raw == 0xFF) return 0;
    return (uint8_t)(raw - 125);
}

void RAMN_Encode_Control_Shift_Joystick_J1939(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload) {
    /* SPN 523 (Current Gear). Byte 4. PGN 61445 (ETC2). Joystick sent separately in PGN 65282. */
    (void)joystick_value;
    RAMN_memset(payload, 0xFF, 8);
    payload[3] = (uint8_t)(shift_value + 125);
}
uint16_t RAMN_Decode_Control_Shift_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 4U) return 0;
    /* SPN 523 (Current Gear). Byte 4. PGN 61445 (ETC2). */
    uint8_t raw = payload[3];
    if (raw < 125 || raw == 0xFF) return 0;
    return (uint8_t)(raw - 125);
}
uint8_t RAMN_Decode_Joystick_J1939(const uint8_t* payload, uint32_t dlc) {
    (void)payload;
    if (dlc < 5U) return 0;
    return 0; /* Legacy, joystick no longer in this message */
}
void RAMN_Encode_JoystickButtons_J1939(uint8_t joystick_state, uint8_t* payload) {
    /* PGN 65282 (Proprietary B), SA 5. byte 1/2. Manufacturer Defined Usage. two bits per button. */
    RAMN_memset(payload, 0xFF, 8);
    uint8_t b1 = 0x00; // UP, DOWN, LEFT, RIGHT
    uint8_t b2 = 0xFC; // PUSH, rest unused

    if (joystick_state == RAMN_SHIFT_UP)         b1 |= 0x01;
    else if (joystick_state == RAMN_SHIFT_DOWN)  b1 |= 0x04;
    else if (joystick_state == RAMN_SHIFT_LEFT)  b1 |= 0x10;
    else if (joystick_state == RAMN_SHIFT_RIGHT) b1 |= 0x40;
    else if (joystick_state == RAMN_SHIFT_PUSH)  b2 |= 0x01;

    // RAMN_SHIFT_RELEASED or UNKNOWN will leave it as all 00s (off)

    payload[0] = b1;
    payload[1] = b2;
}
uint8_t RAMN_Decode_JoystickButtons_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    uint8_t b1 = payload[0];
    uint8_t b2 = payload[1];

    if ((b1 & 0x03) == 0x01) return RAMN_SHIFT_UP;
    if ((b1 & 0x0C) == 0x04) return RAMN_SHIFT_DOWN;
    if ((b1 & 0x30) == 0x10) return RAMN_SHIFT_LEFT;
    if ((b1 & 0xC0) == 0x40) return RAMN_SHIFT_RIGHT;
    if ((b2 & 0x03) == 0x01) return RAMN_SHIFT_PUSH;

    return RAMN_SHIFT_RELEASED;
}

void RAMN_Encode_Command_Horn_J1939(uint16_t value, uint8_t* payload) {
    /* SPN 2641 (Horn Switch). Byte 4 (bits 3-4). PGN 64980 (CM3). */
    RAMN_memset(payload, 0xFF, 8);
    payload[3] &= ~(0x03 << 2);
    payload[3] |= (value & 0x03) << 2;
}
uint16_t RAMN_Decode_Command_Horn_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 4U) return 0;
    /* SPN 2641 (Horn Switch). Byte 4 (bits 3-4). PGN 64980 (CM3). */
    return (payload[3] >> 2) & 0x03;
}

void RAMN_Encode_Control_Horn_J1939(uint16_t value, uint8_t* payload) {
    /* Proprietary A (PGN 61184). Byte 1. */
    RAMN_memset(payload, 0xFF, 8);
    payload[0] = (uint8_t)(value & 0xFF);
}
uint16_t RAMN_Decode_Control_Horn_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    /* Proprietary A (PGN 61184). Byte 1. */
    return payload[0];
}

void RAMN_Encode_Command_TurnIndicator_J1939(uint16_t value, uint8_t* payload) {
    /* SPN 2876 (Turn Signal Indicator). Byte 2 (bits 1-4). PGN 64972 (OEL). */
    uint8_t turn_val = 0;
    if ((value & 0xFF00) && (value & 0x00FF)) turn_val = 3; // Hazard
    else if (value & 0xFF00) turn_val = 1; // Left
    else if (value & 0x00FF) turn_val = 2; // Right

    RAMN_memset(payload, 0xFF, 8);
    payload[1] = (payload[1] & 0xF0) | (turn_val);
}
uint16_t RAMN_Decode_Command_TurnIndicator_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 2876 (Turn Signal Indicator). Byte 2 (bits 1-4). PGN 64972 (OEL). */
    uint8_t turn_val = payload[1] & 0x0F;
    uint16_t value = 0;
    if (turn_val == 1) value = 0x0100;
    else if (turn_val == 2) value = 0x0001;
    else if (turn_val == 3) value = 0x0101;
    return value;
}

void RAMN_Encode_Command_Sidebrake_J1939(uint16_t value, uint8_t* payload) {
    /* SPN 70 (Parking Brake Switch). Byte 1 (bits 3-4). PGN 65265 (CCVS1). */
    RAMN_memset(payload, 0xFF, 8);
    payload[0] = (payload[0] & 0xF3) | (uint8_t)((value & 0x03) << 2);
}
uint16_t RAMN_Decode_Command_Sidebrake_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    /* SPN 70 (Parking Brake Switch). Byte 1 (bits 3-4). PGN 65265 (CCVS1). */
    return (uint16_t)((payload[0] >> 2) & 0x03);
}

void RAMN_Encode_Control_Sidebrake_J1939(uint16_t value, uint8_t* payload) {
    /* SPN 619 (Parking Brake Actuator). Byte 4 (bits 1-2). PGN 65274. */
    RAMN_memset(payload, 0xFF, 8);
    payload[3] &= ~(0x03 << 0);
    payload[3] |= (value & 0x03) << 0;
}
uint16_t RAMN_Decode_Control_Sidebrake_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 4U) return 0;
    /* SPN 619 (Parking Brake Actuator). Byte 4 (bits 1-2). PGN 65274. */
    return (payload[3] >> 0) & 0x03;
}

void RAMN_Encode_Control_EngineKey_J1939(uint16_t value, uint8_t* payload) {
    /* CM3 (PGN 64980). Byte 3. SPN 3996 (bits 3-4), SPN 10145 (bits 5-6). */
    RAMN_memset(payload, 0xFF, 8);
    uint8_t acc_val = 3; // Not available
    uint8_t ign_val = 3; // Not available
    if (value == RAMN_ENGINEKEY_LEFT) { acc_val = 0; ign_val = 0; } // OFF
    else if (value == RAMN_ENGINEKEY_MIDDLE) { acc_val = 1; ign_val = 0; } // ACC
    else if (value == RAMN_ENGINEKEY_RIGHT) { acc_val = 1; ign_val = 1; } // IGN

    payload[2] = 0xFF;
    payload[2] &= ~(0x0F << 2);
    payload[2] |= (acc_val & 0x03) << 2;
    payload[2] |= (ign_val & 0x03) << 4;
}
uint16_t RAMN_Decode_Control_EngineKey_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 3U) return 0;
    /* CM3 (PGN 64980). Byte 3. SPN 3996 (bits 3-4), SPN 10145 (bits 5-6). */
    uint8_t acc_val = (payload[2] >> 2) & 0x03;
    uint8_t ign_val = (payload[2] >> 4) & 0x03;
    if (acc_val == 1 && ign_val == 1) return RAMN_ENGINEKEY_RIGHT;
    if (acc_val == 1 && ign_val == 0) return RAMN_ENGINEKEY_MIDDLE;
    return RAMN_ENGINEKEY_LEFT; // Default or Error
}

void RAMN_Encode_Command_Lights_J1939(uint16_t value, uint8_t* payload) {
    /* PGN 65089 (Lighting Command). Byte 1. SPN 2403/2351/2349/2347. */
    RAMN_memset(payload, 0xFF, 8); // Bytes 2-8 remain "Not Available" (0xFF)

    uint8_t byte1 = 0x00; // Start with all de-activated

    uint8_t pos = (uint8_t)(value & 0x00FF);
    if (pos == RAMN_LIGHTSWITCH_POS2) {       // Park
        byte1 = 0x01; // Running = 1
    } else if (pos == RAMN_LIGHTSWITCH_POS3) { // Lowbeam
        byte1 = 0x11; // Running = 1, Low = 1
    } else if (pos == RAMN_LIGHTSWITCH_POS4) { // Highbeam
        byte1 = 0x51; // Running = 1, Low = 1, High = 1
    }
    // else POS1 (off) or unknown: byte1 = 0x00

    // Encode engine warning flag from the high byte using Alt bits (bits 3-2 = 01)
    if (value & 0xFF00) byte1 |= 0x04;

    payload[0] = byte1;
}
uint16_t RAMN_Decode_Command_Lights_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    /* PGN 65089 (Lighting Command). Byte 1. */
    uint8_t byte1   = payload[0];
    uint8_t running = byte1 & 0x03;
    uint8_t alt     = (byte1 >> 2) & 0x03; // Engine warning flag
    uint8_t low     = (byte1 >> 4) & 0x03;
    uint8_t high    = (byte1 >> 6) & 0x03;

    uint16_t result;
    if (high == 1)    result = RAMN_LIGHTSWITCH_POS4;
    else if (low == 1) result = RAMN_LIGHTSWITCH_POS3;
    else if (running == 1) result = RAMN_LIGHTSWITCH_POS2;
    else              result = RAMN_LIGHTSWITCH_POS1;

    if (alt == 1) result |= 0xFF00;

    return result;
}

void RAMN_Encode_Control_Lights_J1939(uint16_t value, uint8_t* payload) {
    /* PGN 65280 (Proprietary B), SA 33. Manufacturer Defined Usage (LED state). one byte per LED. */
    RAMN_memset(payload, 0xFF, 8);
    payload[0] = (value & 0x08) ? 0xFA : 0x00;
    payload[1] = (value & 0x10) ? 0xFA : 0x00;
    payload[2] = (value & 0x20) ? 0xFA : 0x00;
    payload[3] = (value & 0x40) ? 0xFA : 0x00;
    payload[4] = (value & 0x80) ? 0xFA : 0x00;
    payload[5] = (value & 0x01) ? 0xFA : 0x00;
    payload[6] = (value & 0x02) ? 0xFA : 0x00;
    payload[7] = (value & 0x04) ? 0xFA : 0x00;
}
uint16_t RAMN_Decode_Control_Lights_J1939(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 8U) return 0;
    uint8_t lights = 0;

    if (payload[0] == 0xFA) lights |= 0x08;
    if (payload[1] == 0xFA) lights |= 0x10;
    if (payload[2] == 0xFA) lights |= 0x20;
    if (payload[3] == 0xFA) lights |= 0x40;
    if (payload[4] == 0xFA) lights |= 0x80;
    if (payload[5] == 0xFA) lights |= 0x01;
    if (payload[6] == 0xFA) lights |= 0x02;
    if (payload[7] == 0xFA) lights |= 0x04;

    return lights;
}

/* J1939-only diagnostic/status encoders (no default counterpart). */
void RAMN_Encode_DM1(uint8_t value, uint8_t* payload) {
    /* PGN 65226 (DM1). Byte 1: MIL (bits 7-8), Red Stop (bits 5-6), AWL (bits 3-4), Protect (bits 1-2). */
    RAMN_memset(payload, 0xFF, 8);
    uint8_t mil = (value & 0x02) ? 1 : 0; // LED_CHECKENGINE
    payload[0] = (mil << 6) | 0x3F;
    payload[1] = 0xFF; // Flash states Not Available
    payload[2] = 0x00;
    payload[3] = 0x00;
    payload[4] = 0x00;
    payload[5] = 0x00;
}

void RAMN_Encode_CCVS1(uint8_t value, uint8_t* payload) {
    /* PGN 65265 (CCVS1). Byte 4 (bits 3-4): SPN 70 (Parking Brake Switch). */
    RAMN_memset(payload, 0xFF, 8);
    uint8_t pb_state = (value & 0x04) ? 1 : 0; // LED_SIDEBRAKE
    payload[3] = (payload[3] & 0xF3) | (uint8_t)(pb_state << 2);
}

void RAMN_Encode_EngineRun(uint8_t value, uint8_t* payload) {
    /* PGN 64960. Byte 3 (bits 3-4): SPN 3046 (Run Switch Status). */
    RAMN_memset(payload, 0xFF, 8);
    uint8_t spn_val = (value & 0x01) ? 1 : 0; // LED_BATTERY
    payload[2] = (payload[2] & 0xF3) | (uint8_t)(spn_val << 2);
}

/* ==========================================================================================
 * Codec tables -- one row per logical signal, selected at runtime by the active traffic profile.
 * Signals handled by special hooks (combined Control_Shift + Joystick encode) leave encode NULL.
 * ========================================================================================== */

const RAMN_SignalCodec_t codec_default[NUM_SIGNALS] = {
    [SIG_COMMAND_BRAKE]         = { RAMN_Encode_Command_Brake_Default,         RAMN_Decode_Command_Brake_Default },
    [SIG_CONTROL_BRAKE]         = { RAMN_Encode_Control_Brake_Default,         RAMN_Decode_Control_Brake_Default },
    [SIG_COMMAND_ACCEL]         = { RAMN_Encode_Command_Accel_Default,         RAMN_Decode_Command_Accel_Default },
    [SIG_CONTROL_ACCEL]         = { RAMN_Encode_Control_Accel_Default,         RAMN_Decode_Control_Accel_Default },
    [SIG_STATUS_RPM]            = { RAMN_Encode_Status_RPM_Default,            RAMN_Decode_Status_RPM_Default },
    [SIG_COMMAND_STEERING]      = { RAMN_Encode_Command_Steering_Default,      RAMN_Decode_Command_Steering_Default },
    [SIG_CONTROL_STEERING]      = { RAMN_Encode_Control_Steering_Default,      RAMN_Decode_Control_Steering_Default },
    [SIG_COMMAND_SHIFT]         = { RAMN_Encode_Command_Shift_Default,         RAMN_Decode_Command_Shift_Default },
    [SIG_CONTROL_SHIFT]         = { NULL,                                      RAMN_Decode_Control_Shift_Default },
    [SIG_COMMAND_HORN]          = { RAMN_Encode_Command_Horn_Default,          RAMN_Decode_Command_Horn_Default },
    [SIG_CONTROL_HORN]          = { RAMN_Encode_Control_Horn_Default,          RAMN_Decode_Control_Horn_Default },
    [SIG_COMMAND_TURNINDICATOR] = { RAMN_Encode_Command_TurnIndicator_Default, RAMN_Decode_Command_TurnIndicator_Default },
    [SIG_COMMAND_SIDEBRAKE]     = { RAMN_Encode_Command_Sidebrake_Default,     RAMN_Decode_Command_Sidebrake_Default },
    [SIG_CONTROL_SIDEBRAKE]     = { RAMN_Encode_Control_Sidebrake_Default,     RAMN_Decode_Control_Sidebrake_Default },
    [SIG_CONTROL_ENGINEKEY]     = { RAMN_Encode_Control_EngineKey_Default,     RAMN_Decode_Control_EngineKey_Default },
    [SIG_COMMAND_LIGHTS]        = { RAMN_Encode_Command_Lights_Default,        RAMN_Decode_Command_Lights_Default },
    [SIG_CONTROL_LIGHTS]        = { RAMN_Encode_Control_Lights_Default,        RAMN_Decode_Control_Lights_Default },
};

const RAMN_SignalCodec_t codec_j1939[NUM_SIGNALS] = {
    [SIG_COMMAND_BRAKE]         = { RAMN_Encode_Command_Brake_J1939,         RAMN_Decode_Command_Brake_J1939 },
    [SIG_CONTROL_BRAKE]         = { RAMN_Encode_Control_Brake_J1939,         RAMN_Decode_Control_Brake_J1939 },
    [SIG_COMMAND_ACCEL]         = { RAMN_Encode_Command_Accel_J1939,         RAMN_Decode_Command_Accel_J1939 },
    [SIG_CONTROL_ACCEL]         = { RAMN_Encode_Control_Accel_J1939,         RAMN_Decode_Control_Accel_J1939 },
    [SIG_STATUS_RPM]            = { RAMN_Encode_Status_RPM_J1939,            RAMN_Decode_Status_RPM_J1939 },
    [SIG_COMMAND_STEERING]      = { RAMN_Encode_Command_Steering_J1939,      RAMN_Decode_Command_Steering_J1939 },
    [SIG_CONTROL_STEERING]      = { RAMN_Encode_Control_Steering_J1939,      RAMN_Decode_Control_Steering_J1939 },
    [SIG_COMMAND_SHIFT]         = { RAMN_Encode_Command_Shift_J1939,         RAMN_Decode_Command_Shift_J1939 },
    [SIG_CONTROL_SHIFT]         = { NULL,                                    RAMN_Decode_Control_Shift_J1939 },
    [SIG_COMMAND_HORN]          = { RAMN_Encode_Command_Horn_J1939,          RAMN_Decode_Command_Horn_J1939 },
    [SIG_CONTROL_HORN]          = { RAMN_Encode_Control_Horn_J1939,          RAMN_Decode_Control_Horn_J1939 },
    [SIG_COMMAND_TURNINDICATOR] = { RAMN_Encode_Command_TurnIndicator_J1939, RAMN_Decode_Command_TurnIndicator_J1939 },
    [SIG_COMMAND_SIDEBRAKE]     = { RAMN_Encode_Command_Sidebrake_J1939,     RAMN_Decode_Command_Sidebrake_J1939 },
    [SIG_CONTROL_SIDEBRAKE]     = { RAMN_Encode_Control_Sidebrake_J1939,     RAMN_Decode_Control_Sidebrake_J1939 },
    [SIG_CONTROL_ENGINEKEY]     = { RAMN_Encode_Control_EngineKey_J1939,     RAMN_Decode_Control_EngineKey_J1939 },
    [SIG_COMMAND_LIGHTS]        = { RAMN_Encode_Command_Lights_J1939,        RAMN_Decode_Command_Lights_J1939 },
    [SIG_CONTROL_LIGHTS]        = { RAMN_Encode_Control_Lights_J1939,        RAMN_Decode_Control_Lights_J1939 },
};
