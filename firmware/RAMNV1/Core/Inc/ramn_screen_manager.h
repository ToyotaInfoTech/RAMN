/*
 * ramn_screen_manager.h
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

// This module handles whatever is displayed on the screen

#ifndef INC_RAMN_SCREEN_MANAGER_H_
#define INC_RAMN_SCREEN_MANAGER_H_

#include "main.h"
#include "ramn_spi.h"
#include "ramn_chip8.h"
#include "ramn_usb.h"
#include "ramn_screen_utils.h"
#include "ramn_screen_saver.h"
#include "ramn_screen_chip8.h"
#include "ramn_screen_uds.h"
#include "ramn_screen_canmonitor.h"

//Inits the Screen
void RAMN_ScreenManager_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask);

#ifdef ENABLE_UDS
//Request to draw a picture on screen (Used by UDS services)
//void RAMN_SCREEN_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image);

//Request to start a game on screen
void RAMN_ScreenManager_RequestGame(const uint8_t* game_to_load, uint16_t game_size);

//Request to stop any ongoing game
void RAMN_SCREEN_RequestGameStop();

//Load a game stored in ECU memory
void RAMN_ScreenManager_StartGameFromIndex(uint8_t index);

//Returns 1U if a previous UDS draw command is not completed
uint8_t RAMN_ScreenManager_IsUDSScreenUpdatePending();
#endif

//Updates the Screen. Must be called periodically
void RAMN_SCREEN_Update(uint32_t tick);

//Function to process a CAN message to potentially display on screen
void RAMN_ScreenManager_ProcessRxCANMessage(const FDCAN_RxHeaderTypeDef* pHeader, const uint8_t* data, uint32_t tick);



#endif /* INC_RAMN_SCREEN_MANAGER_H_ */
