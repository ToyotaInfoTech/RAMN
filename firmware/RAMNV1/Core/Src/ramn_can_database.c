#include "ramn_can_database.h"
#include "ramn_signal_defs.h"
#include "ramn_sensors.h"
#include <string.h>

#ifdef CPYTHON_TESTING
#ifndef RAMN_memcpy
#define RAMN_memcpy memcpy
#endif
#endif

// -- Command Brake --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_Brake(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_BRAKE_MASK, COMMAND_BRAKE_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Command_Brake(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_BRAKE_MASK, COMMAND_BRAKE_OFFSET);
}
#else
void RAMN_Encode_Command_Brake(uint16_t value, uint8_t* payload) {
    /* Initialize payload to J1939 "Not Available" */
    memset(payload, 0xFF, 8);
    /* SPN 2920 (XBR External Deceleration Demand). Bytes 1-2. PGN 1024 (XBR). */
    /* 0 m/s2 maps to raw 64254. We assume ramn_val is in 1/2048 m/s2 units starting from 0 m/s2, and represents deceleration. */
    /* Since 1 unit = 1/2048 m/s2 of deceleration = -2 * 1/4096 m/s2 */
    /* So j1939_val = 64254 - (value * 2) */
    uint16_t j1939_val = 64254;
    if (value * 2 <= 64254) {
        j1939_val = 64254 - (value * 2);
    } else {
        j1939_val = 0; /* Max deceleration */
    }
    payload[0] = (uint8_t)(j1939_val & 0xFF);
    payload[1] = (uint8_t)((j1939_val >> 8) & 0xFF);
}

uint16_t RAMN_Decode_Command_Brake(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 2920 (XBR External Deceleration Demand). Bytes 1-2. PGN 1024 (XBR). */
    uint16_t j1939_val = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
    if (j1939_val > 64254) return 0;
    return (uint16_t)((64254U - j1939_val) / 2U);
}
#endif

// -- Control Brake --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_Brake(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, CONTROL_BRAKE_MASK, CONTROL_BRAKE_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Control_Brake(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, CONTROL_BRAKE_MASK, CONTROL_BRAKE_OFFSET);
}
#else
void RAMN_Encode_Control_Brake(uint16_t value, uint8_t* payload) {
    memset(payload, 0xFF, 8);
    /* SPN 521 (Brake Pedal Position). Byte 2. PGN 61441 (EBC1). */
    /* ramn_val 4095 maps to 100% (raw 250). j1939_val = (ramn_val * 250) / 4095. */
    uint8_t j1939_val = (uint8_t)((value * 250U) / 4095U);
    payload[1] = j1939_val;
}

uint16_t RAMN_Decode_Control_Brake(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 521 (Brake Pedal Position). Byte 2. PGN 61441 (EBC1). */
    uint8_t j1939_val = payload[1];
    if (j1939_val > 250) return (j1939_val == 0xFF) ? 0 : 4095;
    return (uint16_t)((j1939_val * 4095U) / 250U);
}
#endif

// -- Command Accel --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_Accel(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_ACCEL_MASK, COMMAND_ACCEL_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Command_Accel(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_ACCEL_MASK, COMMAND_ACCEL_OFFSET);
}
#else
void RAMN_Encode_Command_Accel(uint16_t value, uint8_t* payload) {
    memset(payload, 0xFF, 8);
    /* SPN 898 (Engine Requested Speed/Speed Limit). Bytes 2-3. PGN 0 (TSC1). */
    /* Assume ramn_val is already in 0.125 rpm units for consistency with Status_RPM. */
    uint16_t j1939_val = value;
    payload[1] = (uint8_t)(j1939_val & 0xFF);
    payload[2] = (uint8_t)((j1939_val >> 8) & 0xFF);
}

uint16_t RAMN_Decode_Command_Accel(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 3U) return 0;
    /* SPN 898 (Engine Requested Speed/Speed Limit). Bytes 2-3. PGN 0 (TSC1). */
    uint16_t val = (uint16_t)payload[1] | ((uint16_t)payload[2] << 8);
    return (val == 0xFFFF) ? 0 : val;
}
#endif

// -- Control Accel --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_Accel(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, CONTROL_ACCEL_MASK, CONTROL_ACCEL_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Control_Accel(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, CONTROL_ACCEL_MASK, CONTROL_ACCEL_OFFSET);
}
#else
void RAMN_Encode_Control_Accel(uint16_t value, uint8_t* payload) {
    memset(payload, 0xFF, 8);
    /* SPN 91 (Accelerator Pedal Position 1). Byte 2. PGN 61443 (EEC2). */
    /* ramn_val 4095 maps to 100% (raw 250). j1939_val = (ramn_val * 250) / 4095. */
    uint8_t j1939_val = (uint8_t)((value * 250U) / 4095U);
    payload[1] = j1939_val;
}

uint16_t RAMN_Decode_Control_Accel(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 91 (Accelerator Pedal Position 1). Byte 2. PGN 61443 (EEC2). */
    uint8_t j1939_val = payload[1];
    if (j1939_val > 250) return (j1939_val == 0xFF) ? 0 : 4095;
    return (uint16_t)((j1939_val * 4095U) / 250U);
}
#endif

// -- Status RPM --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Status_RPM(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, STATUS_RPM_MASK, STATUS_RPM_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Status_RPM(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, STATUS_RPM_MASK, STATUS_RPM_OFFSET);
}
#else
void RAMN_Encode_Status_RPM(uint16_t value, uint8_t* payload) {
    memset(payload, 0xFF, 8);
    /* SPN 190 (Engine Speed). Bytes 4-5. PGN 61444 (EEC1). Scale: 0.125 rpm/bit. */
    uint16_t j1939_val = value;
    payload[3] = (uint8_t)(j1939_val & 0xFF);
    payload[4] = (uint8_t)((j1939_val >> 8) & 0xFF);
}

uint16_t RAMN_Decode_Status_RPM(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 5U) return 0;
    /* SPN 190 (Engine Speed). Bytes 4-5. PGN 61444 (EEC1). */
    uint16_t val = (uint16_t)payload[3] | ((uint16_t)payload[4] << 8);
    return (val == 0xFFFF) ? 0 : val;
}
#endif

// -- Command Steering --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_Steering(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_STEERING_MASK, COMMAND_STEERING_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Command_Steering(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_STEERING_MASK, COMMAND_STEERING_OFFSET);
}
#else
void RAMN_Encode_Command_Steering(uint16_t value, uint8_t* payload) {
    /* PGN 61184 (Proprietary A). Bytes 1-2. */
    memset(payload, 0xFF, 8);
    payload[0] = (uint8_t)(value & 0xFF);
    payload[1] = (uint8_t)((value >> 8) & 0xFF);
}

uint16_t RAMN_Decode_Command_Steering(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    uint16_t val = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
    return (val == 0xFFFF) ? 2048 : val;
}
#endif

// -- Control Steering --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_Steering(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, CONTROL_STEERING_MASK, CONTROL_STEERING_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Control_Steering(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, CONTROL_STEERING_MASK, CONTROL_STEERING_OFFSET);
}
#else
void RAMN_Encode_Control_Steering(uint16_t value, uint8_t* payload) {
    /* SPN 2928 (Steering Wheel Angle). Bytes 1-2. PGN 61449 (VDC2). */
    /* 0 rad maps to raw 32127. Ramn_val 0-4095 centers at 2048. */
    memset(payload, 0xFF, 8);
    uint16_t j1939_val = (value > 35456) ? 65535 : (uint16_t)(value + 30079); /* 2048 -> 32127 (0 rad) */
    payload[0] = (uint8_t)(j1939_val & 0xFF);
    payload[1] = (uint8_t)((j1939_val >> 8) & 0xFF);
}

uint16_t RAMN_Decode_Control_Steering(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 2048;
    /* SPN 2928 (Steering Wheel Angle). Bytes 1-2. PGN 61449 (VDC2). */
    uint16_t j1939_val = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
    if (j1939_val == 0xFFFF) return 2048;
    if (j1939_val < 30079) return 0;
    return (uint16_t)(j1939_val - 30079);
}
#endif

// -- Command Shift --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_Shift(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, COMMAND_SHIFT_MASK, COMMAND_SHIFT_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Command_Shift(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, COMMAND_SHIFT_MASK, COMMAND_SHIFT_OFFSET);
}
#else
void RAMN_Encode_Command_Shift(uint8_t value, uint8_t* payload) {
    /* SPN 525 (Transmission Requested Gear). Byte 3. PGN 256 (TC1). */
    memset(payload, 0xFF, 8);
    payload[2] = (uint8_t)(value + 125);
}

uint8_t RAMN_Decode_Command_Shift(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 3U) return 0;
    /* SPN 525 (Transmission Requested Gear). Byte 3. PGN 256 (TC1). */
    uint8_t raw = payload[2];
    if (raw < 125 || raw == 0xFF) return 0;
    return (uint8_t)(raw - 125);
}
#endif

// -- Control Shift & Joystick (Combined) --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_Shift_Joystick(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload) {
    memset(payload, 0xFF, 8);
    uint16_t packed = 0xFFFF;
    packed &= ~((uint16_t)(CONTROL_SHIFT_MASK << CONTROL_SHIFT_OFFSET));
    packed |= (uint16_t)PACK_SIGNAL(shift_value, CONTROL_SHIFT_MASK, CONTROL_SHIFT_OFFSET);
    packed &= ~((uint16_t)(JOYSTICK_MASK << JOYSTICK_OFFSET));
    packed |= (uint16_t)PACK_SIGNAL(joystick_value, JOYSTICK_MASK, JOYSTICK_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint8_t RAMN_Decode_Control_Shift(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    if (dlc >= 2U) {
        RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
        return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_SHIFT_MASK, CONTROL_SHIFT_OFFSET);
    }
    return 0;
}

uint8_t RAMN_Decode_Joystick(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    if (dlc >= 2U) {
        RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
        return (uint8_t)UNPACK_SIGNAL(packed, JOYSTICK_MASK, JOYSTICK_OFFSET);
    }
    return 0;
}

void RAMN_Encode_JoystickButtons(uint8_t joystick_state, uint8_t* payload) {
    memset(payload, 0xFF, 8);
    payload[0] = joystick_state;
}

uint8_t RAMN_Decode_JoystickButtons(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    return payload[0];
}
#else
void RAMN_Encode_Control_Shift_Joystick(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload) {
    /* SPN 523 (Current Gear). Byte 4. PGN 61445 (ETC2). */
    /* Joystick is sent separately in PGN 65282 (PropB) */
    memset(payload, 0xFF, 8);
    payload[3] = (uint8_t)(shift_value + 125);
}

uint8_t RAMN_Decode_Control_Shift(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 4U) return 0;
    /* SPN 523 (Current Gear). Byte 4. PGN 61445 (ETC2). */
    uint8_t raw = payload[3];
    if (raw < 125 || raw == 0xFF) return 0;
    return (uint8_t)(raw - 125);
}

uint8_t RAMN_Decode_Joystick(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 5U) return 0;
    return 0; /* Legacy, joystick no longer in this message */
}

void RAMN_Encode_JoystickButtons(uint8_t joystick_state, uint8_t* payload) {
    /* PGN 65282 (Proprietary B), SA 5. byte 1/2. Manufacturer Defined Usage. two bits per button. */
    memset(payload, 0xFF, 8);
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

uint8_t RAMN_Decode_JoystickButtons(const uint8_t* payload, uint32_t dlc) {
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
#endif

// -- Command Horn --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_Horn(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, COMMAND_HORN_MASK, COMMAND_HORN_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Command_Horn(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, COMMAND_HORN_MASK, COMMAND_HORN_OFFSET);
}
#else
void RAMN_Encode_Command_Horn(uint8_t value, uint8_t* payload) {
    /* SPN 2641 (Horn Switch). Byte 4 (bits 3-4). PGN 64980 (CM3). */
    memset(payload, 0xFF, 8);
    payload[3] &= ~(0x03 << 2);
    payload[3] |= (value & 0x03) << 2;
}

uint8_t RAMN_Decode_Command_Horn(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 4U) return 0;
    /* SPN 2641 (Horn Switch). Byte 4 (bits 3-4). PGN 64980 (CM3). */
    return (payload[3] >> 2) & 0x03;
}
#endif

// -- Control Horn --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_Horn(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_HORN_MASK, CONTROL_HORN_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_Horn(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_HORN_MASK, CONTROL_HORN_OFFSET);
}
#else
void RAMN_Encode_Control_Horn(uint8_t value, uint8_t* payload) {
    /* Proprietary A (PGN 61184). Byte 1. */
    memset(payload, 0xFF, 8);
    payload[0] = (uint8_t)(value & 0xFF);
}

uint8_t RAMN_Decode_Control_Horn(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    /* Proprietary A (PGN 61184). Byte 1. */
    return payload[0];
}
#endif

// -- Command Turn Indicator --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_TurnIndicator(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_TURNINDICATOR_MASK, COMMAND_TURNINDICATOR_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Command_TurnIndicator(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_TURNINDICATOR_MASK, COMMAND_TURNINDICATOR_OFFSET);
}
#else
void RAMN_Encode_Command_TurnIndicator(uint16_t value, uint8_t* payload) {
    /* SPN 2876 (Turn Signal Indicator). Byte 2 (bits 1-4). PGN 64972 (OEL). */
    uint8_t turn_val = 0;
    if ((value & 0xFF00) && (value & 0x00FF)) turn_val = 3; // Hazard
    else if (value & 0xFF00) turn_val = 1; // Left
    else if (value & 0x00FF) turn_val = 2; // Right

    memset(payload, 0xFF, 8);
    payload[1] = (payload[1] & 0xF0) | (turn_val);
}

uint16_t RAMN_Decode_Command_TurnIndicator(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 2U) return 0;
    /* SPN 2876 (Turn Signal Indicator). Byte 2 (bits 1-4). PGN 64972 (OEL). */
    uint8_t turn_val = payload[1] & 0x0F;
    uint16_t value = 0;
    if (turn_val == 1) value = 0x0100;
    else if (turn_val == 2) value = 0x0001;
    else if (turn_val == 3) value = 0x0101;
    return value;
}
#endif

// -- Command Sidebrake --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_Sidebrake(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_SIDEBRAKE_MASK, COMMAND_SIDEBRAKE_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Command_Sidebrake(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_SIDEBRAKE_MASK, COMMAND_SIDEBRAKE_OFFSET);
}
#else
void RAMN_Encode_Command_Sidebrake(uint16_t value, uint8_t* payload) {
    /* SPN 70 (Parking Brake Switch). Byte 1 (bits 3-4). PGN 65265 (CCVS1). */
    memset(payload, 0xFF, 8);
    payload[0] = (payload[0] & 0xF3) | (uint8_t)((value & 0x03) << 2);
}

uint16_t RAMN_Decode_Command_Sidebrake(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    /* SPN 70 (Parking Brake Switch). Byte 1 (bits 3-4). PGN 65265 (CCVS1). */
    return (uint16_t)((payload[0] >> 2) & 0x03);
}
#endif

// -- Control Sidebrake --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_Sidebrake(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_SIDEBRAKE_MASK, CONTROL_SIDEBRAKE_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_Sidebrake(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_SIDEBRAKE_MASK, CONTROL_SIDEBRAKE_OFFSET);
}
#else
void RAMN_Encode_Control_Sidebrake(uint8_t value, uint8_t* payload) {
    /* SPN 619 (Parking Brake Actuator). Byte 4 (bits 1-2). PGN 65274. */
    memset(payload, 0xFF, 8);
    payload[3] &= ~(0x03 << 0);
    payload[3] |= (value & 0x03) << 0;
}

uint8_t RAMN_Decode_Control_Sidebrake(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 4U) return 0;
    /* SPN 619 (Parking Brake Actuator). Byte 4 (bits 1-2). PGN 65274. */
    return (payload[3] >> 0) & 0x03;
}
#endif

// -- Control Engine Key --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_EngineKey(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_ENGINEKEY_MASK, CONTROL_ENGINEKEY_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_EngineKey(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_ENGINEKEY_MASK, CONTROL_ENGINEKEY_OFFSET);
}
#else
void RAMN_Encode_Control_EngineKey(uint8_t value, uint8_t* payload) {
    /* CM3 (PGN 64980). Byte 3. SPN 3996 (bits 3-4), SPN 10145 (bits 5-6). */
    memset(payload, 0xFF, 8);
    uint8_t acc_val = 3; // Not available
    uint8_t ign_val = 3; // Not available
    if (value == RAMN_ENGINEKEY_LEFT) { acc_val = 0; ign_val = 0; } // OFF
    else if (value == RAMN_ENGINEKEY_MIDDLE) { acc_val = 1; ign_val = 0; } // ACC
    else if (value == RAMN_ENGINEKEY_RIGHT) { acc_val = 1; ign_val = 1; } // IGN

    payload[2] &= ~(0x0F << 2);
    payload[2] |= (acc_val & 0x03) << 2;
    payload[2] |= (ign_val & 0x03) << 4;
}

uint8_t RAMN_Decode_Control_EngineKey(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 3U) return 0;
    /* CM3 (PGN 64980). Byte 3. SPN 3996 (bits 3-4), SPN 10145 (bits 5-6). */
    uint8_t acc_val = (payload[2] >> 2) & 0x03;
    uint8_t ign_val = (payload[2] >> 4) & 0x03;
    if (acc_val == 1 && ign_val == 1) return RAMN_ENGINEKEY_RIGHT;
    if (acc_val == 1 && ign_val == 0) return RAMN_ENGINEKEY_MIDDLE;
    return RAMN_ENGINEKEY_LEFT; // Default or Error
}
#endif

// -- Command Lights --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Command_Lights(uint16_t value, uint8_t* payload) {
    uint16_t packed = PACK_SIGNAL(value, COMMAND_LIGHTS_MASK, COMMAND_LIGHTS_OFFSET);
    RAMN_memcpy(payload, (uint8_t*)&packed, sizeof(packed));
}

uint16_t RAMN_Decode_Command_Lights(const uint8_t* payload, uint32_t dlc) {
    uint16_t packed = 0;
    RAMN_memcpy((uint8_t*)&packed, payload, sizeof(packed));
    if (dlc <= 1U) packed = packed & 0xFF;
    return UNPACK_SIGNAL(packed, COMMAND_LIGHTS_MASK, COMMAND_LIGHTS_OFFSET);
}
#else
void RAMN_Encode_Command_Lights(uint16_t value, uint8_t* payload) {
    /* PGN 65089 (Lighting Command). Byte 1. */
    /* J1939 SPN 2403 (Running), 2351 (Alt), 2349 (Low), 2347 (High) */
    /* Bits: 1-2 (Running), 3-4 (Alt), 5-6 (Low), 7-8 (High) */
    memset(payload, 0xFF, 8);
    uint8_t byte1 = 0xFF; // Start with "Don't Care" (11b) for all 2-bit fields

    if (value == RAMN_LIGHTSWITCH_POS1) { // Off
        byte1 = (byte1 & 0xFC) | 0x00; // Running = 0 (De-activate)
        byte1 = (byte1 & 0xF3) | 0x00; // Alt = 0
        byte1 = (byte1 & 0xCF) | 0x00; // Low = 0
        byte1 = (byte1 & 0x3F) | 0x00; // High = 0
    } else if (value == RAMN_LIGHTSWITCH_POS2) { // Park
        byte1 = (byte1 & 0xFC) | 0x01; // Running = 1 (Activate)
        byte1 = (byte1 & 0xF3) | 0x00; // Alt = 0
        byte1 = (byte1 & 0xCF) | 0x00; // Low = 0
        byte1 = (byte1 & 0x3F) | 0x00; // High = 0
    } else if (value == RAMN_LIGHTSWITCH_POS3) { // Lowbeam
        byte1 = (byte1 & 0xFC) | 0x01; // Running = 1
        byte1 = (byte1 & 0xF3) | 0x00; // Alt = 0
        byte1 = (byte1 & 0xCF) | 0x10; // Low = 1 (01b << 4)
        byte1 = (byte1 & 0x3F) | 0x00; // High = 0
    } else if (value == RAMN_LIGHTSWITCH_POS4) { // Highbeam
        byte1 = (byte1 & 0xFC) | 0x01; // Running = 1
        byte1 = (byte1 & 0xF3) | 0x00; // Alt = 0
        byte1 = (byte1 & 0xCF) | 0x10; // Low = 1
        byte1 = (byte1 & 0x3F) | 0x40; // High = 1 (01b << 6)
    }
    payload[0] = byte1;
}

uint16_t RAMN_Decode_Command_Lights(const uint8_t* payload, uint32_t dlc) {
    if (dlc < 1U) return 0;
    /* PGN 65089 (Lighting Command). Byte 1. */
    uint8_t byte1 = payload[0];
    uint8_t running = byte1 & 0x03;
    uint8_t low     = (byte1 >> 4) & 0x03;
    uint8_t high    = (byte1 >> 6) & 0x03;

    if (high == 1) return RAMN_LIGHTSWITCH_POS4;
    if (low == 1)  return RAMN_LIGHTSWITCH_POS3;
    if (running == 1) return RAMN_LIGHTSWITCH_POS2;
    return RAMN_LIGHTSWITCH_POS1;
}
#endif

// -- Control Lights --
#ifndef ENABLE_J1939_MODE
void RAMN_Encode_Control_Lights(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_LIGHTS_MASK, CONTROL_LIGHTS_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_Lights(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_LIGHTS_MASK, CONTROL_LIGHTS_OFFSET);
}
#else
void RAMN_Encode_Control_Lights(uint8_t value, uint8_t* payload) {
    /* PGN 65280 (Proprietary B), SA 33. Manufacturer Defined Usage (LED state). one byte per LED. */
    memset(payload, 0xFF, 8);
    payload[0] = (value & 0x08) ? 0xFA : 0x00;
    payload[1] = (value & 0x10) ? 0xFA : 0x00;
    payload[2] = (value & 0x20) ? 0xFA : 0x00;
    payload[3] = (value & 0x40) ? 0xFA : 0x00;
    payload[4] = (value & 0x80) ? 0xFA : 0x00;
    payload[5] = (value & 0x01) ? 0xFA : 0x00;
    payload[6] = (value & 0x02) ? 0xFA : 0x00;
    payload[7] = (value & 0x04) ? 0xFA : 0x00;
}

uint8_t RAMN_Decode_Control_Lights(const uint8_t* payload, uint32_t dlc) {
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
void RAMN_Encode_DM1(uint8_t value, uint8_t* payload) {
    /* PGN 65226 (DM1). Byte 1: MIL (bits 7-8), Red Stop (bits 5-6), AWL (bits 3-4), Protect (bits 1-2). */
    /* SPN 1213 (Malfunction Indicator Lamp Status). */
    memset(payload, 0xFF, 8);
    uint8_t mil = (value & 0x02) ? 1 : 0; // LED_CHECKENGINE
    // MIL (7-8), Red Stop (5-6)=3, AWL (3-4)=3, Protect (1-2)=3
    payload[0] = (mil << 6) | 0x3F;
    payload[1] = 0xFF; // Flash states Not Available
    // No Active Faults (J1939-73)
    payload[2] = 0x00;
    payload[3] = 0x00;
    payload[4] = 0x00;
    payload[5] = 0x00;
}

void RAMN_Encode_CCVS1(uint8_t value, uint8_t* payload) {
    /* PGN 65265 (CCVS1). Byte 4 (bits 3-4): SPN 70 (Parking Brake Switch). */
    memset(payload, 0xFF, 8);
    uint8_t pb_state = (value & 0x04) ? 1 : 0; // LED_SIDEBRAKE
    payload[3] = (payload[3] & 0xF3) | (uint8_t)(pb_state << 2);
}

void RAMN_Encode_EngineRun(uint8_t value, uint8_t* payload) {
    /* PGN 64960. Byte 3 (bits 3-4): SPN 3046 (Run Switch Status). */
    memset(payload, 0xFF, 8);
    uint8_t spn_val = (value & 0x01) ? 1 : 0; // LED_BATTERY
    payload[2] = (payload[2] & 0xF3) | (uint8_t)(spn_val << 2);
}
#endif
