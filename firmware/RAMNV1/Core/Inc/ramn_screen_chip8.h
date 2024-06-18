/*
 * ramn_screen_chip8.h
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

// This module implements a screen to display a chip8 game

#ifndef INC_RAMN_SCREEN_CHIP8_H_
#define INC_RAMN_SCREEN_CHIP8_H_

#include "main.h"
#include "ramn_screen_utils.h"
#include "ramn_chip8.h"

extern RAMNScreen ScreenChip8;

void RAMN_ScreenChip8_RequestGame(const uint8_t* game_to_load, uint16_t game_size);
void RAMN_ScreenChip8_StartGameFromIndex(uint8_t index);

#endif
