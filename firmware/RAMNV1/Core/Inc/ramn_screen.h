/*
 * ramn_screen.h
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

#ifndef INC_RAMN_SCREEN_H_
#define INC_RAMN_SCREEN_H_

#include "main.h"
#include "ramn_spi.h"
#include "ramn_dbc.h"
#include "ramn_chip8.h"
#include "ramn_usb.h"

#define CONTOUR_WIDTH 2
#define CONTROL_WINDOW_Y LCD_HEIGHT-34

typedef struct COLOR_THEME_STRUCT {
    uint16_t BACKGROUND;
    uint16_t DARK;
    uint16_t MEDIUM;
    uint16_t LIGHT;
    uint16_t WHITE;
} ColorTheme_t ;

//Inits the Screen
void RAMN_SCREEN_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask);

#ifdef ENABLE_UDS
//Request to draw a picture on screen (Used by UDS services)
void RAMN_SCREEN_RequestDrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* image);

//Request to start a game on screen
void RAMN_SCREEN_RequestGame(const uint8_t* game_to_load, uint16_t game_size);

//Request to stop any ongoing game
void RAMN_SCREEN_RequestGameStop();

//Load a game stored in ECU memory
void RAMN_SCREEN_StartGameFromIndex(uint8_t index);

//Returns 1U if a previous UDS draw command is not completed
uint8_t RAMN_SCREEN_IsUDSScreenUpdatePending();
#endif

//Updates the Screen. Must be called periodically
void RAMN_SCREEN_Update(uint32_t tick);

//Update the color theme
void RAMN_SCREEN_UpdateTheme(uint8_t new_theme);

#endif /* INC_RAMN_SCREEN_H_ */
