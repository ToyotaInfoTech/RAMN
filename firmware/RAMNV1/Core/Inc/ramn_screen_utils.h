/*
 * ramn_screen_utils.h
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

#ifndef INC_RAMN_SCREEN_UTILS_H_
#define INC_RAMN_SCREEN_UTILS_H_

#include "main.h"
#include "ramn_spi.h"
#include "ramn_dbc.h"
#include "ramn_joystick.h"


#define NUMBER_OF_THEMES 7

#define CONTOUR_WIDTH 2
#define CONTROL_WINDOW_Y LCD_HEIGHT-34

typedef struct COLOR_THEME_STRUCT {
	uint16_t BACKGROUND;
	uint16_t DARK;
	uint16_t MEDIUM;
	uint16_t LIGHT;
	uint16_t WHITE;
} ColorTheme_t ;

extern volatile uint8_t current_theme;
extern volatile uint8_t theme_change_requested;
extern volatile ColorTheme_t SPI_COLOR_THEME;

extern uint32_t spi_refresh_counter;

void RAMN_ScreenUtils_DrawSubconsoleStatic();

void RAMN_ScreenUtils_DrawSubconsoleUpdate();

void RAMN_ScreenUtils_DrawBase(uint8_t theme);

//Update the color theme
void RAMN_ScreenUtils_UpdateTheme(uint8_t new_theme);

void RAMN_ScreenUtils_Init(SPI_HandleTypeDef* handler, osThreadId_t* pTask);

//Prepare a screen that can be scrolled
void RAMN_ScreenUtils_PrepareScrollScreen();

#endif
