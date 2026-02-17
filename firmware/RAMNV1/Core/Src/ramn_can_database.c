#include "ramn_can_database.h"
#include "ramn_signal_defs.h"
#include <string.h>

#ifdef CPYTHON_TESTING
#ifndef RAMN_memcpy
#define RAMN_memcpy memcpy
#endif
#endif

// -- Command Brake --
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

// -- Control Brake --
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

// -- Command Accel --
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

// -- Control Accel --
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

// -- Status RPM --
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

// -- Command Steering --
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

// -- Control Steering --
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

// -- Command Shift --
void RAMN_Encode_Command_Shift(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, COMMAND_SHIFT_MASK, COMMAND_SHIFT_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Command_Shift(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, COMMAND_SHIFT_MASK, COMMAND_SHIFT_OFFSET);
}

// -- Control Shift & Joystick (Combined) --
void RAMN_Encode_Control_Shift_Joystick(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload) {
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

// -- Command Horn --
void RAMN_Encode_Command_Horn(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, COMMAND_HORN_MASK, COMMAND_HORN_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Command_Horn(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, COMMAND_HORN_MASK, COMMAND_HORN_OFFSET);
}

// -- Control Horn --
void RAMN_Encode_Control_Horn(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_HORN_MASK, CONTROL_HORN_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_Horn(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_HORN_MASK, CONTROL_HORN_OFFSET);
}

// -- Command Turn Indicator --
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

// -- Command Sidebrake --
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

// -- Control Sidebrake --
void RAMN_Encode_Control_Sidebrake(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_SIDEBRAKE_MASK, CONTROL_SIDEBRAKE_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_Sidebrake(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_SIDEBRAKE_MASK, CONTROL_SIDEBRAKE_OFFSET);
}

// -- Control Engine Key --
void RAMN_Encode_Control_EngineKey(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_ENGINEKEY_MASK, CONTROL_ENGINEKEY_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_EngineKey(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_ENGINEKEY_MASK, CONTROL_ENGINEKEY_OFFSET);
}

// -- Command Lights --
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

// -- Control Lights --
void RAMN_Encode_Control_Lights(uint8_t value, uint8_t* payload) {
    uint8_t packed = (uint8_t)PACK_SIGNAL(value, CONTROL_LIGHTS_MASK, CONTROL_LIGHTS_OFFSET);
    payload[0] = packed;
}

uint8_t RAMN_Decode_Control_Lights(const uint8_t* payload, uint32_t dlc) {
    uint8_t packed = payload[0];
    return (uint8_t)UNPACK_SIGNAL(packed, CONTROL_LIGHTS_MASK, CONTROL_LIGHTS_OFFSET);
}
