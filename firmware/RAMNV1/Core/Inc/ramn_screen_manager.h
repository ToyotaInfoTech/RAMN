/*
 * ramn_screen_manager.h
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

// This module handles switching between screens.

#ifndef INC_RAMN_SCREEN_MANAGER_H_
#define INC_RAMN_SCREEN_MANAGER_H_

#include "main.h"

#ifdef ENABLE_SCREEN

#include "ramn_spi.h"
#include "ramn_screen_utils.h"
#include "ramn_screen_saver.h"
#ifdef ENABLE_CHIP8
#include "ramn_chip8.h"
#include "ramn_screen_chip8.h"
#endif
#ifdef ENABLE_UDS
#include "ramn_screen_uds.h"
#endif
#include "ramn_screen_canmonitor.h"
#include "ramn_screen_stats.h"
#include "ramn_screen_canlog.h"

// Number of screens
#define 	SCREEN_COUNT 	(sizeof(screens) / sizeof(screens[0]))

// Default screen to display at startup
#define 	DEFAULT_SCREEN 	&ScreenSaver

// Initializes the screen manager.
void 		RAMN_SCREENMANAGER_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask);

// Updates current screen. Must be called periodically.
void 		RAMN_SCREENMANAGER_Update(uint32_t tick);

// Processes a CAN message (e.g., to display it on screen).
void 		RAMN_SCREENMANAGER_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);

#ifdef ENABLE_CHIP8

// Requests to start a game on screen (using arbitrary memory area, e.g. from UDS).
void 		RAMN_SCREENMANAGER_RequestGame(const uint8_t* game_to_load, uint16_t game_size);

// Loads a game stored in ECU memory.
void 		RAMN_SCREENMANAGER_StartGameFromIndex(uint8_t index);

#endif

#ifdef ENABLE_UDS

// TODO: Refactor code to delete these functions.

// Returns True if a previous UDS draw command is not completed.
RAMN_Bool_t RAMN_SCREENMANAGER_IsUDSScreenUpdatePending();

// Requests to draw an image on the uds screen.
void RAMN_ScreenManager_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image);

#endif

#endif
#endif /* INC_RAMN_SCREEN_MANAGER_H_ */
