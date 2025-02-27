/*
 * ramn_screen_canmonitor.h
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

// This module implements a cansniffer-like module to display the most recent message of each ID

#ifndef INC_RAMN_SCREEN_CANMONITOR_H_
#define INC_RAMN_SCREEN_CANMONITOR_H_

#include "main.h"

#ifdef ENABLE_SCREEN

#include "ramn_screen_utils.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "semphr.h"

typedef struct {
    FDCAN_RxHeaderTypeDef header;
    uint8_t data[8];
} CANMessage;

typedef struct CANMessageNode {
    uint32_t identifier;
    CANMessage messages[2]; // two most recent messages
    uint32_t lastNibbleChange[16]; //last tick when a 4-bit nibble change was seen.
    struct CANMessageNode* next;
} CANMessageNode;

#define MAX_CANMONITOR_IDS 10 //Max number of IDs to display on screen

extern RAMNScreen_t ScreenCANMonitor;

void RAMN_ScreenCANMonitor_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);

#endif
#endif
