#ifndef RAMN_CAN_DATABASE_H
#define RAMN_CAN_DATABASE_H

#include <stdint.h>

/* Special (non-table) joystick codecs, referenced by the traffic profiles in
 * ramn_traffic_profiles.c. The regular per-signal codecs (RAMN_Encode_*_Default / _J1939) are only
 * reached through the codec tables (ramn_traffic.h) and need no declarations here; the host unit
 * tests resolve them by symbol name. */
void    RAMN_Encode_Control_Shift_Joystick_Default(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload);
void    RAMN_Encode_Control_Shift_Joystick_J1939(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload);
uint8_t RAMN_Decode_Joystick_Default(const uint8_t* payload, uint32_t dlc);
uint8_t RAMN_Decode_Joystick_J1939(const uint8_t* payload, uint32_t dlc);
void    RAMN_Encode_JoystickButtons_J1939(uint8_t joystick_state, uint8_t* payload);
uint8_t RAMN_Decode_JoystickButtons_J1939(const uint8_t* payload, uint32_t dlc);

#endif // RAMN_CAN_DATABASE_H
