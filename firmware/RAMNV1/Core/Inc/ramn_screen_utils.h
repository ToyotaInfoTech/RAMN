/*
 * ramn_screen_utils.h
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

// This module provides functions to help manage screens.

#ifndef INC_RAMN_SCREEN_UTILS_H_
#define INC_RAMN_SCREEN_UTILS_H_

#include "main.h"

#ifdef ENABLE_SCREEN

#include "ramn_spi.h"
#include "ramn_dbc.h"
#include "ramn_joystick.h"

// Update configureColorTheme (and potentially RAMN_SCREENUTILS_Init) to add/remove themes.
#define NUMBER_OF_THEMES 7

// Width of the lines on screen
#define CONTOUR_WIDTH 2
// Y coordinate of where the subconsole starts
#define CONTROL_WINDOW_Y (LCD_HEIGHT-34)

// Color theme used by default for all screens
typedef struct COLOR_THEME_STRUCT {
	uint16_t BACKGROUND;
	uint16_t DARK;
	uint16_t MEDIUM;
	uint16_t LIGHT;
	uint16_t WHITE;
} ColorTheme_t ;

// Common loop counter for screen loop execution, that can be used by screen modules
extern uint32_t RAMN_SCREENUTILS_LoopCounter;

// Set to True to force a redraw of the screen
extern volatile RAMN_Bool_t RAMN_SCREENUTILS_RequestRedraw;

// Current color theme
extern volatile ColorTheme_t RAMN_SCREENUTILS_COLORTHEME;

// Initializes the module.
void RAMN_SCREENUTILS_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask);

// Updates the color theme.
void RAMN_SCREENUTILS_UpdateTheme(uint8_t new_theme);

// Selects the next theme in the theme list.
void RAMN_SCREENUTILS_NextTheme();

// Prepares a screen that can be scrolled.
void RAMN_SCREENUTILS_PrepareScrollScreen();

// Draws the base layout for a screen (plain background, contours, etc.).
void RAMN_SCREENUTILS_DrawBase();

// Draws the static part of the sub console (contour and static letters).
void RAMN_SCREENUTILS_DrawSubconsoleStatic();

// Draws the dynamic part of the sub console (sensor values).
void RAMN_SCREENUTILS_DrawSubconsoleUpdate();

#endif
#endif
