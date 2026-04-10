/*
 * ramn_signal_defs.h
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

#ifndef INC_RAMN_SIGNAL_DEFS_H_
#define INC_RAMN_SIGNAL_DEFS_H_

#include "main.h"

// Macros for signal packing/unpacking
// Note: These macros assume the payload container is large enough.
// mask: The mask of the signal value (e.g. 0xFF for 8-bit signal)
// offset: The bit shift count to place/retrieve the signal
#define PACK_SIGNAL(val, mask, offset)       (((val) & (mask)) << (offset))
#define UNPACK_SIGNAL(payload, mask, offset) (((payload) >> (offset)) & (mask))

#ifdef USE_BIG_ENDIAN_CAN
#undef PACK_SIGNAL
#define PACK_SIGNAL(val, mask, offset) \
    ((mask) == 0xFFFF ? \
    ((((((val) & (mask)) << (offset)) & 0xFF) << 8) | (((((val) & (mask)) << (offset)) >> 8) & 0xFF)) : \
    (((val) & (mask)) << (offset)))

#undef UNPACK_SIGNAL
#define UNPACK_SIGNAL(payload, mask, offset) \
    ((mask) == 0xFFFF ? \
    ((((((payload) >> (offset)) & (mask)) & 0xFF) << 8) | (((((payload) >> (offset)) & (mask)) >> 8) & 0xFF)) : \
    (((payload) >> (offset)) & (mask)))
#endif

// Signal Definitions

// Control Brake
// ID: CAN_SIM_CONTROL_BRAKE_CANID (0x24)
#define CONTROL_BRAKE_MASK      0xFFFF
#define CONTROL_BRAKE_OFFSET    0
#define CONTROL_BRAKE_SCALE     1.0f

// Command Brake
// ID: CAN_SIM_COMMAND_BRAKE_CANID (0x1A)
#define COMMAND_BRAKE_MASK      0xFFFF
#define COMMAND_BRAKE_OFFSET    0
#define COMMAND_BRAKE_SCALE     1.0f

// Control Accel
// ID: CAN_SIM_CONTROL_ACCEL_CANID (0x39)
#define CONTROL_ACCEL_MASK      0xFFFF
#define CONTROL_ACCEL_OFFSET    0
#define CONTROL_ACCEL_SCALE     1.0f

// Command Accel
// ID: CAN_SIM_COMMAND_ACCEL_CANID (0x2F)
#define COMMAND_ACCEL_MASK      0xFFFF
#define COMMAND_ACCEL_OFFSET    0
#define COMMAND_ACCEL_SCALE     1.0f

// Status RPM
// ID: CAN_SIM_STATUS_RPM_CANID (0x43)
#define STATUS_RPM_MASK         0xFFFF
#define STATUS_RPM_OFFSET       0
#define STATUS_RPM_SCALE        1.0f

// Control Steering
// ID: CAN_SIM_CONTROL_STEERING_CANID (0x62)
#define CONTROL_STEERING_MASK   0xFFFF
#define CONTROL_STEERING_OFFSET 0
#define CONTROL_STEERING_SCALE  1.0f

// Command Steering
// ID: CAN_SIM_COMMAND_STEERING_CANID (0x58)
#define COMMAND_STEERING_MASK   0xFFFF
#define COMMAND_STEERING_OFFSET 0
#define COMMAND_STEERING_SCALE  1.0f

// Control Shift
// ID: CAN_SIM_CONTROL_SHIFT_CANID (0x77)
#define CONTROL_SHIFT_MASK      0xFF
#define CONTROL_SHIFT_OFFSET    0
#define CONTROL_SHIFT_SCALE     1.0f

// Joystick (Packed with Control Shift)
// ID: CAN_SIM_CONTROL_SHIFT_CANID (0x77)
#define JOYSTICK_MASK           0xFF
#define JOYSTICK_OFFSET         8
#define JOYSTICK_SCALE          1.0f

// Command Shift
// ID: CAN_SIM_COMMAND_SHIFT_CANID (0x6D)
#define COMMAND_SHIFT_MASK      0xFF
#define COMMAND_SHIFT_OFFSET    0
#define COMMAND_SHIFT_SCALE     1.0f

// Command Horn
// ID: CAN_SIM_COMMAND_HORN_CANID (0x98)
#define COMMAND_HORN_MASK       0xFF
#define COMMAND_HORN_OFFSET     0
#define COMMAND_HORN_SCALE      1.0f

// Control Horn
// ID: CAN_SIM_CONTROL_HORN_CANID (0xA2)
#define CONTROL_HORN_MASK       0xFF
#define CONTROL_HORN_OFFSET     0
#define CONTROL_HORN_SCALE      1.0f

// Control Sidebrake
// ID: CAN_SIM_CONTROL_SIDEBRAKE_CANID (0x1D3)
#define CONTROL_SIDEBRAKE_MASK   0xFF
#define CONTROL_SIDEBRAKE_OFFSET 0
#define CONTROL_SIDEBRAKE_SCALE  1.0f

// Command Sidebrake
// ID: CAN_SIM_COMMAND_SIDEBRAKE_CANID (0x1C9)
#define COMMAND_SIDEBRAKE_MASK   0xFFFF
#define COMMAND_SIDEBRAKE_OFFSET 0
#define COMMAND_SIDEBRAKE_SCALE  1.0f

// Command Turn Indicator
// ID: CAN_SIM_COMMAND_TURNINDICATOR_CANID (0x1A7)
#define COMMAND_TURNINDICATOR_MASK   0xFFFF
#define COMMAND_TURNINDICATOR_OFFSET 0
#define COMMAND_TURNINDICATOR_SCALE  1.0f

// Control Engine Key
// ID: CAN_SIM_CONTROL_ENGINEKEY_CANID (0x1B8)
#define CONTROL_ENGINEKEY_MASK   0xFF
#define CONTROL_ENGINEKEY_OFFSET 0
#define CONTROL_ENGINEKEY_SCALE  1.0f

// Command Lights
// ID: CAN_SIM_COMMAND_LIGHTS_CANID (0x150)
#define COMMAND_LIGHTS_MASK      0xFFFF
#define COMMAND_LIGHTS_OFFSET    0
#define COMMAND_LIGHTS_SCALE     1.0f

// Control Lights
// ID: CAN_SIM_CONTROL_LIGHTS_CANID (0x1BB)
#define CONTROL_LIGHTS_MASK      0xFF
#define CONTROL_LIGHTS_OFFSET    0
#define CONTROL_LIGHTS_SCALE     1.0f

#endif /* INC_RAMN_SIGNAL_DEFS_H_ */
