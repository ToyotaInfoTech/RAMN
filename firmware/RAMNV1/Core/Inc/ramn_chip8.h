/*
 * ramn_chip8.h
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

// This module implements a Chip-8 interpreter to play games on ECU A.
// This interpreter follows the same rules as Octo ( https://github.com/JohnEarnest/Octo )
// Games can be written and debugged using the online Octo tool ( https://johnearnest.github.io/Octo/ )
// Interpreters have various "quirks" and differ from each other - not all games are playable on the same interpreter.
// Refer to the "Compatibility" Options of Octo to see what quirks may exist.
// Various Creative Commons Zero "No Rights Reserved" games can be found here: https://johnearnest.github.io/chip8Archive/?sort=platform

#ifndef INC_RAMN_CHIP8_H_
#define INC_RAMN_CHIP8_H_

#include "main.h"
#include "ramn_trng.h"
#include "ramn_spi.h"

#define CHIP8_MEMORY_SIZE 0x1000
#define DEFAULT_PIXEL_SIZE 3U
#define FONT_ADDRESS 0U
#define MAX_SCREEN_WIDTH 128U
#define MAX_SCREEN_HEIGHT 64U

// This is used to define the screen buffer width
// Chip8 screen is larger than Super Chip because pixel size is larger (3), therefore we use Chip8 size
#define SPRITE_BUFFER_HEIGHT 64U
#define SPRITE_BUFFER_WIDTH 32U

// Games stored in ECU memory
extern const uint8_t CAVE_EXPLORER[];
extern const uint16_t CAVE_EXPLORER_SIZE;

extern const uint8_t DANMAKU[];
extern const uint16_t DANMAKU_SIZE;

extern const uint8_t OCTOPEG[];
extern const uint16_t OCTOPEG_SIZE;

// Sets the foreground and background colors used for display.
void RAMN_CHIP8_SetColor(uint16_t fg, uint16_t bg);

// Initializes the Chip8 Interpreter.
void RAMN_CHIP8_Init(const uint8_t* game_to_load, uint16_t game_size);

// Returns whether a game is currently being played or not.
RAMN_Bool_t RAMN_CHIP8_IsGameActive();

// Starts the game currently loaded in memory.
void RAMN_CHIP8_StartGame(uint32_t xLastWakeTime);

// Asks interpreter to stop current game.
void RAMN_CHIP8_StopGame();

// Update the interpreter (main loop to update periodically).
void RAMN_CHIP8_Update(uint32_t xLastWakeTime);

// Redraws the screen.
void RAMN_CHIP8_RedrawScreen();

#endif /* INC_RAMN_CHIP8_H_ */
