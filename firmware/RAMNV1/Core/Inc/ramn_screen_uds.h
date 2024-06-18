/*
 * ramn_screen_uds.h
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2024 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

// This module implements UDS drawing on screen

#ifndef INC_RAMN_SCREEN_UDS_H_
#define INC_RAMN_SCREEN_UDS_H_

#include "main.h"
#include "ramn_screen_utils.h"

extern uint8_t uds_draw_need_refresh;

void RAMN_ScreenUDS_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image);


extern RAMNScreen ScreenUDS;

#endif
