#ifndef RAMN_CAN_DATABASE_H
#define RAMN_CAN_DATABASE_H

#include <stdint.h>

void RAMN_Encode_Command_Brake(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Command_Brake(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_Brake(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Control_Brake(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Command_Accel(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Command_Accel(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_Accel(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Control_Accel(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Status_RPM(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Status_RPM(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Command_Steering(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Command_Steering(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_Steering(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Control_Steering(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Command_Shift(uint8_t value, uint8_t* payload);
uint8_t RAMN_Decode_Command_Shift(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_Shift_Joystick(uint8_t shift_value, uint8_t joystick_value, uint8_t* payload);
uint8_t RAMN_Decode_Control_Shift(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_JoystickButtons(uint8_t joystick_state, uint8_t* payload);
uint8_t RAMN_Decode_JoystickButtons(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Command_Horn(uint8_t value, uint8_t* payload);
uint8_t RAMN_Decode_Command_Horn(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_Horn(uint8_t value, uint8_t* payload);
uint8_t RAMN_Decode_Control_Horn(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Command_TurnIndicator(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Command_TurnIndicator(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Command_Sidebrake(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Command_Sidebrake(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_Sidebrake(uint8_t value, uint8_t* payload);
uint8_t RAMN_Decode_Control_Sidebrake(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_EngineKey(uint8_t value, uint8_t* payload);
uint8_t RAMN_Decode_Control_EngineKey(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Command_Lights(uint16_t value, uint8_t* payload);
uint16_t RAMN_Decode_Command_Lights(const uint8_t* payload, uint32_t dlc);

void RAMN_Encode_Control_Lights(uint8_t value, uint8_t* payload);
uint8_t RAMN_Decode_Control_Lights(const uint8_t* payload, uint32_t dlc);

#endif // RAMN_CAN_DATABASE_H
